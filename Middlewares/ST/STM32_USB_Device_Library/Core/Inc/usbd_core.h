/**
  ******************************************************************************
  * @file    usbd_core.h
  * @author  MCD Application Team
  * @brief   Header file for usbd_core.c file
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2015 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                      www.st.com/SLA0044
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USBD_CORE_H
#define __USBD_CORE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "usbd_def.h"
#include "usbd_ioreq.h"
#include "usbd_ctlreq.h"

/** @addtogroup STM32_USB_DEVICE_LIBRARY
  * @{
  */

/** @defgroup USBD_CORE
  * @brief This file is the Header file for usbd_core.c file
  * @{
  */


/** @defgroup USBD_CORE_Exported_Defines
  * @{
  */
#ifndef USBD_DEBUG_LEVEL
#define USBD_DEBUG_LEVEL           0U
#endif /* USBD_DEBUG_LEVEL */
/**
  * @}
  */


/** @defgroup USBD_CORE_Exported_TypesDefinitions
  * @{
  */


/**
  * @}
  */



/** @defgroup USBD_CORE_Exported_Macros
  * @{
  */

/**
  * @}
  */

/** @defgroup USBD_CORE_Exported_Variables
  * @{
  */
#define USBD_SOF          USBD_LL_SOF
/**
  * @}
  */

/** @defgroup USBD_CORE_Exported_FunctionsPrototype
  * @{
  */
USBD_Status USBD_Init(USBD_Handle *pdev, USBD_Descriptors *pdesc, uint8_t id);
USBD_Status USBD_DeInit(USBD_Handle *pdev);
USBD_Status USBD_Start(USBD_Handle *pdev);
USBD_Status USBD_Stop(USBD_Handle *pdev);
USBD_Status USBD_RegisterClass(USBD_Handle *pdev, USBD_Class *pclass);

USBD_Status USBD_RunTestMode(USBD_Handle *pdev);
USBD_Status USBD_SetClassConfig(USBD_Handle *pdev, uint8_t cfgidx);
USBD_Status USBD_ClrClassConfig(USBD_Handle *pdev, uint8_t cfgidx);

USBD_Status USBD_LL_SetupStage(USBD_Handle *pdev, uint8_t *psetup);
USBD_Status USBD_LL_DataOutStage(USBD_Handle *pdev, uint8_t epnum, uint8_t *pdata);
USBD_Status USBD_LL_DataInStage(USBD_Handle *pdev, uint8_t epnum, uint8_t *pdata);

USBD_Status USBD_LL_Reset(USBD_Handle *pdev);
USBD_Status USBD_LL_SetSpeed(USBD_Handle *pdev, USBD_Speed speed);
USBD_Status USBD_LL_Suspend(USBD_Handle *pdev);
USBD_Status USBD_LL_Resume(USBD_Handle *pdev);

USBD_Status USBD_LL_SOF(USBD_Handle  *pdev);
USBD_Status USBD_LL_IsoINIncomplete(USBD_Handle *pdev, uint8_t epnum);
USBD_Status USBD_LL_IsoOUTIncomplete(USBD_Handle *pdev, uint8_t epnum);

USBD_Status USBD_LL_DevConnected(USBD_Handle *pdev);
USBD_Status USBD_LL_DevDisconnected(USBD_Handle *pdev);

/* USBD Low Level Driver */
USBD_Status USBD_LL_Init(USBD_Handle *pdev);
USBD_Status USBD_LL_DeInit(USBD_Handle *pdev);
USBD_Status USBD_LL_Start(USBD_Handle *pdev);
USBD_Status USBD_LL_Stop(USBD_Handle *pdev);

USBD_Status USBD_LL_OpenEP(USBD_Handle *pdev, uint8_t ep_addr,
                                  uint8_t ep_type, uint16_t ep_mps);

void USBD_LL_CloseEP(USBD_Handle *pdev, uint8_t ep_addr);
USBD_Status USBD_LL_FlushEP(USBD_Handle *pdev, uint8_t ep_addr);
USBD_Status USBD_LL_StallEP(USBD_Handle *pdev, uint8_t ep_addr);
USBD_Status USBD_LL_ClearStallEP(USBD_Handle *pdev, uint8_t ep_addr);
USBD_Status USBD_LL_SetUSBAddress(USBD_Handle *pdev, uint8_t dev_addr);

USBD_Status USBD_LL_Transmit(USBD_Handle *pdev, uint8_t ep_addr,
                                    uint8_t *pbuf, uint32_t size);

USBD_Status USBD_LL_PrepareReceive(USBD_Handle *pdev, uint8_t ep_addr,
                                          uint8_t *pbuf, uint32_t size);

uint8_t USBD_LL_IsStallEP(USBD_Handle *pdev, uint8_t ep_addr);
uint32_t USBD_LL_GetRxDataSize(USBD_Handle *pdev, uint8_t  ep_addr);

void  USBD_LL_Delay(uint32_t Delay);

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* __USBD_CORE_H */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/



