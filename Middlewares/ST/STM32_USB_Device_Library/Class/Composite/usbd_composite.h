#pragma once

#include "usbd_customhid.h"
#include "usbd_cdc.h"

#define host2usb_u16(v) (v)
#define usb2host_u16(v) (v)

void* USBD_CfgDescAppend (USBD_ConfigDesc *cfg_desc, uint8_t *desc, int desc_len);

