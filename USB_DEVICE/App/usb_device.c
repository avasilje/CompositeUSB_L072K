#include "usbd_core.h"
#include "usbd_desc.h"
#include "usbd_composite.h"
#include "usb_device.h"

USBD_Handle hUsbDevice;

USBD_CDC_Handle g_cdc0;
USBD_CDC_Handle g_cdc1;

union _USBD_ConfigDescExt USBD_ConfigDescExt = {
	.config_desc = {
		.bLength = sizeof(USBD_ConfigDesc),
		.bDescriptorType = USB_DESC_TYPE_CONFIGURATION,
		.wTotalLength = TBD,
		.bNumInterfaces = TBD,
		.bConfigurationValue = TBD,
		.iConfiguration = 2,
		.bmAttributes = 0xC0,
		.bMaxPower = 50
	}
};

void MX_USB_DEVICE_Init(void)
{
	USBD_Handle *pdev = &hUsbDevice;

    /* Init Device Library, add supported class and start the library. */
	if (USBD_Init(pdev, &FS_Desc, DEVICE_FS) != USBD_OK) {
		Error_Handler();
	}

	pdev->pClass = &USBD_Class_Composite;

	pdev->config_desc = &USBD_ConfigDescExt.config_desc;
	pdev->config_desc->bLength = sizeof(USBD_ConfigDesc);
	pdev->config_desc->wTotalLength = host2usb_u16(sizeof(USBD_ConfigDesc));
	pdev->config_desc->bConfigurationValue = 1;

	{ /* Link interfaces into composite class */
		int ep_in_use = 1;
		int if_in_use = 0;
		HID_Register(&g_hid0, &pdev->intf[0], pdev->config_desc, &if_in_use, &ep_in_use);
		CDC_Register(&g_cdc0, &pdev->intf[1], pdev->config_desc, &if_in_use, &ep_in_use);
		CDC_Register(&g_cdc1, &pdev->intf[2], pdev->config_desc, &if_in_use, &ep_in_use);

		pdev->config_desc->bNumInterfaces = if_in_use;
	}

	/*
	 * Enable interrupts and enable D+ pull-up resistor
	 * to notify host that we've attached to the USB bus
	 */
	if (USBD_Start(&hUsbDevice) != USBD_OK) {
		Error_Handler();
	}

}

