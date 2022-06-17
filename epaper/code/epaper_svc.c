#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "arch.h"   //for debug
#include "app.h"    //for ble basic func api
#include "co_list.h"    //for list api
#include "app_sec.h"    //for security api
#include "user_mem.h"   //for os mem api
#include "user_timer.h"   //for os mem api
#include "epaper_svc.h"

#include "user_profile.h"
#include "attm.h"       //for struct attm_desc
#include "gattc_task.h"     //for prf_task_func operation
#include "gapm_task.h"      //for marco GAPM_PROFILE_TASK_ADD
#include "att.h"        //for UUID marcos

#include "user_msg_q.h" //for os_msg_post
#include "prf_types.h"
#include "decoder_task.h"
#include "user_sys.h"
#include "audio.h"
#include "system.h"
#include "dec_sbc.h"
#include "watchdog.h"
#include "gap_def.h"

#include "request_handler.h"
#include "common.h"
#include "utils.h"
#include "ssd1680.h"
#include "shared_obj.h"
#include "shared_obj.h"

void profile_epaper(void);
void epaper_start_advertising(void);
static void slave_connected_callback(void *arg);

static uint16_t current_con_interval = 0;

void database_add_completed(void *arg) {
    epaper_start_advertising();
}

void device_cfg_completed(void *arg) {
    profile_epaper();
    appm_add_svc();
}

static os_timer_t update_param_timer;
static void param_timer_func(void *arg) {
    
    if(((app_env.con_param.con_latency == 0)
          || (app_env.con_param.con_interval * app_env.con_param.con_latency < 400))
        && (appm_get_connect_status((uint8_t)arg))) {
            
        if(!app_env.param_update_on_going) {
            struct gapc_conn_param conn_param;
            conn_param.intv_min = current_con_interval;
            conn_param.intv_max = current_con_interval;
            if(current_con_interval == 6)
                conn_param.latency  = 75;
            else
                conn_param.latency  = 50;
            conn_param.time_out = 400;
            appm_update_param((uint8_t)arg, &conn_param);
        }
        else
        {
            os_timer_disarm(&update_param_timer);
            os_timer_setfn( &update_param_timer, param_timer_func, arg);
            os_timer_arm(&update_param_timer, 4000, 0);
        }
    }
}
void slave_connected_callback(void *arg) {
    struct conn_peer_param *param = (struct conn_peer_param *)arg;
    appm_sleep_stop();
    set_runtime_bit(FLAG_PHONE_CONNECTED, BIT_SET);
    appm_exc_mtu_cmd(param->conidx);
    current_con_interval = 6;
    os_timer_disarm(&update_param_timer);
    os_timer_setfn(&update_param_timer, param_timer_func, (void *)(param->conidx));
    os_timer_arm(&update_param_timer, 1000, 0);

}

void slave_disconnected_callback(void *arg) {
    os_timer_disarm(&update_param_timer);
    set_runtime_bit(FLAG_PHONE_CONNECTED, BIT_CLEAR);
    epaper_start_advertising();
#if DEBUG_MODE
    appm_sleep_stop();
#else
    appm_sleep_start();
#endif
}

void epaper_cb_param_ind(void *arg) {
    struct gapc_param_updated_ind1 *ind = (struct gapc_param_updated_ind1 *)arg;
    if((ind->ind.con_latency != 0) && (ind->ind.con_interval * ind->ind.con_latency >= 400)) {
        appm_sleep_start();
    }else {
        appm_sleep_stop();
    }
}

void epaper_cb_param_updated(void *arg) {

}

void epaper_cb_param_rejected(void *arg) {
    struct conidx_status *cs = (struct conidx_status *)arg;
    if( cs->status != GAP_ERR_REJECTED && cs->status != LL_ERR_UNACCEPTABLE_CONN_INT
        && cs->status != GAP_ERR_INVALID_PARAM ) {
        os_timer_disarm(&update_param_timer);
        os_timer_setfn( &update_param_timer,param_timer_func, (void *)(cs->conidx));
        os_timer_arm(&update_param_timer,3000,0);
    } else {
        os_timer_disarm(&update_param_timer);
        struct gapc_conn_param conn_param;

        if( cs->status == LL_ERR_DIFF_TRANSACTION_COLLISION ) {
            // printf("r_u2:%d\r\n",current_con_interval);
            conn_param.intv_min = current_con_interval;
            conn_param.intv_max = current_con_interval;

                if(current_con_interval == 6)
                    conn_param.latency  = 75;
                else
                    conn_param.latency  = 50;

            conn_param.time_out = 400;
            appm_update_param(cs->conidx, &conn_param);
        } else {
            if(current_con_interval == 6) {
                current_con_interval = 9;
                conn_param.intv_min = current_con_interval;
                conn_param.intv_max = current_con_interval;

                if(current_con_interval == 6) {
                    conn_param.latency  = 75;
                }else {
                    conn_param.latency  = 50;
                }

                conn_param.time_out = 400;
                appm_update_param(cs->conidx, &conn_param);
            } else if(current_con_interval == 9) {
                   appm_sleep_start();
            }
        }
    }
}

void epaper_svc_init(void) {
    appm_set_cb_func(APP_EVT_ID_DB_ADDED, database_add_completed);
    appm_set_cb_func(APP_EVT_ID_DEV_CFG_COMPLETED, device_cfg_completed);
    appm_set_cb_func(APP_EVT_ID_SLAVER_CONNECTED, slave_connected_callback);
    appm_set_cb_func(APP_EVT_ID_DISCONNECTED, slave_disconnected_callback);
    appm_set_cb_func(APP_EVT_ID_PARAM_UPDATED, epaper_cb_param_updated);
    appm_set_cb_func(APP_EVT_ID_PARAM_UPDATE_IND, epaper_cb_param_ind);
    appm_set_cb_func(APP_EVT_ID_PARAM_UPDATE_REJECTED, epaper_cb_param_rejected);
    // use default configuration
    appm_set_dev_configuration(NULL);
}

void epaper_start_advertising(void) {
    struct gapm_start_advertise_cmd msg;
    uint8_t *pos ;
    uint16_t value;
    uint8_t *temp;
    uint32_t length;
    
    msg.op.code = GAPM_ADV_UNDIRECT;
    msg.op.addr_src = GAPM_STATIC_ADDR;
    msg.intv_min = GAP_ADV_INT_MIN;  
    value = get_adv_interval();
    msg.intv_max = value;   
    msg.channel_map = APP_ADV_CHMAP;

    msg.info.host.mode = GAP_GEN_DISCOVERABLE;
    msg.info.host.adv_filt_policy = ADV_ALLOW_SCAN_ANY_CON_ANY;
    
    // scan response maximum 31 bytes
    pos = msg.info.host.scan_rsp_data;
    
    temp = get_ble_name();
    if(temp == NULL) {
        temp = ble_get_addr();
        strcpy((char *)shared_tx_buffer, EPAPER_DEFAULT_PREFIX);
        mac_to_str((shared_tx_buffer + 6), temp, 2);
        shared_tx_buffer[12] = '\0';
        length = 13;
    }else {
        length = strlen((const char *)temp);
        memcpy(shared_tx_buffer, temp, length);
        shared_tx_buffer[length] = '\0';
        length++;
    }   
    // payload + type    
    *pos++ = (length + 1);
    *pos++  = GAP_ADVTYPE_LOCAL_NAME_COMPLETE; 
    memcpy(pos, shared_tx_buffer, length);
    pos += length;
    
    msg.info.host.scan_rsp_data_len = ((uint32_t)pos - (uint32_t)(msg.info.host.scan_rsp_data));
    
    // advertising data maximum 28 bytes
    pos = msg.info.host.adv_data;
    const uint8_t manufacturer_value[] = {0x00,0x00};
    *pos++ = sizeof(manufacturer_value) + 1;
    *pos++  = GAP_ADVTYPE_MANUFACTURER_SPECIFIC;
    memcpy(pos, manufacturer_value, sizeof(manufacturer_value));
    pos += sizeof(manufacturer_value);

    const uint8_t short_name_value[] = {0x45,0x50,0x44};
    *pos++ = sizeof(short_name_value) + 1;
    *pos++  = GAP_ADVTYPE_LOCAL_NAME_SHORT;
    memcpy(pos, short_name_value, sizeof(short_name_value));
    pos += sizeof(short_name_value);


    *pos++ = sizeof(uint16_t) + 1;
    *pos++  = GAP_ADVTYPE_16BIT_MORE;
    value = ATT_SVC_HID;
    memcpy(pos, &value, sizeof(uint16_t));
    pos += sizeof(uint16_t);

    uint8_t device_class[] = {0x04,0x05,0x00};
    *pos++ = sizeof(device_class) + 1;
    *pos++  = GAP_ADVTYPE_OOB_CLASS_OF_DEVICE;
    memcpy(pos, (uint8_t *)&device_class, sizeof(device_class));
    pos += sizeof(device_class);
    
    *pos++ = 2;
    *pos++  = GAP_ADVTYPE_POWER_LEVEL;
    *pos++  = BLE_TX_POWER_LEVEL;

    msg.info.host.adv_data_len = ((uint32_t)pos - (uint32_t)(msg.info.host.adv_data));

    appm_start_advertising(&msg);
}

uint16_t demo_profile_id[3];

static const struct attm_desc epaper_att_db[EPAPER_IDX_NB] = {
    [EPAPER_IDX_SVC] = {ATT_DECL_PRIMARY_SERVICE, PERM(RD, ENABLE), 0, 0},

    [EPAPER_IDX_RX_CHAR] = {ATT_DECL_CHARACTERISTIC, PERM(RD, ENABLE), 0, EPAPER_IDX_RX_VALUE},
    [EPAPER_IDX_RX_VALUE] = {ATT_UUID_16(0xD002), PERM(WRITE_REQ, ENABLE)|PERM(WRITE_COMMAND, ENABLE), PERM(RI, ENABLE), 256},
    
    [EPAPER_IDX_NTF_CHAR] = {ATT_DECL_CHARACTERISTIC, PERM(RD, ENABLE), 0, EPAPER_IDX_NTF_VALUE},
    [EPAPER_IDX_NTF_VALUE] = {ATT_UUID_16(0xD003), PERM(RD, ENABLE)|PERM(NTF, ENABLE), PERM(RI, ENABLE), 256},
    [EPAPER_IDX_NTF_CFG] = {ATT_DESC_CLIENT_CHAR_CFG, PERM(RD, ENABLE) | PERM(WRITE_REQ, ENABLE), 0, 0},
};

static int epaper_profile_task_func(os_event_t *msg) {
    switch(msg->event_id) {
        //before write, ask peer max receving length.
        case GATTC_ATT_INFO_REQ_IND: {
            //0xc17
            struct gattc_att_info_req_ind *param = (struct gattc_att_info_req_ind *)msg->param;
            struct gattc_att_info_cfm cfm;
            cfm.handle = param->handle;
            switch(param->handle - user_get_prf_start_hdl(demo_profile_id[2])) {
                case EPAPER_IDX_RX_VALUE:
                    cfm.length = 256;
                    cfm.status = ATT_ERR_NO_ERROR;
                    break;
                case EPAPER_IDX_NTF_VALUE:
                    cfm.length = 256;
                    cfm.status = ATT_ERR_NO_ERROR;
                    break;
                case EPAPER_IDX_NTF_CFG:
                    cfm.length = 2;
                    cfm.status = ATT_ERR_NO_ERROR;
                    break;
                default:
                    cfm.length = 0;
                    cfm.status = ATT_ERR_WRITE_NOT_PERMITTED;
                    break;
            }
            
            os_event_t evt;
            evt.event_id = GATTC_ATT_INFO_CFM;
            evt.src_task_id = user_get_prf_task_num(demo_profile_id[2]);
            evt.param = &cfm;
            evt.param_len = sizeof(struct gattc_att_info_cfm);
            os_msg_post(msg->src_task_id,&evt);
        }
        break;
        case GATTC_WRITE_REQ_IND: {
            //0xc15
            struct gattc_write_req_ind *param = (struct gattc_write_req_ind *)msg->param;
            struct gattc_write_cfm cfm;
            os_event_t evt;
            
            cfm.status = GAP_ERR_NO_ERROR;
            cfm.handle = param->handle;
            
            evt.event_id = GATTC_WRITE_CFM;
            evt.src_task_id = user_get_prf_task_num(demo_profile_id[2]);
            evt.param = &cfm;
            evt.param_len = sizeof(struct gattc_write_cfm);
            os_msg_post(msg->src_task_id, &evt);      
            
            dispatch_write_request(msg->src_task_id, demo_profile_id[2], param->value, param->length);
        }
        break;
        case GATTC_READ_REQ_IND: {
            //0xc13
            struct gattc_read_req_ind *param = (struct gattc_read_req_ind *)msg->param;
            struct gattc_read_cfm *cfm = (struct gattc_read_cfm *)shared_tx_buffer;
            os_event_t evt;
            
            cfm->handle = param->handle;
            cfm->status = GAP_ERR_NO_ERROR;
            cfm->length = 4;

            evt.event_id = GATTC_READ_CFM;
            evt.src_task_id = user_get_prf_task_num(demo_profile_id[2]);
            evt.param = shared_tx_buffer;
            evt.param_len = sizeof(struct gattc_read_cfm) + cfm->length;
            os_msg_post(msg->src_task_id, &evt);
        }
        break;
        default:
            break;

    }
    return (KE_MSG_CONSUMED);
}

void profile_epaper(void) {
    user_svc_req_t req;
    req.app_task = TASK_APP;
    req.prf_id = user_get_free_prf_id();
    req.operation = GAPM_PROFILE_TASK_ADD;
    req.sec_lvl = PERM(SVC_AUTH, ENABLE);
    req.start_hdl = 0;

    user_svc_cfg_t cfg;
    cfg.svc_uuid = ATT_UUID_16(EPD_SERVICE_UUID);
    cfg.svc_att_db = (struct attm_desc *)epaper_att_db;
    cfg.svc_att_nb = EPAPER_IDX_NB;
    cfg.svc_db_func = epaper_profile_task_func;
    cfg.db_ext_uuid128 = NULL;
    
    if(user_add_svc_to_list(&req, &cfg, NULL) == ERR_OK) {
        demo_profile_id[2] = req.prf_id;
    }
}



