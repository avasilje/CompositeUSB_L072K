#include "main.h"
#include "usbd_ioreq.h"
#include "usbd_def.h"
#include "usb_device.h"
#include "dev0.h"

dev0_t g_dev0;

#define DEV0_HID_REPORT_DESC_SIZE     31
#define DEV0_HID_FS_BINTERVAL HID_FS_BINTERVAL_DEFAULT

#define TBD 0x2A        // Values which are initialized during runtime

#pragma pack(push, 1)
static uint8_t Dev0_HID_ReportDesc[] =
{
    USAGE_PAGE(0x81U),                     /* 0x05, 0x81   Usage Page (Vendor defined)     */
    USAGE(0x82U),                          /* 0x09, 0x82     Usage  (Vendor defined)       */
    COLLECTION(COLLECTION_APPLICATION),    /* 0xA1, 0x01   Collection (Application)        */
      USAGE(0x83U),                        /* 0x09, 0x83   Usage (Vendor defined)          */
        LOGICAL_MINIMUM_8(0x00U),          /* 0x15, 0x00   Logical Minimum (0)             */
        LOGICAL_MAXIMUM_8(0xFFU),          /* 0x25, 0xFF   Logical Maximum (max8)          */

        REPORT_SIZE(0x08U),                /* 0x75, 0x08   Report Size (8)                 */
        REPORT_COUNT(sizeof(dev0_in_report_t)),    /* 0x95, 0x02   Report Count (2)                */
        INPUT(DATA_VARIABLE),              /* 0x81, 0x02   Input(Data, Variable, Absolute) */

      USAGE(0x84U),                        /* 0x09, 0x84,  Usage (Vendor defined)          */
        LOGICAL_MINIMUM_8(0x00U),          /* 0x15, 0x00   Logical Minimum (0)             */
        LOGICAL_MAXIMUM_8(0xFFU),          /* 0x25, 0xFF   Logical Maximum (max8)          */
        REPORT_SIZE(0x08),                 /* 0x75, 0x08   Report Size (8)                 */
        REPORT_COUNT(sizeof(dev0_out_report_t)),    /* 0x95, 0x01   Report Count (1)                */
        OUTPUT(DATA_VARIABLE),             /* 0x91, 0x02   Output(Data, Variable, Absolute)*/
    COLLECTION_END,
};
#pragma pack(pop)

/************** Descriptor of CUSTOM HID interface ****************/
USBD_Dev0_HID_ConfigDesc dev0_hid_config_desc_template = {

    .interface_desc = {
        .bLength            = sizeof(USBD_InterfaceDesc),
        .bDescriptorType    = USB_DESC_TYPE_INTERFACE,
        .bInterfaceNumber   = TBD,                      /* Initialized at HID_Register() */
        .bAlternateSetting  = 0x00,
        .bNumEndpoints      = 0x02,
        .bInterfaceClass    = 0x03,                     /* USB Class HID = 3 */
        .bInterfaceSubClass = 0x00,                     /* bInterfaceSubClass : 1=BOOT, 0=no boot*/
        .nInterfaceProtocol = 0x00,                     /* nInterfaceProtocol : 0=none, 1=keyboard, 2=mouse*/
        .iInterface         = 0x00,
    },

    .hid_desc = {
        .bLength              = sizeof(USBD_HidDesc),
        .bDescriptorType      = HID_DESCRIPTOR_TYPE,    /* bDescriptorType: HID                     */
        .bcdHID               = 0x0111,                 /* HID Class Spec release number 1.11       */
        .bCountryCode         = 0x00,                   /* 0x17 */
        .bNumDescriptors      = 0x01,                   /* Number of class descriptors to follow    */
        .bRepDescriptorType   = HID_REPORT_DESC,        /* bDescriptorType                          */
        .wRepDescriptorLength = host2usb_u16(sizeof(Dev0_HID_ReportDesc)),    /* wItemLength: Total length of Report descriptor */
    },

    .ep_in = {
        .bLength             = sizeof(USBD_EpDesc),      /* bLength: Endpoint Descriptor size */
        .bDescriptorType     = USB_DESC_TYPE_ENDPOINT,   /* bDescriptorType:                  */
        .bEndpointAddress    = TBD,                      /* Initialized at HID_Register       */
        .bmAttributes        = 0x03,                     /* bmAttributes: Interrupt endpoint  */
        .wMaxPacketSize      = host2usb_u16(sizeof(dev0_in_report_t)),    /* wMaxPacketSize: 2 Byte max */
        .bInterval           = DEV0_HID_FS_BINTERVAL,    /* bInterval: Polling Interval       */
    },

    .ep_out = {
        .bLength             = sizeof(USBD_EpDesc),      /* bLength: Endpoint Descriptor size */
        .bDescriptorType     = USB_DESC_TYPE_ENDPOINT,   /* bDescriptorType:                  */
        .bEndpointAddress    = TBD,                      /* Initialized at HID_Register       */
        .bmAttributes        = 0x03,                     /* bmAttributes: Interrupt endpoint  */
        .wMaxPacketSize      = host2usb_u16(sizeof(dev0_out_report_t)),/* wMaxPacketSize: 2 Byte max */
        .bInterval           = DEV0_HID_FS_BINTERVAL,    /* bInterval: Polling Interval       */
    }
};

void Dev0_HID_DeInit (USBD_HID_Handle *hhid)
{
    dev0_t *dev0 = &g_dev0;

    dev0->hid = NULL;
    return;
}

int8_t Dev0_HID_OutEvent (USBD_HID_Handle *hhid, uint8_t *buf, int len)
{
    dev0_out_report_t *report = (dev0_out_report_t *)buf;

    uint16_t pb_mask =
    		LED_RED_Pin |
			LED_GREEN_Pin;

    uint16_t pb = 0;

    pb |= (report->leds & 0x01) ? LED_RED_Pin : 0;
    pb |= (report->leds & 0x02) ? LED_GREEN_Pin : 0;

    HAL_GPIO_WritePin(GPIOB, pb, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOB, ~pb & pb_mask, GPIO_PIN_RESET);

    return (USBD_OK);
}

void Dev0_HID_Register (
        USBD_HID_Handle *hhid, USBD_ConfigDesc *config_desc,
        int *ifnum, int *epnum)
{
    USBD_Dev0_HID_ConfigDesc *desc = &dev0_hid_config_desc_template;

    desc->interface_desc.bInterfaceNumber = *ifnum;
    desc->ep_in.bEndpointAddress = EP_IN_ADDR(*epnum);
    desc->ep_out.bEndpointAddress = EP_OUT_ADDR(*epnum);

    hhid->hid_cfg_desc.dev0 = USBD_CfgDescAppend(config_desc, (uint8_t*)desc, sizeof(USBD_Dev0_HID_ConfigDesc));
    *ifnum += 1;
    *epnum += 1;
}

USBD_HidDesc *Dev0_HID_GetHIDDesc(struct _USBD_HID_Handle *hhid)
{
    USBD_Dev0_HID_ConfigDesc *cfg_desc = hhid->hid_cfg_desc.dev0;

    if (hhid && cfg_desc ) {
        return &cfg_desc->hid_desc;
    }
    return NULL;
}

void Dev0_HID_Init (USBD_HID_Handle *hhid, uint8_t cfgidx)
{
    dev0_t *dev0 = &g_dev0;

    USBD_Dev0_HID_ConfigDesc *desc = hhid->hid_cfg_desc.dev0;

    hhid->epin_size = usb2host_u16(desc->ep_in.wMaxPacketSize);
    hhid->epout_size = usb2host_u16(desc->ep_out.wMaxPacketSize);

    hhid->ReportBuf = (uint8_t*)&dev0->hid_out_report;
    hhid->ReportBufLen = sizeof(dev0->hid_out_report);

    hhid->ReportDesc = &Dev0_HID_ReportDesc[0];
    hhid->ReportDescLen = sizeof(Dev0_HID_ReportDesc);

    hhid->ifnum = desc->interface_desc.bInterfaceNumber;
    hhid->epnum = EP_IDX(desc->ep_out.bEndpointAddress);

    dev0->hid = hhid;
}

/* Called from USB side */
USBD_HID_Handle g_hid0 = {
    .Register = Dev0_HID_Register,
    .Init = Dev0_HID_Init,
    .DeInit = Dev0_HID_DeInit,
    .OutEvent = Dev0_HID_OutEvent
};


