
/* Communication Device Class */

#include "usbd_ioreq.h"
#include "usbd_def.h"
#include "usbd_core.h"
#include "usbd_cdc.h"

USBD_CDC_ConfigDesc	cdc_config_desc_template = {
	.if_assoc_desc = {
		.bLength			= sizeof(USBD_IADDesc),
		.bDescriptorType	= USB_DESC_TYPE_IAD,
		.bFirstInterface	= TBD,
		.bInterfaceCount	= 0x02,
		.bFunctionClass		= 0x02, /* bFunctionClass  - Communication Interface Class */
		.bFunctionSubClass	= 0x02, /* bFunctionSubClass - Abstract Control Model  */
		.bFunctionProtocol	= 0x01, /* bFunctionProtocol - Common AT commands */
		.iFunction			= 0x00, /* iFunction (Index of string descriptor describing this function) */
	},

	.interface_desc_cmd = {
		.bLength = sizeof(USBD_InterfaceDesc),          /* Interface Descriptor size                      */
		.bDescriptorType    = USB_DESC_TYPE_INTERFACE,  /* Interface descriptor type                      */
		.bInterfaceNumber   = TBD,                      /* Number of Interface                            */
		.bAlternateSetting  = 0,                        /* Alternate setting                              */
		.bNumEndpoints      = 1,                        /* bNumEndpoints: One endpoints used              */
		.bInterfaceClass    = 2,                        /* bInterfaceClass: Communication Interface Class */
		.bInterfaceSubClass = 2,                        /* bInterfaceSubClass: Abstract Control Model     */
		.nInterfaceProtocol = 1,                        /* bInterfaceProtocol: Common AT commands         */
		.iInterface         = 0,                        /* iInterface: USBD_IDX_INTERFACE_STR             */
	},

	.func_desc_header = {
		.hdr = {
			.bFunctionLength    = sizeof(USBD_FuncDescHdr),
			.bDescriptorType    = CDC_DESC_TYPE_INTERFACE,
			.bDescriptorSubtype = CDC_DESC_SUB_TYPE_HDR,
		},
		.bcdCDC = 0x0110, /* bcdCDC: spec release number */
	},

	/*PSTN120 Table 3: Call Management Functional Descriptor */
	.func_desc_call_mng = {
		.hdr = {
			.bFunctionLength    = sizeof(USBD_FuncDescCallMng),
			.bDescriptorType    = CDC_DESC_TYPE_INTERFACE,
			.bDescriptorSubtype = CDC_DESC_SUB_TYPE_CALL_MNG,
		},
		.bmCapabilities = 0x00,	/* PSTN120 D0+D1 */
		.bDataInterface = TBD	/* Initialized on pClass->Setup */
	},

	/* PSTN120 Table 4: Abstract Control Management Functional Descriptor */
	.func_desc_acm = {
		.hdr = {
			.bFunctionLength    = sizeof(USBD_FuncDescACM),
			.bDescriptorType    = CDC_DESC_TYPE_INTERFACE,
			.bDescriptorSubtype = CDC_DESC_SUB_TYPE_ACM,
		},
		.bmCapabilities = 0x02,
	},

	.func_desc_union = {
		.hdr = {
			.bFunctionLength    = sizeof(USBD_FuncDescUnion),
			.bDescriptorType    = CDC_DESC_TYPE_INTERFACE,
			.bDescriptorSubtype = CDC_DESC_SUB_TYPE_UNION,
		},
		.bMasterInterface = TBD,		  /* Initialized on pClass->Setup */
		.bSlaveInterface0 = TBD			  /* Initialized on pClass->Setup */
	},

	.cmd_ep = {
		.bLength = sizeof(USBD_EpDesc),      		/* bLength: Endpoint Descriptor size*/
		.bDescriptorType = USB_DESC_TYPE_ENDPOINT, 	/* bDescriptorType:*/
		.bEndpointAddress = TBD,     				/* Initialized on Class->Init() */
		.bmAttributes = 0x03,          				/* bmAttributes: Interrupt endpoint*/
		.wMaxPacketSize = host2usb_u16(CDC_CMD_PACKET_SIZE),		/* wMaxPacketSize: 2 Byte max */
		.bInterval = CDC_BINTERVAL,        			/* bInterval: Polling Interval */
	},

	.interface_desc_data = {
		.bLength = sizeof(USBD_InterfaceDesc),          /* Interface Descriptor size    */
		.bDescriptorType    = USB_DESC_TYPE_INTERFACE,  /* Interface descriptor type    */
		.bInterfaceNumber   = TBD,                      /* Number of Interface            */
		.bAlternateSetting  = 0,                        /* Alternate setting             */
		.bNumEndpoints      = 2,                        /* bNumEndpoints: DataIn/DataOut */
		.bInterfaceClass    = 0x0A,                     /* bInterfaceClass: CDC */
		.bInterfaceSubClass = 0,                        /* bInterfaceSubClass: */
		.nInterfaceProtocol = 0,                        /* bInterfaceProtocol: */
		.iInterface         = 0,                        /* iInterface:  */
	},

	.data_ep_out = {
		.bLength = sizeof(USBD_EpDesc),      		/* bLength: Endpoint Descriptor size*/
		.bDescriptorType = USB_DESC_TYPE_ENDPOINT, 	/* bDescriptorType:*/
		.bEndpointAddress = TBD,     				/* Initialized on Class->Init() */
		.bmAttributes = 0x02,          				/* bmAttributes: Bulk endpoint*/
		.wMaxPacketSize = host2usb_u16(CDC_DATA_MAX_PACKET_SIZE),		/* wMaxPacketSize: 2 Byte max */
		.bInterval = 0,        						/* bInterval: ignore for Bulk transfer */
	},

	.data_ep_in = {
		.bLength = sizeof(USBD_EpDesc),      		/* bLength: Endpoint Descriptor size*/
		.bDescriptorType = USB_DESC_TYPE_ENDPOINT, 	/* bDescriptorType:*/
		.bEndpointAddress = TBD,     				/* Initialized on Class->Init() */
		.bmAttributes = 0x02,          				/* bmAttributes: Bulk endpoint*/
		.wMaxPacketSize = host2usb_u16(CDC_DATA_MAX_PACKET_SIZE),		/* wMaxPacketSize: 2 Byte max */
		.bInterval = 0,        						/* bInterval: ignore for Bulk transfer */
	}
};


static int8_t CDC_Init(USBD_CDC_Handle *hcdc, USBD_Handle *pdev, uint8_t cfg_idx)
{
	cdc_dfi_t *dfi = hcdc->dfi;

	dfi->us_start_rx(dfi);

#if NAVIG
	cdc_uart_dfi_us_rx_start(hcdc);
	cdc_ictrl_dfi_us_rx_start(hcdc);
#endif

	return USBD_OK;
}

/**
  * @brief  DeInitializes the CDC media low layer
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_DeInit(USBD_CDC_Handle *hcdc)
{
	hcdc->dfi->us_stop_rx(hcdc->dfi);

#if NAVIG
    cdc_uart_dfi_us_rx_stop();
    cdc_ictrl_dfi_us_rx_stop();
#endif

  return USBD_OK;
}

/**
  * @brief  Manage the CDC class requests
  * @param  cmd: Command code
  * @param  length: Number of data to be sent (in bytes)
  *
  * In HOST->DEVICE direction data passed using Setup packet + data in EP0
  * In DEVICE->HOST direction data need to be put directly to ????
  */
int8_t CDC_Control(USBD_CDC_Handle *hcdc,
		uint8_t cmd, uint16_t length)
{
	hcdc->dfi->ds_on_control(hcdc->dfi, cmd, hcdc->data, length);
#if NAVIG
	cdc_ictrl_dfi_on_control();
	cdc_uart_dfi_on_control();
#endif

	return (USBD_OK);
}

/**
  * @brief  Data received over USB OUT endpoint are sent over CDC interface
  *         through this function.
  *
  *         @note
  *         This function will issue a NAK packet on any OUT packet received on
  *         USB endpoint until exiting this function. If you exit this function
  *         before transfer is complete on CDC interface (ie. using DMA controller)
  *         it will result in receiving more data while previous ones are still
  *         not sent.
  *
  * @param  buf: Buffer of data to be received
  * @param  len: Number of data received (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */

static int8_t CDC_Receive(USBD_CDC_Handle *hcdc, uint32_t len)
{
	hcdc->dfi->ds_on_rx(hcdc->dfi, len);

#if NAVIG
    cdc_uart_dfi_ds_on_rx(*len);
    cdc_ictrl_dfi_ds_on_rx(*len);
#endif

	return (USBD_OK);
}


void USBD_CDC_Compose_ConfigDesc(
		USBD_CDC_Handle *hcdc, USBD_ConfigDesc *config_desc,
		int ifnum, int epnum)
{
	USBD_CDC_ConfigDesc	*desc = &cdc_config_desc_template;

	/* Update CDC template and put copy into config_desc */
	desc->if_assoc_desc.bFirstInterface = ifnum;
	desc->interface_desc_cmd.bInterfaceNumber = ifnum;
	desc->func_desc_call_mng.bDataInterface = ifnum + 1;
	desc->func_desc_union.bMasterInterface = ifnum;
	desc->func_desc_union.bSlaveInterface0 = ifnum + 1;
	desc->cmd_ep.bEndpointAddress = EP_IN_ADDR(epnum);
	desc->interface_desc_data.bInterfaceNumber = ifnum + 1;
	desc->data_ep_out.bEndpointAddress = EP_OUT_ADDR(epnum + 1);
	desc->data_ep_in.bEndpointAddress = EP_IN_ADDR(epnum + 1);

	hcdc->cfg_desc = USBD_CfgDescAppend(config_desc, (uint8_t*)desc, sizeof(USBD_CDC_ConfigDesc));
}

/**
  * @brief  USBD_CDC_Init
  *         Initialize the CDC interface
  * @param  pdev: device instance
  * @param  hcdc: cdc instance
  * @param  cfgidx: Configuration index
  * @retval status
  */

void USBD_CDC_Init (union intf_dev_handle_u h, USBD_Handle *pdev, uint8_t cfgidx)
{
	USBD_CDC_Handle *hcdc = h.cdc;
	USBD_CDC_ConfigDesc	*desc = hcdc->cfg_desc;
	cdc_dfi_t *dfi = hcdc->dfi;

	hcdc->pdev = pdev;

	hcdc->ifnum_cmd  = desc->interface_desc_cmd.bInterfaceNumber;
	hcdc->epnum_cmd =  EP_IDX(desc->cmd_ep.bEndpointAddress);

	hcdc->ifnum_data = desc->interface_desc_data.bInterfaceNumber;
	hcdc->epnum_data = EP_IDX(desc->data_ep_in.bEndpointAddress);

	/* Open EP IN DATA */
	USBD_LL_OpenEP(pdev,
			EP_IN_ADDR(hcdc->epnum_data),
			USBD_EP_TYPE_BULK,
			CDC_DATA_IN_PACKET_SIZE);

	/* Open EP OUT DATA */
	USBD_LL_OpenEP(pdev,
			EP_OUT_ADDR(hcdc->epnum_data),
			USBD_EP_TYPE_BULK,
			CDC_DATA_OUT_PACKET_SIZE);

	/* Open Command IN EP */
	USBD_LL_OpenEP(pdev,
			EP_IN_ADDR(hcdc->epnum_cmd),
			USBD_EP_TYPE_INTR,
			CDC_CMD_PACKET_SIZE);

    hcdc->TxState = 0U;
    hcdc->RxState = 0U;

    /* Init  physical Interface components */
    CDC_Init(hcdc, pdev, cfgidx);

	/* Prepare Out endpoint to receive next packet */
	USBD_LL_PrepareReceive(pdev,
			EP_OUT_ADDR(hcdc->epnum_data),
			dfi->ds_get_buffer(dfi),
			CDC_DATA_OUT_PACKET_SIZE);
}

/**
  * @brief  USBD_CDC_Init
  *         DeInitialize the CDC layer
  * @param  hcdc: CDC interface instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
void USBD_CDC_DeInit (union intf_dev_handle_u h, uint8_t cfgidx)
{
	USBD_CDC_Handle *hcdc = h.cdc;
	USBD_Handle *pdev = hcdc->pdev;

	if (!pdev) return;

	USBD_LL_CloseEP(pdev, EP_IN_ADDR(hcdc->epnum_data));
	USBD_LL_CloseEP(pdev, EP_OUT_ADDR(hcdc->epnum_data));
	USBD_LL_CloseEP(pdev, EP_IN_ADDR(hcdc->epnum_cmd));

	/* DeInit  physical Interface components */
	CDC_DeInit(hcdc);
}

/**
  * @brief  USBD_CDC_Setup
  *         Handle the CDC specific requests
  * @param  pdev: instance
  * @param  req: usb requests
  * @retval status
  */
uint8_t USBD_CDC_Setup (union intf_dev_handle_u h, enum setup_recp_e recp,
		uint8_t recp_idx, USBD_SetupReq *req)
{
	uint8_t ret;
	USBD_CDC_Handle *hcdc = h.cdc;
	USBD_Handle *pdev = hcdc->pdev;

	uint8_t ifalt = 0U;
	uint16_t status_info = 0U;

	/* Check is this request for us */
	if (recp == RECP_INTERFACE &&
		(recp_idx != hcdc->ifnum_cmd &&
		 recp_idx != hcdc->ifnum_data) ) {
		return USBD_OK;
	}

	if (recp == RECP_ENDPOINT) {
		if ( recp_idx == hcdc->epnum_data ) {
			/* Control Request is for CMD endpoint only */
			return USBD_FAIL;
		}
		if (recp_idx != hcdc->epnum_cmd) {
			return USBD_OK;
		}
	}


	ret = USBD_BUSY;

	switch (req->bmRequest & USB_REQ_TYPE_MASK) {
    /* AV: TODO: Why where is no dev_state check?
     *           Is this request valid in any state? */

	case USB_REQ_TYPE_CLASS :
		if ((req->bmRequest & USB_REQ_DIR_MASK) == USB_REQ_DIR_IN) {
			/* Data Phase Transfer Direction = DEV->HOST */
			/* Data to be processed were received in hcdc.data earlier. */
			/* Process request and put result into hcdc.data  */
			CDC_Control(hcdc, req->bRequest, req->wLength);
			USBD_CtlSendData(pdev, hcdc->data, req->wLength);
		}
		else {
			/* Data Phase Transfer Direction = HOST->DEV */
			/* Prepare EP0 to receive announced amount of data */
			if (req->wLength) {
				hcdc->CmdOpCode = req->bRequest;
				hcdc->CmdLength = (uint8_t)req->wLength;
				USBD_CtlPrepareRx(pdev, hcdc->data, req->wLength);
			}
		}
		break;
		/* EOF USB_REQ_TYPE_CLASS */

	case USB_REQ_TYPE_STANDARD:
		/* AV: TODO: pdev->dev_state state check is common for all STANDARD
		 *           requests and is duplicated for each interface HID/CDC/...,
		 *            while it is independent on state of the specific interface.
		 *            Q: Should be a part of Composite device layer ? */
		switch (req->bRequest) {
		case USB_REQ_GET_STATUS:
			if (pdev->dev_state == USBD_STATE_CONFIGURED) {
				USBD_CtlSendData (pdev, (uint8_t *)(void *)&status_info, 2U);
			}
			else {
				USBD_CtlError (pdev, req);
				ret = USBD_FAIL;
			}
			break;

		case USB_REQ_GET_INTERFACE:
			if (pdev->dev_state == USBD_STATE_CONFIGURED) {
				USBD_CtlSendData (pdev, &ifalt, 1U);
			}
			else {
				USBD_CtlError (pdev, req);
				ret = USBD_FAIL;
			}
			break;

		case USB_REQ_SET_INTERFACE:
			if (pdev->dev_state != USBD_STATE_CONFIGURED) {
				USBD_CtlError (pdev, req);
				ret = USBD_FAIL;
			}
			break;
		default:
			USBD_CtlError (pdev, req);
			ret = USBD_FAIL;
			break;
		}
		break;
		/* EOF USB_REQ_TYPE_STANDARD */

	default:
		USBD_CtlError (pdev, req);
		ret = USBD_FAIL;
		break;
	}

	return ret;
}

/**
  * @brief  USBD_CDC_DataIn
  *         Data sent on non-control IN endpoint
  * @param  pdev: device instance
  * @param  hcdc: CDC handle
  * @retval status
  */
uint8_t USBD_CDC_DataIn (union intf_dev_handle_u h, uint8_t epnum)
{
	USBD_CDC_Handle *hcdc = h.cdc;

	if (epnum != hcdc->epnum_data && epnum != hcdc->epnum_cmd) {
		return USBD_OK;
	}

	if (epnum == hcdc->epnum_cmd) {
		return USBD_FAIL;
	}

	assert_param(hcdc->TxState);

	/* CDC is a byte stream, but not packet based. Thus it doesn't use
	 * multipacket transactions. Also, in order to avoid ZLP exchange
	 * (i.e. reduce traffic) the transmitter never use full length
	 * transactions. Just mark tx state as ready and exit.
	 */
	hcdc->TxState = 0U;

	/* packet consumed */
	return USBD_BUSY;
}

/**
  * @brief  USBD_CDC_DataOut
  *         Data received on non-control Out endpoint
  * @param  pdev: device instance
  * @param  hcdc: CDC handle
  * @retval status
  */
uint8_t USBD_CDC_DataOut (union intf_dev_handle_u h, uint8_t epnum)
{
	USBD_CDC_Handle *hcdc = h.cdc;
	uint32_t rx_length;

	if (epnum != hcdc->epnum_cmd && epnum != hcdc->epnum_data) return USBD_OK;

	if (epnum == hcdc->epnum_cmd) return USBD_FAIL;

	USBD_Handle *pdev = hcdc->pdev;

	/* Get the received data length */
	rx_length = USBD_LL_GetRxDataSize(pdev, hcdc->epnum_data);

	/* USB data will be immediately processed, this allow next USB traffic being
       NAKed till the end of the application Xfer */
    CDC_Receive(hcdc, rx_length);

    return USBD_BUSY;
}

/**
  * @brief  USBD_CDC_EP0_RxReady
  *         Handle EP0 Rx Ready event
  * @param  pdev: device instance
  * @retval status
  */
uint8_t USBD_CDC_EP0_RxReady (union intf_dev_handle_u h)
{
	USBD_CDC_Handle *hcdc = h.cdc;

	if(hcdc->CmdOpCode != 0xFF) {
		CDC_Control(hcdc, hcdc->CmdOpCode, hcdc->CmdLength);
		hcdc->CmdOpCode = 0xFFU;
		return USBD_BUSY;		/* Inform composite layer that received data processed */
	}

	return USBD_OK;
}

/**
  * @brief  USBD_CDC_TransmitPacket
  *         Transmit packet on IN endpoint
  * @param  pdev: device instance
  * @retval status
  */
uint8_t USBD_CDC_TransmitPacket(USBD_CDC_Handle *hcdc,
		uint8_t  *buf, uint16_t len)
{

	if (!hcdc->pdev) {
		/* If USB side not initiated yet then work as a null dev */
		return USBD_OK;
	}

    if (hcdc->TxState == 0U) {
    	/* Mark Tx transfer is in progress */
    	hcdc->TxState = 1U;

    	/* Update the packet total length */
    	hcdc->pdev->ep_in[hcdc->epnum_data].total_length = len;
    	USBD_LL_Transmit(hcdc->pdev, hcdc->epnum_data, buf, len);

    	return USBD_OK;
    }
    else {
    	return USBD_BUSY;
    }
}


/**
  * @brief  USBD_CDC_ReceivePacket prepare OUT Endpoint for reception
  * @param  hcdc: CDC interface instance
  * @retval status
  */
uint8_t USBD_CDC_ReceivePacket(USBD_CDC_Handle *hcdc)
{
	if (hcdc->pdev) {
		cdc_dfi_t *dfi = hcdc->dfi;
		/* Prepare Out endpoint to receive next packet */
		USBD_LL_PrepareReceive(hcdc->pdev,
						 hcdc->epnum_data,
						 dfi->ds_get_buffer(dfi),
						 CDC_DATA_OUT_PACKET_SIZE);
#if NAVIG
		cdc_ictrl_dfi_ds_get_buff();
		cdc_uart_dfi_ds_get_buff();
#endif
		return USBD_OK;
	}

	return USBD_FAIL;
}

void CDC_Register (
		USBD_CDC_Handle *hcdc,
		usbd_intf_t *intf,
		USBD_ConfigDesc *config_desc,
		int *ifnum, int *epnum)
{
	intf->h.cdc = hcdc;
	intf->Init = USBD_CDC_Init;
	intf->DeInit = USBD_CDC_DeInit;
	intf->EP0_RxReady = USBD_CDC_EP0_RxReady;
	intf->Setup = USBD_CDC_Setup;
	intf->DataIn = USBD_CDC_DataIn;
	intf->DataOut = USBD_CDC_DataOut;

	USBD_CDC_Compose_ConfigDesc(hcdc, config_desc, *ifnum, *epnum);
	*ifnum += 2;
	*epnum += 2;
}
