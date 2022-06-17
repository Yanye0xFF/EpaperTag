/**
 * Copyright (c) 2019, Tsingtao Freqchip
 * 
 * All rights reserved.
 * 
 * 
 */
#ifndef GAP_API_H
#define GAP_API_H


/*
 * TYPEDEFS (类型定义)
 */

/*
 * CONSTANTS (常量定义)
 */
#define	GAP_ROLE_TYPE_PERIPHERAL	0x01
#define	GAP_ROLE_TYPE_CENTRAL		0x02
#define	GAP_ROLE_TYPE_BROADCASTER	0x04
#define	GAP_ROLE_TYPE_OBSERVER		0x08

#define GAP_ADDR_TYPE_PUBLIC						0x01
#define GAP_ADDR_TYPE_PRIVATE						0x02
#define GAP_ADDR_TYPE_RANDOM_RESOVABLE				0x03
#define GAP_ADDR_TYPE_RANDOM_NONE_RESOVABLE			0x04

/** @defgroup GAP_ADVCHAN_DEFINES GAP Advertisement Channel Map
 * @{
 */
#define GAP_ADV_CHAN_37  0x01  //!< Advertisement Channel 37
#define GAP_ADV_CHAN_38  0x02  //!< Advertisement Channel 38
#define GAP_ADV_CHAN_39  0x04  //!< Advertisement Channel 39
#define GAP_ADV_CHAN_ALL (GAP_ADV_CHAN_37 | GAP_ADV_CHAN_38 | GAP_ADV_CHAN_39) //!< All Advertisement Channels Enabled

#define GAP_ADV_TYPE_UNDIRECT	0x01
#define GAP_ADV_TYPE_DIRECT		0x02
#define GAP_ADV_TYPE_NON_CONN	0x03

/** @defgroup GAP_ADVTYPE_DEFINES GAP Advertisement Data Types
 * These are the data type identifiers for the data tokens in the advertisement data field.
 * @{
 */
#define GAP_ADTVYPE_FLAGS                       0x01 //!< Discovery Mode: @ref GAP_ADTYPE_FLAGS_MODES	
#define GAP_ADVTYPE_16BIT_MORE                  0x02 //!< Service: More 16-bit UUIDs available
#define GAP_ADVTYPE_16BIT_COMPLETE              0x03 //!< Service: Complete list of 16-bit UUIDs
#define GAP_ADVTYPE_32BIT_MORE                  0x04 //!< Service: More 32-bit UUIDs available
#define GAP_ADVTYPE_32BIT_COMPLETE              0x05 //!< Service: Complete list of 32-bit UUIDs
#define GAP_ADVTYPE_128BIT_MORE                 0x06 //!< Service: More 128-bit UUIDs available
#define GAP_ADVTYPE_128BIT_COMPLETE             0x07 //!< Service: Complete list of 128-bit UUIDs
#define GAP_ADVTYPE_LOCAL_NAME_SHORT            0x08 //!< Shortened local name
#define GAP_ADVTYPE_LOCAL_NAME_COMPLETE         0x09 //!< Complete local name
#define GAP_ADVTYPE_POWER_LEVEL                 0x0A //!< TX Power Level: 0xXX: -127 to +127 dBm
#define GAP_ADVTYPE_OOB_CLASS_OF_DEVICE         0x0D //!< Simple Pairing OOB Tag: Class of device (3 octets)
#define GAP_ADVTYPE_OOB_SIMPLE_PAIRING_HASHC    0x0E //!< Simple Pairing OOB Tag: Simple Pairing Hash C (16 octets)
#define GAP_ADVTYPE_OOB_SIMPLE_PAIRING_RANDR    0x0F //!< Simple Pairing OOB Tag: Simple Pairing Randomizer R (16 octets)
#define GAP_ADVTYPE_SM_TK                       0x10 //!< Security Manager TK Value
#define GAP_ADVTYPE_SM_OOB_FLAG                 0x11 //!< Secutiry Manager OOB Flags
#define GAP_ADVTYPE_SLAVE_CONN_INTERVAL_RANGE   0x12 //!< Min and Max values of the connection interval (2 octets Min, 2 octets Max) (0xFFFF indicates no conn interval min or max)
#define GAP_ADVTYPE_SIGNED_DATA                 0x13 //!< Signed Data field
#define GAP_ADVTYPE_SERVICES_LIST_16BIT         0x14 //!< Service Solicitation: list of 16-bit Service UUIDs
#define GAP_ADVTYPE_SERVICES_LIST_128BIT        0x15 //!< Service Solicitation: list of 128-bit Service UUIDs
#define GAP_ADVTYPE_SERVICE_DATA                0x16 //!< Service Data - 16-bit UUID
#define GAP_ADVTYPE_PUBLIC_TARGET_ADDR          0x17 //!< Public Target Address
#define GAP_ADVTYPE_RANDOM_TARGET_ADDR          0x18 //!< Random Target Address
#define GAP_ADVTYPE_APPEARANCE                  0x19 //!< Appearance
#define GAP_ADVTYPE_ADV_INTERVAL                0x1A //!< Advertising Interval
#define GAP_ADVTYPE_LE_BD_ADDR                  0x1B //!< LE Bluetooth Device Address
#define GAP_ADVTYPE_LE_ROLE                     0x1C //!< LE Role
#define GAP_ADVTYPE_SIMPLE_PAIRING_HASHC_256    0x1D //!< Simple Pairing Hash C-256
#define GAP_ADVTYPE_SIMPLE_PAIRING_RANDR_256    0x1E //!< Simple Pairing Randomizer R-256
#define GAP_ADVTYPE_SERVICE_DATA_32BIT          0x20 //!< Service Data - 32-bit UUID
#define GAP_ADVTYPE_SERVICE_DATA_128BIT         0x21 //!< Service Data - 128-bit UUID
#define GAP_ADVTYPE_3D_INFO_DATA                0x3D //!< 3D Information Data
#define GAP_ADVTYPE_MANUFACTURER_SPECIFIC       0xFF //!< Manufacturer Specific Data: first 2 octets contain the Company Identifier Code followed by the additional manufacturer specific data
/** @} End GAP_ADVTYPE_DEFINES */



#endif // end of #ifndef GAP_API_H
