/***************************************************************************//**
* \file cy_ble_common.h
* \version 3.50
*
* \brief
*  This file contains the source code for the API of the PSoC 6 BLE Middleware.
*
********************************************************************************
* \copyright
* Copyright 2017-2020, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#if !defined(CY_BLE_COMMON_H)
#define CY_BLE_COMMON_H

#include "cy_ble.h"

#if defined(CY_IP_MXBLESS)

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*******************************************************************************
* Function Prototypes
*******************************************************************************/

/**
 * \addtogroup group_ble_common_api_functions
 * \{
 */
cy_en_ble_api_result_t Cy_BLE_Init(const cy_stc_ble_config_t *config);
cy_en_ble_api_result_t Cy_BLE_Enable(void);
cy_en_ble_api_result_t Cy_BLE_Disable(void);
void Cy_BLE_EnableLowPowerMode(void);
/** \} group_ble_common_api_functions */

void Cy_BLE_ServiceInit(void);


/******************************************************************************
* Mapping Functions stack size optimization  (internal usage only)
*******************************************************************************/

void Cy_BLE_HAL_EccHeapInit(uint8_t *heapMem, uint8_t numOfConn);
void Cy_BLE_HAL_EccHeapDeInit(void);
void Cy_BLE_HAL_SmpScCmacComplete(uint8_t param);
void Cy_BLE_HAL_EccPointMult(uint8_t param);
uint16_t Cy_BLE_HAL_EccGetFeatureHeapReq(uint8_t numOfConn);
cy_en_ble_api_result_t Cy_BLE_HAL_EccGenerateSecurityKeypair(uint8_t p_publicKey[], uint8_t p_privateKey[],
                                                             uint8_t randomData[]);
cy_en_ble_api_result_t Cy_BLE_HAL_EccGenerateDHKey(const uint8_t p_publicKey[], const uint8_t p_privateKey[],
                                                   uint8_t p_secret[], uint8_t ci);
cy_en_ble_api_result_t Cy_BLE_HAL_EccValidPublicKey(const uint8_t p_publicKey[]);
cy_en_ble_api_result_t Cy_BLE_HAL_PairingLocalPublicKeyHandler(void *param);
cy_en_ble_api_result_t Cy_BLE_HAL_PairingRemoteKeyHandler(void *param);
cy_en_ble_api_result_t Cy_BLE_HAL_PairingDhkeyHandler(void *param);
cy_en_ble_api_result_t Cy_BLE_HAL_PairingDhkeyCheckHandler(void *param);
cy_en_ble_api_result_t Cy_BLE_HAL_PairingKeypressNotificationHandler(void *param);
cy_en_ble_api_result_t Cy_BLE_HAL_PairingRandHandler(void *param);
cy_en_ble_api_result_t Cy_BLE_HAL_PairingConfirmHandler(void *param);
cy_en_ble_api_result_t Cy_BLE_HAL_PairingLrConfirmingHandler(void *param);
cy_en_ble_api_result_t Cy_BLE_HAL_PairingScTbxDhkeyGenerateComplete(void* param);
cy_en_ble_api_result_t Cy_BLE_HAL_PairingScTbxGenerateLocalP256PublicKey(uint8_t param);
cy_en_ble_api_result_t Cy_BLE_HAL_SeSmpScUserPasskeyHandler(void *param1, void *param2);
cy_en_ble_api_result_t Cy_BLE_HAL_PairingScTbxLocalPubkeyGenerateComplete(void *param);
cy_en_ble_api_result_t Cy_BLE_HAL_PairingScTbxGenerateDHkey(void  *param1,
                void  *param2, uint8_t param3);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* CY_IP_MXBLESS */
#endif /* CY_BLE_COMMON_H */

/* [] END OF FILE */
