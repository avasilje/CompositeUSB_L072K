#include "usbd_ioreq.h"
#include "usbd_def.h"
#include "usbd_core.h"

void USBD_HID_Init (union intf_dev_handle_u h, USBD_Handle *pdev, uint8_t cfgidx)
{
	USBD_HID_Handle *hhid = h.hid;

	hhid->pdev = pdev;

	hhid->Init(hhid, cfgidx);
#if NAVIG
	Dev0_HID_Init(hhid, cfgidx);
#endif

	/* Open EP IN */
	USBD_LL_OpenEP(pdev, EP_IN_ADDR(hhid->epnum), USBD_EP_TYPE_INTR,
			hhid->epin_size);

	/* Open EP OUT */
	USBD_LL_OpenEP(pdev, EP_OUT_ADDR(hhid->epnum), USBD_EP_TYPE_INTR,
            hhid->epout_size);

	hhid->state = CUSTOM_HID_IDLE;

	/* Prepare Out endpoint to receive 1st packet */
	USBD_LL_PrepareReceive(pdev, EP_OUT_ADDR(hhid->epnum), hhid->ReportBuf, hhid->ReportBufLen);
}

/**
  * @brief  USBD_CUSTOM_HID_Init
  *         DeInitialize the CUSTOM_HID layer
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
void USBD_HID_DeInit(union intf_dev_handle_u h, uint8_t cfgidx)
{
	USBD_HID_Handle *hhid = h.hid;
	USBD_Handle *pdev = hhid->pdev;

	if (!pdev) return;

	/* Close CUSTOM_HID EP IN */
	USBD_LL_CloseEP(pdev, EP_IN_ADDR(hhid->epnum));

	/* Close CUSTOM_HID EP OUT */
	USBD_LL_CloseEP(pdev, EP_OUT_ADDR(hhid->epnum));

	hhid->DeInit(hhid);
#if NAVIG
	Dev0_HID_DeInit(hhid, cfgidx)
#endif

}

/**
  * @brief  USBD_CUSTOM_HID_Setup
  *         Handle the CUSTOM_HID specific requests
  * @param  pdev: instance
  * @param  req: usb requests
  * @retval status
  */
uint8_t USBD_HID_Setup (
		union intf_dev_handle_u h,
		enum setup_recp_e recp,
		uint8_t recp_idx,
        USBD_SetupReq *req)

{
	USBD_HID_Handle *hhid = h.hid;
	struct _USBD_Handle *pdev = hhid->pdev;
	uint16_t len = 0;
	uint8_t  *pbuf = NULL;
	uint16_t status_info = 0;
	uint8_t ret = USBD_BUSY;

	if (recp == RECP_INTERFACE && recp_idx != hhid->ifnum) {
		return USBD_OK;
	}

	switch (req->bmRequest & USB_REQ_TYPE_MASK) {
    case USB_REQ_TYPE_CLASS :
        /* AV: TODO: Why where is no dev_state check?
         *           Is this request valid in any state? */
    	switch (req->bRequest) {
		case CUSTOM_HID_REQ_SET_PROTOCOL:
			hhid->Protocol = (uint8_t)(req->wValue);
			break;

		case CUSTOM_HID_REQ_GET_PROTOCOL:
			USBD_CtlSendData(pdev, (uint8_t *)(void *)&hhid->Protocol, 1U);
			break;

		case CUSTOM_HID_REQ_SET_IDLE:
			hhid->IdleState = (uint8_t)(req->wValue >> 8);
			break;

		case CUSTOM_HID_REQ_GET_IDLE:
			USBD_CtlSendData(pdev, (uint8_t *)(void *)&hhid->IdleState, 1U);
			break;

		case CUSTOM_HID_REQ_SET_REPORT:
			hhid->IsReportAvailable = 1U;
			USBD_CtlPrepareRx(pdev, hhid->ReportBuf, req->wLength);
			break;

		default:
			USBD_CtlError(pdev, req);
			ret = USBD_FAIL;
			break;
    	}
    	break;

    case USB_REQ_TYPE_STANDARD:

    	switch (req->bRequest) {
    	case USB_REQ_GET_STATUS:
			if (pdev->dev_state == USBD_STATE_CONFIGURED) {
				USBD_CtlSendData(pdev, (uint8_t *)(void *)&status_info, 2U);
			}
			else {
				USBD_CtlError(pdev, req);
				ret = USBD_FAIL;
			}
			break;

    	case USB_REQ_GET_DESCRIPTOR:
    	    /* AV: TODO: Why where is no dev_state check ? */
        	if ((req->wValue >> 8) == HID_REPORT_DESC) {
        		pbuf = hhid->ReportDesc;
        		len = MIN(hhid->ReportDescLen, req->wLength);
        	}
        	else {
        		if ((req->wValue >> 8) == HID_DESCRIPTOR_TYPE) {
        			USBD_HidDesc *hid_desc = hhid->GetHidDescr(hhid);
        			pbuf = (uint8_t*)hid_desc;
        			len = MIN(hid_desc->bLength, req->wLength);
        		}
        	}
        	USBD_CtlSendData(pdev, pbuf, len);
        	break;

        case USB_REQ_GET_INTERFACE :
        	if (pdev->dev_state == USBD_STATE_CONFIGURED) {
        		USBD_CtlSendData(pdev, (uint8_t *)(void *)&hhid->AltSetting, 1U);
        	}
        	else {
        		USBD_CtlError(pdev, req);
        		ret = USBD_FAIL;
        	}
        	break;

        case USB_REQ_SET_INTERFACE :
        	if (pdev->dev_state == USBD_STATE_CONFIGURED) {
        		hhid->AltSetting = (uint8_t)(req->wValue);
        	}
        	else {
        		USBD_CtlError(pdev, req);
        		ret = USBD_FAIL;
        	}
        	break;

        default:
        	USBD_CtlError(pdev, req);
        	ret = USBD_FAIL;
        	break;
    	}
    	break;

    default:
    	USBD_CtlError(pdev, req);
    	ret = USBD_FAIL;
    	break;
	}

	return ret;
}

/**
  * @brief  USBD_CUSTOM_HID_SendReport
  *         Send CUSTOM_HID Report
  * @param  pdev: device instance
  * @param  buff: pointer to report
  * @retval status
  */

uint8_t HID_SendReport (USBD_HID_Handle *hhid,
		uint8_t *report, size_t len)
{
	struct _USBD_Handle *pdev = hhid->pdev;

	if (!pdev || pdev->dev_state != USBD_STATE_CONFIGURED) return USBD_OK;

	if (hhid->state != CUSTOM_HID_IDLE) return USBD_BUSY;

	hhid->state = CUSTOM_HID_BUSY;
	USBD_LL_Transmit(pdev, EP_IN_ADDR(hhid->epnum), report, len);

	return USBD_OK;
}

/**
  * @brief  USBD_CUSTOM_HID_DataIn
  *         handle data IN Stage
  * @param  epnum: endpoint index
  * @retval status
  */
uint8_t  USBD_HID_DataIn(union intf_dev_handle_u h, uint8_t epnum)
{
	USBD_HID_Handle *hhid = h.hid;

	if (epnum != hhid->epnum) return USBD_OK;

	/* Ensure that the FIFO is empty before a new transfer, this condition could
       be caused by a new transfer before the end of the previous transfer */
	hhid->state = CUSTOM_HID_IDLE;

	return USBD_BUSY;
}

/**
  * @brief  USBD_CUSTOM_HID_DataOut
  *         handle data OUT Stage
  * @param  pdev: device instance
  * @param  epnum: endpoint index
  * @retval status
  */
uint8_t  USBD_HID_DataOut(union intf_dev_handle_u h, uint8_t epnum)
{
	USBD_HID_Handle *hhid = h.hid;

	if (epnum != hhid->epnum) return USBD_OK;

	hhid->OutEvent(hhid, hhid->ReportBuf, hhid->ReportBufLen);

#if NAVIG
	Dev0_HID_OutEvent(hhid->ReportBuf, hhid->ReportBufLen);
#endif

    USBD_LL_PrepareReceive(hhid->pdev, EP_OUT_ADDR(hhid->epnum),
    		hhid->ReportBuf, hhid->ReportBufLen);

    return USBD_BUSY;
}

/**
  * @brief  USBD_CUSTOM_HID_EP0_RxReady
  *         Handles control request data.
  * @param  pdev: device instance
  * @retval status
  */
uint8_t USBD_HID_EP0_RxReady(union intf_dev_handle_u h)
{
	USBD_HID_Handle *hhid = h.hid;

	if (hhid->IsReportAvailable) {
		hhid->OutEvent(hhid, hhid->ReportBuf, hhid->ReportBufLen);
#if NAVIG
		Dev0_HID_OutEvent(hhid->ReportBuf, hhid->ReportBufLen);
#endif
		hhid->IsReportAvailable = 0;
		return USBD_BUSY;		/* Inform composite layer that received data processed */
	}

    return USBD_OK;
}

void HID_Register (
		USBD_HID_Handle *hhid,
		usbd_intf_t *intf,
		USBD_ConfigDesc *config_desc,
		int *ifnum, int *epnum)
{
	hhid->pdev = NULL;

	intf->h.hid = hhid;
	intf->Init = USBD_HID_Init;
	intf->DeInit = USBD_HID_DeInit;
	intf->EP0_RxReady = USBD_HID_EP0_RxReady;
	intf->Setup = USBD_HID_Setup;
	intf->DataIn = USBD_HID_DataIn;
	intf->DataOut = USBD_HID_DataOut;

	hhid->Register(hhid, config_desc, ifnum, epnum);
#if NAVIG
	Dev0_HID_Register(hhid, config_desc, ifnum, epnum);
#endif

}
