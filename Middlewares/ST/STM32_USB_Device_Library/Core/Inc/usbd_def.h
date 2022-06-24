#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "usbd_conf.h"

#define TBD 0x2A

#define EP_IDX(ep_addr) ((uint8_t)(ep_addr) & 0x7F)
#define EP_IN_ADDR(ep)  (0x80 | ((uint8_t)(ep) & 0x07))    /* Maximum number of EP == 8 in both directions */
#define EP_OUT_ADDR(ep) (0x00 | ((uint8_t)(ep) & 0x07))

#define EP_IS_IN_ADDR(ep_addr)  (0x80 & (ep_addr))
#define EP_IS_OUT_ADDR(ep_addr)  (!EP_IS_IN_ADDR(ep_addr))

#ifndef NULL
#define NULL                                            0U
#endif /* NULL */

#ifndef USBD_MAX_NUM_INTERFACES
#define USBD_MAX_NUM_INTERFACES                         1U
#endif /* USBD_MAX_NUM_CONFIGURATION */

#ifndef USBD_MAX_NUM_CONFIGURATION
#define USBD_MAX_NUM_CONFIGURATION                      1U
#endif /* USBD_MAX_NUM_CONFIGURATION */

#ifndef USBD_LPM_ENABLED
#define USBD_LPM_ENABLED                                0U
#endif /* USBD_LPM_ENABLED */

#ifndef USBD_SELF_POWERED
#define USBD_SELF_POWERED                               1U
#endif /*USBD_SELF_POWERED */

#ifndef USBD_SUPPORT_USER_STRING
#define USBD_SUPPORT_USER_STRING                        0U
#endif /* USBD_SUPPORT_USER_STRING */

#ifndef USBD_MAX_POWER
#define USBD_MAX_POWER                                  0x32U /* 100 mA */
#endif /* USBD_MAX_POWER */

#ifndef USBD_SUPPORT_USER_STRING_DESC
#define USBD_SUPPORT_USER_STRING_DESC                   0U
#endif /* USBD_SUPPORT_USER_STRING_DESC */

#ifndef USBD_CLASS_USER_STRING_DESC
#define USBD_CLASS_USER_STRING_DESC                     0U
#endif /* USBD_CLASS_USER_STRING_DESC */

#define  USB_LEN_DEV_QUALIFIER_DESC                     0x0AU
#define  USB_LEN_DEV_DESC                               0x12U
#define  USB_LEN_CFG_DESC                               0x09U
#define  USB_LEN_IF_DESC                                0x09U
#define  USB_LEN_EP_DESC                                0x07U
#define  USB_LEN_OTG_DESC                               0x03U
#define  USB_LEN_LANGID_STR_DESC                        0x04U
#define  USB_LEN_OTHER_SPEED_DESC_SIZ                   0x09U

#define  USBD_IDX_LANGID_STR                            0x00U
#define  USBD_IDX_MFC_STR                               0x01U
#define  USBD_IDX_PRODUCT_STR                           0x02U
#define  USBD_IDX_SERIAL_STR                            0x03U
#define  USBD_IDX_CONFIG_STR                            0x04U
#define  USBD_IDX_INTERFACE_STR                         0x05U

/*
 * USB 2.0 USB Device Requests
 * Table 9-2. Format of Setup Data
 * */
#define  USB_REQ_DIR_IN                                   0x80U
#define  USB_REQ_DIR_OUT                                  0x00U
#define  USB_REQ_DIR_MASK                                 0x80U

#define  USB_REQ_TYPE_STANDARD                          0x00U
#define  USB_REQ_TYPE_CLASS                             0x20U
#define  USB_REQ_TYPE_VENDOR                            0x40U
#define  USB_REQ_TYPE_MASK                              0x60U

#define  USB_REQ_RECIPIENT_DEVICE                       0x00U
#define  USB_REQ_RECIPIENT_INTERFACE                    0x01U
#define  USB_REQ_RECIPIENT_ENDPOINT                     0x02U
#define  USB_REQ_RECIPIENT_MASK                         0x1FU

#define  USB_REQ_GET_STATUS                             0x00U
#define  USB_REQ_CLEAR_FEATURE                          0x01U
#define  USB_REQ_SET_FEATURE                            0x03U
#define  USB_REQ_SET_ADDRESS                            0x05U
#define  USB_REQ_GET_DESCRIPTOR                         0x06U
#define  USB_REQ_SET_DESCRIPTOR                         0x07U
#define  USB_REQ_GET_CONFIGURATION                      0x08U
#define  USB_REQ_SET_CONFIGURATION                      0x09U
#define  USB_REQ_GET_INTERFACE                          0x0AU
#define  USB_REQ_SET_INTERFACE                          0x0BU
#define  USB_REQ_SYNCH_FRAME                            0x0CU

#define  USB_DESC_TYPE_DEVICE                           0x01U
#define  USB_DESC_TYPE_CONFIGURATION                    0x02U
#define  USB_DESC_TYPE_STRING                           0x03U
#define  USB_DESC_TYPE_INTERFACE                        0x04U
#define  USB_DESC_TYPE_ENDPOINT                         0x05U
#define  USB_DESC_TYPE_DEVICE_QUALIFIER                 0x06U
#define  USB_DESC_TYPE_OTHER_SPEED_CONFIGURATION        0x07U
#define  USB_DESC_TYPE_IAD                              0x0BU
#define  USB_DESC_TYPE_BOS                              0x0FU

#define USB_CONFIG_REMOTE_WAKEUP                        0x02U
#define USB_CONFIG_SELF_POWERED                         0x01U

#define USB_FEATURE_EP_HALT                             0x00U
#define USB_FEATURE_REMOTE_WAKEUP                       0x01U
#define USB_FEATURE_TEST_MODE                           0x02U

#define USB_DEVICE_CAPABITY_TYPE                        0x10U

#define USB_CONF_DESC_SIZE                              0x09U
#define USB_IF_DESC_SIZE                                0x09U
#define USB_EP_DESC_SIZE                                0x07U
#define USB_IAD_DESC_SIZE                               0x08U
#define USB_CUSTOM_HID_DESC_SIZ                          0x09U

#define USB_HS_MAX_PACKET_SIZE                          512U
#define USB_FS_MAX_PACKET_SIZE                          64U
#define USB_MAX_EP0_SIZE                                64U

/*  Device Status */
#define USBD_STATE_DEFAULT                              0x01U
#define USBD_STATE_ADDRESSED                            0x02U
#define USBD_STATE_CONFIGURED                           0x03U
#define USBD_STATE_SUSPENDED                            0x04U


/*  EP0 State */
#define USBD_EP0_IDLE                                   0x00U
#define USBD_EP0_SETUP                                  0x01U
#define USBD_EP0_DATA_IN                                0x02U
#define USBD_EP0_DATA_OUT                               0x03U
#define USBD_EP0_STATUS_IN                              0x04U
#define USBD_EP0_STATUS_OUT                             0x05U
#define USBD_EP0_STALL                                  0x06U

#define USBD_EP_TYPE_CTRL                               0x00U
#define USBD_EP_TYPE_ISOC                               0x01U
#define USBD_EP_TYPE_BULK                               0x02U
#define USBD_EP_TYPE_INTR                               0x03U

enum setup_recp_e {
    RECP_INVALID   = 0,
    RECP_INTERFACE = 1,
    RECP_ENDPOINT  = 2
};

/* Following USB Device Speed */
typedef enum
{
  USBD_SPEED_HIGH  = 0U,
  USBD_SPEED_FULL  = 1U,
  USBD_SPEED_LOW   = 2U,
} USBD_Speed;

/* Following USB Device status */
typedef enum
{
  USBD_OK = 0U,
  USBD_BUSY,
  USBD_EMEM,
  USBD_FAIL,
} USBD_Status;

#pragma pack(push, 1)

typedef struct _USBD_SetupReq {
  uint8_t   bmRequest;
  uint8_t   bRequest;
  uint16_t  wValue;
  uint16_t  wIndex;
  uint16_t  wLength;
} USBD_SetupReq;

typedef struct {
    uint8_t  bLength;
    uint8_t  data[0];
} USBD_DescGeneric;

typedef struct {
    uint8_t  bLength             ; /* 0 / 1 Numeric expression specifying the size ofthis descriptor. 0x12                                  */
    uint8_t  bDescriptorType     ; /* 1 / 1 Device descriptor type(assigned by USB). 0x01                                                   */
    uint16_t bcdUSB              ; /* 2 / 2 USB HID Specification Release 1.0. 0x100                                                        */
    uint8_t  bDeviceClass        ; /* 4 / 1 Class code(assigned by USB).Note that the HIDclass is defined in the Interface descriptor.      */
    uint8_t  bDeviceSubClass     ; /* 5 / 1 Subclass code(assigned by USB).These codes are qualified by the value of the bDeviceClass field.*/
    uint8_t  bDeviceProtocol     ; /* 6 / 1 Protocol code.These codes are qualified by the value of the bDeviceSubClass field.              */
    uint8_t  bMaxPacketSize0     ; /* 7 / 1 Maximum packet size for endpoint zero(only 8, 16, 32, or 64 are valid).                         */
    uint16_t idVendor            ; /* 8 / 2 Vendor ID(assigned by USB)                                                                      */
    uint16_t idProduct           ; /* 10 / 2 Product ID(assigned by manufacturer).                                                          */
    uint16_t bcdDevice           ; /* 12 / 2 Device release number(assigned by manufacturer).                                               */
    uint8_t  iManufacturer       ; /* 14 / 1 Index of String descriptor describing manufacturer.                                            */
    uint8_t  iProduct            ; /* 15 / 1 Index of string descriptor describing product.                                                 */
    uint8_t  iSerialNumber       ; /* 16 / 1 Index of String descriptor describing the device’s serial number.                              */
    uint8_t  bNumConfigurations  ; /* 17 / 1 Number of possible configurations.                                                             */
} USBD_DeviceDesc;

typedef struct {
    uint8_t   bLength;              /* Size of this descriptor in bytes. 0x09 */
    uint8_t   bDescriptorType;      /* Configuration (assigned by USB). 0x02 */
    uint16_t  wTotalLength;         /* Total length of data returned for this configuration. */
    uint8_t   bNumInterfaces;       /* Number of interfaces supported by this configuration. */
    uint8_t   bConfigurationValue;  /* Value to use as an argument to Set Configuration to
                                       select this configuration.*/
    uint8_t   iConfiguration;       /* Index of string descriptor describing this configuration.
                                       In this case there is none*/
    uint8_t   bmAttributes;         /* Configuration characteristics */
    uint8_t   bMaxPower;            /* Maximum power consumption of USB device from bus
                                       in this specific configuration when the device is fully
                                       operational. Expressed in 2 mA units */
    uint8_t   data[0];
} USBD_ConfigDesc;

typedef struct {
    uint8_t   bLength;
    uint8_t   bDescriptorType;
    uint16_t  wTotalLength;
    uint8_t   bNumDeviceCaps;
} USBD_BosDesc;

typedef struct {
    uint8_t   bLength;
    uint8_t   bDescriptorType;
    uint8_t   bEndpointAddress;
    uint8_t   bmAttributes;
    uint16_t  wMaxPacketSize;
    uint8_t   bInterval;
} USBD_EpDesc;

/************** Interface Descriptor ****************/
typedef struct {
    uint8_t bLength;             /* Interface Descriptor size    */
    uint8_t bDescriptorType;     /* Interface descriptor type    */
    uint8_t bInterfaceNumber;    /* Number of Interface            */
    uint8_t bAlternateSetting;   /* Alternate setting             */
    uint8_t bNumEndpoints;
    uint8_t bInterfaceClass;
    uint8_t bInterfaceSubClass;  /* 1=BOOT, 0=no boot            */
    uint8_t nInterfaceProtocol;  /* 0=none, 1=keyboard, 2=mouse    */
    uint8_t iInterface;          /* Index of string descriptor    */
} USBD_InterfaceDesc;

typedef struct {
    uint8_t  bLength;               /* 0 / 1 Size of this descriptor in bytes.                                                            */
    uint8_t  bDescriptorType;       /* 1 / 1 HID descriptor type(assigned by USB).                                                       */
    uint16_t bcdHID;                /* 2 / 2 HID Class Specification release number in binarycoded decimal—for example, 2.10 is 0x210).*/
    uint8_t  bCountryCode;          /* 4 / 1 Hardware target country.                                                                   */
    uint8_t  bNumDescriptors;       /* 5 / 1 Number of HID class descriptors to follow.                                                   */
    uint8_t  bRepDescriptorType;    /* 6 / 1 Report descriptor type.                                                                   */
    uint16_t wRepDescriptorLength;  /* 7 / 2 Total length of Report descriptor. 0x3F                                                   */
} USBD_HidDesc;

/***********************************/
typedef struct {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bFirstInterface;        /* TBD */
    uint8_t bInterfaceCount;
    uint8_t bFunctionClass;
    uint8_t bFunctionSubClass;
    uint8_t bFunctionProtocol;
    uint8_t iFunction;
} USBD_IADDesc;


/* Universal Serial Bus Class Definitions for Communications Devices CDC120-20101103-track.pdf */
typedef struct {
    uint8_t  bFunctionLength;    /* Number    Size of this descriptor in bytes.*/
    uint8_t  bDescriptorType;    /* 0x24 CS_Interface; 0x25 CS_ENDPOINT; Descriptor type. */
    uint8_t  bDescriptorSubtype; /* Header functional descriptor subtype as defined in Table 13. */
} USBD_FuncDesc;

typedef struct {
    USBD_FuncDesc hdr;
    uint16_t bcdCDC;             /* Number    USB Class Definitions for Communications Devices Specification release number in binary - coded decimal. */
} USBD_FuncDescHdr;

typedef struct {
    USBD_FuncDesc hdr;
    uint8_t bmCapabilities;
    uint8_t bDataInterface;
} USBD_FuncDescCallMng;

typedef struct {
    USBD_FuncDesc hdr;
    uint8_t bmCapabilities;      /* bmCapabilities (PSTN120 5.3.2 Table 4)*/
} USBD_FuncDescACM;

typedef struct {
    USBD_FuncDesc hdr;
    uint8_t bMasterInterface;    /* Communication class interface */
    uint8_t bSlaveInterface0;    /* Data Class Interface */
} USBD_FuncDescUnion;

/***********************************/
/* USB Device handle structure */
typedef struct _USBD_Endpoint {
    uint32_t status;
    uint32_t total_length;
    uint32_t rem_length;
    uint32_t maxpacket;         /* AV: used for EP0 only :/  */
    uint16_t is_used;
    uint16_t bInterval;
} USBD_Endpoint;

#pragma pack(pop)

struct _USBD_Handle;

typedef struct _Device_cb
{
  uint8_t (*Init)(struct _USBD_Handle *pdev, uint8_t cfgidx);
  uint8_t (*DeInit)(struct _USBD_Handle *pdev, uint8_t cfgidx);
  /* Control Endpoints*/
  uint8_t (*Setup)(struct _USBD_Handle *pdev, USBD_SetupReq  *req);
  uint8_t (*EP0_TxSent)(struct _USBD_Handle *pdev);
  uint8_t (*EP0_RxReady)(struct _USBD_Handle *pdev);
  /* Class Specific Endpoints*/
  uint8_t (*DataIn)(struct _USBD_Handle *pdev, uint8_t epnum);
  uint8_t (*DataOut)(struct _USBD_Handle *pdev, uint8_t epnum);
  uint8_t (*SOF)(struct _USBD_Handle *pdev);
  uint8_t (*IsoINIncomplete)(struct _USBD_Handle *pdev, uint8_t epnum);
  uint8_t (*IsoOUTIncomplete)(struct _USBD_Handle *pdev, uint8_t epnum);

  uint8_t  *(*GetHSConfigDescriptor)(struct _USBD_Handle *pdev, uint16_t *length);
  uint8_t  *(*GetFSConfigDescriptor)(struct _USBD_Handle *pdev, uint16_t *length);
  uint8_t  *(*GetOtherSpeedConfigDescriptor)(struct _USBD_Handle *pdev, uint16_t *length);

  uint8_t  *(*GetDeviceQualifierDescriptor)(uint16_t *length);
#if (USBD_SUPPORT_USER_STRING_DESC == 1U)
  uint8_t  *(*GetUsrStrDescriptor)(struct _USBD_Handle *pdev, uint8_t index,  uint16_t *length);
#endif

} USBD_Class;

#if NAVIG
extern USBD_Class USBD_Class_Composite;
#endif

/* USB Device descriptors structure */
typedef struct
{
  uint8_t *(*GetDeviceDescriptor)(USBD_Speed speed, uint16_t *length);
  uint8_t *(*GetLangIDStrDescriptor)(USBD_Speed speed, uint16_t *length);
  uint8_t *(*GetManufacturerStrDescriptor)(USBD_Speed speed, uint16_t *length);
  uint8_t *(*GetProductStrDescriptor)(USBD_Speed speed, uint16_t *length);
  uint8_t *(*GetSerialStrDescriptor)(USBD_Speed speed, uint16_t *length);
  uint8_t *(*GetConfigurationStrDescriptor)(USBD_Speed speed, uint16_t *length);
  uint8_t *(*GetInterfaceStrDescriptor)(USBD_Speed speed, uint16_t *length);
#if (USBD_CLASS_USER_STRING_DESC == 1)
  uint8_t *(*GetUserStrDescriptor)(USBD_Speed speed, uint8_t idx, uint16_t *length);
#endif
#if ((USBD_LPM_ENABLED == 1U) || (USBD_CLASS_BOS_ENABLED == 1))
  uint8_t *(*GetBOSDescriptor)(USBD_Speed speed, uint16_t *length);
#endif
} USBD_Descriptors;

#if NAVIG
extern USBD_Descriptors FS_Desc;
#endif

typedef struct usbd_intf_s {

    union intf_dev_handle_u {
        void *ctx;
        struct _USBD_HID_Handle *hid;
        struct _USBD_CDC_Handle *cdc;
    } h;

    void (*Init)(union intf_dev_handle_u h, struct _USBD_Handle *pdev, uint8_t cfgidx);
    void (*DeInit)(union intf_dev_handle_u h, uint8_t cfgidx);
    uint8_t (*EP0_RxReady)(union intf_dev_handle_u h);
    uint8_t (*Setup)(union intf_dev_handle_u h, enum setup_recp_e, uint8_t recp_idx, USBD_SetupReq  *req);
    uint8_t (*DataIn)(union intf_dev_handle_u h, uint8_t epnum);
    uint8_t (*DataOut)(union intf_dev_handle_u h, uint8_t epnum);

#if 0
    /* Control Endpoints*/
    uint8_t (*Setup)(struct _USBD_Handle *pdev, USBD_SetupReq  *req);
    uint8_t (*EP0_TxSent)(struct _USBD_Handle *pdev);
    /* Class Specific Endpoints*/

    uint8_t (*SOF)(struct _USBD_Handle *pdev);
    uint8_t (*IsoINIncomplete)(struct _USBD_Handle *pdev, uint8_t epnum);
    uint8_t (*IsoOUTIncomplete)(struct _USBD_Handle *pdev, uint8_t epnum);
#endif

} usbd_intf_t;

/* USB Device handle structure */
#define COMPOSITE_INTF_NUM 3

typedef struct _USBD_Handle {
  uint8_t              id;
  uint32_t             dev_config;
  uint32_t             dev_default_config;
  uint32_t             dev_config_status;

  USBD_Speed           dev_speed;
  USBD_Endpoint        ep_in[16];
  USBD_Endpoint        ep_out[16];
  __IO uint32_t        ep0_state;
  uint32_t             ep0_data_len;
  __IO uint8_t         dev_state;
  __IO uint8_t         dev_old_state;
  uint8_t              dev_address;
  uint8_t              dev_connection_status;
  uint8_t              dev_test_mode;
  uint32_t             dev_remote_wakeup;

  USBD_SetupReq        request;
  USBD_Descriptors     *pDesc;
  USBD_Class           *pClass;

  uint8_t              ConfIdx;

  USBD_ConfigDesc      *config_desc;

  usbd_intf_t          intf[COMPOSITE_INTF_NUM];

  PCD_HandleTypeDef    *pPCDHandle;
  
} USBD_Handle;

#include "usbd_composite.h"

extern USBD_Class  USBD_Class_Composite;

/** @defgroup USBD_DEF_Exported_Macros
  * @{
  */
#define  SWAPBYTE(addr)        (((uint16_t)(*((uint8_t *)(addr)))) + \
                               (((uint16_t)(*(((uint8_t *)(addr)) + 1U))) << 8U))

#define LOBYTE(x)  ((uint8_t)(x & 0x00FFU))
#define HIBYTE(x)  ((uint8_t)((x & 0xFF00U) >> 8U))
#define MIN(a, b)  (((a) < (b)) ? (a) : (b))
#define MAX(a, b)  (((a) > (b)) ? (a) : (b))
