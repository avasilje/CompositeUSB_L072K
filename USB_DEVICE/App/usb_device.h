#pragma once

#include "usbd_composite.h"

#pragma pack(push, 1)
union _USBD_ConfigDescExt{
	USBD_ConfigDesc config_desc;
	uint8_t 	raw[512];
};
#pragma pack(pop)

extern USBD_HID_Handle g_hid0;
extern USBD_CDC_Handle g_cdc0;
extern USBD_CDC_Handle g_cdc1;

extern union _USBD_ConfigDescExt USBD_ConfigDescExt;

void MX_USB_DEVICE_Init(void);

