/***************************************************************************//**
* \file cy_ble_cts.c
* \version 3.30
*
* \brief
*  This file contains the source code for the Current Time service.
*
********************************************************************************
* \copyright
* Copyright 2017-2019, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "cy_ble_event_handler.h"

#if defined(CY_IP_MXBLESS)
#if CY_BLE_LIB_HOST_CORE


/*******************************************************************************
* Internal Defines / Macro Functions
*******************************************************************************/

/* Performs validation of 'adjReason' if it has invalid value, it will be set to
 * "Unknown" reason. */
#define CY_BLE_CTS_CHECK_ADJUST_REASON(adjReason)    ((adjReason) =                                            \
                                                          (((adjReason) == CY_BLE_CTS_REASON_UNKNOWN) ?        \
                                                           CY_BLE_CTS_REASON_UNKNOWN :                         \
                                                           (((adjReason) == CY_BLE_CTS_MANUAL_UPDATE) ?        \
                                                            CY_BLE_CTS_MANUAL_UPDATE :                         \
                                                            (((adjReason) == CY_BLE_CTS_EXTERNAL_REF_UPDATE) ? \
                                                             CY_BLE_CTS_EXTERNAL_REF_UPDATE :                  \
                                                             (((adjReason) == CY_BLE_CTS_TIME_ZONE_CHANGE) ?   \
                                                              CY_BLE_CTS_TIME_ZONE_CHANGE :                    \
                                                              (((adjReason) == CY_BLE_CTS_DST_CHANGE) ?        \
                                                               CY_BLE_CTS_DST_CHANGE :                         \
                                                               CY_BLE_CTS_REASON_UNKNOWN))))))




/*******************************************************************************
* Private Function Prototypes
*******************************************************************************/

static cy_en_ble_gatt_err_code_t Cy_BLE_CTS_EventHandler(uint32_t eventCode, void *eventParam);
static cy_en_ble_gatt_err_code_t Cy_BLE_CTSS_EventHandler(uint32_t eventCode, void *eventParam);
static cy_en_ble_gatt_err_code_t Cy_BLE_CTSC_EventHandler(uint32_t eventCode, void *eventParam);

static cy_en_ble_gatt_err_code_t Cy_BLE_CTSS_WriteEventHandler(cy_stc_ble_gatts_write_cmd_req_param_t *eventParam);

static void Cy_BLE_CTSC_DiscoverCharacteristicsEventHandler(cy_stc_ble_disc_char_info_t *discCharInfo);
static void Cy_BLE_CTSC_DiscoverCharDescriptorsEventHandler(cy_stc_ble_disc_descr_info_t *discDescrInfo);
static void Cy_BLE_CTSC_GetCharRange(cy_stc_ble_disc_range_info_t *charRangeInfo);
static void Cy_BLE_CTSC_NotificationEventHandler(cy_stc_ble_gattc_handle_value_ntf_param_t *eventParam);
static void Cy_BLE_CTSC_ReadResponseEventHandler(cy_stc_ble_gattc_read_rsp_param_t *eventParam);
static void Cy_BLE_CTSC_WriteResponseEventHandler(const cy_stc_ble_conn_handle_t *eventParam);
static void Cy_BLE_CTSC_ErrorResponseEventHandler(const cy_stc_ble_gatt_err_param_t *eventParam);


/*******************************************************************************
* Global Variables
*******************************************************************************/

/* The pointers to the callback functions */
static cy_ble_callback_t Cy_BLE_CTS_ApplCallback = NULL;
static cy_ble_event_handler_cb_t Cy_BLE_CTSC_EventHandlerCallback = NULL;
static cy_ble_event_handler_cb_t Cy_BLE_CTSS_EventHandlerCallback = NULL;

/* The internal storage for the last request handle to check response */
static cy_ble_gatt_db_attr_handle_t cy_ble_ctscReqHandle[CY_BLE_MAX_CONNECTION_INSTANCES];

/* The pointer to a global BLE CTS server config structure */
const cy_stc_ble_ctss_config_t *cy_ble_ctssConfigPtr = NULL;

/* The pointer to a global BLE CTS client config structure */
const cy_stc_ble_ctsc_config_t *cy_ble_ctscConfigPtr = NULL;


/******************************************************************************
* Function Name: Cy_BLE_CTSS_Init
***************************************************************************//**
*
*  This function initializes server of the Current Time service.
*
*  \param config: Configuration structure for the CTS.
*
*  \return
*  \ref cy_en_ble_api_result_t : Return value indicates whether the function succeeded or
*                                failed. The following are possible error codes.
*
*   Error Codes                             | Description
*   ------------                            | -----------
*   CY_BLE_SUCCESS                          | The function completed successfully.
*   CY_BLE_ERROR_INVALID_PARAMETER          | On specifying NULL as input parameter.
*   CY_BLE_ERROR_MEMORY_ALLOCATION_FAILED   | Buffer overflow in the registration callback.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_CTSS_Init(const cy_stc_ble_ctss_config_t *config)
{
    cy_en_ble_api_result_t apiResult;

    if(config == NULL)
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else
    {
        /* Registers a pointer to config structure */
        cy_ble_ctssConfigPtr = config;

        /* Registers event handler for the CTS */
        apiResult = Cy_BLE_RegisterServiceEventHandler(&Cy_BLE_CTS_EventHandler);

        /* Registers a pointer to server event handler function */
        Cy_BLE_CTSS_EventHandlerCallback = &Cy_BLE_CTSS_EventHandler;
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_CTSC_Init
***************************************************************************//**
*
*  This function initializes client of the Current Time service.
*
*  \param config: Configuration structure for the CTS.
*
*  \return
*  \ref cy_en_ble_api_result_t : Return value indicates whether the function succeeded or
*                                failed. The following are possible error codes.
*
*   Error Codes                             | Description
*   ------------                            | -----------
*   CY_BLE_SUCCESS                          | The function completed successfully.
*   CY_BLE_ERROR_INVALID_PARAMETER          | On specifying NULL as input parameter.
*   CY_BLE_ERROR_MEMORY_ALLOCATION_FAILED   | Buffer overflow in the registration callback.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_CTSC_Init(const cy_stc_ble_ctsc_config_t *config)
{
    cy_en_ble_api_result_t apiResult;

    if((config == NULL) || ((cy_ble_configPtr->params->gattRole & CY_BLE_GATT_CLIENT) == 0u))
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else
    {
        uint32_t idx;

        /* Registers a pointer to config structure */
        cy_ble_ctscConfigPtr = config;

        /* Registers event handler for the CTS */
        apiResult = Cy_BLE_RegisterServiceEventHandler(&Cy_BLE_CTS_EventHandler);

        /* Registers a pointer to client event handler function */
        Cy_BLE_CTSC_EventHandlerCallback = &Cy_BLE_CTSC_EventHandler;

        for(idx = 0u; idx < cy_ble_configPtr->params->maxClientCount; idx++)
        {
            uint32 servCnt    = cy_ble_configPtr->context->discServiCount;
            uint32 ctsServIdx = cy_ble_ctscConfigPtr->serviceDiscIdx;

            if(cy_ble_configPtr->context->serverInfo[(idx * servCnt) + ctsServIdx].range.startHandle ==
                CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
            {
                /* Get pointer to CTS client structure */
                cy_stc_ble_ctsc_t *ctscPtr = (cy_stc_ble_ctsc_t *)&cy_ble_ctscConfigPtr->attrInfo[idx];

                /* Clean client structure */
                (void)memset(ctscPtr, 0, sizeof(cy_stc_ble_ctsc_t));

                /* Initialize uuid  */
                cy_ble_configPtr->context->serverInfo[(idx * servCnt) + ctsServIdx].uuid =
                    CY_BLE_UUID_CURRENT_TIME_SERVICE;
            }

            cy_ble_ctscReqHandle[idx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
        }
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_CTS_RegisterAttrCallback
***************************************************************************//**
*
*  Registers a callback function for service-specific attribute operations.
*  Service-specific Write Requests from the peer device will not be handled with
*  an unregistered callback function.
*
*  \param callbackFunc: An application layer event callback function to receive
*  events from the PSoC 6 BLE Middleware. The definition of \ref cy_ble_callback_t
*  for CTS service is:<br>
*  typedef void (* cy_ble_callback_t) (uint32_t eventCode, void *eventParam),
*  where:
*       * eventCode:  Indicates the event that triggered this callback.
*         (e.g. #CY_BLE_EVT_CTSS_NOTIFICATION_ENABLED).
*       * eventParam: Contains the parameters corresponding to the
*         current event; (e.g. pointer to \ref cy_stc_ble_cts_char_value_t
*         structure that contains details of the characteristic
*         for which the notification enabled event was triggered).
*
******************************************************************************/
void Cy_BLE_CTS_RegisterAttrCallback(cy_ble_callback_t callbackFunc)
{
    Cy_BLE_CTS_ApplCallback = callbackFunc;
}


/******************************************************************************
* Function Name: Cy_BLE_CTSS_SetCharacteristicValue
***************************************************************************//**
*
*  Sets a characteristic value of Current Time service, which is a value
*  identified by charIndex, to the local database.
*
*  \param charIndex: The index of the service characteristic of type
*                    \ref cy_en_ble_cts_char_index_t.
*  \param attrSize:  The size of the characteristic value attribute.
*  \param attrValue: The pointer to the characteristic value data that should be
*                    stored to the GATT database.
*
*  \return
*  A return value of type \ref cy_en_ble_api_result_t.
*
*   Error Codes                             | Description
*   ------------                            | -----------
*   CY_BLE_SUCCESS                          | The request was handled successfully.
*   CY_BLE_ERROR_INVALID_PARAMETER          | Validation of the input parameter failed.
*   CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE| An optional characteristic is absent.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_CTSS_SetCharacteristicValue(cy_en_ble_cts_char_index_t charIndex,
                                                          uint8_t attrSize,
                                                          uint8_t *attrValue)
{
    uint16_t locYear;
    cy_stc_ble_cts_current_time_t currTime;
    cy_stc_ble_cts_local_time_info_t localTime;
    cy_stc_ble_cts_reference_time_info_t refTime;
    cy_en_ble_api_result_t apiResult = CY_BLE_ERROR_INVALID_PARAMETER;

    if(attrValue != NULL)
    {
        switch(charIndex)
        {
            case CY_BLE_CTS_CURRENT_TIME:
                if(attrSize == CY_BLE_CTS_CURRENT_TIME_SIZE)
                {
                    (void)memcpy((void*)&currTime, (void*)attrValue, (uint32_t)attrSize);

                    /* Validate characteristic value
                     * First, validate "locYear" field
                     */
                    locYear = ((uint16_t)((uint16_t)currTime.yearHigh << CY_BLE_CTS_8_BIT_OFFSET)) |
                              ((uint16_t)currTime.yearLow);

                    if((locYear >= CY_BLE_CTS_YEAR_MIN) && (locYear <= CY_BLE_CTS_YEAR_MAX))
                    {
                        /* Validation passed */
                        apiResult = CY_BLE_SUCCESS;
                    }

                    /* Validate "Month" field */
                    if((CY_BLE_ERROR_INVALID_PARAMETER != apiResult) && (CY_BLE_CTS_MONTH_MAX < currTime.month))
                    {
                        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
                    }

                    /* Next is "Day" field */
                    if((CY_BLE_ERROR_INVALID_PARAMETER != apiResult) && (CY_BLE_CTS_DAY_MAX < currTime.day))
                    {
                        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
                    }

                    /* Validate "Hours" field */
                    if((CY_BLE_ERROR_INVALID_PARAMETER != apiResult) && (CY_BLE_CTS_HOURS_MAX < currTime.hours))
                    {
                        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
                    }

                    /* Validate "Minutes" field */
                    if((CY_BLE_ERROR_INVALID_PARAMETER != apiResult) && (CY_BLE_CTS_MINUTES_MAX < currTime.minutes))
                    {
                        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
                    }

                    /* Validate "Seconds" field */
                    if((CY_BLE_ERROR_INVALID_PARAMETER != apiResult) && (CY_BLE_CTS_SECONDS_MAX < currTime.seconds))
                    {
                        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
                    }

                    /* Validate "dayOfWeek" field */
                    if((CY_BLE_ERROR_INVALID_PARAMETER != apiResult) && (CY_BLE_CTS_DAY_OF_WEEK_MAX <= currTime.dayOfWeek))
                    {
                        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
                    }

                    if(apiResult != CY_BLE_ERROR_INVALID_PARAMETER)
                    {
                        cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;
                        
                        dbAttrValInfo.connHandle.bdHandle        = 0u;
                        dbAttrValInfo.connHandle.attId           = 0u;
                        dbAttrValInfo.handleValuePair.attrHandle = cy_ble_ctssConfigPtr->attrInfo->currTimeCharHandle;
                        dbAttrValInfo.handleValuePair.value.len  = attrSize;
                        dbAttrValInfo.offset                     = 0u;
                        dbAttrValInfo.flags                      = CY_BLE_GATT_DB_LOCALLY_INITIATED;
                        
                        dbAttrValInfo.handleValuePair.value.val = (uint8_t*)&currTime;

                        /* Verify "Adjust Reason". If it is set to incorrect value then it will
                         * be changed to "Unknown" Adjust Reason. */
                        CY_BLE_CTS_CHECK_ADJUST_REASON(currTime.adjustReason);

                        /* Set Current Time Characteristic value to GATT database.
                         * Need to handle return type difference of Cy_BLE_GATTS_WriteAttributeValueCCCD() and
                         * Cy_BLE_CTSS_SetCharacteristicValue(). */

                        if(Cy_BLE_GATTS_WriteAttributeValueCCCD(&dbAttrValInfo) == CY_BLE_GATT_ERR_NONE)
                        {
                            /* Indicate success */
                            apiResult = CY_BLE_SUCCESS;
                        }
                    }
                }
                break;

            case CY_BLE_CTS_LOCAL_TIME_INFO:
                if(attrSize == CY_BLE_CTS_LOCAL_TIME_INFO_SIZE)
                {
                    (void)memcpy((void*)&localTime, (void*)attrValue, (uint32_t)attrSize);

                    if(((localTime.timeZone <= CY_BLE_CTS_UTC_OFFSET_MAX) &&
                        (localTime.timeZone >= CY_BLE_CTS_UTC_OFFSET_MIN)) ||
                       (localTime.timeZone == CY_BLE_CTS_UTC_OFFSET_UNKNOWN))
                    {
                        /* Validation passed */
                        apiResult = CY_BLE_SUCCESS;
                    }

                    if(apiResult != CY_BLE_ERROR_INVALID_PARAMETER)
                    {
                        /* Validate DST offset */
                        switch(localTime.dst)
                        {
                            case CY_BLE_CTS_STANDARD_TIME:
                            case CY_BLE_CTS_DAYLIGHT_TIME_0_5:
                            case CY_BLE_CTS_DAYLIGHT_TIME_1_0:
                            case CY_BLE_CTS_DAYLIGHT_TIME_2_0:
                            case CY_BLE_CTS_DST_UNKNOWN:
                                /* DST offset is correct */
                                apiResult = CY_BLE_SUCCESS;
                                break;

                            default:
                                /* Validation failed */
                                apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
                                break;
                        }
                    }

                    if(apiResult != CY_BLE_ERROR_INVALID_PARAMETER)
                    {
                        if(CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE != cy_ble_ctssConfigPtr->attrInfo->localTimeInfCharHandle)
                        {
                            /* Set Local Time Characteristic value to GATT database.
                             *  Need to handle return type difference of Cy_BLE_GATTS_WriteAttributeValueCCCD() and
                             *  Cy_BLE_CTSS_SetCharacteristicValue(). */
                            cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;
                            
                            dbAttrValInfo.connHandle.bdHandle        = 0u;
                            dbAttrValInfo.connHandle.attId           = 0u;
                            dbAttrValInfo.handleValuePair.attrHandle = cy_ble_ctssConfigPtr->attrInfo->localTimeInfCharHandle;
                            dbAttrValInfo.handleValuePair.value.len  = attrSize;
                            dbAttrValInfo.handleValuePair.value.val  = attrValue;
                            dbAttrValInfo.offset                     = 0u;
                            dbAttrValInfo.flags                      = CY_BLE_GATT_DB_LOCALLY_INITIATED;
                            
                            if(Cy_BLE_GATTS_WriteAttributeValueCCCD(&dbAttrValInfo) == CY_BLE_GATT_ERR_NONE)
                            {
                                /* Indicate success */
                                apiResult = CY_BLE_SUCCESS;
                            }
                        }
                        else
                        {
                            apiResult = CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE;
                        }
                    }
                }
                break;

            case CY_BLE_CTS_REFERENCE_TIME_INFO:
                if(attrSize == CY_BLE_CTS_REFERENCE_TIME_INFO_SIZE)
                {
                    (void)memcpy((void*)&refTime, (void*)attrValue, (uint32_t)attrSize);

                    /* Validate "Time Source" value */
                    if(CY_BLE_CTS_TIME_SRC_CELL_NET >= refTime.timeSource)
                    {
                        /* Time source is correct */
                        apiResult = CY_BLE_SUCCESS;
                    }

                    if(apiResult != CY_BLE_ERROR_INVALID_PARAMETER)
                    {
                        /* Validate "Hours since update" field */
                        if(refTime.hoursSinseUpdate <= CY_BLE_CTS_HOURS_MAX)
                        {
                            /* Value is correct */
                        }
                        else if(refTime.daysSinceUpdate == CY_BLE_CTS_MAX_DAYS_SINCE_UPDATE)
                        {
                            /* Per CTS spec "Hours since update" is set to 255 as "Days since update"
                             * is 255. */
                            refTime.hoursSinseUpdate = CY_BLE_CTS_MAX_HOURS_SINCE_UPDATE;
                        }
                        else
                        {
                            /* Invalid value encountered */
                            apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
                        }
                    }

                    if(apiResult != CY_BLE_ERROR_INVALID_PARAMETER)
                    {
                        if(CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE != cy_ble_ctssConfigPtr->attrInfo->localTimeInfCharHandle)
                        {
                            /* Set Reference Time Characteristic value to GATT database.
                             *  Need to handle return type difference of Cy_BLE_GATTS_WriteAttributeValueCCCD() and
                             *  Cy_BLE_CTSS_SetCharacteristicValue(). */
                            cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;
                            
                            dbAttrValInfo.connHandle.bdHandle        = 0u;
                            dbAttrValInfo.connHandle.attId           = 0u;
                            dbAttrValInfo.handleValuePair.attrHandle = cy_ble_ctssConfigPtr->attrInfo->refTimeInfCharHandle;
                            dbAttrValInfo.handleValuePair.value.len  = attrSize;
                            dbAttrValInfo.handleValuePair.value.val  = (uint8_t*)&refTime;
                            dbAttrValInfo.offset                     = 0u;
                            dbAttrValInfo.flags                      = CY_BLE_GATT_DB_LOCALLY_INITIATED;

                            if(Cy_BLE_GATTS_WriteAttributeValueCCCD(&dbAttrValInfo) == CY_BLE_GATT_ERR_NONE)
                            {
                                /* Indicate success */
                                apiResult = CY_BLE_SUCCESS;
                            }
                        }
                        else
                        {
                            apiResult = CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE;
                        }
                    }
                }
                break;

            default:
                /* Characteristic wasn't found */
                break;
        }
    }

    /* Return status */
    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_CTSS_GetCharacteristicValue
***************************************************************************//**
*
*  Gets a characteristic value of Current Time service. The value is
*  identified by charIndex.
*
*  \param charIndex: The index of the service characteristic of 
                     type \ref  cy_en_ble_cts_char_index_t.
*  \param attrSize:  The size of the Current Time service characteristic value attribute.
*  \param attrValue: The pointer to the location where characteristic value data
*                    should be stored.
*
*  \return
*  A return value of type \ref cy_en_ble_api_result_t.
*
*   Error Codes                              | Description
*   ------------                             | -----------
*   CY_BLE_SUCCESS                           | The characteristic value was read successfully.
*   CY_BLE_ERROR_INVALID_PARAMETER           | Validation of the input parameter failed.
*   CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE | Optional characteristic is absent
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_CTSS_GetCharacteristicValue(cy_en_ble_cts_char_index_t charIndex,
                                                          uint8_t attrSize,
                                                          uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;
    
    dbAttrValInfo.connHandle.bdHandle       = 0u;
    dbAttrValInfo.connHandle.attId          = 0u;
    dbAttrValInfo.handleValuePair.value.len = attrSize;
    dbAttrValInfo.handleValuePair.value.val = attrValue;
    dbAttrValInfo.flags                     = CY_BLE_GATT_DB_LOCALLY_INITIATED;
    dbAttrValInfo.offset                    = 0u;

    if(attrValue != NULL)
    {
        switch(charIndex)
        {
            case CY_BLE_CTS_CURRENT_TIME:
                if(attrSize == CY_BLE_CTS_CURRENT_TIME_SIZE)
                {
                    dbAttrValInfo.handleValuePair.attrHandle = cy_ble_ctssConfigPtr->attrInfo->currTimeCharHandle;
                    apiResult = CY_BLE_SUCCESS;
                }
                break;

            case CY_BLE_CTS_LOCAL_TIME_INFO:
                if(attrSize == CY_BLE_CTS_LOCAL_TIME_INFO_SIZE)
                {
                    dbAttrValInfo.handleValuePair.attrHandle = cy_ble_ctssConfigPtr->attrInfo->localTimeInfCharHandle;
                    apiResult = CY_BLE_SUCCESS;
                }
                break;

            case CY_BLE_CTS_REFERENCE_TIME_INFO:
                if(attrSize == CY_BLE_CTS_REFERENCE_TIME_INFO_SIZE)
                {
                    dbAttrValInfo.handleValuePair.attrHandle = cy_ble_ctssConfigPtr->attrInfo->refTimeInfCharHandle;
                    apiResult = CY_BLE_SUCCESS;
                }
                break;

            default:
                /* Characteristic wasn't found */
                break;
        }

        if(apiResult != CY_BLE_ERROR_INVALID_PARAMETER)
        {
            if(dbAttrValInfo.handleValuePair.attrHandle != CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
            {
                /* Get characteristic value from GATT database */
                if(Cy_BLE_GATTS_ReadAttributeValueCCCD(&dbAttrValInfo) == CY_BLE_GATT_ERR_NONE)
                {
                    /* Indicate success */
                    apiResult = CY_BLE_SUCCESS;
                }
                else
                {
                    apiResult = CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE;
                }
            }
            else
            {
                apiResult = CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE;
            }
        }
    }

    /* Return status */
    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_CTSS_GetCharacteristicDescriptor
***************************************************************************//**
*
*  Gets a characteristic descriptor of a specified characteristic of the Current
*  Time service.
*
*  \param connHandle: The connection handle
*  \param charIndex:  The index of the service characteristic of type 
*                     \ref cy_en_ble_cts_char_index_t.
*  \param descrIndex: The index of the service characteristic descriptor of type
*                     \ref cy_en_ble_cts_char_descriptors_t.
*  \param attrSize:   The size of the descriptor value.
*  \param attrValue:  The pointer to the location where characteristic descriptor value
*                     data should be stored.
*
*  \return
*  A return value of type \ref cy_en_ble_api_result_t.
*
*   Error Codes                              | Description
*   ------------                             | -----------
*   CY_BLE_SUCCESS                           | The characteristic descriptor value was read successfully.
*   CY_BLE_ERROR_INVALID_PARAMETER           | Validation of the input parameter failed.
*   CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE | Optional descriptor is absent.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_CTSS_GetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                               cy_en_ble_cts_char_index_t charIndex,
                                                               cy_en_ble_cts_char_descriptors_t descrIndex,
                                                               uint8_t attrSize,
                                                               uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult = CY_BLE_ERROR_INVALID_PARAMETER;

    if((charIndex == CY_BLE_CTS_CURRENT_TIME) && (descrIndex == CY_BLE_CTS_CURRENT_TIME_CCCD))
    {
        if(cy_ble_ctssConfigPtr->attrInfo->currTimeCccdHandle != CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
        {
            /* Get characteristic value from GATT database */
            cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;
            
            dbAttrValInfo.handleValuePair.attrHandle = cy_ble_ctssConfigPtr->attrInfo->currTimeCccdHandle;
            dbAttrValInfo.handleValuePair.value.len  = attrSize;
            dbAttrValInfo.handleValuePair.value.val  = attrValue;
            dbAttrValInfo.connHandle                 = connHandle;
            dbAttrValInfo.flags                      = CY_BLE_GATT_DB_LOCALLY_INITIATED;
            dbAttrValInfo.offset                     = 0u;

            if(Cy_BLE_GATTS_ReadAttributeValueCCCD(&dbAttrValInfo) == CY_BLE_GATT_ERR_NONE)
            {
                /* Indicate success */
                apiResult = CY_BLE_SUCCESS;
            }
            else
            {
                apiResult = CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE;
            }
        }
        else
        {
            apiResult = CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE;
        }
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_CTSS_WriteEventHandler
***************************************************************************//**
*
*  Handles the Write Request event for the Current Time service.
*
*  \param eventParam: The pointer to the data that came with a Write Request for the
*              Current Time service.
*
* \return
*  Return a value of type cy_en_ble_gatt_err_code_t:
*   * CY_BLE_GATT_ERR_NONE - Function terminated successfully.
*   * CY_BLE_GATT_ERR_INVALID_HANDLE - The Handle of the Current Time Client
*                                      Configuration characteristic descriptor
*                                      is not valid.
*   * CY_BLE_GATT_ERR_UNLIKELY_ERROR - An Internal Stack error occurred.
*   * CY_BLE_GATT_ERR_REQUEST_NOT_SUPPORTED - The notification property of the
*                                             Current Time Client Configuration
*                                             characteristic descriptor is
*                                             disabled.
*
******************************************************************************/
static cy_en_ble_gatt_err_code_t Cy_BLE_CTSS_WriteEventHandler(cy_stc_ble_gatts_write_cmd_req_param_t *eventParam)
{
    uint32_t event = (uint32_t)CY_BLE_EVT_CTSS_NOTIFICATION_DISABLED;
    cy_stc_ble_cts_char_value_t wrReqParam;
    
    wrReqParam.gattErrorCode = CY_BLE_GATT_ERR_NONE;
    wrReqParam.connHandle    = eventParam->connHandle;
    wrReqParam.value         = &eventParam->handleValPair.value;
    
    if(Cy_BLE_CTS_ApplCallback != NULL)
    {
        /* Client Characteristic Configuration descriptor Write Request */
        if(eventParam->handleValPair.attrHandle == cy_ble_ctssConfigPtr->attrInfo->currTimeCccdHandle)
        {
            /* Verify that optional notification property is enabled for Current Time
             * Characteristic.
             */
            if(CY_BLE_IS_NOTIFICATION_SUPPORTED(cy_ble_ctssConfigPtr->attrInfo->currTimeCharHandle))
            {
                cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;
                
                dbAttrValInfo.handleValuePair = eventParam->handleValPair;
                dbAttrValInfo.connHandle      = eventParam->connHandle;
                dbAttrValInfo.offset          = 0u;
                dbAttrValInfo.flags           = CY_BLE_GATT_DB_PEER_INITIATED;
                
                wrReqParam.gattErrorCode = Cy_BLE_GATTS_WriteAttributeValueCCCD(&dbAttrValInfo);

                if(CY_BLE_IS_NOTIFICATION_ENABLED_IN_PTR(eventParam->handleValPair.value.val))
                {
                    event = (uint32_t)CY_BLE_EVT_CTSS_NOTIFICATION_ENABLED;
                }

                if(wrReqParam.gattErrorCode == CY_BLE_GATT_ERR_NONE)
                {
                    wrReqParam.charIndex = CY_BLE_CTS_CURRENT_TIME;
                    wrReqParam.value = NULL;
                    Cy_BLE_CTS_ApplCallback(event, &wrReqParam);
                }
            }
            else
            {
                wrReqParam.gattErrorCode = CY_BLE_GATT_ERR_REQUEST_NOT_SUPPORTED;
            }

            /* Clear the callback flag indicating that request was handled */
            cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
        }
        else if((eventParam->handleValPair.attrHandle == cy_ble_ctssConfigPtr->attrInfo->currTimeCharHandle) ||
                (eventParam->handleValPair.attrHandle == cy_ble_ctssConfigPtr->attrInfo->localTimeInfCharHandle))
        {
            if(eventParam->handleValPair.attrHandle == cy_ble_ctssConfigPtr->attrInfo->currTimeCharHandle)
            {
                wrReqParam.charIndex = CY_BLE_CTS_CURRENT_TIME;
            }
            else
            {
                wrReqParam.charIndex = CY_BLE_CTS_LOCAL_TIME_INFO;
            }
            /* Check whether write property is supported */
            wrReqParam.gattErrorCode = Cy_BLE_GATT_DbCheckPermission(eventParam->handleValPair.attrHandle,
                                                                     &eventParam->connHandle, CY_BLE_GATT_DB_WRITE |
                                                                     CY_BLE_GATT_DB_PEER_INITIATED);

            if(wrReqParam.gattErrorCode == CY_BLE_GATT_ERR_NONE)
            {
                /* Send callback to user. User must validate the fields of the
                 * Current Time Characteristic and then perform database
                 * Characteristic Write procedure to write all or only selected
                 * fields to the database. In case if user want to ignore some
                 * fields of Current Time Characteristic user has to set the
                 * CY_BLE_GATT_ERR_CTS_DATA_FIELD_IGNORED error value in the
                 * "gattErrorCode" field of the structure passed from the with
                 * CY_BLE_EVT_CTSS_WRITE_CHAR event. In case if no fields in the
                 * Current Time Characteristic is ignored the "gattErrorCode"
                 * should be ignored.
                 */
                Cy_BLE_CTS_ApplCallback((uint32_t)CY_BLE_EVT_CTSS_WRITE_CHAR, (void*)&wrReqParam);

                if(wrReqParam.gattErrorCode != CY_BLE_GATT_ERR_CTS_DATA_FIELD_IGNORED)
                {
                    /* Ignore all gatt errors except permitted "Data Field Ignored" */
                    wrReqParam.gattErrorCode = CY_BLE_GATT_ERR_NONE;
                }
            }
            else
            {
                wrReqParam.gattErrorCode = CY_BLE_GATT_ERR_WRITE_NOT_PERMITTED;
            }

            /* Clear the callback flag indicating that request was handled */
            cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
        }
        else
        {
            /* This empty else statement is required by MISRA */
        }
    }

    return(wrReqParam.gattErrorCode);
}


/******************************************************************************
* Function Name: Cy_BLE_CTSS_SendNotification
***************************************************************************//**
*
*  Sends a notification with the characteristic value, as specified by its 
*  charIndex, to the client device.
*  On enabling notification successfully for a service characteristic it sends out a
*  'Handle Value Notification' which results in \ref CY_BLE_EVT_CTSC_NOTIFICATION event
*  at the GATT Client's end.
*
*  \param connHandle: The connection handle.
*  \param charIndex:  The index of the service characteristic of 
                      type \ref cy_en_ble_cts_char_index_t.
*  \param attrSize:   The size of the characteristic value attribute.
*  \param attrValue:  The pointer to the characteristic value data that should be
*                     sent to the client device.
*
*  \return
*  A return value of type \ref cy_en_ble_api_result_t.
*
*   Error Codes                              | Description
*   ------------                             | -----------
*   CY_BLE_SUCCESS                           | The request was handled successfully.
*   CY_BLE_ERROR_INVALID_PARAMETER           | Validation of the input parameter failed.
*   CY_BLE_ERROR_INVALID_OPERATION           | Operation is invalid for this characteristic.
*   CY_BLE_ERROR_INVALID_STATE               | Connection with the client is not established.
*   CY_BLE_ERROR_MEMORY_ALLOCATION_FAILED    | Memory allocation failed.
*   CY_BLE_ERROR_NTF_DISABLED                | Notification is not enabled by the client.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_CTSS_SendNotification(cy_stc_ble_conn_handle_t connHandle,
                                                    cy_en_ble_cts_char_index_t charIndex,
                                                    uint8_t attrSize,
                                                    uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult;

    if(CY_BLE_CTS_CURRENT_TIME != charIndex)
    {
        apiResult = CY_BLE_ERROR_INVALID_OPERATION;
    }
    else
    {
        apiResult = Cy_BLE_CTSS_SetCharacteristicValue(charIndex, attrSize, attrValue);
        if(apiResult == CY_BLE_SUCCESS)
        {
            if(Cy_BLE_GetConnectionState(connHandle) < CY_BLE_CONN_STATE_CONNECTED)
            {
                apiResult = CY_BLE_ERROR_INVALID_STATE;
            }
            else if((cy_ble_ctssConfigPtr->attrInfo->currTimeCccdHandle == CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE) ||
                    (!CY_BLE_IS_NOTIFICATION_ENABLED(connHandle.attId, cy_ble_ctssConfigPtr->attrInfo->currTimeCccdHandle)))
            {
                apiResult = CY_BLE_ERROR_NTF_DISABLED;
            }
            else
            {
                /* Fill all fields of Write Request structure ... */
                cy_stc_ble_gatts_handle_value_ntf_t ntfReqParam;
                
                ntfReqParam.handleValPair.attrHandle = cy_ble_ctssConfigPtr->attrInfo->currTimeCharHandle;
                ntfReqParam.handleValPair.value.val  = attrValue;
                ntfReqParam.handleValPair.value.len  = attrSize;
                ntfReqParam.connHandle               = connHandle;
                
                /* Send notification to client using previously filled structure */
                apiResult = Cy_BLE_GATTS_Notification(&ntfReqParam);
            }
        }
    }

    /* Return status */
    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_CTSC_DiscoverCharacteristicsEventHandler
***************************************************************************//**
*
*  This function is called on receiving a #CY_BLE_EVT_GATTC_READ_BY_TYPE_RSP
*  event. Based on the service UUID, an appropriate data structure is populated
*  using the data received as part of the callback.
*
*  \param discCharInfo: The pointer to a characteristic information structure.
*
******************************************************************************/
static void Cy_BLE_CTSC_DiscoverCharacteristicsEventHandler(cy_stc_ble_disc_char_info_t *discCharInfo)
{
    /* CTS characteristics UUIDs */
    static const cy_ble_uuid16_t cy_ble_ctsCharUuid[CY_BLE_CTS_CHAR_COUNT] =
    {
        CY_BLE_UUID_CHAR_CURRENT_TIME,
        CY_BLE_UUID_CHAR_LOCAL_TIME_INFO,
        CY_BLE_UUID_CHAR_REF_TIME_INFO
    };
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(discCharInfo->connHandle);
    uint32_t ctsDiscIdx = cy_ble_ctscConfigPtr->serviceDiscIdx;
    uint32_t i;

    if((discCharInfo->uuidFormat == CY_BLE_GATT_16_BIT_UUID_FORMAT) &&
       (cy_ble_configPtr->context->discovery[discIdx].servCount == ctsDiscIdx))
    {
        /* Get pointer (with offset) to CTS client structure with attribute handles */
        cy_stc_ble_ctsc_t *ctscPtr = (cy_stc_ble_ctsc_t *)&cy_ble_ctscConfigPtr->attrInfo[discIdx];
        
        for(i = 0u; i < (uint32_t)CY_BLE_CTS_CHAR_COUNT; i++)
        {
            if(cy_ble_ctsCharUuid[i] == discCharInfo->uuid.uuid16)
            {
                if(ctscPtr->currTimeCharacteristics[i].valueHandle == CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
                {
                    ctscPtr->currTimeCharacteristics[i].valueHandle = discCharInfo->valueHandle;
                    ctscPtr->currTimeCharacteristics[i].properties = discCharInfo->properties;
                }
                else
                {
                    Cy_BLE_ApplCallback((uint32_t)CY_BLE_EVT_GATTC_CHAR_DUPLICATION, &discCharInfo);
                }
            }
        }

        /* Indicate that request was handled */
        cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
    }
}


/******************************************************************************
* Function Name: Cy_BLE_CTSC_DiscoverCharDescriptorsEventHandler
***************************************************************************//**
*
*  This function is called on receiving a #CY_BLE_EVT_GATTC_FIND_INFO_RSP event.
*  This event is generated when a server successfully sends the data for
*  #CY_BLE_EVT_GATTC_FIND_INFO_REQ Based on the service UUID, an appropriate data
*  structure is populated to the service with a service callback.
*
*  \param discCharInfo: The pointer to a characteristic information structure.
*
******************************************************************************/
static void Cy_BLE_CTSC_DiscoverCharDescriptorsEventHandler(cy_stc_ble_disc_descr_info_t * discDescrInfo)
{
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(discDescrInfo->connHandle);
    uint32_t ctsDiscIdx = cy_ble_ctscConfigPtr->serviceDiscIdx;

    if(cy_ble_configPtr->context->discovery[discIdx].servCount == ctsDiscIdx)
    {
        if(discDescrInfo->uuid.uuid16 == CY_BLE_UUID_CHAR_CLIENT_CONFIG)
        {
            /* Get pointer (with offset) to CTS client structure with attribute handles */
            cy_stc_ble_ctsc_t *ctscPtr = (cy_stc_ble_ctsc_t *)&cy_ble_ctscConfigPtr->attrInfo[discIdx];

            if(ctscPtr->currTimeCccdHandle == CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
            {
                ctscPtr->currTimeCccdHandle = discDescrInfo->descrHandle;
            }
            else    /* Duplication of descriptor */
            {
                Cy_BLE_ApplCallback((uint32_t)CY_BLE_EVT_GATTC_DESCR_DUPLICATION, &discDescrInfo->uuid);
            }
        }

        /* Indicate that request was handled */
        cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
    }
}


/******************************************************************************
* Function Name: Cy_BLE_CTSC_GetCharRange
***************************************************************************//**
*
* Returns a possible range of the current characteristic descriptor via
* input parameter charRangeInfo->range
*
* \param *charRangeInfo: The pointer to a descriptor range information structure.
*
******************************************************************************/
static void Cy_BLE_CTSC_GetCharRange(cy_stc_ble_disc_range_info_t *charRangeInfo)
{
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(charRangeInfo->connHandle);
    uint32_t ctsDiscIdx = cy_ble_ctscConfigPtr->serviceDiscIdx;
    
    if(cy_ble_configPtr->context->discovery[discIdx].servCount == ctsDiscIdx)
    {
        uint32_t discServiNum = cy_ble_configPtr->context->discServiCount;
                
        /* Get pointer (with offset) to CTS client structure with attribute handles */
        cy_stc_ble_ctsc_t *ctscPtr = (cy_stc_ble_ctsc_t *)&cy_ble_ctscConfigPtr->attrInfo[discIdx];

        if(charRangeInfo->srviIncIdx != CY_BLE_DISCOVERY_INIT)
        {
            cy_ble_configPtr->context->discovery[discIdx].charCount++;
        }

        if(cy_ble_configPtr->context->discovery[discIdx].charCount == 0u)
        {
            /* Read characteristic range */
            charRangeInfo->range.startHandle = ctscPtr->currTimeCharacteristics[CY_BLE_CTS_CURRENT_TIME].
                                                valueHandle + 1u;
            charRangeInfo->range.endHandle =
                cy_ble_configPtr->context->serverInfo[(discIdx * discServiNum) + ctsDiscIdx].range.endHandle;
        }
        /* Indicate that request was handled */
        cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
    }
}


/******************************************************************************
* Function Name: Cy_BLE_CTSC_NotificationEventHandler
***************************************************************************//**
*
*  Handles the Notification event.
*
*  \param eventParam: The pointer to the cy_stc_ble_gattc_handle_value_ntf_param_t data
*              structure specified by the event.
*
******************************************************************************/
static void Cy_BLE_CTSC_NotificationEventHandler(cy_stc_ble_gattc_handle_value_ntf_param_t *eventParam)
{
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(eventParam->connHandle);

    if((discIdx < cy_ble_configPtr->params->maxClientCount) && (Cy_BLE_CTS_ApplCallback != NULL))
    {
        /* Get pointer (with offset) to CTS client structure with attribute handles */
        cy_stc_ble_ctsc_t *ctscPtr = (cy_stc_ble_ctsc_t *)&cy_ble_ctscConfigPtr->attrInfo[discIdx];
        
        if(ctscPtr->currTimeCharacteristics[CY_BLE_CTS_CURRENT_TIME].valueHandle ==
           eventParam->handleValPair.attrHandle)
        {
            cy_stc_ble_cts_char_value_t ntfParam;
           
            ntfParam.connHandle = eventParam->connHandle;
            ntfParam.charIndex  = CY_BLE_CTS_CURRENT_TIME;
            ntfParam.value      = &eventParam->handleValPair.value;
            
            Cy_BLE_CTS_ApplCallback((uint32_t)CY_BLE_EVT_CTSC_NOTIFICATION, (void*)&ntfParam);
            cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
        }
    }
}


/******************************************************************************
* Function Name: Cy_BLE_CTSC_ReadResponseEventHandler
***************************************************************************//**
*
*  Handles the Read Response event for the Current Time service.
*
*  \param eventParam: The pointer to the data that came with a read response for CTS.
*
******************************************************************************/
static void Cy_BLE_CTSC_ReadResponseEventHandler(cy_stc_ble_gattc_read_rsp_param_t *eventParam)
{
    uint32_t fFlag = 1u;
    uint32_t attrVal = 0u;
    cy_en_ble_cts_char_index_t idx = CY_BLE_CTS_CURRENT_TIME;

    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(eventParam->connHandle);

    if((discIdx < cy_ble_configPtr->params->maxClientCount) && (Cy_BLE_CTS_ApplCallback != NULL) &&
       (cy_ble_ctscReqHandle[discIdx] != CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE))
    {
        /* Get pointer (with offset) to CTS client structure with attribute handles */
        cy_stc_ble_ctsc_t *ctscPtr = (cy_stc_ble_ctsc_t *)&cy_ble_ctscConfigPtr->attrInfo[discIdx];

        if(ctscPtr->currTimeCharacteristics[CY_BLE_CTS_CURRENT_TIME].valueHandle ==
           cy_ble_ctscReqHandle[discIdx])
        {
            idx = CY_BLE_CTS_CURRENT_TIME;
        }
        else
            if(ctscPtr->currTimeCharacteristics[CY_BLE_CTS_LOCAL_TIME_INFO].valueHandle ==
               cy_ble_ctscReqHandle[discIdx])
            {
                idx = CY_BLE_CTS_LOCAL_TIME_INFO;
            }
            else
                if(ctscPtr->currTimeCharacteristics[CY_BLE_CTS_REFERENCE_TIME_INFO].valueHandle ==
                   cy_ble_ctscReqHandle[discIdx])
                {
                    idx = CY_BLE_CTS_REFERENCE_TIME_INFO;
                }
                else if(ctscPtr->currTimeCccdHandle == cy_ble_ctscReqHandle[discIdx])
                {
                    /* Attribute is characteristic descriptor  */
                    attrVal = 1u;
                }
                else
                {
                    /* No CTS characteristics were requested for read */
                    fFlag = 0u;
                }

        if(fFlag != 0u)
        {
            /* Read response for characteristic */
            if(attrVal == 0u)
            {
                /* Fill Current Time service read response parameter structure with
                 * characteristic info. */
                cy_stc_ble_cts_char_value_t rdRspParam = { .connHandle = eventParam->connHandle };
                rdRspParam.charIndex = idx;
                rdRspParam.value = &eventParam->value;

                Cy_BLE_CTS_ApplCallback((uint32_t)CY_BLE_EVT_CTSC_READ_CHAR_RESPONSE, (void*)&rdRspParam);
            }
            else /* Read response for characteristic descriptor */
            {
                /* Fill Current Time service read response parameter structure with
                 *  characteristic descriptor info. */
                cy_stc_ble_cts_descr_value_t rdRspParam;
                
                rdRspParam.connHandle = eventParam->connHandle;
                rdRspParam.charIndex  = CY_BLE_CTS_CURRENT_TIME;
                rdRspParam.descrIndex = CY_BLE_CTS_CURRENT_TIME_CCCD;
                rdRspParam.value      = &eventParam->value;
                
                Cy_BLE_CTS_ApplCallback((uint32_t)CY_BLE_EVT_CTSC_READ_DESCR_RESPONSE, (void*)&rdRspParam);
            }

            cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
            cy_ble_ctscReqHandle[discIdx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
        }
    }
}


/******************************************************************************
* Function Name: Cy_BLE_CTSC_WriteResponseEventHandler
***************************************************************************//**
*
*  Handles the Write Response event for the Current Time service.
*
*  \param eventParam: The pointer to the cy_stc_ble_conn_handle_t data structure.
*
******************************************************************************/
static void Cy_BLE_CTSC_WriteResponseEventHandler(const cy_stc_ble_conn_handle_t *eventParam)
{
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(*eventParam);

    if((discIdx < cy_ble_configPtr->params->maxClientCount) && (Cy_BLE_CTS_ApplCallback != NULL) &&
       (cy_ble_ctscReqHandle[discIdx] != CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE))
    {
        /* Get pointer (with offset) to CTS client structure with attribute handles */
        cy_stc_ble_ctsc_t *ctscPtr = (cy_stc_ble_ctsc_t *)&cy_ble_ctscConfigPtr->attrInfo[discIdx];

        if(ctscPtr->currTimeCccdHandle == cy_ble_ctscReqHandle[discIdx])
        {
            cy_stc_ble_cts_descr_value_t wrRspParam;
            
            wrRspParam.connHandle = *eventParam;
            wrRspParam.charIndex  = CY_BLE_CTS_CURRENT_TIME;
            wrRspParam.descrIndex = CY_BLE_CTS_CURRENT_TIME_CCCD;
            wrRspParam.value      = NULL;
            
            cy_ble_ctscReqHandle[discIdx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
            cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
            Cy_BLE_CTS_ApplCallback((uint32_t)CY_BLE_EVT_CTSC_WRITE_DESCR_RESPONSE, (void*)&wrRspParam);
        }
        else if((ctscPtr->currTimeCharacteristics[CY_BLE_CTS_CURRENT_TIME].valueHandle ==
                 cy_ble_ctscReqHandle[discIdx]) ||
                (ctscPtr->currTimeCharacteristics[CY_BLE_CTS_LOCAL_TIME_INFO].valueHandle ==
                 cy_ble_ctscReqHandle[discIdx]))
        {
            cy_stc_ble_cts_char_value_t wrRspParam;
            if(ctscPtr->currTimeCharacteristics[CY_BLE_CTS_CURRENT_TIME].valueHandle ==
               cy_ble_ctscReqHandle[discIdx])
            {
                wrRspParam.charIndex = CY_BLE_CTS_CURRENT_TIME;
            }
            else
            {
                wrRspParam.charIndex = CY_BLE_CTS_LOCAL_TIME_INFO;
            }
            wrRspParam.connHandle = *eventParam;
            wrRspParam.value = NULL;

            cy_ble_ctscReqHandle[discIdx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
            cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
            Cy_BLE_CTS_ApplCallback((uint32_t)CY_BLE_EVT_CTSC_WRITE_CHAR_RESPONSE, (void*)&wrRspParam);
        }
        else
        {
        }
    }
}


/******************************************************************************
* Function Name: Cy_BLE_CTSC_ErrorResponseEventHandler
***************************************************************************//**
*
*  Handles the Error Response event for the Current Time service.
*
*  \param eventParam: The pointer to the cy_stc_ble_gatt_err_param_t structure.
*
******************************************************************************/
static void Cy_BLE_CTSC_ErrorResponseEventHandler(const cy_stc_ble_gatt_err_param_t *eventParam)
{
    if(eventParam != NULL)
    {
        uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(eventParam->connHandle);
        if(eventParam->errInfo.attrHandle == cy_ble_ctscReqHandle[discIdx])
        {
            cy_ble_ctscReqHandle[discIdx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
        }
    }
}

/******************************************************************************
* Function Name: Cy_BLE_CTSC_SetCharacteristicValue
***************************************************************************//**
*
*  This function is used to write the characteristic (which is identified by
*  charIndex) value attribute in the server. As a result a Write Request is
*  sent to the GATT Server and on successful execution of the request on the
*  server side, the \ref CY_BLE_EVT_CTSS_WRITE_CHAR event is generated.
*  On successful request execution on the server side, the Write Response is
*  sent to the client.
*
*  \param connHandle: The connection handle.
*  \param charIndex:  The index of the service characteristic of 
                      type \ref cy_en_ble_cts_char_index_t.
*  \param attrSize:   The size of the characteristic descriptor attribute.
*  \param attrValue:  Pointer to the characteristic value data that should be
*                     sent to the server device.
*
*  \return
*  A return value of type \ref cy_en_ble_api_result_t.
*    
*   Error Codes                              | Description
*   ------------                             | -----------
*   CY_BLE_SUCCESS                           | The request was sent successfully.
*   CY_BLE_ERROR_INVALID_PARAMETER           | Validation of the input parameter failed.
*   CY_BLE_ERROR_INVALID_OPERATION           | Operation is invalid for this characteristic.
*   CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE | The peer device doesn't have the particular characteristic.
*   CY_BLE_ERROR_INVALID_STATE               | Connection with the server is not established.
*   CY_BLE_ERROR_MEMORY_ALLOCATION_FAILED    | Memory allocation failed.
*
* \events
*   In case of successful execution (return value = #CY_BLE_SUCCESS)
*   the following events can appear: \n
*   If the CTS service-specific callback is registered
*      with \ref Cy_BLE_CTS_RegisterAttrCallback():
*   * #CY_BLE_EVT_CTSC_WRITE_CHAR_RESPONSE - In case if the requested attribute is
*     successfully written on the peer device, the details 
*     (charIndex, etc.) are provided with event parameter structure
*     of type \ref cy_stc_ble_cts_char_value_t.
*   .
*   Otherwise (if the CTS service-specific callback is not registered):
*   * #CY_BLE_EVT_GATTC_WRITE_RSP - In case if the requested attribute is
*     successfully written on the peer device.
*
*   * #CY_BLE_EVT_GATTC_ERROR_RSP - In case if an error occurred with the
*     requested attribute on the peer device, the details are provided with event
*     parameters structure \ref cy_stc_ble_gatt_err_param_t.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_CTSC_SetCharacteristicValue(cy_stc_ble_conn_handle_t connHandle,
                                                          cy_en_ble_cts_char_index_t charIndex,
                                                          uint8_t attrSize,
                                                          uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult = CY_BLE_SUCCESS;
    cy_stc_ble_gattc_write_req_t writeReqParam;
    
    writeReqParam.handleValPair.value.val = attrValue;
    writeReqParam.handleValPair.value.len = attrSize;
    writeReqParam.connHandle              = connHandle;
    

    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(connHandle);

    /* Get pointer (with offset) to CTS client structure with attribute handles */
    cy_stc_ble_ctsc_t *ctscPtr = (cy_stc_ble_ctsc_t *)&cy_ble_ctscConfigPtr->attrInfo[discIdx];

    if(Cy_BLE_GetConnectionState(connHandle) != CY_BLE_CONN_STATE_CLIENT_DISCOVERED)
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }
    else if(discIdx >= cy_ble_configPtr->params->maxClientCount)
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else
    {
        /* Select characteristic */
        switch(charIndex)
        {
            case CY_BLE_CTS_CURRENT_TIME:
                writeReqParam.handleValPair.attrHandle = ctscPtr->
                                                          currTimeCharacteristics[CY_BLE_CTS_CURRENT_TIME].valueHandle;
                break;

            case CY_BLE_CTS_LOCAL_TIME_INFO:
                writeReqParam.handleValPair.attrHandle = ctscPtr->
                                                          currTimeCharacteristics[CY_BLE_CTS_LOCAL_TIME_INFO].valueHandle;
                break;

            case CY_BLE_CTS_REFERENCE_TIME_INFO:
                /* Reference Time Information is read only */
                apiResult = CY_BLE_ERROR_INVALID_OPERATION;
                break;

            default:
                /* Characteristic wasn't found */
                apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
                break;
        }

        if(apiResult == CY_BLE_SUCCESS)
        {
            if(writeReqParam.handleValPair.attrHandle != CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
            {
                /* Send request to read characteristic value */
                apiResult = Cy_BLE_GATTC_WriteCharacteristicValue(&writeReqParam);

                if(apiResult == CY_BLE_SUCCESS)
                {
                    cy_ble_ctscReqHandle[discIdx] = writeReqParam.handleValPair.attrHandle;
                }
            }
            else
            {
                apiResult = CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE;
            }
        }
    }

    /* Return status */
    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_CTSC_GetCharacteristicValue
***************************************************************************//**
*
*  Gets a characteristic value of the Current Time service, which is identified
*  by charIndex.
*
*  \param connHandle: The connection handle.
*  \param charIndex:  The index of the service characteristic of 
*                     type \ref cy_en_ble_cts_char_index_t.
*
*  \return
*  A return value of type \ref cy_en_ble_api_result_t.
*    
*   Error Codes                              | Description
*   ------------                             | -----------
*   CY_BLE_SUCCESS                           | The request was sent successfully.
*   CY_BLE_ERROR_INVALID_PARAMETER           | Validation of the input parameter failed.
*   CY_BLE_ERROR_INVALID_OPERATION           | Operation is invalid for this characteristic.
*   CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE | The peer device doesn't have the particular characteristic.
*   CY_BLE_ERROR_INVALID_STATE               | Connection with the server is not established.
*   CY_BLE_ERROR_MEMORY_ALLOCATION_FAILED    | Memory allocation failed.
*
* \events
*   In case of successful execution (return value = #CY_BLE_SUCCESS)
*   the following events can appear: \n
*   If the CTS service-specific callback is registered
*   with \ref Cy_BLE_CTS_RegisterAttrCallback():
*   * #CY_BLE_EVT_CTSC_READ_CHAR_RESPONSE - In case if the requested attribute is
*     successfully read on the peer device, the details 
*     (charIndex , value, etc.) are provided with event parameter structure
*     of type \ref cy_stc_ble_cts_char_value_t.
*   .
*   Otherwise (if an CTS service-specific callback is not registered):
*   * #CY_BLE_EVT_GATTC_READ_RSP - If the requested attribute is
*     successfully read on the peer device, the details (handle, value, etc.) are
*     provided with an event parameter structure 
*     \ref cy_stc_ble_gattc_read_rsp_param_t.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_CTSC_GetCharacteristicValue(cy_stc_ble_conn_handle_t connHandle,
                                                          cy_en_ble_cts_char_index_t charIndex)
{
    cy_stc_ble_gattc_read_req_t readReqParam;
    cy_en_ble_api_result_t apiResult = CY_BLE_SUCCESS;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(connHandle);

    readReqParam.connHandle = connHandle;
    
    if(Cy_BLE_GetConnectionState(connHandle) != CY_BLE_CONN_STATE_CLIENT_DISCOVERED)
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }
    else if(discIdx >= cy_ble_configPtr->params->maxClientCount)
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else
    {
        /* Get pointer (with offset) to CTS client structure with attribute handles */
        cy_stc_ble_ctsc_t *ctscPtr = (cy_stc_ble_ctsc_t *)&cy_ble_ctscConfigPtr->attrInfo[discIdx];

        /* Select characteristic */
        switch(charIndex)
        {
            case CY_BLE_CTS_CURRENT_TIME:
                readReqParam.attrHandle = ctscPtr->currTimeCharacteristics[CY_BLE_CTS_CURRENT_TIME].valueHandle;
                break;

            case CY_BLE_CTS_LOCAL_TIME_INFO:
                readReqParam.attrHandle = ctscPtr->currTimeCharacteristics[CY_BLE_CTS_LOCAL_TIME_INFO].
                                           valueHandle;
                break;

            case CY_BLE_CTS_REFERENCE_TIME_INFO:
                readReqParam.attrHandle = ctscPtr->currTimeCharacteristics[CY_BLE_CTS_REFERENCE_TIME_INFO].
                                           valueHandle;
                break;

            default:
                /* Characteristic wasn't found */
                apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
                break;
        }

        if(apiResult == CY_BLE_SUCCESS)
        {
            if(readReqParam.attrHandle != CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
            {
                /* Send request to read characteristic value */
                apiResult = Cy_BLE_GATTC_ReadCharacteristicValue(&readReqParam);

                if(apiResult == CY_BLE_SUCCESS)
                {
                    cy_ble_ctscReqHandle[discIdx] = readReqParam.attrHandle;
                }
            }
            else
            {
                apiResult = CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE;
            }
        }
    }

    /* Return status */
    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_CTSC_SetCharacteristicDescriptor
***************************************************************************//**
*
*  Sets a characteristic descriptor of the Current Time Characteristic of the
*  Current Time service.
*
*  Internally, Write Request is sent to the GATT Server and on successful
*  execution of the request on the server side, the following events can be
*  generated:
*    * \ref CY_BLE_EVT_CTSS_NOTIFICATION_ENABLED
*    * \ref CY_BLE_EVT_CTSS_NOTIFICATION_DISABLED
*
*  \param connHandle: The connection handle.
*  \param charIndex:  The index of the service characteristic of type 
*                     \ref cy_en_ble_cts_char_index_t.
*  \param descrIndex: The index of the service characteristic descriptor of type
*                     \ref cy_en_ble_cts_char_descriptors_t.
*  \param attrSize:   The size of the characteristic descriptor attribute.
*  \param attrValue:  Pointer to the characteristic descriptor value data that should be
*                     sent to the server device.
*
*  \return
*  A return value of type \ref cy_en_ble_api_result_t.
*
*   Error Codes                              | Description
*   ------------                             | -----------
*   CY_BLE_SUCCESS                           | The request was sent successfully.
*   CY_BLE_ERROR_INVALID_PARAMETER           | Validation of the input parameter failed.
*   CY_BLE_ERROR_INVALID_OPERATION           | Operation is invalid for this characteristic.
*   CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE | The peer device doesn't have the particular characteristic.
*   CY_BLE_ERROR_INVALID_STATE               | The state is not valid.
*   CY_BLE_ERROR_MEMORY_ALLOCATION_FAILED    | Memory allocation failed.
*
* \events
*   In case of successful execution (return value = #CY_BLE_SUCCESS)
*   the following events can appear: \n
*   If the CTS service-specific callback is registered
*   with \ref Cy_BLE_CTS_RegisterAttrCallback():
*   * #CY_BLE_EVT_CTSC_WRITE_DESCR_RESPONSE - In case if the requested attribute is
*     successfully written on the peer device, the details 
*     (charIndex, descrIndex etc.) are provided with event parameter structure
*     of type \ref cy_stc_ble_cts_descr_value_t.
*   .
*   Otherwise (if an CTS service-specific callback is not registered):
*   * #CY_BLE_EVT_GATTC_WRITE_RSP - If the requested attribute is
*     successfully written on the peer device.
*
*   * #CY_BLE_EVT_GATTC_ERROR_RSP - If an error occurred with the
*     requested attribute on the peer device, the details are provided with 
*     an event parameter structure ( \ref cy_stc_ble_gatt_err_param_t).
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_CTSC_SetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                               cy_en_ble_cts_char_index_t charIndex,
                                                               cy_en_ble_cts_char_descriptors_t descrIndex,
                                                               uint8_t attrSize,
                                                               uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(connHandle);

    if(Cy_BLE_GetConnectionState(connHandle) != CY_BLE_CONN_STATE_CLIENT_DISCOVERED)
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }
    else if((discIdx < cy_ble_configPtr->params->maxClientCount) && (attrValue != NULL) &&
            (charIndex == CY_BLE_CTS_CURRENT_TIME) && (descrIndex == CY_BLE_CTS_CURRENT_TIME_CCCD) && 
            (attrSize == CY_BLE_CCCD_LEN))
    {
        /* Get pointer (with offset) to CTS client structure with attribute handles */
        cy_stc_ble_ctsc_t *ctscPtr = (cy_stc_ble_ctsc_t *)&cy_ble_ctscConfigPtr->attrInfo[discIdx];

        if(ctscPtr->currTimeCccdHandle != CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
        {
            /* Fill all fields of Write Request structure ... */
            cy_stc_ble_gattc_write_req_t writeReqParam;
            
            writeReqParam.handleValPair.value.val  = attrValue;
            writeReqParam.handleValPair.value.len  = attrSize;
            writeReqParam.handleValPair.attrHandle = ctscPtr->currTimeCccdHandle;
            writeReqParam.connHandle               = connHandle;
            
            /* ... and send request to server device. */
            apiResult = Cy_BLE_GATTC_WriteCharacteristicDescriptors(&writeReqParam);

            if(apiResult == CY_BLE_SUCCESS)
            {
                cy_ble_ctscReqHandle[discIdx] = ctscPtr->currTimeCccdHandle;
            }
        }
        else
        {
            apiResult = CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE;
        }
    }
    else
    {
        /* Parameter validation failed */
    }

    /* Return the status */
    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_CTSC_GetCharacteristicDescriptor
***************************************************************************//**
*
*  Gets a characteristic descriptor of the Current Time Characteristic of the
*  Current Time service.
*
*  \param connHandle: The connection handle.
*  \param charIndex:  The index of the service characteristic of type 
*                     \ref cy_en_ble_cts_char_index_t.
*  \param descrIndex: The index of the service characteristic descriptor of type
*                     \ref cy_en_ble_cts_char_descriptors_t.
*
*  \return
*   A return value of type \ref cy_en_ble_api_result_t.
*
*   Error Codes                              | Description
*   ------------                             | -----------
*   CY_BLE_SUCCESS                           | The request was sent successfully.
*   CY_BLE_ERROR_INVALID_PARAMETER           | Validation of the input parameter failed.
*   CY_BLE_ERROR_INVALID_OPERATION           | This operation is not permitted on the specified attribute.
*   CY_BLE_ERROR_INVALID_STATE               | The state is not valid.
*   CY_BLE_ERROR_MEMORY_ALLOCATION_FAILED    | Memory allocation failed.
*   CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE | Peer device doesn't have a particular descriptor.
*
* \events
*   In case of successful execution (return value = #CY_BLE_SUCCESS)
*   the following events can appear: \n
*   If the CTS service-specific callback is registered
*   with \ref Cy_BLE_CTS_RegisterAttrCallback():
*   * #CY_BLE_EVT_CTSC_READ_DESCR_RESPONSE - In case if the requested attribute is
*     successfully read on the peer device, the details (charIndex,
*     descrIndex, value, etc.) are provided with event parameter structure
*     of type \ref cy_stc_ble_cts_descr_value_t.
*   .
*   Otherwise (if an CTS service-specific callback is not registered):
*   * #CY_BLE_EVT_GATTC_READ_RSP - If the requested attribute is
*     successfully read on the peer device, the details (handle, value, etc.) are
*     provided with an event parameter structure 
*     \ref cy_stc_ble_gattc_read_rsp_param_t.
*
*   * #CY_BLE_EVT_GATTC_ERROR_RSP - If an error occurred with the
*     requested attribute on the peer device, the details are provided with 
*     an event parameter structure \ref cy_stc_ble_gatt_err_param_t.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_CTSC_GetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                               cy_en_ble_cts_char_index_t charIndex,
                                                               uint8_t descrIndex)
{
    cy_en_ble_api_result_t apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(connHandle);

    if(Cy_BLE_GetConnectionState(connHandle) != CY_BLE_CONN_STATE_CLIENT_DISCOVERED)
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }
    else if((discIdx < cy_ble_configPtr->params->maxClientCount) && (charIndex == CY_BLE_CTS_CURRENT_TIME) &&
            (descrIndex == ((uint8_t)CY_BLE_CTS_CURRENT_TIME_CCCD)))
    {
        /* Get pointer (with offset) to CTS client structure with attribute handles */
        cy_stc_ble_ctsc_t *ctscPtr = (cy_stc_ble_ctsc_t *)&cy_ble_ctscConfigPtr->attrInfo[discIdx];

        if(ctscPtr->currTimeCccdHandle != CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
        {
            cy_stc_ble_gattc_read_req_t readReqParam;
            
            readReqParam.attrHandle = ctscPtr->currTimeCccdHandle;
            readReqParam.connHandle = connHandle;
            
            apiResult = Cy_BLE_GATTC_ReadCharacteristicDescriptors(&readReqParam);

            if(apiResult == CY_BLE_SUCCESS)
            {
                cy_ble_ctscReqHandle[discIdx] = ctscPtr->currTimeCccdHandle;
            }
        }
        else
        {
            apiResult = CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE;
        }
    }
    else
    {
        /* Validation of input parameters failed */
    }

    /* Return status */
    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_CTS_EventHandler
***************************************************************************//**
*
*  Handles the events from the BLE Stack for the CTS service.
*
*  \param eventCode:  the event code
*  \param eventParam: the event parameters
*
*  \return
*   A return value of type cy_en_ble_gatt_err_code_t.
*
******************************************************************************/
static cy_en_ble_gatt_err_code_t Cy_BLE_CTS_EventHandler(uint32_t eventCode,
                                                          void *eventParam)
{
    cy_en_ble_gatt_err_code_t gattErr = CY_BLE_GATT_ERR_NONE;

    /************************************
     * Handling GATT Server events
     ***********************************/
    if(Cy_BLE_CTSS_EventHandlerCallback != NULL)
    {
        gattErr = Cy_BLE_CTSS_EventHandlerCallback(eventCode, eventParam);
    }

    /************************************
     * Handling GATT Client events
     ***********************************/
    if(Cy_BLE_CTSC_EventHandlerCallback != NULL)
    {
        (void)Cy_BLE_CTSC_EventHandlerCallback(eventCode, eventParam);
    }

    return(gattErr);
}


/******************************************************************************
* Function Name: Cy_BLE_CTSS_EventHandler
***************************************************************************//**
*
*  Handles the server events from the BLE Stack for the CTS service
*
*  \param eventCode:  the event code
*  \param eventParam: the event parameters
*
*  \return
*   A return value of type cy_en_ble_gatt_err_code_t.
*
******************************************************************************/
static cy_en_ble_gatt_err_code_t Cy_BLE_CTSS_EventHandler(uint32_t eventCode, void *eventParam)
{
    cy_en_ble_gatt_err_code_t gattErr = CY_BLE_GATT_ERR_NONE;

    /* Handling GATT Server events */
    switch((cy_en_ble_event_t)eventCode)
    {
        case CY_BLE_EVT_GATTS_WRITE_REQ:
            gattErr = Cy_BLE_CTSS_WriteEventHandler((cy_stc_ble_gatts_write_cmd_req_param_t*)eventParam);
            break;

        default:
        break;
    }

    return (gattErr);
}


/******************************************************************************
* Function Name: Cy_BLE_CTSC_EventHandler
***************************************************************************//**
*
*  Handles the client events from the BLE Stack for the CTS service
*
*  \param eventCode:  the event code
*  \param eventParam: the event parameters
*
*  \return
*   A return value of type cy_en_ble_gatt_err_code_t.
*
******************************************************************************/
static cy_en_ble_gatt_err_code_t Cy_BLE_CTSC_EventHandler(uint32_t eventCode, void *eventParam)
{
    cy_en_ble_gatt_err_code_t gattErr = CY_BLE_GATT_ERR_NONE;

    if(eventCode > (uint32_t)CY_BLE_EVT_MAX)
    {
        /* Handling service-specific events */
        switch((cy_en_ble_evt_t)eventCode)
        {
            /* Discovery events */
            case CY_BLE_EVT_GATTC_DISC_CHAR:
                Cy_BLE_CTSC_DiscoverCharacteristicsEventHandler((cy_stc_ble_disc_char_info_t*)eventParam);
                break;

            case CY_BLE_EVT_GATTC_DISC_DESCR:
                Cy_BLE_CTSC_DiscoverCharDescriptorsEventHandler((cy_stc_ble_disc_descr_info_t*)eventParam);
                break;

            case CY_BLE_EVT_GATTC_DISC_DESCR_GET_RANGE:
                Cy_BLE_CTSC_GetCharRange((cy_stc_ble_disc_range_info_t*)eventParam);
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
            case CY_BLE_EVT_GATTC_ERROR_RSP:
            {
                uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(((cy_stc_ble_gatt_err_param_t*)eventParam)->connHandle);
                if((discIdx < cy_ble_configPtr->params->maxClientCount) && (cy_ble_configPtr->context->discovery[discIdx].autoDiscoveryFlag == 0u) &&
                   (((cy_stc_ble_gatt_err_param_t*)eventParam)->errInfo.errorCode !=
                    CY_BLE_GATT_ERR_ATTRIBUTE_NOT_FOUND))
                {
                    Cy_BLE_CTSC_ErrorResponseEventHandler((cy_stc_ble_gatt_err_param_t*)eventParam);
                }
            }
            break;

            case CY_BLE_EVT_GATTC_HANDLE_VALUE_NTF:
                Cy_BLE_CTSC_NotificationEventHandler((cy_stc_ble_gattc_handle_value_ntf_param_t*)eventParam);
                break;

            case CY_BLE_EVT_GATTC_READ_RSP:
                Cy_BLE_CTSC_ReadResponseEventHandler((cy_stc_ble_gattc_read_rsp_param_t*)eventParam);
                break;

            case CY_BLE_EVT_GATTC_WRITE_RSP:
                Cy_BLE_CTSC_WriteResponseEventHandler((cy_stc_ble_conn_handle_t*)eventParam);
                break;

            default:
                break;
        }
    }

    return(gattErr);
}

#endif /* CY_BLE_LIB_HOST_CORE */
#endif /* CY_IP_MXBLESS */


/* [] END OF FILE */
