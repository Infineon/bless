/***************************************************************************//**
* \file cy_ble_custom.c
* \version 3.60
*
* \brief
*  Contains the source code for the Custom service.
*
********************************************************************************
* \copyright
* Copyright 2017-2021, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "cy_ble_event_handler.h"

#if defined(CY_IP_MXBLESS)
#if CY_BLE_LIB_HOST_CORE


/*******************************************************************************
* Private Function Prototypes
*******************************************************************************/

static void Cy_BLE_CUSTOMC_DiscoverServiceEventHandler(const cy_stc_ble_disc_srv_info_t *discServInfo);
static void Cy_BLE_CUSTOMC_DiscoverCharacteristicsEventHandler(const cy_stc_ble_disc_char_info_t *discCharInfo);
static void Cy_BLE_CUSTOMC_DiscoverCharDescriptorsEventHandler(const cy_stc_ble_disc_descr_info_t *discDescrInfo);
static void Cy_BLE_CUSTOMC_GetCharRange(cy_stc_ble_disc_range_info_t *charRangeInfo);
static cy_en_ble_gatt_err_code_t Cy_BLE_CUSTOM_EventHandler(uint32_t eventCode, void *eventParam);


/*******************************************************************************
* Global Variables
*******************************************************************************/

/* The pointer to a global BLE CUSTOM server config structure */
const cy_stc_ble_customs_config_t *cy_ble_customsConfigPtr = NULL;

/* The pointer to a global BLE CUSTOM client config structure */
const cy_stc_ble_customc_config_t *cy_ble_customcConfigPtr = NULL;


/******************************************************************************
* Function Name: Cy_BLE_CUSTOMS_Init
***************************************************************************//**
*
*  This function initializes server of the Custom service.
*
*  \param config: The configuration structure for the Custom service.
*
*  \return
*  \ref cy_en_ble_api_result_t : The return value indicates whether the function succeeded or
*  failed. The following are possible error codes.
*
*   Error Codes                             | Description
*   ------------                            | -----------
*   CY_BLE_SUCCESS                          | The function completed successfully.
*   CY_BLE_ERROR_INVALID_PARAMETER          | On specifying NULL as input parameter.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_CUSTOMS_Init(const cy_stc_ble_customs_config_t *config)
{
    cy_en_ble_api_result_t apiResult = CY_BLE_SUCCESS;

    if(config == NULL)
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else
    {
        /* Register the pointer to the config structure */
        cy_ble_customsConfigPtr = config;
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_CUSTOMC_Init
***************************************************************************//**
*
*  This function initializes client of the Custom service.
*
*  \param config: The configuration structure for the Custom service.
*
*  \return
*  \ref cy_en_ble_api_result_t : The return value indicates whether the function succeeded or
*  failed. The following are possible error codes.
*
*   Error Codes                             | Description
*   ------------                            | -----------
*   CY_BLE_SUCCESS                          | The function completed successfully.
*   CY_BLE_ERROR_INVALID_PARAMETER          | On specifying NULL as input parameter.
*   CY_BLE_ERROR_MEMORY_ALLOCATION_FAILED   | Buffer overflow in the registration callback.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_CUSTOMC_Init(const cy_stc_ble_customc_config_t *config)
{
    cy_en_ble_api_result_t apiResult;

    if(config == NULL)
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else
    {
        /* Register the pointer to the config structure */
        cy_ble_customcConfigPtr = config;

        /* Register an event Handler for the Custom service */
        apiResult = Cy_BLE_RegisterServiceEventHandler(&Cy_BLE_CUSTOM_EventHandler);
        {
            uint32_t discIdx;
            uint32_t locCharIdx;

            for(discIdx = 0u; discIdx < cy_ble_configPtr->params->maxClientCount; discIdx++)
            {
                uint32_t locServIdx;
                for(locServIdx = 0u; locServIdx < cy_ble_customcConfigPtr->serviceCount; locServIdx++)
                {
                    uint32 servCnt = cy_ble_configPtr->context->discServiCount;
                    uint32 customServIdx = cy_ble_customcConfigPtr->serviceDiscIdx + locServIdx;

                    if(cy_ble_configPtr->context->serverInfo[(discIdx * servCnt) + customServIdx].range.startHandle ==
                        CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
                    {
                        for(locCharIdx = 0u; locCharIdx < cy_ble_customcConfigPtr->attrInfo[locServIdx].charCount;
                            locCharIdx++)
                        {
                            uint32_t locDescIdx;

                            cy_ble_customcConfigPtr->attrInfo[locServIdx].customServChar[locCharIdx].
                            customServCharHandle[discIdx] = 0u;

                            for(locDescIdx = 0u; locDescIdx < cy_ble_customcConfigPtr->attrInfo[locServIdx].
                                 customServChar[locCharIdx].descCount; locDescIdx++)
                            {
                                cy_ble_customcConfigPtr->attrInfo[locServIdx].customServChar[locCharIdx].
                                 customServCharDesc[locDescIdx].descHandle[discIdx] = 0u;
                            }

                            /* Initialize uuid  */
                            cy_ble_configPtr->context->serverInfo[(discIdx * servCnt) + customServIdx].uuid = 0x0000u;
                        }
                    }

                    /* Initialize uuid  */
                    if(cy_ble_customcConfigPtr->attrInfo[locServIdx].uuidFormat == 1u)
                    {
                        cy_ble_configPtr->context->serverInfo[(discIdx * servCnt) + customServIdx].uuid =
                        *(uint16 *)cy_ble_customcConfigPtr->attrInfo[locServIdx].uuid;
                    }
                    else
                    {
                        cy_ble_configPtr->context->serverInfo[(discIdx * servCnt) + customServIdx].uuid = 0x0000u;
                    }
                }
            }
        }

    }

    return(apiResult);
}

/******************************************************************************
* Function Name: Cy_BLE_CUSTOMC_DiscoverServiceEventHandler
***************************************************************************//**
*
*  This function is called on receiving a Read By Group Response event or
*  Read response with the 128-bit service uuid.
*
*  \param discServInfo: The pointer to a service information structure.
*
******************************************************************************/
static void Cy_BLE_CUSTOMC_DiscoverServiceEventHandler(const cy_stc_ble_disc_srv_info_t *discServInfo)
{
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(discServInfo->connHandle);
    uint32_t fFlag = 0u;
    uint32_t j;

    /* Services with the 128-bit UUID have discServInfo->uuid equal to 0 and the address to
     * the 128-uuid is stored in cy_ble_customCServ.uuid128.
     */
    for(j = 0u; (j < cy_ble_customcConfigPtr->serviceCount) && (fFlag == 0u); j++)
    {
        if((memcmp(cy_ble_customcConfigPtr->attrInfo[j].uuid, &discServInfo->srvcInfo->uuid.uuid128,
                      CY_BLE_GATT_128_BIT_UUID_SIZE) == 0) &&
              (cy_ble_customcConfigPtr->attrInfo[j].uuidFormat == CY_BLE_GATT_128_BIT_UUID_FORMAT))
        {
            uint32 servCnt = cy_ble_configPtr->context->discServiCount;
            uint32 customServIdx = cy_ble_customcConfigPtr->serviceDiscIdx + j;

            if(cy_ble_configPtr->context->serverInfo[(discIdx * servCnt) + customServIdx].range.startHandle ==
               CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
            {
               cy_ble_configPtr->context->serverInfo[(discIdx * servCnt) + customServIdx].range.startHandle =
                       discServInfo->srvcInfo->range.startHandle;
                    
               cy_ble_configPtr->context->serverInfo[(discIdx * servCnt) + customServIdx].range.endHandle =
                       discServInfo->srvcInfo->range.endHandle;

               cy_ble_customcConfigPtr->attrInfo[j].customServHandle[discIdx] =
                       discServInfo->srvcInfo->range.startHandle;

                cy_ble_configPtr->context->discovery[discIdx].servCount++;
                fFlag = 1u;
            }

            /* Indicate that request was handled */
            cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
        }
    }
}


/******************************************************************************
* Function Name: Cy_BLE_CUSTOMC_DiscoverCharacteristicsEventHandler
***************************************************************************//**
*
*  This function is called on receiving a #CY_BLE_EVT_GATTC_READ_BY_TYPE_RSP
*  event. Based on the service index,the appropriate data structure is populated
*  using the data received as part of the callback.
*
*  \param discCharInfo: The pointer to the characteristic information structure.
*
******************************************************************************/
static void Cy_BLE_CUSTOMC_DiscoverCharacteristicsEventHandler(const cy_stc_ble_disc_char_info_t *discCharInfo)
{
    static cy_ble_gatt_db_attr_handle_t *customsLastEndHandle[CY_BLE_MAX_CONNECTION_INSTANCES] = { 0u };
    static uint32_t discoveryLastServ[CY_BLE_MAX_CONNECTION_INSTANCES] = { 0u };

    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(discCharInfo->connHandle);
    uint32_t servIdx = cy_ble_configPtr->context->discovery[discIdx].servCount - cy_ble_customcConfigPtr->serviceDiscIdx;
    uint32_t customDiscIdx = cy_ble_customcConfigPtr->serviceDiscIdx;
    uint32_t locReqHandle = 0u;
    uint32_t locCharIndex;

    if((cy_ble_configPtr->context->discovery[discIdx].servCount >= customDiscIdx) &&
       (cy_ble_configPtr->context->discovery[discIdx].servCount <=
            ((customDiscIdx + cy_ble_customcConfigPtr->serviceCount) - 1u)))
    {

        /* Update the last characteristic endHandle to the declaration handle of this characteristic */
        if(customsLastEndHandle[discIdx] != NULL)
        {
            if(discoveryLastServ[discIdx] == servIdx)
            {
                *customsLastEndHandle[discIdx] = discCharInfo->charDeclHandle - 1u;
            }
            customsLastEndHandle[discIdx] = NULL;
        }

        for(locCharIndex = 0u; (locCharIndex < cy_ble_customcConfigPtr->attrInfo[servIdx].charCount) && (locReqHandle == 0u);
            locCharIndex++)
        {
            uint32_t fFlag = 0u;

            /* Support the 128-bit uuid */
            if((discCharInfo->uuidFormat == CY_BLE_GATT_128_BIT_UUID_FORMAT) &&
               (cy_ble_customcConfigPtr->attrInfo[servIdx].customServChar[locCharIndex].uuidFormat ==
                CY_BLE_GATT_128_BIT_UUID_FORMAT))
            {
                if(memcmp(cy_ble_customcConfigPtr->attrInfo[servIdx].customServChar[locCharIndex].uuid,
                          &discCharInfo->uuid.uuid128, CY_BLE_GATT_128_BIT_UUID_SIZE) == 0)
                {
                    fFlag = 1u;
                }
            }

            /* And support the 16-bit uuid */
            if((discCharInfo->uuidFormat == CY_BLE_GATT_16_BIT_UUID_FORMAT) &&
               (cy_ble_customcConfigPtr->attrInfo[servIdx].customServChar[locCharIndex].uuidFormat ==
                CY_BLE_GATT_16_BIT_UUID_FORMAT))
            {
                if(memcmp(cy_ble_customcConfigPtr->attrInfo[servIdx].customServChar[locCharIndex].uuid,
                          &discCharInfo->uuid.uuid16, CY_BLE_GATT_16_BIT_UUID_SIZE) == 0)
                {
                    fFlag = 1u;
                }
            }

            if((fFlag == 1u) &&
               (cy_ble_customcConfigPtr->attrInfo[servIdx].customServChar[locCharIndex].customServCharHandle[discIdx] ==
                CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE))
            {
                cy_ble_customcConfigPtr->attrInfo[servIdx].customServChar[locCharIndex].customServCharHandle[discIdx] =
                    discCharInfo->valueHandle;
                cy_ble_customcConfigPtr->attrInfo[servIdx].customServChar[locCharIndex].properties[discIdx] =
                    discCharInfo->properties;

                /* Init the pointer to the characteristic endHandle */
                customsLastEndHandle[discIdx] = &cy_ble_customcConfigPtr->attrInfo[servIdx].customServChar[locCharIndex].
                                                 customServCharEndHandle[discIdx];

                /* Init the service index of the discovered characteristic */
                discoveryLastServ[discIdx] = servIdx;
                locReqHandle = 1u;
            }
        }

        /* Init the characteristic endHandle to the service endHandle.
         * The characteristic endHandle will be updated to the declaration
         * Handler of the following characteristic,
         * in the following characteristic discovery procedure. */
        if(customsLastEndHandle[discIdx] != NULL)
        {
            uint32 servCnt = cy_ble_configPtr->context->discServiCount;
            *customsLastEndHandle[discIdx] = cy_ble_configPtr->context->serverInfo[(discIdx * servCnt) +
                                             cy_ble_configPtr->context->discovery[discIdx].servCount].range.endHandle;
        }

        /* Indicate that the request was handled */
        cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
    }
}


/******************************************************************************
* Function Name: Cy_BLE_CUSTOMC_GetCharRange
***************************************************************************//**
*
* Returns a possible range of the current characteristic descriptor via
* the input parameter charRangeInfo->range.
*
* \param *charRangeInfo: The pointer to the descriptor range information structure.
*
******************************************************************************/
static void Cy_BLE_CUSTOMC_GetCharRange(cy_stc_ble_disc_range_info_t *charRangeInfo)
{
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(charRangeInfo->connHandle);
    uint32_t customDiscIdx = cy_ble_customcConfigPtr->serviceDiscIdx;
    uint32_t exitFlag = 0u;

    if((cy_ble_configPtr->context->discovery[discIdx].servCount >= customDiscIdx) &&
       (cy_ble_configPtr->context->discovery[discIdx].servCount <= ((customDiscIdx +
                                                                       cy_ble_customcConfigPtr->serviceCount) - 1u)))
    {
        uint32_t servIdx = cy_ble_configPtr->context->discovery[discIdx].servCount - customDiscIdx;

        if(charRangeInfo->srviIncIdx != CY_BLE_DISCOVERY_INIT)
        {
            cy_ble_configPtr->context->discovery[discIdx].charCount++;
        }

        while((cy_ble_configPtr->context->discovery[discIdx].charCount <
                cy_ble_customcConfigPtr->attrInfo[servIdx].charCount) && (exitFlag == 0u))
        {
            uint32_t charIdx = cy_ble_configPtr->context->discovery[discIdx].charCount;

            if((cy_ble_customcConfigPtr->attrInfo[servIdx].customServChar[charIdx].descCount > 0u) &&
               (cy_ble_customcConfigPtr->attrInfo[servIdx].customServChar[charIdx].customServCharEndHandle[discIdx] != 0u))
            {
                /* Read the characteristic range */
                charRangeInfo->range.startHandle = cy_ble_customcConfigPtr->attrInfo[servIdx].customServChar[charIdx].
                                                    customServCharHandle[discIdx] + 1u;
                charRangeInfo->range.endHandle = cy_ble_customcConfigPtr->attrInfo[servIdx].customServChar[charIdx].
                                                  customServCharEndHandle[discIdx];
                exitFlag = 1u;
            }
            else
            {
                cy_ble_configPtr->context->discovery[discIdx].charCount++;
            }
        }

        /* Indicate that the request was handled */
        cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
    }
}


/******************************************************************************
* Function Name: Cy_BLE_CUSTOMC_DiscoverCharDescriptorsEventHandler
***************************************************************************//**
*
*  This function is called on receiving a #CY_BLE_EVT_GATTC_FIND_INFO_RSP event.
*  Based on the descriptor UUID, the appropriate data structure is populated using
*  the data received as part of the callback.
*
*  \param discDescrInfo: The pointer to the descriptor information structure.
*
******************************************************************************/
static void Cy_BLE_CUSTOMC_DiscoverCharDescriptorsEventHandler(const cy_stc_ble_disc_descr_info_t *discDescrInfo)
{
    uint32_t locDescIndex;
    uint32_t locReqHandle = 0u;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(discDescrInfo->connHandle);
    uint32_t customDiscIdx = cy_ble_customcConfigPtr->serviceDiscIdx;


    if((cy_ble_configPtr->context->discovery[discIdx].servCount >= customDiscIdx) &&
       (cy_ble_configPtr->context->discovery[discIdx].servCount <= ((customDiscIdx +
                                                                       cy_ble_customcConfigPtr->serviceCount) - 1u)))
    {
        uint32_t servIdx = cy_ble_configPtr->context->discovery[discIdx].servCount - customDiscIdx;
        uint32_t charIdx = cy_ble_configPtr->context->discovery[discIdx].charCount;

        for(locDescIndex = 0u; (locDescIndex < cy_ble_customcConfigPtr->attrInfo[servIdx].customServChar[charIdx].descCount) &&
            (locReqHandle == 0u); locDescIndex++)
        {
            uint32_t fFlag = 0u;

            if((discDescrInfo->uuidFormat == CY_BLE_GATT_128_BIT_UUID_FORMAT) &&
               (cy_ble_customcConfigPtr->attrInfo[servIdx].customServChar[charIdx].customServCharDesc[locDescIndex].
                 uuidFormat == CY_BLE_GATT_128_BIT_UUID_FORMAT))
            {
                if(memcmp(cy_ble_customcConfigPtr->attrInfo[servIdx].customServChar[charIdx].
                           customServCharDesc[locDescIndex].uuid, &discDescrInfo->uuid.uuid128,
                          CY_BLE_GATT_128_BIT_UUID_SIZE) == 0)
                {
                    fFlag = 1u;
                }
            }

            if((discDescrInfo->uuidFormat == CY_BLE_GATT_16_BIT_UUID_FORMAT) &&
               (cy_ble_customcConfigPtr->attrInfo[servIdx].customServChar[charIdx].customServCharDesc[locDescIndex].
                 uuidFormat == CY_BLE_GATT_16_BIT_UUID_FORMAT))
            {
                if(memcmp(cy_ble_customcConfigPtr->attrInfo[servIdx].customServChar[charIdx].
                           customServCharDesc[locDescIndex].uuid, &discDescrInfo->uuid.uuid16,
                          CY_BLE_GATT_16_BIT_UUID_SIZE) == 0)
                {
                    fFlag = 1u;
                }
            }

            if((fFlag == 1u) &&
               (cy_ble_customcConfigPtr->attrInfo[servIdx].customServChar[charIdx].customServCharDesc[locDescIndex].
                 descHandle[discIdx] == CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE))
            {
                cy_ble_customcConfigPtr->attrInfo[servIdx].customServChar[charIdx].customServCharDesc[locDescIndex].
                 descHandle[discIdx] = discDescrInfo->descrHandle;
                locReqHandle = 1u;
            }
        }
    }
}


/******************************************************************************
* Function Name: Cy_BLE_CUSTOM_EventHandler
***************************************************************************//**
*
*  Handles events from the BLE Stack for the Custom service.
*
*  \param eventCode:  The event code.
*  \param eventParam: The event parameters.
*
* \return
*  The return value is of the type cy_en_ble_gatt_err_code_t.
*
******************************************************************************/
static cy_en_ble_gatt_err_code_t Cy_BLE_CUSTOM_EventHandler(uint32_t eventCode,
                                                            void *eventParam)
{
    if(eventCode > (uint32_t)CY_BLE_EVT_MAX)
    {
        /* Handling service-specific events */
        switch((cy_en_ble_evt_t)eventCode)
        {
            /* Discovery events */
            case CY_BLE_EVT_GATTC_DISC_SERVICE:
                Cy_BLE_CUSTOMC_DiscoverServiceEventHandler((cy_stc_ble_disc_srv_info_t*)eventParam);
                break;

            case CY_BLE_EVT_GATTC_DISC_CHAR:
                Cy_BLE_CUSTOMC_DiscoverCharacteristicsEventHandler((cy_stc_ble_disc_char_info_t*)eventParam);
                break;

            case CY_BLE_EVT_GATTC_DISC_DESCR:
                Cy_BLE_CUSTOMC_DiscoverCharDescriptorsEventHandler((cy_stc_ble_disc_descr_info_t*)eventParam);
                break;

            case CY_BLE_EVT_GATTC_DISC_DESCR_GET_RANGE:
                Cy_BLE_CUSTOMC_GetCharRange((cy_stc_ble_disc_range_info_t*)eventParam);
                break;

            default:
                break;
        }
    }
    else
    {
        /* Handling GATT Client events */
        switch((cy_en_ble_event_t)eventCode)
        {
            case CY_BLE_EVT_GATTC_READ_RSP:
            case CY_BLE_EVT_GATTC_READ_MULTI_RSP:
            {
                cy_stc_ble_gattc_read_rsp_param_t *eventParamCast = (cy_stc_ble_gattc_read_rsp_param_t*)eventParam;
                cy_stc_ble_conn_handle_t locConnHandle = eventParamCast->connHandle;
                uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(locConnHandle);

                /* Read the response with the 128-bit included service uuid */
                if((Cy_BLE_GetConnectionState(locConnHandle) == CY_BLE_CONN_STATE_CLIENT_INCL_DISCOVERING) &&
                   (discIdx < cy_ble_configPtr->params->maxClientCount) &&
                   (cy_ble_configPtr->context->discovery[discIdx].inclInfo.inclDefHandle != 0u))
                {
                    cy_stc_ble_disc_srvc128_info_t discServ128Info;
                    cy_stc_ble_disc_srv_info_t locDiscServInfo;
                    uint32_t servCnt = cy_ble_configPtr->context->discServiCount;
                                        
                    /* Store the service range */
                    discServ128Info.range = cy_ble_configPtr->context->discovery[discIdx].inclInfo.inclHandleRange;
                                        
                    (void)memcpy((void*)discServ128Info.uuid.uuid128.value, (void*)eventParamCast->value.val,
                                 (uint32_t)eventParamCast->value.len);
                                 
                    locDiscServInfo.srvcInfo   = &discServ128Info;
                    locDiscServInfo.connHandle = locConnHandle;
             
                    Cy_BLE_CUSTOMC_DiscoverServiceEventHandler(&locDiscServInfo);

                    /* Re-initiate the function, setting the start
                     * handle to the attribute handle which is placed next to the one used in
                     * the above step.
                     */
                    discServ128Info.range.startHandle =
                        cy_ble_configPtr->context->discovery[discIdx].inclInfo.inclDefHandle + 1u;

                    discServ128Info.range.endHandle =
                        cy_ble_configPtr->context->serverInfo[(discIdx * servCnt) +
                        cy_ble_configPtr->context->discovery[discIdx].servCount].range.endHandle;

                    if(discServ128Info.range.startHandle <= discServ128Info.range.endHandle)
                    {
                        cy_stc_ble_gattc_read_by_type_req_t requestParam;
                        requestParam.range      = discServ128Info.range;
                        requestParam.connHandle = eventParamCast->connHandle;

                        if(Cy_BLE_GATTC_FindIncludedServices(&requestParam) != CY_BLE_SUCCESS)
                        {
                            Cy_BLE_ApplCallback((uint32_t)CY_BLE_EVT_GATTC_INCL_DISCOVERY_FAILED,
                                                &eventParamCast->connHandle);
                            cy_ble_configPtr->context->discovery[discIdx].autoDiscoveryFlag = 0u;
                        }
                    }
                    else     /* Continue discovery of the following service */
                    {
                        Cy_BLE_NextInclDiscovery(eventParamCast->connHandle, CY_BLE_DISCOVERY_CONTINUE);
                    }

                    cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
                    cy_ble_configPtr->context->discovery[discIdx].inclInfo.inclDefHandle = 0u;
                }
            }
                break;

            default:
                break;
        }
    }

    return(CY_BLE_GATT_ERR_NONE);
}

#endif /* CY_BLE_LIB_HOST_CORE */
#endif /* CY_IP_MXBLESS */


/* [] END OF FILE */
