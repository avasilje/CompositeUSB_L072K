#pragma once

#include "usbd_def.h"


#define HID_DESCRIPTOR_TYPE           0x21U
#define HID_REPORT_DESC               0x22U

#define HID_HS_BINTERVAL_DEFAULT      0x05U
#define HID_FS_BINTERVAL_DEFAULT      0x05U

#define CUSTOM_HID_REQ_SET_PROTOCOL   0x0BU
#define CUSTOM_HID_REQ_GET_PROTOCOL   0x03U

#define CUSTOM_HID_REQ_SET_IDLE       0x0AU
#define CUSTOM_HID_REQ_GET_IDLE       0x02U

#define CUSTOM_HID_REQ_SET_REPORT     0x09U
#define CUSTOM_HID_REQ_GET_REPORT     0x01U

#define USAGE_PAGE(X) 0x05, X
#define USAGE_PAGE_GENERIC_DESKTOP_CTRL 0x01
#define USAGE_PAGE_GAME_CTRL            0x05
#define USAGE_PAGE_LED                  0x08
#define USAGE_PAGE_BUTTON               0x09

#define USAGE(X) 0x09, X
#define USAGE_GDC_POINTER   0x01    /* PHY */
#define USAGE_GDC_MOUSE     0x02    /* APP */
#define USAGE_GDC_JOYSTICK  0x04    /* APP */
#define USAGE_GDC_GAMEPAD   0x05    /* APP */
#define USAGE_GDC_WAKEUP    0x3C    /* OSC */

#define USAGE_GDC_X         0x30    /* DV */
#define USAGE_GDC_Y         0x31
#define USAGE_GDC_WHEEL     0x38


/* USB 6.2.2.4 Main Items */
#define INPUT(X)        0x81, X
#define OUTPUT(X)       0x91, X
#define COLLECTION(X)   0xA1, X
#define FEATURE(X)      0xB1, X
#define COLLECTION_END  0xC0

#define DATA_CONSTANT   (1 << 0)        /* Bit 0 {Data (0) | Constant (1)}                  */
#define DATA_VARIABLE   (1 << 1)        /* Bit 1 {Array (0) | Variable (1)}                 */
#define DATA_RELATIVE   (1 << 2)        /* Bit 2 {Absolute (0) | Relative (1)}              */
#define DATA_WRAP       (1 << 3)        /* Bit 3 {No Wrap (0) | Wrap (1)}                   */
#define DATA_NONLINEAR  (1 << 4)        /* Bit 4 {Linear (0) | Non Linear (1)}              */
#define DATA_NOPREF     (1 << 5)        /* Bit 5 {Preferred State (0) | No Preferred (1)}   */
#define DATA_NULL       (1 << 6)        /* Bit 6 {No Null position (0) | Null state(1)}     */
                                        /* Bit 7 Reserved (0)                               */
#define DATA_BUFFERED   (1 << 8)        /* Bit 8 {Bit Field (0) | Buffered Bytes (1)}       */

/* USB HID 1.11 6.2.2.6 */
#define COLLECTION_PHYSICAL     0x00
#define COLLECTION_APPLICATION  0x01
#define COLLECTION_LOGICAL      0x02
#define COLLECTION_REPORT       0x03
#define COLLECTION_NAMED_ARR    0x04
#define COLLECTION_USAGE_SWITCH 0x05
#define COLLECTION_USAGE_MODIF  0x06

#define USAGE_MINIMUM(X) 0x19, X
#define USAGE_MAXIMUM(X) 0x29, X


#define LOGICAL_MINIMUM_8(X) 0x15, X
#define LOGICAL_MAXIMUM_8(X) 0x25, X

#define LOGICAL_MINIMUM_16(X0, X1) 0x16, X0, X1
#define LOGICAL_MAXIMUM_16(X0, X1) 0x26, X0, X1

#define LOGICAL_MINIMUM_32(X) 0x17, X
#define LOGICAL_MAXIMUM_32(X) 0x27, X

#define REPORT_COUNT(X) 0x95, X
#define REPORT_SIZE(X) 0x75, X

typedef enum {
    CUSTOM_HID_IDLE = 0U, CUSTOM_HID_BUSY,
} CUSTOM_HID_StateTypeDef;

#pragma pack(push, 1)
typedef struct _USBD_Dev0_HID_ConfigDesc {
    USBD_InterfaceDesc  interface_desc;
    USBD_HidDesc        hid_desc;
    USBD_EpDesc         ep_in;
    USBD_EpDesc         ep_out;
} USBD_Dev0_HID_ConfigDesc;
#pragma pack(pop)

typedef struct _USBD_HID_Handle {

    union {
        USBD_Dev0_HID_ConfigDesc *dev0;        /* Pointer to HID part in Composite device descriptor
                                                  Initialized during HID registration upon startup */
    } hid_cfg_desc;

    /********** Configuration specific parameters *****/
    /* Initialized @ USBD_Composite_Init -> USBD_HID_Init */
    uint8_t ifnum;
    uint8_t epnum;

    size_t epin_size;
    size_t epout_size;

    uint8_t *ReportBuf;
    size_t  ReportBufLen;

    uint8_t *ReportDesc;
    size_t  ReportDescLen;

    struct _USBD_Handle *pdev;
    /***********************************************/

    uint8_t *pReport;
    uint32_t Protocol;
    uint32_t IdleState;
    uint32_t AltSetting;
    uint32_t IsReportAvailable;
    CUSTOM_HID_StateTypeDef     state;

    void (* Register)(struct _USBD_HID_Handle *hhid, USBD_ConfigDesc *config_desc, int *ifnum, int *epnum);
    USBD_HidDesc* (* GetHidDescr)(struct _USBD_HID_Handle *hhid);
    void (* Init)(struct _USBD_HID_Handle *hhid, uint8_t cfgidx);
    void (* DeInit)(struct _USBD_HID_Handle *hhid);
    int8_t (* OutEvent)(struct _USBD_HID_Handle *hhid, uint8_t *buf, int len);

#if NAVIG
    Dev0_HID_Register();
    Dev0_HID_GetHIDDesc();
    Dev0_HID_Init();
    Dev0_HID_DeInit();
    Dev0_HID_OutEvent();
#endif

} USBD_HID_Handle;

void HID_Register(USBD_HID_Handle *hhid, usbd_intf_t *intf, USBD_ConfigDesc *config_desc, int *ifnum, int *epnum);
uint8_t HID_SendReport(USBD_HID_Handle *hhid, uint8_t *report, size_t len);

