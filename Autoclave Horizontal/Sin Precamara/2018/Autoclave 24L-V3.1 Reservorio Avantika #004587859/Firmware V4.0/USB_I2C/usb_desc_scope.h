///////////////////////////////////////////////////////////////////////////
////                         usb_desc_scope.h                          ////
////                                                                   ////
//// An example set of device / configuration descriptors for use with ////
//// the USB Bulk demo (see ex_usb_scope.c)                            ////
////                                                                   ////
//// This file is part of CCS's PIC USB driver code.  See USB.H        ////
//// for more documentation and a list of examples.                    ////
////                                                                   ////
///////////////////////////////////////////////////////////////////////////
////                                                                   ////
//// Version History:                                                  ////
////                                                                   ////
//// March 5th, 2009:                                                  ////
////   Cleanup for Wizard.                                             ////
////   PIC24 Initial release.                                          ////
////                                                                   ////
//// July 13th, 2005:                                                  ////
////   Endpoint descriptor works if USB_EP1_TX_SIZE is 16bits          ////
////   Endpoint descriptor works if USB_EP1_RX_SIZE is 16bits          ////
////                                                                   ////
//// June 20th, 2005:                                                  ////
////   18fxx5x Initial release.                                        ////
////                                                                   ////
//// March 21st, 2005:                                                 ////
////   EP 0x01 and EP 0x81 now use USB_EP1_TX_SIZE and USB_EP1_RX_SIZE ////
////      to define max packet size, to make it easier for dynamically ////
////      changed code.                                                ////
////                                                                   ////
//// June 24th, 2002: Cleanup                                          ////
////                                                                   ////
//// May 6th, 2003: Fixed non-HID descriptors pointing to faulty       ////
////                strings                                            ////
////                                                                   ////
//// August 2nd, 2002: Initial Public Release                          ////
////                                                                   ////
////                                                                   ////
///////////////////////////////////////////////////////////////////////////
////        (C) Copyright 1996,2009 Custom Computer Services           ////
//// This source code may only be used by licensed users of the CCS    ////
//// C compiler.  This source code may only be distributed to other    ////
//// licensed users of the CCS C compiler.  No other use,              ////
//// reproduction or distribution is permitted without written         ////
//// permission.  Derivative programs created using this software      ////
//// in object code form are not restricted in any way.                ////
///////////////////////////////////////////////////////////////////////////

#IFNDEF __USB_DESCRIPTORS__
#DEFINE __USB_DESCRIPTORS__

///////// config options, although it's best to leave alone for this demo /////
#define  USB_CONFIG_PID       0x0003   //changing this value may make the driver incompatible
#define  USB_CONFIG_VID       0x0461   //changing this value may make the driver incompatible
#define  USB_CONFIG_BUS_POWER 100   //100mA  (range is 0..500)
#define  USB_CONFIG_VERSION   0x0100      //01.00  //range is 00.00 to 99.99
//////// end config ///////////////////////////////////////////////////////////

#DEFINE USB_HID_DEVICE     FALSE
#define USB_EP1_TX_ENABLE  USB_ENABLE_BULK  //turn on EP1 for IN bulk/interrupt transfers
#define USB_EP1_RX_ENABLE  USB_ENABLE_BULK  //turn on EP1 for OUT bulk/interrupt transfers

#ifndef USB_EP1_TX_SIZE
#define USB_EP1_TX_SIZE    64  //size to allocate for the tx endpoint 1 buffer
#endif

#ifndef USB_EP1_RX_SIZE
#define USB_EP1_RX_SIZE    8   //size to allocate for the rx endpoint 1 buffer
#endif

#include <usb.h>

//////////////////////////////////////////////////////////////////
///
///   start config descriptor
///   right now we only support one configuration descriptor.
///   the config, interface, class, and endpoint goes into this array.
///
//////////////////////////////////////////////////////////////////

   #DEFINE USB_TOTAL_CONFIG_LEN      32 //config+interface+class+endpoint

   //configuration descriptor
   char const USB_CONFIG_DESC[] = {
   //config_descriptor for config index 1
         USB_DESC_CONFIG_LEN,     //length of descriptor size
         USB_DESC_CONFIG_TYPE,         //constant CONFIGURATION (0x02)
         USB_TOTAL_CONFIG_LEN,0,  //size of all data returned for this config
         1,      //number of interfaces this device supports
         0x01,                //identifier for this configuration.  (IF we had more than one configurations)
         0x00,                //index of string descriptor for this configuration
        #if USB_CONFIG_BUS_POWER
         0x80, //bit 6=1 if self powered, bit 5=1 if supports remote wakeup (we don't), bits 0-4 unused and bit7=1         ==7
        #else
         0xC0, //bit 6=1 if self powered, bit 5=1 if supports remote wakeup (we don't), bits 0-4 unused and bit7=1         ==7
        #endif
         USB_CONFIG_BUS_POWER/2, //maximum bus power required (maximum milliamperes/2)  (0x32 = 100mA)   ==8

   //interface descriptor 0 alt 0
         USB_DESC_INTERFACE_LEN,  //length of descriptor
         USB_DESC_INTERFACE_TYPE,      //constant INTERFACE (0x04)
         0x00,                //number defining this interface (IF we had more than one interface)
         0x00,                //alternate setting
         2,       //number of endpoints, not counting endpoint 0.
         0xFF,                //class code, FF = vendor defined
         0xFF,                //subclass code, FF = vendor
         0xFF,                //protocol code, FF = vendor
         0x00,                //index of string descriptor for interface

   //endpoint descriptor
         USB_DESC_ENDPOINT_LEN, //length of descriptor
         USB_DESC_ENDPOINT_TYPE,     //constant ENDPOINT (0x05)
         0x81,              //endpoint number and direction (0x81 = EP1 IN)
         USB_EP1_TX_ENABLE,              //transfer type supported (0 is control, 1 is iso, 2 is bulk, 3 is interrupt)
         USB_EP1_TX_SIZE & 0xFF,USB_EP1_TX_SIZE >> 8,         //maximum packet size supported
         0x01,              //polling interval in ms. (for interrupt transfers ONLY)

   //endpoint descriptor
         USB_DESC_ENDPOINT_LEN, //length of descriptor
         USB_DESC_ENDPOINT_TYPE,     //constant ENDPOINT (0x05)
         0x01,              //endpoint number and direction (0x01 = EP1 OUT)
         USB_EP1_RX_ENABLE,              //transfer type supported (0 is control, 1 is iso, 2 is bulk, 3 is interrupt)
         USB_EP1_RX_SIZE & 0xFF,USB_EP1_RX_SIZE >> 8,         //maximum packet size supported
         0x01,              //polling interval in ms. (for interrupt transfers ONLY)
  };

   //****** BEGIN CONFIG DESCRIPTOR LOOKUP TABLES ********
   //since we can't make pointers to constants in certain pic16s, this is an offset table to find
   //  a specific descriptor in the above table.

   //NOTE: DO TO A LIMITATION OF THE CCS CODE, ALL HID INTERFACES MUST START AT 0 AND BE SEQUENTIAL
   //      FOR EXAMPLE, IF YOU HAVE 2 HID INTERFACES THEY MUST BE INTERFACE 0 AND INTERFACE 1
   #define USB_NUM_HID_INTERFACES   0

   //the maximum number of interfaces seen on any config
   //for example, if config 1 has 1 interface and config 2 has 2 interfaces you must define this as 2
   #define USB_MAX_NUM_INTERFACES   1

   //define how many interfaces there are per config.  [0] is the first config, etc.
   const char USB_NUM_INTERFACES[USB_NUM_CONFIGURATIONS]={1};

   #if (sizeof(USB_CONFIG_DESC) != USB_TOTAL_CONFIG_LEN)
      #error USB_TOTAL_CONFIG_LEN not defined correctly
   #endif


//////////////////////////////////////////////////////////////////
///
///   start device descriptors
///
//////////////////////////////////////////////////////////////////

   //device descriptor
   char const USB_DEVICE_DESC[] ={
         USB_DESC_DEVICE_LEN,          //the length of this report
         0x01,                //constant DEVICE (0x01)
         0x10,0x01,           //usb version in bcd
         0x00,                //class code (if 0, interface defines class.  FF is vendor defined)
         0x00,                //subclass code
         0x00,                //protocol code
         USB_MAX_EP0_PACKET_LENGTH,   //max packet size for endpoint 0. (SLOW SPEED SPECIFIES 8)
         USB_CONFIG_VID & 0xD8, ((USB_CONFIG_VID >> 8) & 0x04), //vendor id       ==9, 10
         USB_CONFIG_PID & 0x0B, ((USB_CONFIG_PID >> 8) & 0x00), //product id, don't use 0xffff       ==11, 12
         USB_CONFIG_VERSION & 0x01, ((USB_CONFIG_VERSION >> 8) & 0x00), //device release number  ==13,14
         0x01,                //index of string description of manufacturer. therefore we point to string_1 array (see below)
         0x02,                //index of string descriptor of the product
         0x00,                //index of string descriptor of serial number
         USB_NUM_CONFIGURATIONS   //number of possible configurations
   };


//////////////////////////////////////////////////////////////////
///
///   start string descriptors
///   String 0 is a special language string, and must be defined.  People in U.S.A. can leave this alone.
///
///   You must define the length else get_next_string_character() will not see the string
///   Current code only supports 10 strings (0 thru 9)
///
//////////////////////////////////////////////////////////////////

//the offset of the starting location of each string.
//offset[0] is the start of string 0, offset[1] is the start of string 1, etc.
const char USB_STRING_DESC_OFFSET[]={0,4,12};

#define USB_STRING_DESC_COUNT sizeof(USB_STRING_DESC_OFFSET)

// Here is where the "CCS" Manufacturer string and "CCS Bulk Demo" are stored.
// Strings are saved as unicode.
// These strings are mostly only displayed during the add hardware wizard.
// Once the operating system drivers have been installed it will usually display
// the name from the drivers .INF.
char const USB_STRING_DESC[]={
   //string 0
         4, //length of string index
         USB_DESC_STRING_TYPE, //descriptor type 0x03 (STRING)
         0x09,0x04,   //Microsoft Defined for US-English
   //string 1
         8, //length of string index
         USB_DESC_STRING_TYPE, //descriptor type 0x03 (STRING)
         'C',0,
         'C',0,
         'S',0,
   //string 2
         28, //length of string index
         USB_DESC_STRING_TYPE, //descriptor type 0x03 (STRING)
         'C',0,
         'C',0,
         'S',0,
         ' ',0,
         'B',0,
         'u',0,
         'l',0,
         'k',0,
         ' ',0,
         'D',0,
         'e',0,
         'm',0,
         'o',0
};


#ENDIF
