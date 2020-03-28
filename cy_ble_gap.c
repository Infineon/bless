/***************************************************************************//**
* \file cy_ble_gap.c
* \version 3.40
*
* \brief
*  This file contains the source code for the GAP API of the PSoC 6 BLE Middleware.
*
********************************************************************************
* \copyright
* Copyright 2017-2020, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "cy_ble.h"
#include "cy_ble_hal_pvt.h"

#if defined(CY_IP_MXBLESS)

/*******************************************************************************
* Global Variables
*******************************************************************************/

/* BLE state */
cy_en_ble_state_t      cy_ble_state;

/* BLE Advertising state */
cy_en_ble_adv_state_t  cy_ble_advState;

/* BLE Scanning state  */
cy_en_ble_scan_state_t cy_ble_scanState;

/* BLE Connection state */
cy_en_ble_conn_state_t cy_ble_connState[CY_BLE_MAX_SUPPORTED_CONN_COUNT];

/** Variable is initialized after connection with peer device,
 *  after CY_BLE_EVT_GATT_CONNECT_IND event, and could be used by application code
 *  to send data to peer device */
cy_stc_ble_conn_handle_t cy_ble_connHandle[CY_BLE_MAX_SUPPORTED_CONN_COUNT];

/* Connection parameter  */
cy_stc_ble_gapc_conn_info_t cy_ble_connectionParameters;

/**
 * Connecting timeout is set to 30 seconds in Cy_BLE_Init function.
 * Not zero value starts timer in Cy_BLE_GAPC_ConnectDevice().
 */
cy_stc_ble_timer_info_t cy_ble_connectingTimeout;

/* Active Advertising configuration */
uint8_t cy_ble_advIndex;

/* Active Scanning configuration */
uint8_t cy_ble_scanIndex;

/**
 * Store information that device connected as role:
 * CY_BLE_GAP_LL_ROLE_SLAVE or CY_BLE_GAP_LL_ROLE_MASTER.
 */
uint8_t cy_ble_devConnRole[CY_BLE_MAX_SUPPORTED_CONN_COUNT];

/* Stored information is device paired */
bool cy_ble_pairStatus[CY_BLE_MAX_SUPPORTED_CONN_COUNT];

/**
 * Bonding type setting of peer device, CY_BLE_GAP_BONDING_NONE or
 * CY_BLE_GAP_BONDING. It is initialized after pairing with peer device and
 * used for cy_ble_pendingFlashWrite variable setting.
 */
uint8_t cy_ble_peerBonding[CY_BLE_MAX_SUPPORTED_CONN_COUNT];

/* Store advertising interval type (fast or slow) with timings entered. Need to process advertising timeout */
uint8_t cy_ble_advertisingIntervalType;

/* Store scanning interval type (fast or slow) with timings entered. Need to process scanning timeout */
uint8_t cy_ble_scanningIntervalType;

/* The pointer to the global BLE GAP server config structure */
const cy_stc_ble_gaps_config_t *cy_ble_gapsConfigPtr = NULL;

/* The pointer to the global BLE GAP client config structure */
const cy_stc_ble_gapc_config_t *cy_ble_gapcConfigPtr = NULL;

/* Stored BD handle to clear CCCD values by Cy_BLE_StoreBondingData() */
uint8_t cy_ble_pendingFlashClearCccdHandle = 0u;

/**
 * \addtogroup group_ble_common_api_global_variables
 * \{
 */
/**
 * Variable is used to detect status of pending write to flash operation for
 * BLE Stack data and CCCD.
 */
 uint8_t cy_ble_pendingFlashWrite;
/** \} group_ble_common_api_global_variables */

#if CY_BLE_LIB_HOST_CORE

/*******************************************************************************
* Private Function Prototypes
*******************************************************************************/
static void Cy_BLE_ChangeAdLocalName(const char8 name[], uint8_t dest);


/******************************************************************************
* Function Name: Cy_BLE_GAPS_Init
***************************************************************************//**
*
*  This function initializes server of the GAP service.
*
*  \param config: Configuration structure for the GAP.
*
*  \return
*  \ref cy_en_ble_api_result_t : Return value indicates whether the function succeeded or
*  failed. The following are possible error codes.
*
*   Error Codes                             | Description
*   ------------                            | -----------
*   CY_BLE_SUCCESS                          | The function completed successfully.
*   CY_BLE_ERROR_INVALID_PARAMETER          | On specifying NULL as input parameter.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_GAPS_Init(const cy_stc_ble_gaps_config_t *config)
{
    cy_en_ble_api_result_t apiResult = CY_BLE_SUCCESS;

    if(config == NULL)
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else
    {
        /* Registers a pointer to config structure */
        cy_ble_gapsConfigPtr = config;
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_GAPC_Init
***************************************************************************//**
*
*  This function initializes client of the GAP service.
*
*  \param config: Configuration structure for the GAP.
*
*  \return
*  \ref cy_en_ble_api_result_t : Return value indicates whether the function succeeded or
*  failed. The following are possible error codes.
*
*   Error Codes                             | Description
*   ------------                            | -----------
*   CY_BLE_SUCCESS                          | The function completed successfully.
*   CY_BLE_ERROR_INVALID_PARAMETER          | On specifying NULL as input parameter.
*   CY_BLE_ERROR_MEMORY_ALLOCATION_FAILED   | Buffer overflow in the registration callback.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_GAPC_Init(const cy_stc_ble_gapc_config_t *config)
{
    cy_en_ble_api_result_t apiResult = CY_BLE_SUCCESS;

    if((config == NULL) || ((cy_ble_configPtr->params->gattRole & CY_BLE_GATT_CLIENT) == 0u))
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else
    {
        uint32_t idx;

        /* Registers a pointer to config structure */
        cy_ble_gapcConfigPtr = config;

        for(idx = 0u; idx < cy_ble_configPtr->params->maxClientCount; idx++)
        {
            uint32 servCnt    = cy_ble_configPtr->context->discServiCount;
            uint32 gapServIdx = cy_ble_gapcConfigPtr->serviceDiscIdx;

            /* Check service range before clearing to support partial discovery */
            if(cy_ble_configPtr->context->serverInfo[(idx * servCnt) + gapServIdx].range.startHandle ==
                CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
            {
                (void)memset(&cy_ble_gapcConfigPtr->attrInfo[idx], 0, sizeof(cy_stc_ble_gapc_t));

                /* Initialize UUID  */
                cy_ble_configPtr->context->serverInfo[(idx * servCnt) + gapServIdx].uuid = CY_BLE_UUID_GAP_SERVICE;
            }
        }
    }
    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_StoreAppData
***************************************************************************//**
*
*  This function is used to back up application-specific data into the flash.
*
*  This function works in two Write modes: blocking or non-blocking:
*  * Blocking (#CY_BLE_STORE_DATA_MODE_BLOCKING) - The function writes
*    all data from srcBuff and returns #CY_BLE_SUCCESS when finished.
*  <br>
*  * Non-blocking (#CY_BLE_STORE_DATA_MODE_NON_BLOCKING) - The function initiates
*    Write data and returns #CY_BLE_INFO_FLASH_WRITE_IN_PROGRESS when the Write
*    is in progress, or #CY_BLE_SUCCESS when finished. In order to write complete
*    data, the application needs to continue calling this function with
*    the same input parameter (param), until this function returns CY_BLE_SUCCESS.
*    The application must not change the input parameter's structure and content
*    until this function returns CY_BLE_SUCCESS.
*
*  \param param: The parameter of type \ref cy_stc_ble_app_flash_param_t.
*   * param->srcBuff:   The source buffer.
*   * param->destAddr:  The destination address. The destination buffer size
*                       should be greater than or equal to the source buffer.
*   * param->buffLen:   The length of srcData (in bytes).
*   * param->writeMode: Blocking or non-blocking Write mode.
*
*  param->destAddr - An array address to be defined in the
*  application and aligned to the size of the device's flash row.
*
*  An example of a declaration of such an array (100 bytes):
*   - CY_ALIGN(CY_FLASH_SIZEOF_ROW) const uint8_t appBuff[100u] = {0u}; <br>
*  <br>
*  CY_ALIGN() - Aligns the array to the size of the device's flash row.
*  Even if the length of srcData is not a multiple of the flash row size,
*  Cy_BLE_StoreAppData() forms whole rows, and programs them in sequence.
*
*  The approximate time to write one row - 16ms.
*
*  The following code snippet shows the usage of Cy_BLE_StoreAppData()
*  \snippet ble/snippet/main.c BLE Common API: Cy_BLE_StoreAppData()
*
* \return
* \ref cy_en_ble_api_result_t : Return value indicates whether the function
*      succeeded or failed. Following are the possible error codes.
*
*   Error codes                              | Description
*   ------------                             | -----------
*   CY_BLE_SUCCESS                           | On successful operation.
*   CY_BLE_INFO_FLASH_WRITE_IN_PROGRESS      | Write operation is in progress.
*   CY_BLE_ERROR_INVALID_PARAMETER           | An invalid input parameter.
*   CY_BLE_ERROR_FLASH_WRITE                 | An error in the flash Write.
*   CY_BLE_ERROR_FLASH_WRITE_NOT_PERMITED    | Flash operation is not permitted (see Note)
*
* \note: Flash operation is not permitted with protection context (PC)
*        value > 0 and core voltage 0.9V, because of a preproduction
*        hardware limitation.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_StoreAppData(const cy_stc_ble_app_flash_param_t *param)
{
    static cy_en_ble_api_result_t storeState = CY_BLE_SUCCESS;
    static uint32_t tLength = 0u;


    if ((param == NULL) || (param->writeMode > CY_BLE_STORE_DATA_MODE_NON_BLOCKING))
    {
        storeState = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else
    {
        uint8_t *tSrcPtr   = (uint8_t *) param->srcBuff;
        uint32_t tOffset   = (uint32) param->destAddr - CY_FLASH_BASE;  /* Get offset */
        uint32_t blockSize;
        bool exitFlag = false;

        switch (storeState)
        {
            case CY_BLE_SUCCESS:
            case CY_BLE_ERROR_INVALID_PARAMETER:
            case CY_BLE_ERROR_FLASH_WRITE:
                tLength = param->buffLen;
                break;

            case CY_BLE_INFO_FLASH_WRITE_IN_PROGRESS:
                if (tLength == 0u)
                {
                    storeState = CY_BLE_SUCCESS;
                    exitFlag = true;
                }
                else
                {
                    /* Restore offset and srcPtr according to length */
                    if(tSrcPtr != NULL)
                    {
                        tSrcPtr += param->buffLen - tLength;
                    }
                    tOffset += param->buffLen - tLength;
                }
                break;

           default:
                break;
        }

        while((tLength != 0u) && (exitFlag == false))
        {
            /* Split srcBuff into parts of flash row size (512 bytes) */
            blockSize = (tOffset >> 9) << 9;                            /* Get current row size regarding to address */
            blockSize = CY_FLASH_SIZEOF_ROW - (tOffset - blockSize);    /* Calculate final length of the block which
                                                                           we can write to row */
            blockSize = (blockSize > tLength ) ? tLength : blockSize;

            /* Write 512 byte block */
            storeState = Cy_BLE_HAL_NvramWrite(tSrcPtr, (uint8_t *)(tOffset + CY_FLASH_BASE),
                                               blockSize, CY_BLE_FLASH_NON_BLOCKING_MODE);

            if((storeState & CY_PDL_STATUS_ERROR) != 0u)
            {
                exitFlag = true;
            }

            if(tSrcPtr != NULL)
            {
                tSrcPtr += blockSize;
            }
            tOffset += blockSize;
            tLength -= blockSize;

            /* For CY_BLE_STORE_DATA_MODE_NON_BLOCKING mode write one row per call */
            if(param->writeMode == CY_BLE_STORE_DATA_MODE_NON_BLOCKING)
            {
                if (storeState == CY_BLE_SUCCESS)
                {
                    storeState = CY_BLE_INFO_FLASH_WRITE_IN_PROGRESS;
                }
                exitFlag = true;
            }

            /* Cy_BLE_ProcessEvents() allows BLE Stack to process pending events */
            Cy_BLE_ProcessEvents();
        }
    }
    return(storeState);
}


/******************************************************************************
* Function Name: Cy_BLE_StoreBondingData
***************************************************************************//**
*
*  This function writes the new bonding data from RAM to the dedicated flash
*  location as defined by the PSoC 6 BLE Middleware. It performs a data comparison
*  between RAM and flash before writing to flash. If there is no change between
*  RAM and flash data, then no write is performed. It writes one flash row
*  in one call.
*  The application should keep calling this function until it returns
*  #CY_BLE_SUCCESS. This function is available only when Bonding requirement
*  is selected in Security settings.
*
* \return
* \ref cy_en_ble_api_result_t : Return value indicates whether the function
*      succeeded or failed. Following are the possible error codes.
*
*   Error Codes                           | Description
*   ----------------------------------    | ------------------------------------
*   CY_BLE_SUCCESS                        | On successful operation
*   CY_BLE_INFO_FLASH_WRITE_IN_PROGRESS   | Writing in progress
*   CY_BLE_ERROR_FLASH_WRITE              | Error in flash write
*   CY_BLE_ERROR_FLASH_WRITE_NOT_PERMITED | Flash operation is not permitted (see Note)
*
* \note: Flash operation is not permitted with protection context (PC)
*        value > 0 and core voltage 0.9V, because of a preproduction
*        hardware limitation.
*
* \globalvars
*  The \ref cy_ble_pendingFlashWrite variable is used to detect status
*  of pending write to flash operation for BLE Stack data and CCCD.
*  This function automatically clears pending bits after the write operation
*  completes.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_StoreBondingData(void)
{
    cy_en_ble_api_result_t apiResult = CY_BLE_SUCCESS;

    if(cy_ble_configPtr != NULL) 
    {
        /* Store BLE Stack data */
        if((cy_ble_pendingFlashWrite & CY_BLE_PENDING_STACK_FLASH_WRITE_BIT) != 0u)
        {
            cy_stc_ble_stack_flash_param_t stackFlashParam =
            {
                .bitField = CY_BLE_PERSISTENT_BOND_LIST_BITMASK |
                            CY_BLE_PERSISTENT_RPA_LIST_BITMASK |
                            CY_BLE_PERSISTENT_WHITELIST_BITMASK
            };

            apiResult = Cy_BLE_StoreStackData(&stackFlashParam);
            if(apiResult == CY_BLE_SUCCESS)
            {
                cy_ble_pendingFlashWrite &= (uint8_t) ~CY_BLE_PENDING_STACK_FLASH_WRITE_BIT;

                /* Change apiResult if there are more pending data to store */
                if((cy_ble_pendingFlashWrite & CY_BLE_PENDING_CCCD_FLASH_WRITE_BIT) != 0u)
                {
                    apiResult = CY_BLE_INFO_FLASH_WRITE_IN_PROGRESS;
                }
            }
        }

        /* Store CCCD values */
        if( (apiResult == CY_BLE_SUCCESS) &&  (cy_ble_configPtr->flashStorage->cccdCount != 0u) &&
            ((cy_ble_pendingFlashWrite & CY_BLE_PENDING_CCCD_FLASH_WRITE_BIT) != 0u) )
        {
            uint32_t i;
            cy_stc_ble_app_flash_param_t appFlashParam;

            /* Update CCCD values from RAM for active connections supporting bonding */
            for(i = 0u; i < cy_ble_configPtr->stackParam->maxConnCount; i++)
            {
                if((Cy_BLE_GetConnectionState(cy_ble_connHandle[i]) >= CY_BLE_CONN_STATE_CONNECTED) &&
                (cy_ble_peerBonding[cy_ble_connHandle[i].attId] == CY_BLE_GAP_BONDING))
                {
                    uint32_t cccdBlockSize = (uint32_t) cy_ble_configPtr->flashStorage->cccdCount + CY_BLE_CCCD_CRC_BYTE;
                    uint32_t cccdBlockCrcOffset = cccdBlockSize - CY_BLE_CCCD_CRC_BYTE;
                    uint32_t cccdBlockOffsetRam = cy_ble_connHandle[i].attId * cccdBlockSize;
                    uint32_t cccdBlockOffsetFlash = cy_ble_connHandle[i].bdHandle * cccdBlockSize;
                    uint8_t calcCrc;

                    appFlashParam.buffLen = (uint32_t) cy_ble_configPtr->flashStorage->cccdCount + CY_BLE_CCCD_CRC_BYTE;
                    appFlashParam.srcBuff = &cy_ble_configPtr->flashStorage->cccdRamPtr[cccdBlockOffsetRam];
                    appFlashParam.destAddr = &cy_ble_configPtr->flashStorage->cccdFlashPtr[cccdBlockOffsetFlash];
                    appFlashParam.writeMode = CY_BLE_STORE_DATA_MODE_BLOCKING;

                    /* Calculate CRC for CCCD block */
                    calcCrc = Cy_BLE_HAL_CalcCRC8(&cy_ble_configPtr->flashStorage->cccdRamPtr[cccdBlockOffsetRam],
                                                cy_ble_configPtr->flashStorage->cccdCount);

                    /* Store new CRC value */
                    cy_ble_configPtr->flashStorage->cccdRamPtr[cccdBlockOffsetRam  + cccdBlockCrcOffset] = calcCrc;

                    apiResult = Cy_BLE_StoreAppData(&appFlashParam);
                    if(apiResult != CY_BLE_SUCCESS)
                    {
                        break;
                    }
                }
            }
            if(apiResult == CY_BLE_SUCCESS)
            {
                cy_ble_pendingFlashWrite &= (uint8_t) ~CY_BLE_PENDING_CCCD_FLASH_WRITE_BIT;
            }
        }

        /* Clear requested CCCD values */
        if( (apiResult == CY_BLE_SUCCESS) && (cy_ble_configPtr->flashStorage->cccdCount != 0u) &&
            ((cy_ble_pendingFlashWrite & CY_BLE_PENDING_CCCD_FLASH_CLEAR_MASK) != 0u) )
        {
            if((cy_ble_pendingFlashWrite & CY_BLE_PENDING_CCCD_FLASH_CLEAR_ALL_BIT) != 0u)
            {
                /* Remove CCCD values for all bonded devices */
                uint32_t maxBond = (cy_ble_configPtr->stackParam->maxBondedDevListSize + cy_ble_configPtr->stackParam->maxConnCount);
                uint32_t cccdBlockSizeAll = (cy_ble_configPtr->flashStorage->cccdCount + CY_BLE_CCCD_CRC_BYTE) * maxBond;

                cy_stc_ble_app_flash_param_t appFlashParam;
                appFlashParam.srcBuff  = NULL;
                appFlashParam.destAddr = (uint8_t*)cy_ble_configPtr->flashStorage->cccdFlashPtr;
                appFlashParam.buffLen  = cccdBlockSizeAll;
                appFlashParam.writeMode = CY_BLE_STORE_DATA_MODE_BLOCKING;

                apiResult = Cy_BLE_StoreAppData(&appFlashParam);
                if(apiResult == CY_BLE_SUCCESS)
                {
                    cy_ble_pendingFlashWrite &= (uint8_t) ~CY_BLE_PENDING_CCCD_FLASH_CLEAR_ALL_BIT;
                }
            }
            else /* Remove CCCD values for particular device */
            {
                uint32_t cccdBlockSize = cy_ble_configPtr->flashStorage->cccdCount + CY_BLE_CCCD_CRC_BYTE;

                cy_stc_ble_app_flash_param_t appFlashParam;
                appFlashParam.srcBuff  = NULL;
                appFlashParam.destAddr = (uint8_t*)&cy_ble_configPtr->flashStorage->
                                        cccdFlashPtr[cy_ble_pendingFlashClearCccdHandle * cccdBlockSize];
                appFlashParam.buffLen  = cccdBlockSize;
                appFlashParam.writeMode = CY_BLE_STORE_DATA_MODE_BLOCKING;

                apiResult = Cy_BLE_StoreAppData(&appFlashParam);
                if(apiResult == CY_BLE_SUCCESS)
                {
                    cy_ble_pendingFlashWrite &= (uint8_t) ~CY_BLE_PENDING_CCCD_FLASH_CLEAR_BIT;
                }
            }
        }

        /* Flash operation is not allowed */
        if (apiResult == CY_BLE_ERROR_FLASH_WRITE_NOT_PERMITED)
        {
            cy_ble_pendingFlashWrite = 0u;
        }
    }
    else
    {
        apiResult = CY_BLE_ERROR_INVALID_OPERATION;
    }
    
    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_GAP_RemoveBondedDevice
***************************************************************************//**
*
*  This function removes the bonding information of the device including CCCD
*  values.
*
*  This function is available only when Bonding requirement is selected in
*  Security settings.
*
*  \param bdAddr: Pointer to peer device address,
*                   of type \ref cy_stc_ble_gap_bd_addr_t.
*
*  \return
*  \ref cy_en_ble_api_result_t : Return value indicates whether the function
*   succeeded or failed. The following are possible error codes.
*
*   Error Codes                       | Description
*   --------------------------------- | ------------------------------------
*   CY_BLE_SUCCESS                    | On successful operation.
*   CY_BLE_ERROR_INVALID_PARAMETER    | On specifying NULL as input parameter. for 'bdAddr'.
*   CY_BLE_ERROR_INVALID_OPERATION    | Whitelist is already in use or there is pending write to flash operation.
*   CY_BLE_ERROR_NO_DEVICE_ENTITY     | Device does not exist in the bond list.
*
*  \globalvars
*   The bdHandle is set in \ref cy_ble_pendingFlashWrite variable to indicate that
*   data should be stored to flash by Cy_BLE_StoreBondingData() afterwards.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_GAP_RemoveBondedDevice(cy_stc_ble_gap_bd_addr_t* bdAddr)
{
    cy_en_ble_api_result_t apiResult = CY_BLE_SUCCESS;
    cy_stc_ble_gap_peer_addr_info_t peerAddrInfoParam;

    if(bdAddr == NULL)
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else if( (cy_ble_configPtr != NULL) && (cy_ble_pendingFlashWrite == 0u) )
    {
        if(cy_ble_configPtr->flashStorage->cccdCount != 0u)
        {
            /* Request to clear CCCD values which will be done by Cy_BLE_StoreBondingData() */
            cy_stc_ble_gap_bd_addr_t invalidBdAddr = { { 0u, 0u, 0u, 0u, 0u, 0u }, 0u };

            if(memcmp(((uint8_t*)&(bdAddr->bdAddr)), ((uint8_t*)&(invalidBdAddr.bdAddr)), CY_BLE_GAP_BD_ADDR_SIZE) == 0u)
            {
                /* Request to remove all bonded devices by Cy_BLE_StoreBondingData() */
                cy_ble_pendingFlashWrite |= CY_BLE_PENDING_CCCD_FLASH_CLEAR_ALL_BIT;
            }
            else
            {
                /* Get the BD handle from Address */
                peerAddrInfoParam.bdAddr = *bdAddr;
                apiResult = Cy_BLE_GAP_GetPeerBdHandle(&peerAddrInfoParam);
                if(apiResult == CY_BLE_SUCCESS)
                {
                    /* Store BD handle to clear CCCD values by Cy_BLE_StoreBondingData() */
                    cy_ble_pendingFlashWrite |= CY_BLE_PENDING_CCCD_FLASH_CLEAR_BIT;
                    cy_ble_pendingFlashClearCccdHandle = peerAddrInfoParam.bdHandle;
                }
            }
        }

        if(apiResult == CY_BLE_SUCCESS)
        {
            apiResult = Cy_BLE_GAP_RemoveDeviceFromBondList(bdAddr);
        }
    }
    else
    {
        apiResult = CY_BLE_ERROR_INVALID_OPERATION;
    }

    return(apiResult);
}


/******************************************************************************
 *  Function Name: Cy_BLE_GAPP_StartAdvertisement
 ***************************************************************************//**
 *
 *  This function is used to start the advertisement using the advertisement data
 *  set in the BLE Component customizer's GUI indicated by the advertisingParamIndex.
 *  After invoking this function, the device will be available for connection by
 *  the devices configured for GAP central role. It is only included if the
 *  device is configured for GAP Peripheral or GAP Peripheral + Central role.
 *
 *  On start of advertisement, GAP Peripheral receives the
 *  #CY_BLE_EVT_GAPP_ADVERTISEMENT_START_STOP event. The following events are
 *  possible on invoking this function:
 *
 *  * #CY_BLE_EVT_GAP_DEVICE_CONNECTED - If the device connects to a GAP Central and
 *     Link Layer Privacy is disabled.
 *  * #CY_BLE_EVT_GAP_ENHANCE_CONN_COMPLETE - If the device connects to a GAP Central and
 *     Link Layer Privacy is enabled.
 *  * #CY_BLE_EVT_TIMEOUT: If no device in GAP Central mode connects to this
 *                       device within the specified timeout limit. BLE Stack
 *                       automatically initiate stop advertising when Slow
 *                       advertising was initiated, or starts Slow advertising
 *                       after Fast advertising timeout occur.
 *  * #CY_BLE_EVT_GAPP_ADVERTISEMENT_START_STOP: If device started or stopped
 *                       advertising. Use Cy_BLE_GetAdvertisementState() to
 *                       determine the state. Sequential advertising could be
 *                       started when #CY_BLE_ADV_STATE_STOPPED state is returned.
 *
 *  \param advertisingIntervalType: Fast or slow advertising interval with timings
 *                                  entered in Advertising settings section of the
 *                                  BT Configurator.
 *  * CY_BLE_ADVERTISING_FAST   0x00u
 *  * CY_BLE_ADVERTISING_SLOW   0x01u
 *  * CY_BLE_ADVERTISING_CUSTOM 0x02u
 *
 *  \param advertisingParamIndex: The index of the peripheral and broadcast
 *                                configuration in BT Configurator. For example:
 *  * CY_BLE_PERIPHERAL_CONFIGURATION_0_INDEX     0x00
 *  * CY_BLE_BROADCASTER_CONFIGURATION_0_INDEX    0x01
 *
 *  \return
 *  \ref cy_en_ble_api_result_t : Return value indicates whether the function succeeded or
 *  failed. The following are possible error codes.
 *
 *   Error Codes                       |  Description
 *   --------------------------------- |  --------------------------------
 *   CY_BLE_SUCCESS                    |  On successful operation.
 *   CY_BLE_ERROR_INVALID_PARAMETER    |  On passing an invalid parameter.
 *   CY_BLE_ERROR_INVALID_STATE        |  On calling this function not in Stopped state.
 *   CY_BLE_ERROR_INVALID_OPERATION    |  The operation is not permitted due to connection
 *                                     |  limit exceeded.
 *
 * \note
 *  #CY_BLE_EVT_GAPP_ADVERTISEMENT_START_STOP event is generated after calling
 *  Cy_BLE_GAPP_StartAdvertisement() and Cy_BLE_GAPP_StopAdvertisement() functions.
 *  Application should keep track of which function call resulted into this event
 *
 *******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_GAPP_StartAdvertisement(uint8_t advertisingIntervalType,
                                                      uint8_t advertisingParamIndex)
{
    cy_en_ble_api_result_t apiResult;
    if(cy_ble_configPtr == NULL)
    {
        apiResult = CY_BLE_ERROR_INVALID_OPERATION;
    }
    else if((advertisingIntervalType > CY_BLE_ADVERTISING_CUSTOM) ||
       (advertisingParamIndex >= cy_ble_configPtr->params->gappConfCount))
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else if( ((cy_ble_configPtr->params->gapRole & CY_BLE_GAP_PERIPHERAL) != 0u) &&
             (Cy_BLE_GetNumOfActiveConn() == cy_ble_configPtr->stackParam->maxConnCount) &&
             ((cy_ble_configPtr->discoveryModeInfo[advertisingParamIndex].advParam->advType <
                CY_BLE_GAPP_SCANNABLE_UNDIRECTED_ADV) ||
              (cy_ble_configPtr->discoveryModeInfo[advertisingParamIndex].advParam->advType >
                CY_BLE_GAPP_NON_CONNECTABLE_UNDIRECTED_ADV)) )
    {
        apiResult = CY_BLE_ERROR_INVALID_OPERATION;
    }
    else if(Cy_BLE_GetAdvertisementState() == CY_BLE_ADV_STATE_STOPPED)
    {
        cy_ble_advIndex = advertisingParamIndex;
        if(advertisingIntervalType == CY_BLE_ADVERTISING_FAST)
        {
            cy_ble_configPtr->discoveryModeInfo[advertisingParamIndex].advTo =
                cy_ble_configPtr->gappAdvParams[advertisingParamIndex].fastAdvTimeOut;
            cy_ble_configPtr->discoveryModeInfo[advertisingParamIndex].advParam->advIntvMin =
                cy_ble_configPtr->gappAdvParams[advertisingParamIndex].fastAdvIntervalMin;
            cy_ble_configPtr->discoveryModeInfo[advertisingParamIndex].advParam->advIntvMax =
                cy_ble_configPtr->gappAdvParams[advertisingParamIndex].fastAdvIntervalMax;
        }
        else if(advertisingIntervalType == CY_BLE_ADVERTISING_SLOW)
        {
            cy_ble_configPtr->discoveryModeInfo[advertisingParamIndex].advTo =
                cy_ble_configPtr->gappAdvParams[advertisingParamIndex].slowAdvTimeOut;
            cy_ble_configPtr->discoveryModeInfo[advertisingParamIndex].advParam->advIntvMin =
                cy_ble_configPtr->gappAdvParams[advertisingParamIndex].slowAdvIntervalMin;
            cy_ble_configPtr->discoveryModeInfo[advertisingParamIndex].advParam->advIntvMax =
                cy_ble_configPtr->gappAdvParams[advertisingParamIndex].slowAdvIntervalMax;
        }
        else /* Do not update advertising intervals */
        {
        }
        cy_ble_advertisingIntervalType = advertisingIntervalType;
        apiResult = Cy_BLE_GAPP_EnterDiscoveryMode(&cy_ble_configPtr->discoveryModeInfo[cy_ble_advIndex]);

        if(apiResult == CY_BLE_SUCCESS)
        {
            Cy_BLE_SetAdvertisementState(CY_BLE_ADV_STATE_ADV_INITIATED);
        }
    }
    else
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_GAPP_StopAdvertisement
***************************************************************************//**
*
*   This function can be used to exit from discovery mode. After the execution
*   of this function, there will no longer be any advertisements. On stopping
*   advertising, GAP Peripheral receives #CY_BLE_EVT_GAPP_ADVERTISEMENT_START_STOP
*   event.
*
*   The following event occurs on invoking this function:
*   * #CY_BLE_EVT_GAPP_ADVERTISEMENT_START_STOP
*
*
*  \return
*  \ref cy_en_ble_api_result_t : Return value indicates whether the function succeeded or
*  failed. The following are possible error codes.
*
*   Error Codes                           | Description
*   ------------                          | -----------
*   CY_BLE_SUCCESS                        | On successful operation.
*   CY_BLE_ERROR_MEMORY_ALLOCATION_FAILED | Memory allocation failed.
*   CY_BLE_ERROR_INVALID_STATE            | On calling this function not in Advertising state.
*
* \note
*   #CY_BLE_EVT_GAPP_ADVERTISEMENT_START_STOP event is generated after calling
*   Cy_BLE_GAPP_StartAdvertisement() and Cy_BLE_GAPP_StopAdvertisement() functions.
*   Application should keep track of which function call resulted into this event.
*
*******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_GAPP_StopAdvertisement(void)
{
    cy_en_ble_api_result_t apiResult;

    if(Cy_BLE_GetAdvertisementState() != CY_BLE_ADV_STATE_STOPPED)
    {
        apiResult = Cy_BLE_GAPP_ExitDiscoveryMode();
        if(apiResult == CY_BLE_SUCCESS)
        {
            Cy_BLE_SetAdvertisementState(CY_BLE_ADV_STATE_STOP_INITIATED);
        }
    }
    else
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_ChangeAdDeviceAddress
***************************************************************************//**
*
*  This function is used to set the Bluetooth device address into the
*  advertisement or scan response data structures.
*
*  \param bdAddr: Bluetooth Device address. The variable is of type #cy_stc_ble_gap_bd_addr_t.
*  \param dest:  0 - selects advertisement structure, not zero value selects scan
*                response structure.
*
*******************************************************************************/
void Cy_BLE_ChangeAdDeviceAddress(const cy_stc_ble_gap_bd_addr_t* bdAddr,
                                  uint8_t dest)
{

    uint32_t fFlag;
    uint32_t byteCounter;
    uint32_t maxLength;
    uint32_t advIndex = 0u;
    uint8_t *destBuffer;

    if(cy_ble_configPtr != NULL)
    {
        do
        {
            fFlag = 0u;
            byteCounter = 0u;

            if(dest == 0u)      /* Destination - advertising structure */
            {
                destBuffer = cy_ble_configPtr->discoveryModeInfo[advIndex].advData->advData;
                maxLength = cy_ble_configPtr->discoveryModeInfo[advIndex].advData->advDataLen;
            }
            else                /* Destination - scan response structure */
            {
                destBuffer = cy_ble_configPtr->discoveryModeInfo[advIndex].scanRspData->scanRspData;
                maxLength = cy_ble_configPtr->discoveryModeInfo[advIndex].scanRspData->scanRspDataLen;
            }

            while((byteCounter < maxLength) && (fFlag == 0u))
            {
                uint32_t adLength = destBuffer[byteCounter];

                if(adLength != 0u)
                {
                    /* Increment byte counter so it can point to AD type */
                    byteCounter++;

                    if(destBuffer[byteCounter] == CY_BLE_ADV_DEVICE_ADDR)
                    {
                        /* Start of the device address type was found. Set flag and exit the loop. */
                        fFlag = 1u;
                    }
                    else
                    {
                        byteCounter += adLength;
                    }
                }
                else
                {
                    /* The end of advertisement data structure was encountered though exit the loop. */
                    break;
                }
            }

            if(fFlag != 0u)
            {
                uint32_t i;

                /* Increment byte counter so it can point to Device address */
                byteCounter++;

                /* Update Device Address type */
                destBuffer[byteCounter] = bdAddr->type;

                for(i = 0u; i < CY_BLE_GAP_BD_ADDR_SIZE; i++)
                {
                    destBuffer[byteCounter + i + 1u] = bdAddr->bdAddr[i];
                }
            }
            advIndex++;
        }
        while(advIndex < cy_ble_configPtr->params->gappConfCount);
    }
}


/******************************************************************************
* Function Name: Cy_BLE_GAPC_StartScan
***************************************************************************//**
*
*   This function is used for discovering GAP peripheral devices that are
*   available for connection. It performs the scanning routine using the
*   parameters entered in the Component's customizer indicated by
*   scanParamIndex parameter.
*
*   As soon as the discovery operation starts, #CY_BLE_EVT_GAPC_SCAN_START_STOP
*   event is generated. The #CY_BLE_EVT_GAPC_SCAN_PROGRESS_RESULT event is
*   generated when a GAP peripheral device is located. There are three discovery
*   procedures can be selected in the customizer's GUI:
*
*   * Observation procedure: A device performing the observer role receives only
*                            advertisement data from devices irrespective of
*                            their discoverable mode settings. Advertisement
*                            data received is provided by the event,
*                            #CY_BLE_EVT_GAPC_SCAN_PROGRESS_RESULT. This procedure
*                            requires the scanType sub parameter to be passive
*                            scanning.
*
*   * Discovery procedure: A device performing the discovery procedure receives
*                          the advertisement data and scan response data from
*                          devices in both limited discoverable mode and the
*                          general discoverable mode. Received data is provided
*                          by the event, #CY_BLE_EVT_GAPC_SCAN_PROGRESS_RESULT.
*                          This procedure requires the scanType sub-parameter
*                          to be active scanning.
*
*   Every Advertisement / Scan response packet is received in a new event,
*   #CY_BLE_EVT_GAPC_SCAN_PROGRESS_RESULT.
*   If 'scanTo' sub-parameter is a non-zero value, then upon commencement of
*   discovery procedure and elapsed time = 'scanTo', #CY_BLE_EVT_TIMEOUT event
*   is generated with the event parameter indicating #CY_BLE_GAP_SCAN_TO.
*   Possible generated events are:
*   * #CY_BLE_EVT_GAPC_SCAN_START_STOP: If a device started or stopped scanning.
*                                     Use Cy_BLE_GetScanState() to determine the
*                                     state. Sequential scanning could be
*                                     started when #CY_BLE_ADV_STATE_STOPPED
*                                     state is returned.
*   * #CY_BLE_EVT_GAPC_SCAN_PROGRESS_RESULT
*   * #CY_BLE_EVT_TIMEOUT (CY_BLE_GAP_SCAN_TO)
*
*   \param scanningIntervalType:  Fast or slow scanning interval with
*       timings entered in Scan settings section of the customizer.
*       * CY_BLE_SCANNING_FAST   0x00u
*       * CY_BLE_SCANNING_SLOW   0x01u
*       * CY_BLE_SCANNING_CUSTOM 0x02u
*
*   \param scanParamIndex:  The index of the central and scan configuration
*                           in customizer.
*
*   \return
*   \ref cy_en_ble_api_result_t : Return value indicates whether the function succeeded or
*   failed. The following are possible error codes.
*
*   Error Codes                        | Description
*   ---------------------------------- | -----------------------------------
*   CY_BLE_SUCCESS                     | On successful operation.
*   CY_BLE_ERROR_INVALID_PARAMETER     | On passing an invalid parameter.
*   CY_BLE_ERROR_INVALID_STATE         | On calling this function not in Stopped state.
*   CY_BLE_ERROR_INVALID_OPERATION     |  
*******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_GAPC_StartScan(uint8_t scanningIntervalType,
                                             uint8_t scanParamIndex)
{
    cy_en_ble_api_result_t apiResult;

    if(scanningIntervalType > CY_BLE_SCANNING_CUSTOM)
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else if((cy_ble_configPtr != NULL) && (Cy_BLE_GetScanState() == CY_BLE_SCAN_STATE_STOPPED))
    {
        cy_ble_scanIndex = scanParamIndex;
        if(scanningIntervalType == CY_BLE_SCANNING_FAST)
        {
            cy_ble_configPtr->discoveryInfo[scanParamIndex].scanTo =
                cy_ble_configPtr->gapcScanParams[scanParamIndex].fastScanTimeOut;
            cy_ble_configPtr->discoveryInfo[scanParamIndex].scanIntv =
                cy_ble_configPtr->gapcScanParams[scanParamIndex].fastScanInterval;
            cy_ble_configPtr->discoveryInfo[scanParamIndex].scanWindow =
                cy_ble_configPtr->gapcScanParams[scanParamIndex].fastScanWindow;
        }
        else if(scanningIntervalType == CY_BLE_SCANNING_SLOW)
        {
            cy_ble_configPtr->discoveryInfo[scanParamIndex].scanTo =
                cy_ble_configPtr->gapcScanParams[scanParamIndex].slowScanTimeOut;
            cy_ble_configPtr->discoveryInfo[scanParamIndex].scanIntv =
                cy_ble_configPtr->gapcScanParams[scanParamIndex].slowScanInterval;
            cy_ble_configPtr->discoveryInfo[scanParamIndex].scanWindow =
                cy_ble_configPtr->gapcScanParams[scanParamIndex].slowScanWindow;
        }
        else /* Do not update scanning intervals */
        {
        }

        cy_ble_scanningIntervalType = scanningIntervalType;
        apiResult = Cy_BLE_GAPC_StartDiscovery(&cy_ble_configPtr->discoveryInfo[scanParamIndex]);

        if(apiResult == CY_BLE_SUCCESS)
        {
            Cy_BLE_SetScanState(CY_BLE_SCAN_STATE_SCAN_INITIATED);
        }
    }
    else
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_GAPC_StopScan
***************************************************************************//**
*
*   This function used to stop the discovery of devices. On stopping discovery
*   operation, #CY_BLE_EVT_GAPC_SCAN_START_STOP event is generated. Application
*   layer needs to keep track of the function call made before receiving this
*   event to associate this event with either the start or stop discovery
*   function.
*
*   Possible events generated are:
*    * #CY_BLE_EVT_GAPC_SCAN_START_STOP
*
*  \return
*  \ref cy_en_ble_api_result_t : Return value indicates whether the function succeeded or
*  failed. The following are possible error codes.
*
*   Error Codes                           | Description
*   ------------------------------------- | -----------------------------------
*   CY_BLE_SUCCESS                        | On successful operation.
*   CY_BLE_ERROR_MEMORY_ALLOCATION_FAILED | Memory allocation failed.
*   CY_BLE_ERROR_INVALID_STATE            | On calling this function not in Scanning state.
*
*******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_GAPC_StopScan(void)
{
    cy_en_ble_api_result_t apiResult;

    if(Cy_BLE_GetScanState() == CY_BLE_SCAN_STATE_SCANNING)
    {
        apiResult = Cy_BLE_GAPC_StopDiscovery();
        if(apiResult == CY_BLE_SUCCESS)
        {
            Cy_BLE_SetScanState(CY_BLE_SCAN_STATE_STOP_INITIATED);
        }
    }
    else
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_GAPC_ConnectDevice
***************************************************************************//**
*
*  This function is used to send a connection request to the remote device with
*  the connection parameters set in the BLE Component customizer. This function
*  needs to be called only once after the target device is discovered by
*  Cy_BLE_GAPC_StartScan() and further scanning has stopped. Scanning is
*  successfully stopped on invoking Cy_BLE_GAPC_StopScan() and then receiving the
*  event #CY_BLE_EVT_GAPC_SCAN_START_STOP with sub-parameter 'success' = 0x01u.
*
*  On successful connection, the following events are generated at the GAP
*  Central device (as well as the GAP Peripheral device), in the following order.
*  * #CY_BLE_EVT_GATT_CONNECT_IND
*  * #CY_BLE_EVT_GAP_DEVICE_CONNECTED - If the device connects to a GAP Central and
*    Link Layer Privacy is disabled.
*  * #CY_BLE_EVT_GAP_ENHANCE_CONN_COMPLETE - If the device connects to a GAP Central and
*    Link Layer Privacy is enabled.
*  * #CY_BLE_EVT_GAP_DEVICE_CONNECTED
*
*  A procedure is considered to have timed out if a connection response packet is
*  not received within time set by cy_ble_connectingTimeout global variable
*  (30 seconds by default). #CY_BLE_EVT_TIMEOUT event with #CY_BLE_GENERIC_APP_TO parameter
*  will indicate about connection procedure timeout. Connection will automatically
*  be canceled and state will be changed to #CY_BLE_STATE_ON.
*
*  \param address: The device address of the remote device to connect to.
*  \param centralParamIndex: The index of the central configuration in customizer.
*    For example:
*  * CY_BLE_CENTRAL_CONFIGURATION_0_INDEX    0x00
*  * CY_BLE_CENTRAL_CONFIGURATION_1_INDEX    0x01
*
* \return
* \ref cy_en_ble_api_result_t : Return value indicates whether the function succeeded or
*  failed. The following are possible error codes.
*
*   Error Codes                       | Description
*   ------------                      | -----------
*   CY_BLE_SUCCESS                    | On successful operation.
*   CY_BLE_ERROR_INVALID_PARAMETER    | On passing an invalid parameter.
*   CY_BLE_ERROR_INVALID_STATE        | On calling this function not in Disconnected state.
*   CY_BLE_ERROR_INVALID_OPERATION    | The operation is not permitted due to connection limit exceeded.
*
*   Note: Please refer the description of Cy_BLE_GAPC_InitConnection() for recommended
*   Connection Interval values.
*
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_GAPC_ConnectDevice(const cy_stc_ble_gap_bd_addr_t * address,
                                                 uint8_t centralParamIndex)
{
    cy_en_ble_adv_state_t advState = Cy_BLE_GetAdvertisementState();
    cy_en_ble_api_result_t apiResult;
    uint32_t i;

    if(address == NULL)
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else if(Cy_BLE_IsPeerConnected((uint8_t *)address->bdAddr) == true)
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }
    else if((cy_ble_configPtr == NULL) || 
            (Cy_BLE_GetNumOfActiveConn() == cy_ble_configPtr->stackParam->maxConnCount))
    {
        apiResult = CY_BLE_ERROR_INVALID_OPERATION;
    }
    else if( ((cy_ble_configPtr->params->gapRole & CY_BLE_GAP_PERIPHERAL) != 0u) &&
             ((cy_ble_configPtr->stackParam->maxConnCount - Cy_BLE_GetNumOfActiveConn()) == 1u) &&
             (advState == CY_BLE_ADV_STATE_ADVERTISING) &&
             ((cy_ble_configPtr->discoveryModeInfo[cy_ble_advIndex].advParam->advType <
                CY_BLE_GAPP_SCANNABLE_UNDIRECTED_ADV) ||
              (cy_ble_configPtr->discoveryModeInfo[cy_ble_advIndex].advParam->advType >
                CY_BLE_GAPP_NON_CONNECTABLE_UNDIRECTED_ADV)) )
    {
        apiResult = CY_BLE_ERROR_INVALID_OPERATION;
    }
    else
    {
        /* Init default connection parameters.
         * Application may change it before initiating a connection */
        cy_ble_connectionParameters.scanIntv =
                cy_ble_configPtr->discoveryInfo[centralParamIndex].scanIntv;
        cy_ble_connectionParameters.scanWindow =
                cy_ble_configPtr->discoveryInfo[centralParamIndex].scanWindow;
        cy_ble_connectionParameters.ownAddrType =
                cy_ble_configPtr->discoveryInfo[centralParamIndex].ownAddrType;
        cy_ble_connectionParameters.initiatorFilterPolicy =
                cy_ble_configPtr->discoveryInfo[centralParamIndex].scanFilterPolicy;
        cy_ble_connectionParameters.connIntvMin =
                cy_ble_configPtr->gapcScanParams[centralParamIndex].gapcConnectionIntervalMin;
        cy_ble_connectionParameters.connIntvMax =
                cy_ble_configPtr->gapcScanParams[centralParamIndex].gapcConnectionIntervalMax;
        cy_ble_connectionParameters.connLatency =
            cy_ble_configPtr->gapcScanParams[centralParamIndex].gapcConnectionSlaveLatency;
        cy_ble_connectionParameters.supervisionTO =
                cy_ble_configPtr->gapcScanParams[centralParamIndex].gapcConnectionTimeOut;
        cy_ble_connectionParameters.minCeLength = 0x0000u;
        cy_ble_connectionParameters.maxCeLength = 0xFFFFu;
        cy_ble_connectingTimeout.timeout = CY_BLE_GAPC_CONNECTING_TIMEOUT;

        for(i = 0u; i < CY_BLE_GAP_BD_ADDR_SIZE; i++)
        {
            cy_ble_connectionParameters.peerBdAddr[i] = address->bdAddr[i];
        }
        cy_ble_connectionParameters.peerAddrType = address->type;

        apiResult = Cy_BLE_StartTimer(&cy_ble_connectingTimeout);

        if(apiResult == CY_BLE_SUCCESS)
        {
            apiResult = Cy_BLE_GAPC_InitConnection(&cy_ble_connectionParameters);

            if(apiResult == CY_BLE_SUCCESS)
            {
                Cy_BLE_SetState(CY_BLE_STATE_CONNECTING);
            }
        }
        else
        {
            (void)Cy_BLE_StopTimer(&cy_ble_connectingTimeout);
        }
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_GAPC_CancelDeviceConnection
***************************************************************************//**
*
*  This function cancels a previously initiated connection with the remote
*  device. It is a blocking function. No event is generated on calling this
*  function. If the devices are already connected then this function should not
*  be used. If you intend to disconnect from an existing connection, the function
*  Cy_BLE_GAP_Disconnect() should be used.
*
* \return
* \ref cy_en_ble_api_result_t : Return value indicates whether the function succeeded or
*  failed. The following are possible error codes.
*
*   Error Codes                         | Description
*   ----------------------------------- | --------------------------------------
*   CY_BLE_SUCCESS                      | On successful operation.
*   CY_BLE_ERROR_INVALID_STATE          | On calling this function not in Connecting state.
*
*******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_GAPC_CancelDeviceConnection(void)
{
    cy_en_ble_api_result_t apiResult;

    if(Cy_BLE_GetState() != CY_BLE_STATE_CONNECTING)
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }
    else
    {
        apiResult = Cy_BLE_GAPC_CancelConnection();

        if(cy_ble_connectingTimeout.timeout != 0u)
        {
            (void)Cy_BLE_StopTimer(&cy_ble_connectingTimeout);
        }

        if(apiResult == CY_BLE_SUCCESS)
        {
            Cy_BLE_SetState(CY_BLE_STATE_ON);
        }
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_ChangeAdLocalName
***************************************************************************//**
*  This function is used to set the local device name in the advertisement or scan
*  response data structure.
*
*  \param name: The local device name string to be set in advertisement data
*            structure.
*  \param dest: 0 - advertisement structure, not zero value selects scan response
*              structure.
*
*******************************************************************************/
static void Cy_BLE_ChangeAdLocalName(const char8 name[],
                                     uint8_t dest)
{

    uint8_t fFlag;
    uint8_t adLength;
    uint8_t byteCounter;
    uint8_t *destBuffer;
    uint8_t maxLength;
    uint8_t advIndex = 0u;

    if(cy_ble_configPtr != NULL) 
    {
        do
        {
            fFlag = 0u;
            adLength = 0u;
            byteCounter = 0u;

            if(dest == 0u)      /* Destination - advertising structure */
            {
                destBuffer = cy_ble_configPtr->discoveryModeInfo[advIndex].advData->advData;
                maxLength = cy_ble_configPtr->discoveryModeInfo[advIndex].advData->advDataLen;
            }
            else                /* Destination - scan response structure */
            {
                destBuffer = cy_ble_configPtr->discoveryModeInfo[advIndex].scanRspData->scanRspData;
                maxLength = cy_ble_configPtr->discoveryModeInfo[advIndex].scanRspData->scanRspDataLen;
            }

            while((byteCounter < maxLength) && (fFlag == 0u))
            {
                adLength = destBuffer[byteCounter];

                if(adLength != 0u)
                {
                    /* Increment byte counter so it can point to AD type */
                    byteCounter++;

                    if((CY_BLE_SHORT_LOCAL_NAME == destBuffer[byteCounter]) ||
                    (CY_BLE_COMPLETE_LOCAL_NAME == destBuffer[byteCounter]))
                    {
                        /* Start of the Local Name AD type was found. Set flag and exit the loop. */
                        fFlag = 1u;
                    }
                    else
                    {
                        byteCounter += adLength;
                    }
                }
                else
                {
                    /* The end of advertisement data structure was encountered though exit the loop. */
                    break;
                }
            }

            if(fFlag != 0u)
            {
                uint32_t i;

                /* Reuse "adLength" to hold location of the last character of local name in
                * AD structure. */
                adLength += byteCounter;

                /* Increment byte counter to point to start of the local Name string */
                byteCounter++;

                for(i = byteCounter; ((i < (adLength)) && (name[i - byteCounter] != CY_BLE_NULL_CHARCTER)); i++)
                {
                    destBuffer[i] = (uint8_t)name[i - byteCounter];
                }

                /* This loop handles the case when new local name is shorter than old one.
                * In this case all remaining characters should be null characters. */
                while(adLength > i)
                {
                    /* Terminate string */
                    destBuffer[i] = (uint8_t)CY_BLE_NULL_CHARCTER;
                    i++;
                }
            }
            advIndex++;
        }
        while(advIndex < cy_ble_configPtr->params->gappConfCount);
    }
}


/******************************************************************************
 * Function Name: Cy_BLE_SetLocalName
 ***************************************************************************//**
 *  This function is used to set the local device name - a Characteristic of the
 *  GAP service. If the characteristic length entered in the Component customizer
 *  is shorter than the string specified by the "name" parameter, the local device
 *  name will be cut to the length specified in the customizer.
 *
 *  \param name: The local device name string. The name string to be written as
 *              the local device name. It represents a UTF-8 encoded User
 *              Friendly Descriptive Name for the device. The length of the local
 *              device string is entered into the Component customizer and it can
 *              be set to a value from 0 to 248 bytes. If the name contained in
 *              the parameter is shorter than the length from the customizer, the
 *              end of the name is indicated by a NULL octet (0x00).
 *
 *  \return
 *  \ref cy_en_ble_api_result_t : Return value indicates whether the function succeeded or
 *   failed. The following are possible error codes.
 *
 *   Error Codes                         | Description
 *   ------------                        | -----------
 *   CY_BLE_SUCCESS                      | The function completed successfully.
 *   CY_BLE_ERROR_INVALID_PARAMETER      | On specifying NULL as input parameter.
 *
 *******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_SetLocalName(const char8 name[])
{
    cy_en_ble_api_result_t apiResult = CY_BLE_ERROR_INVALID_PARAMETER;

    if((name != NULL) && (cy_ble_configPtr != NULL) && (cy_ble_gapsConfigPtr != NULL))
    {
        uint32_t i;
        char8 *ptr;
        uint8_t charLen;

        /* Get the pointer to the Device Name characteristic  */
        ptr = (char8*)CY_BLE_GATT_DB_ATTR_GET_ATTR_GEN_PTR(cy_ble_gapsConfigPtr->attrInfo->deviceNameCharHandle);

        /* First need to get the maximum length of the characteristic data in the GATT
         *  database to make sure there is enough place for the data. The length
         *  can't be longer than 248, so only the LSB of 16 bit of length is to
         *  be used. */
        charLen = (uint8_t)CY_BLE_GATT_DB_ATTR_GET_ATTR_GEN_MAX_LEN(cy_ble_gapsConfigPtr->attrInfo->deviceNameCharHandle);

        /* Copy name into characteristic */
        for(i = 0u; ((i < charLen) && (name[i] != CY_BLE_NULL_CHARCTER)); i++)
        {
            ptr[i] = name[i];
        }

        if(i < charLen)
        {
            /* Terminate string */
            ptr[i] = CY_BLE_NULL_CHARCTER;
        }

        /* Update device name in advertising and scan response structure */
        if((cy_ble_configPtr->params->gapRole & CY_BLE_GAP_PERIPHERAL) != 0u)
        {
            Cy_BLE_ChangeAdLocalName(name, 0u);
            Cy_BLE_ChangeAdLocalName(name, 1u);
        }

        apiResult = CY_BLE_SUCCESS;

    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_GetLocalName
***************************************************************************//**
*
*  This function is used to read the local device name - a Characteristic of
*  the GAP service.
*
*  \param name: The local device name string. Used to read the local name to the
*             given string array. It represents a UTF-8 encoded User Friendly
*             Descriptive Name for the device. The length of the local device
*             string is entered into the Component customizer and it can be set
*             to a value from 0 to 248 bytes. If the name contained in the
*             parameter is shorter than the length from the customizer, the end
*             of the name is indicated by a NULL octet (0x00).
*
*  \return
*  \ref cy_en_ble_api_result_t : Return value indicates whether the function succeeded or
*   failed. The following are possible error codes.
*
*   Error Codes                         | Description
*   ------------                        | -----------
*   CY_BLE_SUCCESS                      | The function completed successfully.
*   CY_BLE_ERROR_INVALID_PARAMETER      | On specifying NULL as input parameter.
*
*******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_GetLocalName(char8 name[])
{
    cy_en_ble_api_result_t apiResult = CY_BLE_ERROR_INVALID_PARAMETER;

    if((name != NULL) && (cy_ble_configPtr != NULL) && (cy_ble_gapsConfigPtr != NULL))
    {
        uint32_t i;
        uint8_t charLen;
        char8 *ptr;

        /* Get the pointer to the Device Name characteristic  */
        ptr = (char8*)CY_BLE_GATT_DB_ATTR_GET_ATTR_GEN_PTR(cy_ble_gapsConfigPtr->attrInfo->deviceNameCharHandle);

        /* First need to get the length of the characteristic data in the GATT
         *  database to make sure there is enough place for the data. The length
         *  can't be longer than 248, so only the LSB of 16 bit of length is to
         *  be used. */
        charLen = (uint8_t)CY_BLE_GATT_DB_ATTR_GET_ATTR_GEN_LEN(cy_ble_gapsConfigPtr->attrInfo->deviceNameCharHandle);

        /* Copy name from characteristic */
        for(i = 0u; ((i < charLen) && (CY_BLE_NULL_CHARCTER != ptr[i])); i++)
        {
            name[i] = ptr[i];
        }

        /* Terminate string */
        name[i] = CY_BLE_NULL_CHARCTER;

        apiResult = CY_BLE_SUCCESS;
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_IsPeerConnected
***************************************************************************//**
*
*  This function checks if the peer device represented by the bdAddr parameter
*  is connected or not.
*
*  \param *bdAddr: Pointer to the peer bdAddr array. Peer bdAddr is returned
*                  as an event parameter of #CY_BLE_EVT_GAP_DEVICE_CONNECTED
*                  or similar events.
*
*  \return
*   * true  - The peer device is connected.
*   * false - The peer device is not connected.
*
*******************************************************************************/
bool Cy_BLE_IsPeerConnected(uint8_t *bdAddr)
{
    cy_en_ble_api_result_t apiResult = CY_BLE_SUCCESS;
    uint32_t i;
    bool isConnected = false;

    if(cy_ble_configPtr != NULL)
    {
        for(i = 0u; i < cy_ble_configPtr->stackParam->maxConnCount; i++)
        {
            if(Cy_BLE_GetConnectionState(cy_ble_connHandle[i]) >= CY_BLE_CONN_STATE_CONNECTED)
            {
                /* Get BdAddress from bdHandle of the connected device */
                cy_stc_ble_gap_peer_addr_info_t param =
                {
                    .bdHandle = cy_ble_connHandle[i].bdHandle
                };
                apiResult = Cy_BLE_GAP_GetPeerBdAddr(&param);

                if(apiResult == CY_BLE_SUCCESS)
                {
                    if(memcmp(&param.bdAddr.bdAddr,(void*) bdAddr, CY_BLE_GAP_BD_ADDR_SIZE) == 0u)
                    {
                        isConnected = true;
                        break;
                    }
                }
            }
        }
    }
    return (isConnected);
}


/******************************************************************************
* Function Name: Cy_BLE_IsDevicePaired
***************************************************************************//**
*
*  The function is used to get the device pairing state.
*
*  \param connHandle: Pointer to the connection handle of the peer device.
*
*  \return
*   * true  - The peer device pairing is successful. A successful pairing is
*             indicated when #CY_BLE_EVT_GAP_AUTH_COMPLETE event is received.
*   * false - The peer device is not yet paired.
*
******************************************************************************/
bool Cy_BLE_IsDevicePaired(cy_stc_ble_conn_handle_t *connHandle)
{
    bool isPaired = false;

    if( (cy_ble_configPtr != NULL) && (connHandle != NULL) && 
        (connHandle->attId < cy_ble_configPtr->stackParam->maxConnCount) )
    {
        isPaired = cy_ble_pairStatus[connHandle->attId];
    }

    return(isPaired);
}


/******************************************************************************
* Function Name: Cy_BLE_GetDeviceRole
***************************************************************************//**
*
*  The function returns the local link layer device role which is connected
*  to peer device with connection handle indicated by connHandle parameter.
*
*  \param connHandle: Pointer to the connection handle of the peer device.
*
*  \return
*  * CY_BLE_GAP_LL_ROLE_MASTER        (0x00): The local device is connected as a master.
*  * CY_BLE_GAP_LL_ROLE_SLAVE         (0x01): The local device is connected as a slave.
*  * CY_BLE_INVALID_CONN_HANDLE_VALUE (0xff): Invalid connection handle.
*
******************************************************************************/
uint8_t Cy_BLE_GetDeviceRole(cy_stc_ble_conn_handle_t *connHandle)
{
    uint8_t ret = CY_BLE_INVALID_CONN_HANDLE_VALUE;

    if( (cy_ble_configPtr != NULL) && (connHandle != NULL) && 
        (connHandle->attId < cy_ble_configPtr->stackParam->maxConnCount) )
    {
        ret = cy_ble_devConnRole[connHandle->attId];
    }

    return(ret);
}


/******************************************************************************
* Function Name: Cy_BLE_GetRssiPeer
***************************************************************************//**
*
*  Replacement for Cy_BLE_GetRssi() API where unused parameters are removed.
*
*  This function reads the recorded Received Signal Strength Indicator (RSSI)
*  value of the last received packet on the specified connection.
*  sub-system.
*  The RSSI value is informed through #CY_BLE_EVT_GET_RSSI_COMPLETE.
*
*  Deprecate the Cy_BLE_GetRssi() API in BLE_PDL v2_0.
*
*  \param bdHandle: The bd handle of the peer device.
*
*  \return
*  \ref cy_en_ble_api_result_t : The return value indicates whether the
*   function succeeded or failed. Following are the possible error codes.
*
*   Errors codes                             | Description
*   ------------                             | -----------
*   CY_BLE_SUCCESS                           | On successful operation.
*   CY_BLE_ERROR_NO_DEVICE_ENTITY            | If there is no connection for corresponding bdHandle.
*   CY_BLE_ERROR_MEMORY_ALLOCATION_FAILED    | If Memory allocation failed.
*   CY_BLE_ERROR_INSUFFICIENT_RESOURCES      | If BLE Stack resources are unavailable.
*
*   Information     | Description
*   -----------     | -----------
*   Range           | -85 <= N <= 5
*   \note: The value is in dBm.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_GetRssiPeer(uint8_t  bdHandle)
{
    cy_stc_ble_rssi_info_t param = { .bdHandle = bdHandle };
    return(Cy_BLE_GetRssi(&param));
}


/******************************************************************************
* Function Name: Cy_BLE_GetNumOfActiveConn
***************************************************************************//**
*
*  Used to get active number of connections.
*
*  \return
*  connNum - number of active connections
*
******************************************************************************/
uint8_t Cy_BLE_GetNumOfActiveConn(void)
{
    uint32_t i;
    uint8_t connNum = 0u;

    if(cy_ble_configPtr != NULL)
    {
        for(i = 0u; i < cy_ble_configPtr->stackParam->maxConnCount; i++)
        {
            if(cy_ble_connState[i] >= CY_BLE_CONN_STATE_CONNECTED)
            {
                connNum++;
            }
        }
    }
    return(connNum);
}


/******************************************************************************
* Function Name: Cy_BLE_GetConnHandleByBdHandle
***************************************************************************//**
*
*  Used to get connection handle by bdHandle
*  This function halts in debug mode if bdHandle does not exist in the connected device
*  list
*
*  \param bdHandle: Peer device handle
*
*  \return
*  * connHandle: Full connection handle.
*  * connHandle.attId = CY_BLE_INVALID_CONN_HANDLE_VALUE: invalid bdHandle
*
******************************************************************************/
cy_stc_ble_conn_handle_t Cy_BLE_GetConnHandleByBdHandle(uint8_t bdHandle)
{
    uint32_t i;
    cy_stc_ble_conn_handle_t cHandle =
    {
        .attId    = CY_BLE_INVALID_CONN_HANDLE_VALUE,
        .bdHandle = CY_BLE_INVALID_CONN_HANDLE_VALUE
    };

    if (cy_ble_configPtr != NULL)
    {
        for(i = 0u; i < cy_ble_configPtr->stackParam->maxConnCount; i++)
        {
            if((Cy_BLE_GetConnectionState(cy_ble_connHandle[i]) >= CY_BLE_CONN_STATE_CONNECTED) &&
                    (cy_ble_connHandle[i].bdHandle == bdHandle))
            {
                cHandle = cy_ble_connHandle[i];
                break;
            }
        }
        if(cHandle.attId == CY_BLE_INVALID_CONN_HANDLE_VALUE)
        {
            /* HALT in debug mode */
            CY_ASSERT(cHandle.attId != CY_BLE_INVALID_CONN_HANDLE_VALUE);
        }
    }

    return cHandle;
}

#endif /* CY_BLE_LIB_HOST_CORE */
#endif /* CY_IP_MXBLESS */


/* [] END OF FILE */
