/**
  ******************************************************************************
  * @file           : usbd_conf.c
  * @version        : v2.0_Cube
  * @brief          : This file implements the board support package for the USB device library
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "usbd_def.h"
#include "usbd_core.h"
#include "usb_device.h"

PCD_HandleTypeDef hpcd_USB;
void Error_Handler(void);

static USBD_Status USBD_Get_USB_Status(HAL_StatusTypeDef hal_status);
static void SystemClockConfig_Resume(void);
extern void SystemClock_Config(void);

/*******************************************************************************
                       LL Driver Callbacks (PCD -> USB Device Library)
*******************************************************************************/
/* MSP Init */
void HAL_PCD_MspInit(PCD_HandleTypeDef* pcdHandle)
{
	RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

	if (pcdHandle->Instance == USB) {
		PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB;
		PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_HSI48;
		if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK) {
			Error_Handler();
		}
		/* Peripheral clock enable */
		__HAL_RCC_USB_CLK_ENABLE();

		/* Peripheral interrupt init */
		HAL_NVIC_SetPriority(USB_IRQn, 0, 0);
		HAL_NVIC_EnableIRQ(USB_IRQn);
  }
}

void HAL_PCD_MspDeInit(PCD_HandleTypeDef* pcdHandle)
{
	if(pcdHandle->Instance==USB)
	{
		/* Peripheral clock disable */
		__HAL_RCC_USB_CLK_DISABLE();

		/* Peripheral interrupt Deinit*/
		HAL_NVIC_DisableIRQ(USB_IRQn);
	}
}

/**
  * @brief  Setup stage callback
  * @param  hpcd: PCD handle
  * @retval None
  */
void HAL_PCD_SetupStageCallback(PCD_HandleTypeDef *hpcd)
{
	USBD_LL_SetupStage((USBD_Handle*)hpcd->pData, (uint8_t *)hpcd->Setup);
}

/**
  * @brief  Data Out stage callback.
  * @param  hpcd: PCD handle
  * @param  epnum: Endpoint number
  * @retval None
  */
void HAL_PCD_DataOutStageCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum)
{
	USBD_LL_DataOutStage((USBD_Handle*)hpcd->pData, epnum, hpcd->OUT_ep[epnum].xfer_buff);
}

/**
  * @brief  Data In stage callback.
  * @param  hpcd: PCD handle
  * @param  epnum: Endpoint number
  * @retval None
  */
void HAL_PCD_DataInStageCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum)
{
  USBD_LL_DataInStage((USBD_Handle*)hpcd->pData, epnum, hpcd->IN_ep[epnum].xfer_buff);
}

/**
  * @brief  SOF callback.
  * @param  hpcd: PCD handle
  * @retval None
  */
void HAL_PCD_SOFCallback(PCD_HandleTypeDef *hpcd)
{
	USBD_LL_SOF((USBD_Handle*)hpcd->pData);
}

/**
  * @brief  Reset callback.
  * @param  hpcd: PCD handle
  * @retval None
  */
#if (USE_HAL_PCD_REGISTER_CALLBACKS == 1U)
static void PCD_ResetCallback(PCD_HandleTypeDef *hpcd)
#else
void HAL_PCD_ResetCallback(PCD_HandleTypeDef *hpcd)
#endif /* USE_HAL_PCD_REGISTER_CALLBACKS */
{
	USBD_Speed speed = USBD_SPEED_FULL;

	if (hpcd->Init.speed != PCD_SPEED_FULL) {
		Error_Handler();
	}

	/* Set Speed. */
	USBD_LL_SetSpeed((USBD_Handle*)hpcd->pData, speed);

	/* Reset Device. */
	USBD_LL_Reset((USBD_Handle*)hpcd->pData);
}

/**
  * @brief  Suspend callback.
  * When Low power mode is enabled the debug cannot be used (IAR, Keil doesn't support it)
  * @param  hpcd: PCD handle
  * @retval None
  */
#if (USE_HAL_PCD_REGISTER_CALLBACKS == 1U)
static void PCD_SuspendCallback(PCD_HandleTypeDef *hpcd)
#else
void HAL_PCD_SuspendCallback(PCD_HandleTypeDef *hpcd)
#endif /* USE_HAL_PCD_REGISTER_CALLBACKS */
{
  /* Inform USB library that core enters in suspend Mode. */
  USBD_LL_Suspend((USBD_Handle*)hpcd->pData);
  /* Enter in STOP mode. */
  /* USER CODE BEGIN 2 */
  if (hpcd->Init.low_power_enable)
  {
    /* Set SLEEPDEEP bit and SleepOnExit of Cortex System Control Register. */
    SCB->SCR |= (uint32_t)((uint32_t)(SCB_SCR_SLEEPDEEP_Msk | SCB_SCR_SLEEPONEXIT_Msk));
  }
  /* USER CODE END 2 */
}

/**
  * @brief  Resume callback.
  * When Low power mode is enabled the debug cannot be used (IAR, Keil doesn't support it)
  * @param  hpcd: PCD handle
  * @retval None
  */
#if (USE_HAL_PCD_REGISTER_CALLBACKS == 1U)
static void PCD_ResumeCallback(PCD_HandleTypeDef *hpcd)
#else
void HAL_PCD_ResumeCallback(PCD_HandleTypeDef *hpcd)
#endif /* USE_HAL_PCD_REGISTER_CALLBACKS */
{
  /* USER CODE BEGIN 3 */
  if (hpcd->Init.low_power_enable)
  {
    /* Reset SLEEPDEEP bit of Cortex System Control Register. */
    SCB->SCR &= (uint32_t)~((uint32_t)(SCB_SCR_SLEEPDEEP_Msk | SCB_SCR_SLEEPONEXIT_Msk));
    SystemClockConfig_Resume();
  }
  /* USER CODE END 3 */
  USBD_LL_Resume((USBD_Handle*)hpcd->pData);
}

/**
  * @brief  ISOOUTIncomplete callback.
  * @param  hpcd: PCD handle
  * @param  epnum: Endpoint number
  * @retval None
  */
#if (USE_HAL_PCD_REGISTER_CALLBACKS == 1U)
static void PCD_ISOOUTIncompleteCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum)
#else
void HAL_PCD_ISOOUTIncompleteCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum)
#endif /* USE_HAL_PCD_REGISTER_CALLBACKS */
{
  USBD_LL_IsoOUTIncomplete((USBD_Handle*)hpcd->pData, epnum);
}

/**
  * @brief  ISOINIncomplete callback.
  * @param  hpcd: PCD handle
  * @param  epnum: Endpoint number
  * @retval None
  */
#if (USE_HAL_PCD_REGISTER_CALLBACKS == 1U)
static void PCD_ISOINIncompleteCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum)
#else
void HAL_PCD_ISOINIncompleteCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum)
#endif /* USE_HAL_PCD_REGISTER_CALLBACKS */
{
  USBD_LL_IsoINIncomplete((USBD_Handle*)hpcd->pData, epnum);
}

/**
  * @brief  Connect callback.
  * @param  hpcd: PCD handle
  * @retval None
  */
#if (USE_HAL_PCD_REGISTER_CALLBACKS == 1U)
static void PCD_ConnectCallback(PCD_HandleTypeDef *hpcd)
#else
void HAL_PCD_ConnectCallback(PCD_HandleTypeDef *hpcd)
#endif /* USE_HAL_PCD_REGISTER_CALLBACKS */
{
  USBD_LL_DevConnected((USBD_Handle*)hpcd->pData);
}

/**
  * @brief  Disconnect callback.
  * @param  hpcd: PCD handle
  * @retval None
  */
#if (USE_HAL_PCD_REGISTER_CALLBACKS == 1U)
static void PCD_DisconnectCallback(PCD_HandleTypeDef *hpcd)
#else
void HAL_PCD_DisconnectCallback(PCD_HandleTypeDef *hpcd)
#endif /* USE_HAL_PCD_REGISTER_CALLBACKS */
{
  USBD_LL_DevDisconnected((USBD_Handle*)hpcd->pData);
}

/*******************************************************************************
                       LL Driver Interface (USB Device Library --> PCD)
*******************************************************************************/

/**
  * @brief  Initializes the low level portion of the device driver.
  * @param  pdev: Device handle
  * @retval USBD status
  */
USBD_Status USBD_LL_Init(USBD_Handle *pdev)
{
	/* Link the driver to the stack. */
	hpcd_USB.pData = pdev;
	pdev->pPCDHandle = &hpcd_USB;

	hpcd_USB.Instance = USB;
	hpcd_USB.Init.dev_endpoints = 8;
	hpcd_USB.Init.speed = PCD_SPEED_FULL;
	hpcd_USB.Init.phy_itface = PCD_PHY_EMBEDDED;
	hpcd_USB.Init.low_power_enable = DISABLE;
	hpcd_USB.Init.lpm_enable = DISABLE;
	hpcd_USB.Init.battery_charging_enable = DISABLE;

	if (HAL_PCD_Init(&hpcd_USB) != HAL_OK) {
		Error_Handler( );
	}

	/* Preserve memory for Buffer descriptor table at addr 0 */
	hpcd_USB.pma_map = 0;
	HAL_PCD_PMA_Alloc(&hpcd_USB, 0x40);

	return USBD_OK;
}

/**
  * @brief  De-Initializes the low level portion of the device driver.
  * @param  pdev: Device handle
  * @retval USBD status
  */
USBD_Status USBD_LL_DeInit(USBD_Handle *pdev)
{
  HAL_StatusTypeDef hal_status = HAL_OK;
  USBD_Status usb_status = USBD_OK;

  hal_status = HAL_PCD_DeInit(pdev->pPCDHandle);

  usb_status =  USBD_Get_USB_Status(hal_status);

  return usb_status;
}

USBD_Status USBD_LL_Start(USBD_Handle *pdev)
{
  HAL_StatusTypeDef hal_status = HAL_OK;
  USBD_Status usb_status = USBD_OK;

  hal_status = HAL_PCD_Start(pdev->pPCDHandle);
  usb_status =  USBD_Get_USB_Status(hal_status);

  return usb_status;
}

USBD_Status USBD_LL_Stop(USBD_Handle *pdev)
{
  HAL_StatusTypeDef hal_status = HAL_OK;
  USBD_Status usb_status = USBD_OK;

  hal_status = HAL_PCD_Stop(pdev->pPCDHandle);

  usb_status =  USBD_Get_USB_Status(hal_status);

  return usb_status;
}

/**
  * @brief  Opens an endpoint of the low level driver.
  * @param  pdev: Device handle
  * @param  ep_addr: Endpoint number
  * @param  ep_type: Endpoint type
  * @param  ep_mps: Endpoint max packet size
  * @retval USBD status
  */
USBD_Status USBD_LL_OpenEP(USBD_Handle *pdev, uint8_t ep_addr, uint8_t ep_type, uint16_t ep_mps)
{
	USBD_Endpoint *usbd_ep = EP_IS_IN_ADDR(ep_addr) ?
			&pdev->ep_in[EP_IDX(ep_addr)] :
			&pdev->ep_out[EP_IDX(ep_addr)];

	assert(usbd_ep->is_used == 0);

	usbd_ep->is_used = 1;
	usbd_ep->maxpacket = ep_mps;

	HAL_PCD_EP_Open(pdev->pPCDHandle, ep_addr, ep_mps, ep_type);

	return USBD_OK;
}

/**
  * @brief  Closes an endpoint of the low level driver.
  * @param  pdev: Device handle
  * @param  ep_addr: Endpoint number
  * @retval USBD status
  */
void USBD_LL_CloseEP(USBD_Handle *pdev, uint8_t ep_addr)
{
	USBD_Endpoint *usbd_ep = EP_IS_IN_ADDR(ep_addr) ?
			&pdev->ep_in[EP_IDX(ep_addr)] :
			&pdev->ep_out[EP_IDX(ep_addr)];

	if (!usbd_ep->is_used) return;

	usbd_ep->is_used = 0;
	usbd_ep->maxpacket = 0;

	HAL_PCD_EP_Close(pdev->pPCDHandle, ep_addr);
}

/**
  * @brief  Flushes an endpoint of the Low Level Driver.
  * @param  pdev: Device handle
  * @param  ep_addr: Endpoint number
  * @retval USBD status
  */
USBD_Status USBD_LL_FlushEP(USBD_Handle *pdev, uint8_t ep_addr)
{
  HAL_StatusTypeDef hal_status = HAL_OK;
  USBD_Status usb_status = USBD_OK;

  hal_status = HAL_PCD_EP_Flush(pdev->pPCDHandle, ep_addr);

  usb_status =  USBD_Get_USB_Status(hal_status);

  return usb_status;
}

/**
  * @brief  Sets a Stall condition on an endpoint of the Low Level Driver.
  * @param  pdev: Device handle
  * @param  ep_addr: Endpoint number
  * @retval USBD status
  */
USBD_Status USBD_LL_StallEP(USBD_Handle *pdev, uint8_t ep_addr)
{
  HAL_StatusTypeDef hal_status = HAL_OK;
  USBD_Status usb_status = USBD_OK;

  hal_status = HAL_PCD_EP_SetStall(pdev->pPCDHandle, ep_addr);

  usb_status =  USBD_Get_USB_Status(hal_status);

  return usb_status;
}

/**
  * @brief  Clears a Stall condition on an endpoint of the Low Level Driver.
  * @param  pdev: Device handle
  * @param  ep_addr: Endpoint number
  * @retval USBD status
  */
USBD_Status USBD_LL_ClearStallEP(USBD_Handle *pdev, uint8_t ep_addr)
{
  HAL_StatusTypeDef hal_status = HAL_OK;
  USBD_Status usb_status = USBD_OK;

  hal_status = HAL_PCD_EP_ClrStall(pdev->pPCDHandle, ep_addr);

  usb_status =  USBD_Get_USB_Status(hal_status);

  return usb_status;
}

/**
  * @brief  Returns Stall condition.
  * @param  pdev: Device handle
  * @param  ep_addr: Endpoint number
  * @retval Stall (1: Yes, 0: No)
  */
uint8_t USBD_LL_IsStallEP(USBD_Handle *pdev, uint8_t ep_addr)
{
  PCD_HandleTypeDef *hpcd = pdev->pPCDHandle;

  if((ep_addr & 0x80) == 0x80)
  {
    return hpcd->IN_ep[ep_addr & 0x7F].is_stall;
  }
  else
  {
    return hpcd->OUT_ep[ep_addr & 0x7F].is_stall;
  }
}

/**
  * @brief  Assigns a USB address to the device.
  * @param  pdev: Device handle
  * @param  dev_addr: Device address
  * @retval USBD status
  */
USBD_Status USBD_LL_SetUSBAddress(USBD_Handle *pdev, uint8_t dev_addr)
{
  HAL_StatusTypeDef hal_status = HAL_OK;
  USBD_Status usb_status = USBD_OK;

  hal_status = HAL_PCD_SetAddress(pdev->pPCDHandle, dev_addr);

  usb_status =  USBD_Get_USB_Status(hal_status);

  return usb_status;
}

/**
  * @brief  Transmits data over an endpoint.
  * @param  pdev: Device handle
  * @param  ep_addr: Endpoint number
  * @param  pbuf: Pointer to data to be sent
  * @param  size: Data size
  * @retval USBD status
  */
USBD_Status USBD_LL_Transmit(USBD_Handle *pdev, uint8_t ep_addr, uint8_t *pbuf, uint32_t size)
{
  HAL_StatusTypeDef hal_status = HAL_OK;
  USBD_Status usb_status = USBD_OK;

  hal_status = HAL_PCD_EP_Transmit(pdev->pPCDHandle, ep_addr, pbuf, size);
  usb_status = USBD_Get_USB_Status(hal_status);

  return usb_status;
}

/**
  * @brief  Prepares an endpoint for reception.
  * @param  pdev: Device handle
  * @param  ep_addr: Endpoint number
  * @param  pbuf: Pointer to data to be received
  * @param  size: Data size
  * @retval USBD status
  */
USBD_Status USBD_LL_PrepareReceive(USBD_Handle *pdev, uint8_t ep_addr, uint8_t *pbuf, uint32_t size)
{
  HAL_StatusTypeDef hal_status = HAL_OK;
  USBD_Status usb_status = USBD_OK;

  hal_status = HAL_PCD_EP_Receive(pdev->pPCDHandle, ep_addr, pbuf, size);

  usb_status =  USBD_Get_USB_Status(hal_status);

  return usb_status;
}

/**
  * @brief  Returns the last transfered packet size.
  * @param  pdev: Device handle
  * @param  ep_addr: Endpoint number
  * @retval Recived Data Size
  */
uint32_t USBD_LL_GetRxDataSize(USBD_Handle *pdev, uint8_t ep_addr)
{
  return HAL_PCD_EP_GetRxCount(pdev->pPCDHandle, ep_addr);
}

/**
  * @brief  Delays routine for the USB device library.
  * @param  Delay: Delay in ms
  * @retval None
  */
void USBD_LL_Delay(uint32_t Delay)
{
  HAL_Delay(Delay);
}

/* USER CODE BEGIN 5 */
/**
  * @brief  Configures system clock after wake-up from USB resume callBack:
  *         enable HSI, PLL and select PLL as system clock source.
  * @retval None
  */
static void SystemClockConfig_Resume(void)
{
  SystemClock_Config();
}
/* USER CODE END 5 */

/**
  * @brief  Retuns the USB status depending on the HAL status:
  * @param  hal_status: HAL status
  * @retval USB status
  */
USBD_Status USBD_Get_USB_Status(HAL_StatusTypeDef hal_status)
{
  USBD_Status usb_status = USBD_OK;

  switch (hal_status)
  {
    case HAL_OK :
      usb_status = USBD_OK;
    break;
    case HAL_ERROR :
      usb_status = USBD_FAIL;
    break;
    case HAL_BUSY :
      usb_status = USBD_BUSY;
    break;
    case HAL_TIMEOUT :
      usb_status = USBD_FAIL;
    break;
    default :
      usb_status = USBD_FAIL;
    break;
  }
  return usb_status;
}
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
