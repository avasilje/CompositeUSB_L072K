#include "usbd_def.h"
#include "usb_device.h"

#define TBD 0x2A

__ALIGN_BEGIN static uint8_t USBD_Composite_DeviceQualifierDesc[USB_LEN_DEV_QUALIFIER_DESC] __ALIGN_END =
{
  USB_LEN_DEV_QUALIFIER_DESC,
  USB_DESC_TYPE_DEVICE_QUALIFIER,
  0x00,
  0x02,
  0x00,
  0x00,
  0x00,
  0x40,
  0x01,
  0x00,
};

void* USBD_CfgDescAppend (USBD_ConfigDesc *cfg_desc, uint8_t *desc, int desc_len)
{
	uint16_t tot_len = usb2host_u16(cfg_desc->wTotalLength);
	uint8_t *dst = ((uint8_t*)cfg_desc) + tot_len;

	assert(tot_len + desc_len <= sizeof(USBD_ConfigDescExt));

	memcpy(dst, desc, desc_len);
	tot_len += desc_len;
	cfg_desc->wTotalLength = host2usb_u16(tot_len);

	return dst;
}

/**
  * @brief  USBD_Composite_Init
  *         Initialize the CDC interface
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t USBD_Composite_Init (USBD_Handle *pdev, uint8_t cfgidx)
{
	int i;

	if (!pdev) {
		return USBD_FAIL;
	}

	for (i = 0; i < COMPOSITE_INTF_NUM; i ++) {
		usbd_intf_t *intf = &pdev->intf[i];

		if (!intf || !intf->Init || !intf->h.ctx) continue;

		intf->Init(intf->h, pdev, cfgidx);

#if NAVIG
		USBD_HID_Init(hhid, pdev, cfgidx);
		USBD_CDC_Init(hcdc, pdev, cfgidx);
#endif
	}

	return USBD_OK;
}

/**
  * @brief  USBD_CDC_Init
  *         DeInitialize the CDC layer
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t USBD_Composite_DeInit (USBD_Handle *pdev, uint8_t cfgidx)
{
	int i;

	for (i = 0; i < COMPOSITE_INTF_NUM; i ++) {
		usbd_intf_t *intf = &pdev->intf[i];

		if (!intf || !intf->Init || !intf->h.ctx) continue;

		intf->DeInit(intf->h, cfgidx);

#if NAVIG
		USBD_HID_DeInit(chid, cfgidx);
		USBD_CDC_DeInit(USBD_CDC_HandleTypeDef *hcdc, cfgidx);
#endif
	}

   	return USBD_OK;
}

uint8_t USBD_Composite_Setup (
		USBD_Handle *pdev,
        USBD_SetupReq *req)
{
	USBD_Status rc;
	int i;
	uint8_t bmReqRecp = req->bmRequest & USB_REQ_RECIPIENT_MASK;
	enum setup_recp_e recp =
			(bmReqRecp == USB_REQ_RECIPIENT_INTERFACE) ? RECP_INTERFACE :
			(bmReqRecp == USB_REQ_RECIPIENT_ENDPOINT) ? RECP_ENDPOINT :
			RECP_INVALID;

	uint8_t recp_idx =
			(recp == RECP_INTERFACE) ? req->wIndex : req->wIndex & 0x7F;

	if (recp == RECP_INVALID) return USBD_OK;

	for (i = 0; i < COMPOSITE_INTF_NUM; i++) {
		usbd_intf_t *intf = &pdev->intf[i];

		if (!intf || !intf->Setup) continue;

		rc = intf->Setup(intf->h, recp, recp_idx, req);
#if NAVIG
		USBD_CDC_Setup();
		USBD_HID_Setup();
#endif

		if (rc == USBD_BUSY || rc == USBD_FAIL) break;
		/* rc == USBD_OK means request is not accepted by the interface */
	}

	return USBD_OK;
}

uint8_t USBD_Composite_DataIn (USBD_Handle *pdev, uint8_t epnum)
{
	USBD_Status rc;
	int i;

	for (i = 0; i < COMPOSITE_INTF_NUM; i++) {
		usbd_intf_t *intf = &pdev->intf[i];

		if (!intf || !intf->DataIn) continue;

		rc = intf->DataIn(intf->h, epnum);
#if NAVIG
		USBD_HID_DataIn();
		USBD_CDC_DataIn();
#endif

		if (rc == USBD_BUSY || rc == USBD_FAIL) break;
		/* rc == USBD_OK means request is not accepted by the interface */
	}

	return 0;
}

uint8_t USBD_Composite_DataOut (USBD_Handle *pdev, uint8_t epnum)
{
	USBD_Status rc;
	int i;

	for (i = 0; i < COMPOSITE_INTF_NUM; i++) {
		usbd_intf_t *intf = &pdev->intf[i];

		if (!intf || !intf->DataOut) continue;

		rc = intf->DataOut(intf->h, epnum);
#if NAVIG
		USBD_CDC_DataOut(h.cdc, epnum);
		USBD_HID_DataOut(h.chid, epnum);
#endif

		if (rc == USBD_BUSY || rc == USBD_FAIL) break;
		/* rc == USBD_OK means request is not accepted by the
		 * interface and next one will be tried in the loop */
	}

	return USBD_OK;
}

uint8_t USBD_Composite_EP0_RxReady (USBD_Handle *pdev)
{
	USBD_Status rc;
	int i;

	for (i = 0; i < COMPOSITE_INTF_NUM; i ++) {
		usbd_intf_t *intf = &pdev->intf[i];

		if (!intf || !intf->EP0_RxReady) continue;

		rc = intf->EP0_RxReady(intf->h);
#if NAVIG
		USBD_CDC_EP0_RxReady();
		USBD_CHID_EP0_RxReady();
#endif
		if (rc == USBD_BUSY || rc == USBD_FAIL) break;
	}

	return USBD_OK;
}

uint8_t* USBD_Composite_GetCfgDesc (struct _USBD_Handle *pdev, uint16_t *length)
{
	*length = pdev->config_desc->wTotalLength;
	return (uint8_t*)pdev->config_desc;
}

uint8_t  *USBD_Composite_GetDeviceQualifierDescriptor (uint16_t *length)
{
	*length = sizeof (USBD_Composite_DeviceQualifierDesc);
	return USBD_Composite_DeviceQualifierDesc;
}


USBD_Class USBD_Class_Composite =
{
    .Init = USBD_Composite_Init,
    .DeInit = USBD_Composite_DeInit,
    .Setup = USBD_Composite_Setup,
    .EP0_TxSent = NULL,
    .EP0_RxReady = USBD_Composite_EP0_RxReady,
    .DataIn = USBD_Composite_DataIn,
    .DataOut = USBD_Composite_DataOut,
    .SOF = NULL,
    .IsoINIncomplete = NULL,
    .IsoOUTIncomplete = NULL,
    .GetHSConfigDescriptor =  USBD_Composite_GetCfgDesc,
    .GetFSConfigDescriptor =   USBD_Composite_GetCfgDesc,
    .GetOtherSpeedConfigDescriptor = USBD_Composite_GetCfgDesc,
    .GetDeviceQualifierDescriptor = USBD_Composite_GetDeviceQualifierDescriptor,
};

