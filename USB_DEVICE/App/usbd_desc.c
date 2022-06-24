#include "usbd_core.h"
#include "usbd_desc.h"
#include "usbd_conf.h"

#define DEVICE_ID1 (UID_BASE)
#define DEVICE_ID2 (UID_BASE + 0x4)
#define DEVICE_ID3 (UID_BASE + 0x8)

#define USBD_VID                        1155
#define USBD_LANGID_STRING              1033
#define USBD_PID_FS                     22322

#define USBD_MANUFACTURER_STRING        "AV"

#define USBD_PRODUCT_STRING_FS          "2xCDC HID Composite device"
#define USBD_CONFIGURATION_STRING_FS    "Default"
#define USBD_INTERFACE_STRING_FS        "Interface X3"

static void Get_SerialNum(void);
static void IntToUnicode(uint32_t value, uint8_t * pbuf, uint8_t len);

uint8_t * USBD_FS_DeviceDescriptor(USBD_Speed speed, uint16_t *length);
uint8_t * USBD_FS_LangIDStrDescriptor(USBD_Speed speed, uint16_t *length);
uint8_t * USBD_FS_ManufacturerStrDescriptor(USBD_Speed speed, uint16_t *length);
uint8_t * USBD_FS_ProductStrDescriptor(USBD_Speed speed, uint16_t *length);
uint8_t * USBD_FS_SerialStrDescriptor(USBD_Speed speed, uint16_t *length);
uint8_t * USBD_FS_ConfigStrDescriptor(USBD_Speed speed, uint16_t *length);
uint8_t * USBD_FS_InterfaceStrDescriptor(USBD_Speed speed, uint16_t *length);

USBD_Descriptors FS_Desc =
{
    .GetDeviceDescriptor           = USBD_FS_DeviceDescriptor,
    .GetLangIDStrDescriptor        = USBD_FS_LangIDStrDescriptor,
    .GetManufacturerStrDescriptor  = USBD_FS_ManufacturerStrDescriptor,
    .GetProductStrDescriptor       = USBD_FS_ProductStrDescriptor,
    .GetSerialStrDescriptor        = USBD_FS_SerialStrDescriptor,
    .GetConfigurationStrDescriptor = USBD_FS_ConfigStrDescriptor,
    .GetInterfaceStrDescriptor     = USBD_FS_InterfaceStrDescriptor
};

USBD_DeviceDesc USBD_FS_DeviceDesc = {
    .bLength             = sizeof(USBD_DeviceDesc),
    .bDescriptorType     = USB_DESC_TYPE_DEVICE,
    .bcdUSB              = 0x0200,						/* USB 2.0*/
    .bDeviceClass        = 0xEF,						/* Miscellaneous */
    .bDeviceSubClass     = 0x02,
    .bDeviceProtocol     = 0x01,						/* Interface Association Descriptor */
    .bMaxPacketSize0     = USB_MAX_EP0_SIZE,
    .idVendor            = host2usb_u16(USBD_VID),
    .idProduct           = host2usb_u16(USBD_PID_FS),
    .bcdDevice           = 0x0001,
    .iManufacturer       = USBD_IDX_MFC_STR,
    .iProduct            = USBD_IDX_PRODUCT_STR,
    .iSerialNumber       = USBD_IDX_SERIAL_STR,
    .bNumConfigurations  = USBD_MAX_NUM_CONFIGURATION
};

/* USB language identifier descriptor. */
uint8_t USBD_LangIDDesc[USB_LEN_LANGID_STR_DESC] =
{
     USB_LEN_LANGID_STR_DESC,
     USB_DESC_TYPE_STRING,
     LOBYTE(USBD_LANGID_STRING),
     HIBYTE(USBD_LANGID_STRING)
};

uint8_t USBD_StrDesc[USBD_MAX_STR_DESC_SIZ];

#define  USB_SIZ_STRING_SERIAL       0x1A

uint8_t USBD_StringSerial[USB_SIZ_STRING_SERIAL] = {
	USB_SIZ_STRING_SERIAL,
	USB_DESC_TYPE_STRING,
};

/**
  * @brief  Return the device descriptor
  * @param  speed : Current device speed
  * @param  length : Pointer to data length variable
  * @retval Pointer to descriptor buffer
  */
uint8_t * USBD_FS_DeviceDescriptor(USBD_Speed speed, uint16_t *length)
{
  UNUSED(speed);
  *length = sizeof(USBD_DeviceDesc);
  return (void*)&USBD_FS_DeviceDesc;
}

/**
  * @brief  Return the LangID string descriptor
  * @param  speed : Current device speed
  * @param  length : Pointer to data length variable
  * @retval Pointer to descriptor buffer
  */
uint8_t * USBD_FS_LangIDStrDescriptor(USBD_Speed speed, uint16_t *length)
{
  UNUSED(speed);
  *length = sizeof(USBD_LangIDDesc);
  return USBD_LangIDDesc;
}

/**
  * @brief  Return the product string descriptor
  * @param  speed : Current device speed
  * @param  length : Pointer to data length variable
  * @retval Pointer to descriptor buffer
  */
uint8_t * USBD_FS_ProductStrDescriptor(USBD_Speed speed, uint16_t *length)
{
  if(speed == 0)
  {
    USBD_GetString((uint8_t *)USBD_PRODUCT_STRING_FS, USBD_StrDesc, length);
  }
  else
  {
    USBD_GetString((uint8_t *)USBD_PRODUCT_STRING_FS, USBD_StrDesc, length);
  }
  return USBD_StrDesc;
}

/**
  * @brief  Return the manufacturer string descriptor
  * @param  speed : Current device speed
  * @param  length : Pointer to data length variable
  * @retval Pointer to descriptor buffer
  */
uint8_t * USBD_FS_ManufacturerStrDescriptor(USBD_Speed speed, uint16_t *length)
{
  UNUSED(speed);
  USBD_GetString((uint8_t *)USBD_MANUFACTURER_STRING, USBD_StrDesc, length);
  return USBD_StrDesc;
}

/**
  * @brief  Return the serial number string descriptor
  * @param  speed : Current device speed
  * @param  length : Pointer to data length variable
  * @retval Pointer to descriptor buffer
  */
uint8_t * USBD_FS_SerialStrDescriptor(USBD_Speed speed, uint16_t *length)
{
  UNUSED(speed);
  *length = USB_SIZ_STRING_SERIAL;

  /* Update the serial number string descriptor with the MCU unique ID */
  Get_SerialNum();

  return (uint8_t *) USBD_StringSerial;
}

/**
  * @brief  Return the configuration string descriptor
  * @param  speed : Current device speed
  * @param  length : Pointer to data length variable
  * @retval Pointer to descriptor buffer
  */
uint8_t * USBD_FS_ConfigStrDescriptor(USBD_Speed speed, uint16_t *length)
{
  if(speed == USBD_SPEED_HIGH)
  {
    USBD_GetString((uint8_t *)USBD_CONFIGURATION_STRING_FS, USBD_StrDesc, length);
  }
  else
  {
    USBD_GetString((uint8_t *)USBD_CONFIGURATION_STRING_FS, USBD_StrDesc, length);
  }
  return USBD_StrDesc;
}

/**
  * @brief  Return the interface string descriptor
  * @param  speed : Current device speed
  * @param  length : Pointer to data length variable
  * @retval Pointer to descriptor buffer
  */
uint8_t * USBD_FS_InterfaceStrDescriptor(USBD_Speed speed, uint16_t *length)
{
  if(speed == 0)
  {
    USBD_GetString((uint8_t *)USBD_INTERFACE_STRING_FS, USBD_StrDesc, length);
  }
  else
  {
    USBD_GetString((uint8_t *)USBD_INTERFACE_STRING_FS, USBD_StrDesc, length);
  }
  return USBD_StrDesc;
}

/**
  * @brief  Create the serial number string descriptor
  * @param  None
  * @retval None
  */
static void Get_SerialNum(void)
{
  uint32_t deviceserial0, deviceserial1, deviceserial2;

  deviceserial0 = *(uint32_t *) DEVICE_ID1;
  deviceserial1 = *(uint32_t *) DEVICE_ID2;
  deviceserial2 = *(uint32_t *) DEVICE_ID3;

  deviceserial0 += deviceserial2;

  if (deviceserial0 != 0)
  {
    IntToUnicode(deviceserial0, &USBD_StringSerial[2], 8);
    IntToUnicode(deviceserial1, &USBD_StringSerial[18], 4);
  }
}

/**
  * @brief  Convert Hex 32Bits value into char
  * @param  value: value to convert
  * @param  pbuf: pointer to the buffer
  * @param  len: buffer length
  * @retval None
  */
static void IntToUnicode(uint32_t value, uint8_t * pbuf, uint8_t len)
{
  uint8_t idx = 0;

  for (idx = 0; idx < len; idx++)
  {
    if (((value >> 28)) < 0xA)
    {
      pbuf[2 * idx] = (value >> 28) + '0';
    }
    else
    {
      pbuf[2 * idx] = (value >> 28) + 'A' - 10;
    }

    value = value << 4;

    pbuf[2 * idx + 1] = 0;
  }
}
