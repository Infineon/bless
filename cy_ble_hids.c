/***************************************************************************//**
* \file cy_ble_hids.c
* \version 3.20
*
* \brief
*  Contains the source code for the HID service.
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

#define CY_BLE_HIDSS_CHAR_COUNT      ((uint8_t)CY_BLE_HIDS_CHAR_COUNT + cy_ble_hidssConfigPtr->reportCount)
#define CY_BLE_HIDSC_CHAR_COUNT      ((uint8_t)CY_BLE_HIDS_CHAR_COUNT + cy_ble_hidscConfigPtr->reportCount)
#define CY_BLE_HIDSS_REPORT_END      ((uint8_t)CY_BLE_HIDS_REPORT + cy_ble_hidssConfigPtr->reportCount)
#define CY_BLE_HIDSC_REPORT_END      ((uint8_t)CY_BLE_HIDS_REPORT + cy_ble_hidscConfigPtr->reportCount)


/*******************************************************************************
* Private Function Prototypes
*******************************************************************************/

static cy_en_ble_gatt_err_code_t Cy_BLE_HIDS_EventHandler(uint32_t eventCode, void *eventParam);
static cy_en_ble_gatt_err_code_t Cy_BLE_HIDSS_EventHandler(uint32_t eventCode, void *eventParam);
static cy_en_ble_gatt_err_code_t Cy_BLE_HIDSC_EventHandler(uint32_t eventCode, void *eventParam);

static cy_en_ble_gatt_err_code_t Cy_BLE_HIDSS_WriteEventHandler(cy_stc_ble_gatts_write_cmd_req_param_t *eventParam);
static uint32_t Cy_BLE_HIDSS_CccdWriteEventHandler(const cy_stc_ble_gatts_write_cmd_req_param_t *eventParam);
static void Cy_BLE_HIDSS_OnDeviceConnected(void);

static void Cy_BLE_HIDSC_DiscoverCharacteristicsEventHandler(cy_stc_ble_disc_char_info_t *discCharInfo);
static void Cy_BLE_HIDSC_DiscoverCharDescriptorsEventHandler(cy_stc_ble_disc_descr_info_t *discDescrInfo);
static void Cy_BLE_HIDSC_InclDiscoveryEventHandler(const cy_stc_ble_disc_incl_info_t *discInclInfo);
static void Cy_BLE_HIDSC_GetCharRange(cy_stc_ble_disc_range_info_t *charRangeInfo);
static void Cy_BLE_HIDSC_NotificationEventHandler(cy_stc_ble_gattc_handle_value_ntf_param_t *eventParam);
static void Cy_BLE_HIDSC_ReadResponseEventHandler(cy_stc_ble_gattc_read_rsp_param_t *eventParam);
static void Cy_BLE_HIDSC_WriteResponseEventHandler(const cy_stc_ble_conn_handle_t *eventParam);
static void Cy_BLE_HIDSC_ErrorResponseEventHandler(const cy_stc_ble_gatt_err_param_t *eventParam);


/*******************************************************************************
* Global Variables
*******************************************************************************/

/* The pointers to the callback functions */
static cy_ble_callback_t Cy_BLE_HIDS_ApplCallback;
static cy_ble_event_handler_cb_t Cy_BLE_HIDSC_EventHandlerCallback = NULL;
static cy_ble_event_handler_cb_t Cy_BLE_HIDSS_EventHandlerCallback = NULL;

/* The internal storage for the last request handle to check response */
static cy_ble_gatt_db_attr_handle_t cy_ble_hidscReqHandle[CY_BLE_MAX_CONNECTION_INSTANCES];

/* The pointer to a global BLE HIDS server config structure */
const cy_stc_ble_hidss_config_t *cy_ble_hidssConfigPtr = NULL;

/* The pointer to a global BLE HIDS client config structure */
const cy_stc_ble_hidsc_config_t *cy_ble_hidscConfigPtr = NULL;


/******************************************************************************
* Function Name: Cy_BLE_HIDSS_Init
***************************************************************************//**
*
*  This function initializes server of the HID service.
*
*  \param config: Configuration structure for the HID.
*
*  \return
*  \ref cy_en_ble_api_result_t : Return value indicates whether the function succeeded or
*    failed. The following are possible error codes.
*
*   Error Codes                             | Description
*   ------------                            | -----------
*   CY_BLE_SUCCESS                          | The function completed successfully.
*   CY_BLE_ERROR_INVALID_PARAMETER          | On specifying NULL as input parameter.
*   CY_BLE_ERROR_MEMORY_ALLOCATION_FAILED   | Buffer overflow in the registration callback.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_HIDSS_Init(const cy_stc_ble_hidss_config_t *config)
{
    cy_en_ble_api_result_t apiResult;

    if(config == NULL)
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else
    {
        /* Registers a pointer to config structure */
        cy_ble_hidssConfigPtr = config;

        /* Registers event handler for the HID */
        apiResult = Cy_BLE_RegisterServiceEventHandler(&Cy_BLE_HIDS_EventHandler);

        /* Registers a pointer to server event handler function */
        Cy_BLE_HIDSS_EventHandlerCallback = &Cy_BLE_HIDSS_EventHandler;
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_HIDSC_Init
***************************************************************************//**
*
*  This function initializes client of the HID service.
*
*  \param config: Configuration structure for the HID.
*
*  \return
*  \ref cy_en_ble_api_result_t : Return value indicates whether the function succeeded or
*    failed. The following are possible error codes.
*
*   Error Codes                             | Description
*   ------------                            | -----------
*   CY_BLE_SUCCESS                          | The function completed successfully.
*   CY_BLE_ERROR_INVALID_PARAMETER          | On specifying NULL as input parameter.
*   CY_BLE_ERROR_MEMORY_ALLOCATION_FAILED   | Buffer overflow in the registration callback.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_HIDSC_Init(const cy_stc_ble_hidsc_config_t *config)
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
        cy_ble_hidscConfigPtr = config;

        /* Registers event handler for the HID */
        apiResult = Cy_BLE_RegisterServiceEventHandler(&Cy_BLE_HIDS_EventHandler);

        /* Registers a pointer to client event handler function */
        Cy_BLE_HIDSC_EventHandlerCallback = &Cy_BLE_HIDSC_EventHandler;

        /* Init GATT client part */
        for(idx = 0u; idx < cy_ble_configPtr->params->maxClientCount; idx++)
        {
            uint32_t locServIdx;

            for(locServIdx = 0u; locServIdx < cy_ble_hidscConfigPtr->serviceCount; locServIdx++)
            {
                uint32 servCnt     = cy_ble_configPtr->context->discServiCount;
                uint32 hidsServIdx = cy_ble_hidscConfigPtr->serviceDiscIdx + locServIdx;

                if(cy_ble_configPtr->context->serverInfo[(idx * servCnt) + hidsServIdx].range.startHandle ==
                   CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
                {
                    /* Get pointer (with offset) to HIDS client structure with attribute handles */
                    cy_stc_ble_hidsc_t *hidscPtr = (cy_stc_ble_hidsc_t *)&cy_ble_hidscConfigPtr->
                                                    attrInfo[(idx * cy_ble_hidscConfigPtr->serviceCount) + locServIdx];

                    /* Get pointer to HIDS client structure */
                    (void)memset(hidscPtr, 0, sizeof(cy_stc_ble_hidsc_t));

                    /* initialize uuid  */
                    cy_ble_configPtr->context->serverInfo[(idx * servCnt) + hidsServIdx].uuid =
                        CY_BLE_UUID_HIDS_SERVICE;
                }
            }
            cy_ble_hidscReqHandle[idx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
        }
    }
    return (apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_HIDS_RegisterAttrCallback
***************************************************************************//**
*
*  Registers a callback function for HID service-specific attribute operations.
*  Service-specific Write Requests from peer device will not be handled with
*  unregistered callback function.
*
*  \param callbackFunc:  An application layer event callback function to receive
*    events from the PSoC 6 BLE Middleware. The definition of \ref cy_ble_callback_t
*    for HID service is:<br>
*    typedef void (* cy_ble_callback_t) (uint32_t eventCode, void *eventParam),
*    where:
*      * eventCode indicates the event that triggered this callback.
*        (e.g. #CY_BLE_EVT_HIDSS_NOTIFICATION_ENABLED).
*      * eventParam: Contains the parameters corresponding to the
*        current event; (e.g. pointer to \ref cy_stc_ble_hids_char_value_t
*        structure that contains details of the characteristic
*        for which the notification enabled event was triggered).
*
*  \sideeffect
*  The *eventParams in the callback function should not be used by the
*  application once the callback function execution is finished. Otherwise
*  this data may become corrupted.
*
******************************************************************************/
void Cy_BLE_HIDS_RegisterAttrCallback(cy_ble_callback_t callbackFunc)
{
    Cy_BLE_HIDS_ApplCallback = callbackFunc;
}


/******************************************************************************
* Function Name: Cy_BLE_HIDSS_SetCharacteristicValue
***************************************************************************//**
*
*  Sets a characteristic value of HID service, which is a value
*  identified by charIndex, to the local database.
*
*  \param serviceIndex: The index of the service instance. e.g. If two HID services are
*                       supported in your design, then first service will be identified
*                       by serviceIndex of 0 and the second by serviceIndex of 1.
*
*  \param charIndex:    The index of the service characteristic of type
*                       \ref cy_en_ble_hids_char_index_t. The valid values are,
*                       * \ref CY_BLE_HIDS_PROTOCOL_MODE      - Protocol Mode characteristic
*                       * \ref CY_BLE_HIDS_REPORT_MAP         - Report Map characteristic
*                       * \ref CY_BLE_HIDS_INFORMATION        - HID Information characteristic
*                       * \ref CY_BLE_HIDS_CONTROL_POINT      - HID Control Point characteristic
*                       * \ref CY_BLE_HIDS_BOOT_KYBRD_IN_REP  - Boot Keyboard Input Report Characteristic
*                       * \ref CY_BLE_HIDS_BOOT_KYBRD_OUT_REP - Boot Keyboard Output Report Characteristic
*                       * \ref CY_BLE_HIDS_BOOT_MOUSE_IN_REP  - Boot Mouse Input Report Characteristic
*                       * \ref CY_BLE_HIDS_REPORT             - Report Characteristic
*
*  \param attrSize:    The size of the characteristic value attribute.
*
*  \param attrValue:   The pointer to the characteristic value data that should be
*                      stored in the GATT database.
*
*  \return
*   A return value of type \ref cy_en_ble_api_result_t.
*
*   Error Codes                             | Description
*   ------------                            | -----------
*   CY_BLE_SUCCESS                          | The request was handled successfully.
*   CY_BLE_ERROR_INVALID_PARAMETER          | Validation of the input parameter failed.
*   CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE| An optional characteristic is absent.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_HIDSS_SetCharacteristicValue(uint8_t serviceIndex,
                                                           cy_en_ble_hids_char_index_t charIndex,
                                                           uint8_t attrSize,
                                                           uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult = CY_BLE_SUCCESS;
    cy_ble_gatt_db_attr_handle_t charValueHandle;

    if((serviceIndex >= cy_ble_hidssConfigPtr->serviceCount) || ((uint8_t)charIndex >= CY_BLE_HIDSS_CHAR_COUNT))
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else
    {
        switch(charIndex)
        {
            case CY_BLE_HIDS_PROTOCOL_MODE:
                charValueHandle = cy_ble_hidssConfigPtr->attrInfo[serviceIndex].protocolModeHandle;
                break;

            case CY_BLE_HIDS_REPORT_MAP:
                charValueHandle = cy_ble_hidssConfigPtr->attrInfo[serviceIndex].reportMapHandle;
                break;

            case CY_BLE_HIDS_INFORMATION:
                charValueHandle = cy_ble_hidssConfigPtr->attrInfo[serviceIndex].informationHandle;
                break;

            case CY_BLE_HIDS_CONTROL_POINT:
                charValueHandle = cy_ble_hidssConfigPtr->attrInfo[serviceIndex].controlPointHandle;
                break;

            case CY_BLE_HIDS_BOOT_KYBRD_IN_REP:
                charValueHandle = cy_ble_hidssConfigPtr->attrInfo[serviceIndex].
                                   bootReportArray[CY_BLE_HIDS_BOOT_KYBRD_IN_REP_INDX].reportHandle;
                break;

            case CY_BLE_HIDS_BOOT_KYBRD_OUT_REP:
                charValueHandle = cy_ble_hidssConfigPtr->attrInfo[serviceIndex].
                                   bootReportArray[CY_BLE_HIDS_BOOT_KYBRD_OUT_REP_INDX].reportHandle;
                break;

            case CY_BLE_HIDS_BOOT_MOUSE_IN_REP:
                charValueHandle = cy_ble_hidssConfigPtr->attrInfo[serviceIndex].
                                   bootReportArray[CY_BLE_HIDS_BOOT_MOUSE_IN_REP_INDX].reportHandle;
                break;

            default:                    /* Report characteristic */
                /* Verify that requested report exists in particular service */
                if(((uint8_t)charIndex - (uint8_t)CY_BLE_HIDS_REPORT) <
                   cy_ble_hidssConfigPtr->attrInfo[serviceIndex].reportCount)
                {
                    charValueHandle = cy_ble_hidssConfigPtr->attrInfo[serviceIndex].
                                       reportArray[charIndex - CY_BLE_HIDS_REPORT].reportHandle;
                }
                else
                {
                    charValueHandle = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
                }
                break;
        }
        if(charValueHandle == CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
        {
            apiResult = CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE;
        }
        else
        {
            /* Store data in database */
            cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;

            dbAttrValInfo.connHandle.bdHandle        = 0u;
            dbAttrValInfo.connHandle.attId           = 0u;
            dbAttrValInfo.handleValuePair.attrHandle = charValueHandle;
            dbAttrValInfo.handleValuePair.value.len  = attrSize;
            dbAttrValInfo.handleValuePair.value.val  = attrValue;
            dbAttrValInfo.offset                     = 0u;
            dbAttrValInfo.flags                      = CY_BLE_GATT_DB_LOCALLY_INITIATED;

            if(Cy_BLE_GATTS_WriteAttributeValueCCCD(&dbAttrValInfo) != CY_BLE_GATT_ERR_NONE)
            {
                apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
            }
        }
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_HIDSS_GetCharacteristicValue
***************************************************************************//**
*
*  Gets local characteristic value of the specified HID service characteristics.
*  The value is identified by charIndex.
*
*  \param serviceIndex: The index of the service instance. e.g. If two HID services are
*                       supported in your design, then first service will be identified
*                       by serviceIndex of 0 and the second by serviceIndex of 1.
*
*  \param charIndex:    The index of the service characteristic of type
*                       \ref cy_en_ble_hids_char_index_t. The valid values are,
*                       * \ref CY_BLE_HIDS_PROTOCOL_MODE      - Protocol Mode characteristic
*                       * \ref CY_BLE_HIDS_REPORT_MAP         - Report Map characteristic
*                       * \ref CY_BLE_HIDS_INFORMATION        - HID Information characteristic
*                       * \ref CY_BLE_HIDS_CONTROL_POINT      - HID Control Point characteristic
*                       * \ref CY_BLE_HIDS_BOOT_KYBRD_IN_REP  - Boot Keyboard Input Report Characteristic
*                       * \ref CY_BLE_HIDS_BOOT_KYBRD_OUT_REP - Boot Keyboard Output Report Characteristic
*                       * \ref CY_BLE_HIDS_BOOT_MOUSE_IN_REP  - Boot Mouse Input Report Characteristic
*                       * \ref CY_BLE_HIDS_REPORT             - Report Characteristic
*
*  \param attrSize:     The size of the characteristic value attribute.
*
*  \param attrValue:    The pointer to the location where characteristic value data
*                       should be stored.
*
*  \return
*   A return value of type \ref cy_en_ble_api_result_t.
*
*   Error Codes                              | Description
*   ------------                             | -----------
*   CY_BLE_SUCCESS                           | The characteristic value was read successfully.
*   CY_BLE_ERROR_INVALID_PARAMETER           | Validation of the input parameter failed.
*   CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE | Optional characteristic is absent
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_HIDSS_GetCharacteristicValue(uint8_t serviceIndex,
                                                           cy_en_ble_hids_char_index_t charIndex,
                                                           uint8_t attrSize,
                                                           uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult = CY_BLE_SUCCESS;
    cy_ble_gatt_db_attr_handle_t charValueHandle;

    if((serviceIndex >= cy_ble_hidssConfigPtr->serviceCount) || ((uint8_t)charIndex >= CY_BLE_HIDSS_CHAR_COUNT))
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else
    {
        switch(charIndex)
        {
            case CY_BLE_HIDS_PROTOCOL_MODE:
                charValueHandle = cy_ble_hidssConfigPtr->attrInfo[serviceIndex].protocolModeHandle;
                break;

            case CY_BLE_HIDS_REPORT_MAP:
                charValueHandle = cy_ble_hidssConfigPtr->attrInfo[serviceIndex].reportMapHandle;
                break;

            case CY_BLE_HIDS_INFORMATION:
                charValueHandle = cy_ble_hidssConfigPtr->attrInfo[serviceIndex].informationHandle;
                break;

            case CY_BLE_HIDS_CONTROL_POINT:
                charValueHandle = cy_ble_hidssConfigPtr->attrInfo[serviceIndex].controlPointHandle;
                break;

            case CY_BLE_HIDS_BOOT_KYBRD_IN_REP:
                charValueHandle = cy_ble_hidssConfigPtr->attrInfo[serviceIndex].
                                   bootReportArray[CY_BLE_HIDS_BOOT_KYBRD_IN_REP_INDX].reportHandle;
                break;

            case CY_BLE_HIDS_BOOT_KYBRD_OUT_REP:
                charValueHandle = cy_ble_hidssConfigPtr->attrInfo[serviceIndex].
                                   bootReportArray[CY_BLE_HIDS_BOOT_KYBRD_OUT_REP_INDX].reportHandle;
                break;

            case CY_BLE_HIDS_BOOT_MOUSE_IN_REP:
                charValueHandle = cy_ble_hidssConfigPtr->attrInfo[serviceIndex].
                                   bootReportArray[CY_BLE_HIDS_BOOT_MOUSE_IN_REP_INDX].reportHandle;
                break;

            default:                    /* Report characteristic */
                /* Verify that requested report exists in particular service */
                if(((uint8_t)charIndex - (uint8_t)CY_BLE_HIDS_REPORT) <
                   cy_ble_hidssConfigPtr->attrInfo[serviceIndex].reportCount)
                {
                    charValueHandle = cy_ble_hidssConfigPtr->attrInfo[serviceIndex].
                                       reportArray[charIndex - CY_BLE_HIDS_REPORT].reportHandle;
                }
                else
                {
                    charValueHandle = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
                }
                break;
        }

        if(charValueHandle == CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
        {
            apiResult = CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE;
        }
        else
        {
            /* Read characteristic value from database */
            cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;

            dbAttrValInfo.connHandle.bdHandle        = 0u;
            dbAttrValInfo.connHandle.attId           = 0u;
            dbAttrValInfo.handleValuePair.attrHandle = charValueHandle;
            dbAttrValInfo.handleValuePair.value.len  = attrSize;
            dbAttrValInfo.handleValuePair.value.val  = attrValue;
            dbAttrValInfo.offset                     = 0u;
            dbAttrValInfo.flags                      = CY_BLE_GATT_DB_LOCALLY_INITIATED;

            if(Cy_BLE_GATTS_ReadAttributeValueCCCD(&dbAttrValInfo) != CY_BLE_GATT_ERR_NONE)
            {
                apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
            }
        }
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_HIDSS_GetCharacteristicDescriptor
***************************************************************************//**
*
*  Gets local characteristic descriptor of the specified HID service
*  characteristic.
*
*  \param connHandle:   BLE peer device connection handle.
*
*  \param serviceIndex: The index of the service instance. e.g. If two HID services are
*                       supported in your design, then first service will be identified
*                       by serviceIndex of 0 and the second by serviceIndex of 1.
*
*  \param charIndex:    The index of the service characteristic of type
*                       \ref cy_en_ble_hids_char_index_t. The valid values are,
*                       * \ref CY_BLE_HIDS_REPORT_MAP         - Report Map characteristic
*                       * \ref CY_BLE_HIDS_BOOT_KYBRD_IN_REP  - Boot Keyboard Input Report Characteristic
*                       * \ref CY_BLE_HIDS_BOOT_KYBRD_OUT_REP - Boot Keyboard Output Report Characteristic
*                       * \ref CY_BLE_HIDS_BOOT_MOUSE_IN_REP  - Boot Mouse Input Report Characteristic
*                       * \ref CY_BLE_HIDS_REPORT             - Report Characteristic
*
*  \param descrIndex:   The index of the service characteristic descriptor of type
*                       \ref cy_en_ble_hids_descr_t. The valid values are,
*                       * \ref CY_BLE_HIDS_REPORT_CCCD     - Client Characteristic Configuration descriptor
*                       * \ref CY_BLE_HIDS_REPORT_RRD      - Report Reference descriptor
*                       * \ref CY_BLE_HIDS_REPORT_MAP_ERRD - Report Map External Report Reference descriptor
*
*  \param attrSize:     The size of the descriptor value attribute.
*
*  \param attrValue:    The pointer to the location where characteristic descriptor
*                       value data should be stored.
*
*  \return
*   A return value of type \ref cy_en_ble_api_result_t.
*
*   Error Codes                              | Description
*   ------------                             | -----------
*   CY_BLE_SUCCESS                           | The characteristic descriptor value was read successfully.
*   CY_BLE_ERROR_INVALID_PARAMETER           | Validation of the input parameter failed.
*   CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE | Optional descriptor is absent
*
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_HIDSS_GetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                                uint8_t serviceIndex,
                                                                cy_en_ble_hids_char_index_t charIndex,
                                                                cy_en_ble_hids_descr_t descrIndex,
                                                                uint8_t attrSize,
                                                                uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult = CY_BLE_SUCCESS;
    cy_ble_gatt_db_attr_handle_t locDescrHandle = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
    const cy_stc_ble_hidss_report_t *locReport;

    if((serviceIndex >= cy_ble_hidssConfigPtr->serviceCount) || (descrIndex >= CY_BLE_HIDS_DESCR_COUNT) ||
       ((uint8_t)charIndex >= CY_BLE_HIDSS_CHAR_COUNT) || (charIndex < CY_BLE_HIDS_REPORT_MAP) ||
       (charIndex == CY_BLE_HIDS_BOOT_KYBRD_OUT_REP))
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else
    {
        if(charIndex == CY_BLE_HIDS_REPORT_MAP)
        {
            if(descrIndex == CY_BLE_HIDS_REPORT_MAP_ERRD)
            {
                locDescrHandle = cy_ble_hidssConfigPtr->attrInfo[serviceIndex].reportMapErrdHandle;
            }
            else
            {
                apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
            }
        }
        else /* Report characteristics */
        {
            /* Get report structure */
            if((charIndex >= CY_BLE_HIDS_BOOT_KYBRD_IN_REP) && (charIndex <= CY_BLE_HIDS_BOOT_MOUSE_IN_REP))
            {
                locReport = &cy_ble_hidssConfigPtr->attrInfo[serviceIndex].
                             bootReportArray[charIndex - CY_BLE_HIDS_BOOT_KYBRD_IN_REP];
            }
            else
            {
                locReport = &cy_ble_hidssConfigPtr->attrInfo[serviceIndex].
                             reportArray[charIndex - CY_BLE_HIDS_REPORT];
            }

            /* Get descriptor handle from report structure */
            if(descrIndex == CY_BLE_HIDS_REPORT_CCCD)
            {
                locDescrHandle = locReport->cccdHandle;
            }
            else if(descrIndex == CY_BLE_HIDS_REPORT_RRD)
            {
                locDescrHandle = locReport->rrdHandle;
            }
            else /* External Report Reference descriptor doesn't exist for report characteristic */
            {
                apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
            }
        }
    }
    if(apiResult != CY_BLE_ERROR_INVALID_PARAMETER)
    {
        if(locDescrHandle == CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
        {
            apiResult = CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE;
        }
        else
        {
            /* Read value from database */
            cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;

            dbAttrValInfo.handleValuePair.attrHandle = locDescrHandle;
            dbAttrValInfo.handleValuePair.value.len  = attrSize;
            dbAttrValInfo.handleValuePair.value.val  = attrValue;
            dbAttrValInfo.connHandle                 = connHandle;
            dbAttrValInfo.offset                     = 0u;
            dbAttrValInfo.flags                      = CY_BLE_GATT_DB_LOCALLY_INITIATED;

            if(Cy_BLE_GATTS_ReadAttributeValueCCCD(&dbAttrValInfo) != CY_BLE_GATT_ERR_NONE)
            {
                apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
            }
        }
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_HIDSS_CccdWriteEventHandler
***************************************************************************//**
*
*  Handles the Write Request event of the client Characteristic Configuration
*  descriptor.
*
*  \param eventParam: The pointer to the data structure specified by the event.
*
*  \return
*   uint32_t eventCode: The event code to be send to application.
*
******************************************************************************/
static uint32_t Cy_BLE_HIDSS_CccdWriteEventHandler(const cy_stc_ble_gatts_write_cmd_req_param_t *eventParam)
{
    uint32_t eventCode;

    if(CY_BLE_IS_NOTIFICATION_ENABLED_IN_PTR(eventParam->handleValPair.value.val))
    {
        eventCode = (uint32_t)CY_BLE_EVT_HIDSS_NOTIFICATION_ENABLED;
    }
    else
    {
        eventCode = (uint32_t)CY_BLE_EVT_HIDSS_NOTIFICATION_DISABLED;
    }
    cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;

    return(eventCode);
}


/******************************************************************************
* Function Name: Cy_BLE_HIDSS_WriteEventHandler
***************************************************************************//**
*
*  Handles the Write Request event.
*
*  \param eventParam: The pointer to the data structure specified by the event.
*
*  \return
*   A return value of type cy_en_ble_gatt_err_code_t.
*   * CY_BLE_GATT_ERR_NONE - Write is successful.
*
******************************************************************************/
static cy_en_ble_gatt_err_code_t Cy_BLE_HIDSS_WriteEventHandler(cy_stc_ble_gatts_write_cmd_req_param_t *eventParam)
{
    cy_en_ble_gatt_err_code_t gattErr = CY_BLE_GATT_ERR_NONE;
    uint8_t locServIndex = 0u;
    uint32_t reportIndex;
    cy_en_ble_hids_char_index_t locCharIndex;
    cy_stc_ble_hids_char_value_t locCharValue;
    uint32_t eventCode = 0u;

    if(Cy_BLE_HIDS_ApplCallback != NULL)
    {
        locCharValue.connHandle = eventParam->connHandle;
        locCharValue.value = NULL;
        do
        {
            locCharValue.serviceIndex = locServIndex;
            /* Protocol Mode characteristic Write Request */
            if(eventParam->handleValPair.attrHandle == cy_ble_hidssConfigPtr->attrInfo[locServIndex].
                                                        protocolModeHandle)
            {
                locCharValue.charIndex = CY_BLE_HIDS_PROTOCOL_MODE;
                if(eventParam->handleValPair.value.len == CY_BLE_HIDS_PM_CHAR_LEN)
                {
                    switch(eventParam->handleValPair.value.val[0u])
                    {
                        case CY_BLE_HIDS_PROTOCOL_MODE_BOOT:
                            eventCode = (uint32_t)CY_BLE_EVT_HIDSS_BOOT_MODE_ENTER;
                            cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
                            break;

                        case CY_BLE_HIDS_PROTOCOL_MODE_REPORT:
                            eventCode = (uint32_t)CY_BLE_EVT_HIDSS_REPORT_MODE_ENTER;
                            cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
                            break;

                        default:    /* Reserved for Future Use. */
                            break;
                    }
                }
            }
            /* Control Point characteristic Write Request */
            else if(eventParam->handleValPair.attrHandle ==
                     cy_ble_hidssConfigPtr->attrInfo[locServIndex].controlPointHandle)
            {
                locCharValue.charIndex = CY_BLE_HIDS_CONTROL_POINT;
                if(eventParam->handleValPair.value.len == CY_BLE_HIDS_CP_CHAR_LEN)
                {
                    switch(eventParam->handleValPair.value.val[0u])
                    {
                        case CY_BLE_HIDS_CP_SUSPEND:
                            eventCode = (uint32_t)CY_BLE_EVT_HIDSS_SUSPEND;
                            cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
                            break;

                        case CY_BLE_HIDS_CP_EXIT_SUSPEND:
                            eventCode = (uint32_t)CY_BLE_EVT_HIDSS_EXIT_SUSPEND;
                            cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
                            break;

                        default:    /* Reserved for Future Use. */
                            break;
                    }
                }
            }
            else if(eventParam->handleValPair.attrHandle == cy_ble_hidssConfigPtr->attrInfo[locServIndex].
                     bootReportArray[CY_BLE_HIDS_BOOT_KYBRD_IN_REP_INDX].reportHandle)
            {
                locCharValue.charIndex = CY_BLE_HIDS_BOOT_KYBRD_IN_REP;
                locCharValue.value = &eventParam->handleValPair.value;
                eventCode = (uint32_t)CY_BLE_EVT_HIDSS_REPORT_WRITE_CHAR;
                cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
            }
            else if(eventParam->handleValPair.attrHandle == cy_ble_hidssConfigPtr->attrInfo[locServIndex].
                     bootReportArray[CY_BLE_HIDS_BOOT_KYBRD_IN_REP_INDX].cccdHandle)
            {
                locCharValue.charIndex = CY_BLE_HIDS_BOOT_KYBRD_IN_REP;
                eventCode = Cy_BLE_HIDSS_CccdWriteEventHandler(eventParam);
            }
            else if(eventParam->handleValPair.attrHandle == cy_ble_hidssConfigPtr->attrInfo[locServIndex].
                     bootReportArray[CY_BLE_HIDS_BOOT_KYBRD_OUT_REP_INDX].reportHandle)
            {
                locCharValue.charIndex = CY_BLE_HIDS_BOOT_KYBRD_OUT_REP;
                locCharValue.value = &eventParam->handleValPair.value;
                eventCode = (uint32_t)CY_BLE_EVT_HIDSS_REPORT_WRITE_CHAR;
                cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
            }
            else if(eventParam->handleValPair.attrHandle == cy_ble_hidssConfigPtr->attrInfo[locServIndex].
                     bootReportArray[CY_BLE_HIDS_BOOT_MOUSE_IN_REP_INDX].reportHandle)
            {
                locCharValue.charIndex = CY_BLE_HIDS_BOOT_MOUSE_IN_REP;
                locCharValue.value = &eventParam->handleValPair.value;
                eventCode = (uint32_t)CY_BLE_EVT_HIDSS_REPORT_WRITE_CHAR;
                cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
            }
            else if(eventParam->handleValPair.attrHandle == cy_ble_hidssConfigPtr->attrInfo[locServIndex].
                     bootReportArray[CY_BLE_HIDS_BOOT_MOUSE_IN_REP_INDX].cccdHandle)
            {
                locCharValue.charIndex = CY_BLE_HIDS_BOOT_MOUSE_IN_REP;
                eventCode = Cy_BLE_HIDSS_CccdWriteEventHandler(eventParam);
            }
            else
            {
                uint8_t locReqHandle = 0u;

                locCharIndex = CY_BLE_HIDS_REPORT;
                for(reportIndex = 0u; (reportIndex < cy_ble_hidssConfigPtr->attrInfo[locServIndex].reportCount) &&
                    (locReqHandle == 0u); reportIndex++)
                {
                    if(eventParam->handleValPair.attrHandle ==
                       cy_ble_hidssConfigPtr->attrInfo[locServIndex].reportArray[reportIndex].reportHandle)
                    {
                        locCharValue.charIndex = locCharIndex;
                        locCharValue.value = &eventParam->handleValPair.value;
                        eventCode = (uint32_t)CY_BLE_EVT_HIDSS_REPORT_WRITE_CHAR;
                        cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
                        locReqHandle = 1u;
                    }
                    if(eventParam->handleValPair.attrHandle ==
                       cy_ble_hidssConfigPtr->attrInfo[locServIndex].reportArray[reportIndex].cccdHandle)
                    {
                        locCharValue.charIndex = locCharIndex;
                        eventCode = Cy_BLE_HIDSS_CccdWriteEventHandler(eventParam);
                        locReqHandle = 1u;
                    }
                    locCharIndex++;
                }
            }
            locServIndex++;

            /* Store data to database if event is handled */
            if((cy_ble_eventHandlerFlag & CY_BLE_CALLBACK) == 0u)
            {
                cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;

                /* Fill GATT database attribute value parameters */
                dbAttrValInfo.handleValuePair.attrHandle = eventParam->handleValPair.attrHandle;
                dbAttrValInfo.handleValuePair.value.len  = eventParam->handleValPair.value.len;
                dbAttrValInfo.handleValuePair.value.val  = eventParam->handleValPair.value.val;
                dbAttrValInfo.offset                     = 0u;
                dbAttrValInfo.flags                      = CY_BLE_GATT_DB_PEER_INITIATED;
                dbAttrValInfo.connHandle                 = eventParam->connHandle;

                gattErr = Cy_BLE_GATTS_WriteAttributeValueCCCD(&dbAttrValInfo);
                if(gattErr == CY_BLE_GATT_ERR_NONE)
                {
                    Cy_BLE_HIDS_ApplCallback(eventCode, &locCharValue);
                }
                break;
            }
        }
        while(locServIndex < cy_ble_hidssConfigPtr->serviceCount);
    }

    return(gattErr);
}


/******************************************************************************
* Function Name: Cy_BLE_HIDSS_SendNotification
***************************************************************************//**
*
*  Sends specified HID service characteristic notification to the client device.
*  \ref CY_BLE_EVT_HIDSC_NOTIFICATION event is received by the peer device, on invoking
*  this function.
*  On enabling notification successfully for a service characteristic it sends out a
*  'Handle Value Notification' which results in \ref CY_BLE_EVT_HIDSC_NOTIFICATION event
*  at the GATT Client's end.
*
*  \param connHandle:   BLE peer device connection handle.
*
*  \param serviceIndex: The index of the HID service instance. e.g. If two HID services
*                       are supported in your design, then first service will be
*                       identified by serviceIndex of 0 and the second by serviceIndex
*                       of 1.
*
*  \param charIndex:    The index of the service characteristic.
*
*  \param attrSize:     The size of the characteristic value attribute.
*
*  \param attrValue:    Pointer to the characteristic value data that should be sent to
*                       the client device.
*
*  \return
*   A return value of type \ref cy_en_ble_api_result_t.
*
*   Error Codes                              | Description
*   ------------                             | -----------
*   CY_BLE_SUCCESS                           | The request was handled successfully.
*   CY_BLE_ERROR_INVALID_PARAMETER           | Validation of the input parameter failed.
*   CY_BLE_ERROR_INVALID_OPERATION           | Operation is invalid for this characteristic.
*   CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE | An optional characteristic is absent.
*   CY_BLE_ERROR_INVALID_STATE               | Connection with the client is not established.
*   CY_BLE_ERROR_MEMORY_ALLOCATION_FAILED    | Memory allocation failed.
*   CY_BLE_ERROR_NTF_DISABLED                | Notification is not enabled by the client.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_HIDSS_SendNotification(cy_stc_ble_conn_handle_t connHandle,
                                                     uint8_t serviceIndex,
                                                     cy_en_ble_hids_char_index_t charIndex,
                                                     uint8_t attrSize,
                                                     uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult = CY_BLE_SUCCESS;
    const cy_stc_ble_hidss_report_t *locReport = NULL;

    if((Cy_BLE_GetConnectionState(connHandle) < CY_BLE_CONN_STATE_CONNECTED) || (cy_ble_hidssConfigPtr == NULL))
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }
    else if((serviceIndex >= cy_ble_hidssConfigPtr->serviceCount) || ((uint8_t)charIndex >= CY_BLE_HIDSS_CHAR_COUNT))
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else
    {
        if(charIndex == CY_BLE_HIDS_BOOT_KYBRD_IN_REP)
        {
            locReport = &cy_ble_hidssConfigPtr->attrInfo[serviceIndex].
                         bootReportArray[CY_BLE_HIDS_BOOT_KYBRD_IN_REP_INDX];
        }
        else if(charIndex == CY_BLE_HIDS_BOOT_MOUSE_IN_REP)
        {
            locReport = &cy_ble_hidssConfigPtr->attrInfo[serviceIndex].
                         bootReportArray[CY_BLE_HIDS_BOOT_MOUSE_IN_REP_INDX];
        }
        else if((charIndex >= CY_BLE_HIDS_REPORT) && ((uint8_t)charIndex <= CY_BLE_HIDSS_REPORT_END))
        {
            /* Verify that requested report exists in particular service */
            if(((uint8_t)charIndex - (uint8_t)CY_BLE_HIDS_REPORT) <
                                                    cy_ble_hidssConfigPtr->attrInfo[serviceIndex].reportCount)
            {
                locReport = &cy_ble_hidssConfigPtr->attrInfo[serviceIndex].
                                                   reportArray[charIndex - CY_BLE_HIDS_REPORT];
            }
            else
            {
                apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
            }
        }
        else
        {
            apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
        }

        if((apiResult == CY_BLE_SUCCESS) && (locReport != NULL))
        {
            if(locReport->reportHandle == CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
            {
                apiResult = CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE;
            }
            else
            {
                /* Send Notification if it is enabled */
                if(CY_BLE_IS_NOTIFICATION_ENABLED(connHandle.attId, locReport->cccdHandle))
                {
                    /* Fill all fields of Write Request structure ... */
                    cy_stc_ble_gatts_handle_value_ntf_t ntfReqParam;
                    ntfReqParam.handleValPair.attrHandle = locReport->reportHandle;
                    ntfReqParam.handleValPair.value.val  = attrValue;
                    ntfReqParam.handleValPair.value.len  = attrSize;
                    ntfReqParam.connHandle               = connHandle;

                    /* ... and send notification to client device. */
                    apiResult = Cy_BLE_GATTS_Notification(&ntfReqParam);
                }
                else
                {
                    apiResult = CY_BLE_ERROR_NTF_DISABLED;
                }
            }
        }
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_HIDSS_OnDeviceConnected
***************************************************************************//**
*
*  Handles the connection establishment request.
*
******************************************************************************/
static void Cy_BLE_HIDSS_OnDeviceConnected(void)
{
    /* The Protocol Mode characteristic value shall be reset to the default value
     * following a connection establishment */
    uint8_t defaultProtocol = CY_BLE_HIDS_PROTOCOL_DEFAULT;
    uint8_t locServIndex = 0u;

    do
    {
        if(cy_ble_hidssConfigPtr->attrInfo[locServIndex].protocolModeHandle !=
           CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
        {
            /* Store default protocol value in database */
            CY_BLE_GATT_DB_ATTR_SET_GEN_VALUE(cy_ble_hidssConfigPtr->attrInfo[locServIndex].protocolModeHandle,
                                              &defaultProtocol, sizeof(defaultProtocol));
        }
        ++locServIndex;
    }
    while(locServIndex < cy_ble_hidssConfigPtr->serviceCount);
}


/******************************************************************************
* Function Name: Cy_BLE_HIDSC_InclDiscoveryEventHandler
***************************************************************************//**
*
*  This function is called on receiving a #CY_BLE_EVT_GATTC_READ_BY_TYPE_RSP event
*  Based on the service UUID, an appropriate data structure is populated using
*  the data received as part of the callback.
*
*  \param discCharInfo: The pointer to a characteristic information structure.
*  \param discoveryService: The index of the service instance.
*
******************************************************************************/
static void Cy_BLE_HIDSC_InclDiscoveryEventHandler(const cy_stc_ble_disc_incl_info_t *discInclInfo)
{
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(discInclInfo->connHandle);
    uint32_t hidsDiscIdx = cy_ble_hidscConfigPtr->serviceDiscIdx;

    if((discInclInfo->uuidFormat == CY_BLE_GATT_16_BIT_UUID_FORMAT) &&
       (cy_ble_configPtr->context->discovery[discIdx].servCount >= hidsDiscIdx) &&
       (cy_ble_configPtr->context->discovery[discIdx].servCount <= ((hidsDiscIdx + cy_ble_hidscConfigPtr->serviceCount) - 1u)))
    {
        /* The index of the service instance */
        uint32_t hidsServIdx = cy_ble_configPtr->context->discovery[discIdx].servCount - hidsDiscIdx;

        /* Get pointer (with offset) to HIDS client structure with attribute handles */
        cy_stc_ble_hidsc_t *hidscPtr = (cy_stc_ble_hidsc_t *)&cy_ble_hidscConfigPtr->
                                        attrInfo[(discIdx * cy_ble_hidscConfigPtr->serviceCount) + hidsServIdx];

        hidscPtr->includeHandle = discInclInfo->inclDefHandle;

        /* Indicate that request was handled */
        cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
    }
}


/******************************************************************************
* Function Name: Cy_BLE_HIDSC_DiscoverCharacteristicsEventHandler
***************************************************************************//**
*
*  This function is called on receiving a #CY_BLE_EVT_GATTC_READ_BY_TYPE_RSP event
*  Based on the service UUID, an appropriate data structure is populated using
*  the data received as part of the callback.
*
*  \param discCharInfo: The pointer to a characteristic information structure.
*
******************************************************************************/
static void Cy_BLE_HIDSC_DiscoverCharacteristicsEventHandler(cy_stc_ble_disc_char_info_t *discCharInfo)
{
    static cy_ble_gatt_db_attr_handle_t *lastEndHandle[CY_BLE_MAX_CONNECTION_INSTANCES] =  { 0u };
    static uint32_t discoveryLastServ[CY_BLE_MAX_CONNECTION_INSTANCES] = { 0u };

    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(discCharInfo->connHandle);
    uint32_t hidsDiscIdx = cy_ble_hidscConfigPtr->serviceDiscIdx;

    if((discCharInfo->uuidFormat == CY_BLE_GATT_16_BIT_UUID_FORMAT) &&
       (cy_ble_configPtr->context->discovery[discIdx].servCount >= hidsDiscIdx) &&
       (cy_ble_configPtr->context->discovery[discIdx].servCount <= ((hidsDiscIdx + cy_ble_hidscConfigPtr->serviceCount) - 1u)))

    {
        /* The index of the service instance */
        uint32_t hidsServIdx = cy_ble_configPtr->context->discovery[discIdx].servCount - hidsDiscIdx;

        /* Get pointer (with offset) to HIDS client structure with attribute handles */
        cy_stc_ble_hidsc_t *hidscPtr = (cy_stc_ble_hidsc_t *)&cy_ble_hidscConfigPtr->
                                        attrInfo[(discIdx * cy_ble_hidscConfigPtr->serviceCount) + hidsServIdx];

        /* Update last characteristic endHandle to declaration handle of this characteristic */
        if(lastEndHandle[discIdx] != NULL)
        {
            if(discoveryLastServ[discIdx] == hidsServIdx)
            {
                *lastEndHandle[discIdx] = discCharInfo->charDeclHandle - 1u;
            }
            lastEndHandle[discIdx] = NULL;
        }

        switch(discCharInfo->uuid.uuid16)
        {
            case CY_BLE_UUID_CHAR_HIDS_PROTOCOL_MODE:
                Cy_BLE_CheckStoreCharHandle(hidscPtr->protocolMode);
                break;

            case CY_BLE_UUID_CHAR_HIDS_INFORMATION:
                Cy_BLE_CheckStoreCharHandle(hidscPtr->information);
                break;

            case CY_BLE_UUID_CHAR_HIDS_REPORT_MAP:
                Cy_BLE_CheckStoreCharHandle(hidscPtr->reportMap);
                lastEndHandle[discIdx] = &hidscPtr->reportMap.endHandle;
                break;

            case CY_BLE_UUID_CHAR_HIDS_CONTROL_POINT:
                Cy_BLE_CheckStoreCharHandle(hidscPtr->controlPoint);
                break;

            case CY_BLE_UUID_CHAR_HIDS_REPORT:
                Cy_BLE_CheckStoreCharHandle(hidscPtr->report[hidscPtr->reportCount]);
                lastEndHandle[discIdx] = &hidscPtr->report[hidscPtr->reportCount].endHandle;

                if(hidscPtr->reportCount < cy_ble_hidscConfigPtr->reportCount)
                {
                    hidscPtr->reportCount++;
                }
                break;

            case CY_BLE_UUID_CHAR_HIDS_BOOT_KEYBOARD_IN_RPT:
                Cy_BLE_CheckStoreCharHandle(hidscPtr->bootReport[CY_BLE_HIDS_BOOT_KYBRD_IN_REP_INDX]);
                lastEndHandle[discIdx] = &hidscPtr->bootReport[CY_BLE_HIDS_BOOT_KYBRD_IN_REP_INDX].endHandle;
                break;

            case CY_BLE_UUID_CHAR_HIDS_BOOT_KEYBOARD_OUT_RPT:
                Cy_BLE_CheckStoreCharHandle(hidscPtr->bootReport[CY_BLE_HIDS_BOOT_KYBRD_OUT_REP_INDX]);
                lastEndHandle[discIdx] = &hidscPtr->bootReport[CY_BLE_HIDS_BOOT_KYBRD_OUT_REP_INDX].endHandle;
                break;

            case CY_BLE_UUID_CHAR_HIDS_BOOT_MOUSE_IN_RPT:
                Cy_BLE_CheckStoreCharHandle(hidscPtr->bootReport[CY_BLE_HIDS_BOOT_MOUSE_IN_REP_INDX]);
                lastEndHandle[discIdx] = &hidscPtr->bootReport[CY_BLE_HIDS_BOOT_MOUSE_IN_REP_INDX].endHandle;
                break;

            default:
                break;
        }
        /* Init characteristic endHandle to service endHandle.
         * Characteristic endHandle will be updated to the declaration
         * Handler of the following characteristic, in the following characteristic discovery procedure. */
        if(lastEndHandle[discIdx] != NULL)
        {
            uint32_t locServCount = cy_ble_configPtr->context->discovery[discIdx].servCount;
            uint32_t discServiNum = cy_ble_configPtr->context->discServiCount;

            *lastEndHandle[discIdx] =
                cy_ble_configPtr->context->serverInfo[(discIdx * discServiNum) + locServCount].range.endHandle;
            /* Init service index of discovered characteristic */
            discoveryLastServ[discIdx] = hidsServIdx;
        }

        /* Indicate that request was handled */
        cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
    }
}


/******************************************************************************
* Function Name: Cy_BLE_HIDSC_DiscoverCharDescriptorsEventHandler
***************************************************************************//**
*
*  This function is called on receiving a #CY_BLE_EVT_GATTC_FIND_INFO_RSP event.
*  This event is generated when the server successfully sends the data for
*  #CY_BLE_EVT_GATTC_FIND_INFO_REQ. Based on the service UUID, an appropriate data
*  structure is populated to the service with a service callback.
*
*  \param discDescrInfo: The pointer to a descriptor information structure.
*
******************************************************************************/
static void Cy_BLE_HIDSC_DiscoverCharDescriptorsEventHandler(cy_stc_ble_disc_descr_info_t *discDescrInfo)
{
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(discDescrInfo->connHandle);
    uint32_t hidsDiscIdx = cy_ble_hidscConfigPtr->serviceDiscIdx;

    if((cy_ble_configPtr->context->discovery[discIdx].servCount >= hidsDiscIdx) &&
       (cy_ble_configPtr->context->discovery[discIdx].servCount <= ((hidsDiscIdx + cy_ble_hidscConfigPtr->serviceCount) - 1u)))
    {
        uint32_t hidsServIdx = cy_ble_configPtr->context->discovery[discIdx].servCount -
                                cy_ble_hidscConfigPtr->serviceDiscIdx;

        /* Get pointer (with offset) to HIDS client structure with attribute handles */
        cy_stc_ble_hidsc_t *hidscPtr = (cy_stc_ble_hidsc_t *)&cy_ble_hidscConfigPtr->
                                        attrInfo[(discIdx * cy_ble_hidscConfigPtr->serviceCount) + hidsServIdx];

        switch((cy_en_ble_hids_char_index_t)cy_ble_configPtr->context->discovery[discIdx].charCount)
        {
            case CY_BLE_HIDS_REPORT_MAP:
                /* Descriptors for reportMap characteristics */
                if(discDescrInfo->uuid.uuid16 == CY_BLE_UUID_CHAR_EXTERNAL_REPORT_REF)
                {
                    Cy_BLE_CheckStoreCharDescrHandle(hidscPtr->reportMap.errdHandle);
                }
                break;

            case CY_BLE_HIDS_BOOT_KYBRD_IN_REP:
            case CY_BLE_HIDS_BOOT_KYBRD_OUT_REP:
            case CY_BLE_HIDS_BOOT_MOUSE_IN_REP:
            {
                /* Descriptors for report characteristics */
                uint8_t bootRepIdx = cy_ble_configPtr->context->discovery[discIdx].charCount - (uint8_t)CY_BLE_HIDS_BOOT_KYBRD_IN_REP;
                if(discDescrInfo->uuid.uuid16 == CY_BLE_UUID_CHAR_CLIENT_CONFIG)
                {
                    Cy_BLE_CheckStoreCharDescrHandle(hidscPtr->bootReport[bootRepIdx].cccdHandle);
                }
                else if(discDescrInfo->uuid.uuid16 == CY_BLE_UUID_CHAR_REPORT_REFERENCE)
                {
                    Cy_BLE_CheckStoreCharDescrHandle(hidscPtr->bootReport[bootRepIdx].rrdHandle);
                }
                else        /* Report Characteristic doesn't support other descriptors */
                {
                }
                break;
            }

            default:
            {
                if((cy_ble_configPtr->context->discovery[discIdx].charCount >= (uint8_t)CY_BLE_HIDS_REPORT) &&
                   (cy_ble_configPtr->context->discovery[discIdx].charCount <= CY_BLE_HIDSC_REPORT_END))
                {
                    /* Descriptors for report characteristics */
                    uint32_t reportIdx = cy_ble_configPtr->context->discovery[discIdx].charCount - (uint32_t)CY_BLE_HIDS_REPORT;

                    if(discDescrInfo->uuid.uuid16 == CY_BLE_UUID_CHAR_CLIENT_CONFIG)
                    {
                        Cy_BLE_CheckStoreCharDescrHandle(hidscPtr->report[reportIdx].cccdHandle);
                    }
                    else if(discDescrInfo->uuid.uuid16 == CY_BLE_UUID_CHAR_REPORT_REFERENCE)
                    {
                        Cy_BLE_CheckStoreCharDescrHandle(hidscPtr->report[reportIdx].rrdHandle);
                    }
                    else        /* Report Characteristic doesn't support other descriptors */
                    {
                    }
                }
                break;
            }
        }

        /* Indicate that request was handled */
        cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
    }
}


/******************************************************************************
* Function Name: Cy_BLE_HIDSC_GetCharRange
***************************************************************************//**
*
* Returns a possible range of the current characteristic descriptor via
* input parameter charRangeInfo->range
*
* \param *charRangeInfo: The pointer to a descriptor range information structure.
*
******************************************************************************/
static void Cy_BLE_HIDSC_GetCharRange(cy_stc_ble_disc_range_info_t *charRangeInfo)
{
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(charRangeInfo->connHandle);
    uint32_t hidsDiscIdx = cy_ble_hidscConfigPtr->serviceDiscIdx;
    uint32_t exitFlag = 0u;

    if((cy_ble_configPtr->context->discovery[discIdx].servCount >= hidsDiscIdx) &&
       (cy_ble_configPtr->context->discovery[discIdx].servCount <= ((hidsDiscIdx + cy_ble_hidscConfigPtr->serviceCount) - 1u)))
    {
        /* The index of the service instance */
        uint32_t hidsServIdx = cy_ble_configPtr->context->discovery[discIdx].servCount - hidsDiscIdx;

        /* Get pointer (with offset) to HIDS client structure with attribute handles */
        cy_stc_ble_hidsc_t *hidscPtr = (cy_stc_ble_hidsc_t *)&cy_ble_hidscConfigPtr->
                                        attrInfo[(discIdx * cy_ble_hidscConfigPtr->serviceCount) + hidsServIdx];

        if(charRangeInfo->srviIncIdx != CY_BLE_DISCOVERY_INIT)
        {
            cy_ble_configPtr->context->discovery[discIdx].charCount++;
        }

        while((cy_ble_configPtr->context->discovery[discIdx].charCount < CY_BLE_HIDSC_CHAR_COUNT) && (exitFlag == 0u))
        {
            switch((cy_en_ble_hids_char_index_t)cy_ble_configPtr->context->discovery[discIdx].charCount)
            {
                case CY_BLE_HIDS_REPORT_MAP:
                    /* Read report map characteristic range */
                    if((hidscPtr->reportMap.endHandle - hidscPtr->reportMap.valueHandle) != 0u)
                    {
                        charRangeInfo->range.startHandle = hidscPtr->reportMap.valueHandle + 1u;
                        charRangeInfo->range.endHandle = hidscPtr->reportMap.endHandle;
                        exitFlag = 1u;
                    }
                    break;

                case CY_BLE_HIDS_BOOT_KYBRD_IN_REP:
                case CY_BLE_HIDS_BOOT_KYBRD_OUT_REP:
                case CY_BLE_HIDS_BOOT_MOUSE_IN_REP:
                {
                    /* Read boot report characteristic range */
                    uint8_t bootRepIdx = cy_ble_configPtr->context->discovery[discIdx].charCount - (uint8_t)CY_BLE_HIDS_BOOT_KYBRD_IN_REP;

                    if((hidscPtr->bootReport[bootRepIdx].endHandle - hidscPtr->bootReport[bootRepIdx].valueHandle) != 0u)
                    {
                        charRangeInfo->range.startHandle = hidscPtr->bootReport[bootRepIdx].valueHandle + 1u;
                        charRangeInfo->range.endHandle   = hidscPtr->bootReport[bootRepIdx].endHandle;
                        exitFlag = 1u;
                    }
                    break;
                }

                default:
                {
                    if((cy_ble_configPtr->context->discovery[discIdx].charCount >=  (uint8_t)CY_BLE_HIDS_REPORT) &&
                       (cy_ble_configPtr->context->discovery[discIdx].charCount <= CY_BLE_HIDSC_REPORT_END))
                    {
                        /* Read report characteristic range */
                        uint8_t reportIdx = cy_ble_configPtr->context->discovery[discIdx].charCount - (uint8_t)CY_BLE_HIDS_REPORT;
                        if((hidscPtr->report[reportIdx].endHandle - hidscPtr->report[reportIdx].valueHandle) != 0u)
                        {
                            charRangeInfo->range.startHandle = hidscPtr->report[reportIdx].valueHandle + 1u;
                            charRangeInfo->range.endHandle  = hidscPtr->report[reportIdx].endHandle;
                            exitFlag = 1u;
                        }
                    }
                    break;
                }
            }

            if(exitFlag == 0u)
            {
                cy_ble_configPtr->context->discovery[discIdx].charCount++;
            }
        }

        /* Indicate that request was handled */
        cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
    }
}

/******************************************************************************
* Function Name: Cy_BLE_HIDSC_SetCharacteristicValue
***************************************************************************//**
*
*  Sends a request to set characteristic value of the specified HID service,
*  which is identified by serviceIndex and reportIndex, on the server device.
*
*  This function call can result in generation of the following events based on
*  the response from the server device:
*  * \ref CY_BLE_EVT_HIDSC_WRITE_CHAR_RESPONSE
*  * \ref CY_BLE_EVT_GATTC_ERROR_RSP
*
*  \param connHandle:   The connection handle.
*
*  \param subProcedure: Characteristic value write sub-procedure.
*                       * \ref CY_BLE_HIDSC_WRITE_WITHOUT_RESPONSE
*                       * \ref CY_BLE_HIDSC_WRITE_CHAR_VALUE
*
*  \param serviceIndex: The index of the service instance. e.g. If two HID services
*                       are supported in your design, then first service will be
*                       identified by serviceIndex of 0 and the second by
*                       serviceIndex of 1.
*
*  \param charIndex:    The index of the service characteristic of
*                       type \ref cy_en_ble_hids_char_index_t.
*
*  \param attrSize:     The size of the characteristic value attribute.
*
*  \param attrValue:    The pointer to the characteristic value data that should be
*                       sent to the server device.
*
*  \return
*   A return value of type \ref cy_en_ble_api_result_t.
*
*   Error Codes                              | Description
*   ------------                             | -----------
*   CY_BLE_SUCCESS                           | The request was sent successfully.
*   CY_BLE_ERROR_INVALID_PARAMETER           | Validation of the input parameter failed.
*   CY_BLE_ERROR_INVALID_OPERATION           | Operation is invalid for this characteristic.
*   CY_BLE_ERROR_INVALID_STATE               | Connection with the server is not established.
*   CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE | The peer device doesn't have the particular characteristic.
*   CY_BLE_ERROR_MEMORY_ALLOCATION_FAILED    | Memory allocation failed.
*
* \events
*  In case of successful execution (return value = \ref CY_BLE_SUCCESS)
*  the following events can appear: \n
*  If the HIDS service-specific callback is registered
*  with \ref Cy_BLE_HIDS_RegisterAttrCallback():
*  * \ref CY_BLE_EVT_HIDSC_WRITE_CHAR_RESPONSE - In case if the requested attribute is
*    successfully written on the peer device, the details (charIndex, etc.) are
*    provided with event parameter structure of type \ref cy_stc_ble_hids_char_value_t.
*  .
*  Otherwise, if the HIDS service-specific callback is not registered:
*  * \ref CY_BLE_EVT_GATTC_WRITE_RSP - In case if the requested attribute is
*    successfully written on the peer device.
*  * \ref CY_BLE_EVT_GATTC_ERROR_RSP - In case if an error occurred with the
*    requested attribute on the peer device, the details are provided with
*    event parameters structure \ref cy_stc_ble_gatt_err_param_t.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_HIDSC_SetCharacteristicValue(cy_stc_ble_conn_handle_t connHandle,
                                                           cy_en_ble_hidsc_char_write_t subProcedure,
                                                           uint8_t serviceIndex,
                                                           cy_en_ble_hids_char_index_t charIndex,
                                                           uint8_t attrSize,
                                                           uint8_t * attrValue)
{
    cy_en_ble_api_result_t apiResult;
    cy_ble_gatt_db_attr_handle_t charValueHandle;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(connHandle);

    if(Cy_BLE_GetConnectionState(connHandle) != CY_BLE_CONN_STATE_CLIENT_DISCOVERED)
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }
    else if((serviceIndex >= cy_ble_hidscConfigPtr->serviceCount) || ((uint8_t)charIndex >= CY_BLE_HIDSC_CHAR_COUNT) ||
            (discIdx >= cy_ble_configPtr->params->maxClientCount))
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else
    {
        charValueHandle = Cy_BLE_HIDSC_GetCharacteristicValueHandle(connHandle, serviceIndex, charIndex);

        if(charValueHandle != CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
        {
            /* Fill Write command request parameters */
            cy_stc_ble_gattc_write_cmd_req_t writeReqParam;
            writeReqParam.handleValPair.attrHandle = charValueHandle;
            writeReqParam.handleValPair.value.val  = attrValue;
            writeReqParam.handleValPair.value.len  = attrSize;
            writeReqParam.connHandle               = connHandle;

            /* Use WriteWithoutResponse sub-procedure for the following characteristics:
             * CY_BLE_HIDS_PROTOCOL_MODE
             * CY_BLE_HIDS_CONTROL_POINT
             */
            if(subProcedure == CY_BLE_HIDSC_WRITE_WITHOUT_RESPONSE)
            {
                apiResult = Cy_BLE_GATTC_WriteWithoutResponse(&writeReqParam);
            }
            else
            {
                apiResult = Cy_BLE_GATTC_WriteCharacteristicValue(&writeReqParam);
                /* Save handle to support service-specific Write Response from device */
                if(apiResult == CY_BLE_SUCCESS)
                {
                    cy_ble_hidscReqHandle[discIdx] = charValueHandle;
                }
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
* Function Name: Cy_BLE_HIDSC_GetCharacteristicValue
***************************************************************************//**
*
*  This function is used to read the characteristic value from a server
*  which is identified by charIndex.
*  The Read Response returns the characteristic value in the Attribute Value
*  parameter.
*  The Read Response only contains the characteristic value that is less than or
*  equal to (MTU - 1) octets in length. If the characteristic value is greater
*  than (MTU - 1) octets in length, the Read Long Characteristic Value procedure
*  may be used if the rest of the characteristic Value is required.
*
*  \param connHandle:   The connection handle.
*
*  \param subProcedure: The characteristic value read sub-procedure.
*                       * \ref CY_BLE_HIDSC_READ_CHAR_VALUE
*                       * \ref CY_BLE_HIDSC_READ_LONG_CHAR_VALUE.
*
*  \param serviceIndex: The index of the service instance. e.g. If two HID services
*                       are supported in your design, then first service will be
*                       identified by serviceIndex of 0 and the second by
*                       serviceIndex of 1.
*
*  \param charIndex:    The index of the service characteristic of
*                       type \ref cy_en_ble_hids_char_index_t.
*
*  \return
*   A return value of type \ref cy_en_ble_api_result_t.
*
*   Error Codes                              | Description
*   ------------                             | -----------
*   CY_BLE_SUCCESS                           | The request was sent successfully.
*   CY_BLE_ERROR_INVALID_PARAMETER           | Validation of the input parameter failed.
*   CY_BLE_ERROR_INVALID_OPERATION           | Operation is invalid for this characteristic.
*   CY_BLE_ERROR_INVALID_STATE               | Connection with the server is not established.
*   CY_BLE_ERROR_MEMORY_ALLOCATION_FAILED    | Memory allocation failed.
*   CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE | The peer device doesn't have the particular characteristic
*
*  \events
*   In case of successful execution (return value = #CY_BLE_SUCCESS)
*   the following events can appear: \n
*   If the HID service-specific callback is registered
*      with \ref Cy_BLE_HIDS_RegisterAttrCallback():
*   * #CY_BLE_EVT_HIDSC_READ_CHAR_RESPONSE - In case if the requested attribute is
*     successfully read on the peer device, the details
*     (charIndex , value, etc.) are provided with event parameter structure
*     of type \ref cy_stc_ble_hids_char_value_t.
*   .
*   Otherwise (if an HID service-specific callback is not registered):
*   * #CY_BLE_EVT_GATTC_READ_RSP - If the requested attribute is
*     successfully read on the peer device, the details (handle, value, etc.) are
*     provided with an event parameter structure
*     \ref cy_stc_ble_gattc_read_rsp_param_t.
*
*   * #CY_BLE_EVT_GATTC_READ_BLOB_RSP - In case if the requested attribute is
*     successfully read on the peer device, the details (handle, value, etc.) are
*     provided with event parameters structure \ref cy_stc_ble_gattc_read_rsp_param_t.
*
*   * #CY_BLE_EVT_GATTC_ERROR_RSP - If an error occurred with the
*     requested attribute on the peer device, the details are provided with
*     an event parameter structure \ref cy_stc_ble_gatt_err_param_t.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_HIDSC_GetCharacteristicValue(cy_stc_ble_conn_handle_t connHandle,
                                                           cy_en_ble_hidsc_char_read_t subProcedure,
                                                           uint8_t serviceIndex,
                                                           cy_en_ble_hids_char_index_t charIndex)
{
    cy_en_ble_api_result_t apiResult;
    cy_ble_gatt_db_attr_handle_t charValueHandle;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(connHandle);

    if(Cy_BLE_GetConnectionState(connHandle) != CY_BLE_CONN_STATE_CLIENT_DISCOVERED)
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }
    else if((serviceIndex >= cy_ble_hidscConfigPtr->serviceCount) || ((uint8_t)charIndex >= CY_BLE_HIDSC_CHAR_COUNT) ||
            (discIdx >= cy_ble_configPtr->params->maxClientCount))
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else
    {
        charValueHandle = Cy_BLE_HIDSC_GetCharacteristicValueHandle(connHandle, serviceIndex, charIndex);

        if(charValueHandle != CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
        {
            /* Use Read Long Characteristic procedure for Report map characteristic */
            if(subProcedure == CY_BLE_HIDSC_READ_LONG_CHAR_VALUE)
            {
                cy_stc_ble_gattc_read_blob_req_t charReadLongParam;

                /* Fill Read blob request parameter */
                charReadLongParam.handleOffset.attrHandle = charValueHandle;
                charReadLongParam.handleOffset.offset     = 0u;
                charReadLongParam.connHandle              = connHandle;

                apiResult = Cy_BLE_GATTC_ReadLongCharacteristicValues(&charReadLongParam);
            }
            else
            {
                /* Fill Read Request parameter */
                cy_stc_ble_gattc_read_req_t readReqParam;
                readReqParam.attrHandle = charValueHandle;
                readReqParam.connHandle = connHandle;

                apiResult = Cy_BLE_GATTC_ReadCharacteristicValue(&readReqParam);
            }
            /* Save handle to support service-specific read response from device */
            if(apiResult == CY_BLE_SUCCESS)
            {
                cy_ble_hidscReqHandle[discIdx] = charValueHandle;
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
* Function Name: Cy_BLE_HIDSC_SetCharacteristicDescriptor
***************************************************************************//**
*
*  Sends a request to the peer device to set the characteristic descriptor of the
*  specified characteristic of HID service.
*
*  Internally, Write Request is sent to the GATT Server and on successful
*  execution of the request on the server side, the following events can be
*  generated:
*  * \ref CY_BLE_EVT_HIDSS_NOTIFICATION_ENABLED
*  * \ref CY_BLE_EVT_HIDSS_NOTIFICATION_DISABLED
*
*  \param connHandle:   The BLE peer device connection handle.
*
*  \param serviceIndex: The index of the service instance. e.g. If two HID services
*                       are supported in your design, then first service will be
*                       identified by serviceIndex of 0 and the second by
*                       serviceIndex of 1.
*
*  \param charIndex:    The index of the service characteristic of type
*                       \ref cy_en_ble_hids_char_index_t.
*
*  \param descrIndex:   The index of the service characteristic descriptor of type
*                       \ref cy_en_ble_hids_descr_t.
*
*  \param attrSize:     The size of the characteristic value attribute.
*
*  \param attrValue:    The pointer to the characteristic descriptor value data that
*                       should be sent to the server device.
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
*  \events
*   In case of successful execution (return value = #CY_BLE_SUCCESS)
*   the following events can appear: \n
*   If the HIDS service-specific callback is registered
*   with \ref Cy_BLE_HIDS_RegisterAttrCallback():
*   * #CY_BLE_EVT_HIDSC_WRITE_DESCR_RESPONSE - In case if the requested attribute is
*     successfully written on the peer device, the details
*     (charIndex, descrIndex etc.) are provided with event parameter structure
*     of type \ref cy_stc_ble_hids_descr_value_t.
*   .
*   Otherwise (if an HID service-specific callback is not registered):
*   * #CY_BLE_EVT_GATTC_WRITE_RSP - If the requested attribute is
*     successfully written on the peer device.
*
*   * #CY_BLE_EVT_GATTC_ERROR_RSP - If an error occurred with the
*     requested attribute on the peer device, the details are provided with
*     an event parameter structure \ref cy_stc_ble_gatt_err_param_t.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_HIDSC_SetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                                uint8_t serviceIndex,
                                                                cy_en_ble_hids_char_index_t charIndex,
                                                                cy_en_ble_hids_descr_t descrIndex,
                                                                uint8_t attrSize,
                                                                uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult;
    cy_ble_gatt_db_attr_handle_t locDescrHandle;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(connHandle);

    if(Cy_BLE_GetConnectionState(connHandle) != CY_BLE_CONN_STATE_CLIENT_DISCOVERED)
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }
    else if((serviceIndex >= cy_ble_hidscConfigPtr->serviceCount) || ((uint8_t)charIndex >= CY_BLE_HIDSC_CHAR_COUNT) ||
            (descrIndex >= CY_BLE_HIDS_DESCR_COUNT) || (charIndex < CY_BLE_HIDS_REPORT_MAP) ||
            (discIdx >= cy_ble_configPtr->params->maxClientCount))
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else
    {
        locDescrHandle = Cy_BLE_HIDSC_GetCharacteristicDescriptorHandle(connHandle, serviceIndex, charIndex, descrIndex);

        if(locDescrHandle != CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
        {
            /* Fill all fields of Write Request structure ... */
            cy_stc_ble_gattc_write_req_t writeReqParam;
            writeReqParam.handleValPair.attrHandle = locDescrHandle;
            writeReqParam.handleValPair.value.val  = attrValue;
            writeReqParam.handleValPair.value.len  = attrSize;
            writeReqParam.connHandle               = connHandle;

            /* ... and send request to server device. */
            apiResult = Cy_BLE_GATTC_WriteCharacteristicDescriptors(&writeReqParam);
            /* Save handle to support service-specific read response from device */
            if(apiResult == CY_BLE_SUCCESS)
            {
                cy_ble_hidscReqHandle[discIdx] = locDescrHandle;
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
* Function Name: Cy_BLE_HIDSC_GetCharacteristicDescriptor
***************************************************************************//**
*
*  Gets a characteristic descriptor of the specified characteristic of the HID
*  service from the server device.
*  This function call can result in generation of the following events based on
*  the response from the server device.
*  * \ref CY_BLE_EVT_HIDSC_READ_DESCR_RESPONSE
*  * \ref CY_BLE_EVT_GATTC_ERROR_RSP
*
*  \param connHandle:   The connection handle.
*
*  \param serviceIndex: The index of the service instance. e.g. If two HID services
*                       are supported in your design, then first service will be
*                       identified by serviceIndex of 0 and the second by
*                       serviceIndex of 1.
*
*  \param charIndex:    The index of the service characteristic of type
*                       \ref cy_en_ble_hids_char_index_t.
*
*  \param descrIndex:   The index of the service characteristic descriptor of type
*                       \ref cy_en_ble_hids_descr_t.
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
*   CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE | The peer device doesn't have the particular descriptor
*
*  \events
*   In case of successful execution (return value = #CY_BLE_SUCCESS)
*   the following events can appear: \n
*   If the HID service-specific callback is registered
*   with \ref Cy_BLE_HIDS_RegisterAttrCallback():
*   * #CY_BLE_EVT_HIDSC_READ_DESCR_RESPONSE - In case if the requested attribute is
*     successfully read on the peer device, the details (charIndex,
*     descrIndex, value, etc.) are provided with event parameter structure
*     of type \ref cy_stc_ble_hids_descr_value_t.
*   .
*   Otherwise (if an HID service-specific callback is not registered):
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
cy_en_ble_api_result_t Cy_BLE_HIDSC_GetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                                uint8_t serviceIndex,
                                                                cy_en_ble_hids_char_index_t charIndex,
                                                                cy_en_ble_hids_descr_t descrIndex)
{
    cy_en_ble_api_result_t apiResult;
    cy_stc_ble_gattc_read_req_t readReqParam;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(connHandle);

    if((serviceIndex >= cy_ble_hidscConfigPtr->serviceCount) || ((uint8_t)charIndex >= CY_BLE_HIDSC_CHAR_COUNT) ||
       (descrIndex >= CY_BLE_HIDS_DESCR_COUNT) || (charIndex < CY_BLE_HIDS_REPORT_MAP) ||
       (discIdx >= cy_ble_configPtr->params->maxClientCount))
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else
    {
        readReqParam.attrHandle = Cy_BLE_HIDSC_GetCharacteristicDescriptorHandle(connHandle, serviceIndex, charIndex,
                                                                                 descrIndex);

        if(readReqParam.attrHandle != CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
        {
            readReqParam.connHandle = connHandle;
            apiResult = Cy_BLE_GATTC_ReadCharacteristicDescriptors(&readReqParam);

            /* Save handle to support service-specific read response from device */
            if(apiResult == CY_BLE_SUCCESS)
            {
                cy_ble_hidscReqHandle[discIdx] = readReqParam.attrHandle;
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
* Function Name: Cy_BLE_HIDSC_NotificationEventHandler
***************************************************************************//**
*
*  Handles the Notification event.
*
*  \param eventParam:  pointer to the data structure specified by the event.
*
******************************************************************************/
static void Cy_BLE_HIDSC_NotificationEventHandler(cy_stc_ble_gattc_handle_value_ntf_param_t *eventParam)
{
    uint8_t locServIndex;
    cy_en_ble_hids_char_index_t locCharIndex;

    if(Cy_BLE_HIDS_ApplCallback != NULL)
    {
        for(locServIndex = 0u; locServIndex < cy_ble_hidscConfigPtr->serviceCount; locServIndex++)
        {
            for(locCharIndex = CY_BLE_HIDS_PROTOCOL_MODE; (uint8_t)locCharIndex < CY_BLE_HIDSC_CHAR_COUNT; locCharIndex++)
            {
                if(Cy_BLE_HIDSC_GetCharacteristicValueHandle(eventParam->connHandle, locServIndex, locCharIndex) ==
                   eventParam->handleValPair.attrHandle)
                {
                    cy_stc_ble_hids_char_value_t notifValue;
                    notifValue.connHandle   = eventParam->connHandle;
                    notifValue.serviceIndex = locServIndex;
                    notifValue.charIndex    = locCharIndex;
                    notifValue.value        = &eventParam->handleValPair.value;

                    Cy_BLE_HIDS_ApplCallback((uint32_t)CY_BLE_EVT_HIDSC_NOTIFICATION, &notifValue);
                    cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
                    break;
                }
            }
        }
    }
}


/******************************************************************************
* Function Name: Cy_BLE_HIDSC_ReadResponseEventHandler
***************************************************************************//**
*
*  Handles the Read Response event.
*
*  \param eventParam:  pointer to the data structure specified by the event.
*
*
******************************************************************************/
static void Cy_BLE_HIDSC_ReadResponseEventHandler(cy_stc_ble_gattc_read_rsp_param_t *eventParam)
{
    uint8_t locReqHandle = 0u;
    uint8_t locServIndex;
    cy_en_ble_hids_char_index_t locCharIndex;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(eventParam->connHandle);

    if((discIdx < cy_ble_configPtr->params->maxClientCount) && (Cy_BLE_HIDS_ApplCallback != NULL) &&
       (CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE != cy_ble_hidscReqHandle[discIdx]))
    {
        for(locServIndex = 0u; (locServIndex < cy_ble_hidscConfigPtr->serviceCount) && (locReqHandle == 0u); locServIndex++)
        {
            for(locCharIndex = CY_BLE_HIDS_PROTOCOL_MODE; ((uint8_t)locCharIndex < CY_BLE_HIDSC_CHAR_COUNT) && (locReqHandle == 0u);
                locCharIndex++)
            {
                if(cy_ble_hidscReqHandle[discIdx] ==
                   Cy_BLE_HIDSC_GetCharacteristicValueHandle(eventParam->connHandle, locServIndex, locCharIndex))
                {
                    cy_stc_ble_hids_char_value_t locCharValue;
                    locCharValue.connHandle   = eventParam->connHandle;
                    locCharValue.serviceIndex = locServIndex;
                    locCharValue.charIndex    = locCharIndex;
                    locCharValue.value        = &eventParam->value;

                    cy_ble_hidscReqHandle[discIdx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
                    Cy_BLE_HIDS_ApplCallback((uint32_t)CY_BLE_EVT_HIDSC_READ_CHAR_RESPONSE, &locCharValue);
                    locReqHandle = 1u;
                }
                if(locCharIndex >= CY_BLE_HIDS_REPORT_MAP)
                {
                    cy_en_ble_hids_descr_t locDescIndex;

                    for(locDescIndex = CY_BLE_HIDS_REPORT_CCCD; (locDescIndex < CY_BLE_HIDS_DESCR_COUNT) &&
                        (locReqHandle == 0u); locDescIndex++)
                    {
                        if(cy_ble_hidscReqHandle[discIdx] ==
                           Cy_BLE_HIDSC_GetCharacteristicDescriptorHandle(eventParam->connHandle, locServIndex,
                                                                          locCharIndex, locDescIndex))
                        {
                            cy_stc_ble_hids_descr_value_t locDescrValue;
                            locDescrValue.connHandle   = eventParam->connHandle;
                            locDescrValue.serviceIndex = locServIndex;
                            locDescrValue.charIndex    = locCharIndex;
                            locDescrValue.descrIndex   = locDescIndex;
                            locDescrValue.value        = &eventParam->value;

                            cy_ble_hidscReqHandle[discIdx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
                            Cy_BLE_HIDS_ApplCallback((uint32_t)CY_BLE_EVT_HIDSC_READ_DESCR_RESPONSE, &locDescrValue);
                            locReqHandle = 1u;
                        }
                    }
                }
            }
        }
        if(locReqHandle != 0u)
        {
            cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
        }
    }
}


/******************************************************************************
* Function Name: Cy_BLE_HIDSC_WriteResponseEventHandler
***************************************************************************//**
*  Handles the Write Response event.
*
*  \param eventParam:  the pointer to the data structure specified by the event.
*
*
******************************************************************************/
static void Cy_BLE_HIDSC_WriteResponseEventHandler(const cy_stc_ble_conn_handle_t *eventParam)
{
    uint8_t locServIndex;
    uint8_t locReqHandle = 0u;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(*eventParam);
    cy_en_ble_hids_char_index_t locCharIndex;

    if((discIdx < cy_ble_configPtr->params->maxClientCount) && (Cy_BLE_HIDS_ApplCallback != NULL) &&
       (CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE != cy_ble_hidscReqHandle[discIdx]))
    {
        for(locServIndex = 0u; (locServIndex < cy_ble_hidscConfigPtr->serviceCount) && (locReqHandle == 0u); locServIndex++)
        {
            for(locCharIndex = CY_BLE_HIDS_PROTOCOL_MODE; ((uint8_t)locCharIndex < CY_BLE_HIDSC_CHAR_COUNT) && (locReqHandle == 0u);
                locCharIndex++)
            {
                if(cy_ble_hidscReqHandle[discIdx] == Cy_BLE_HIDSC_GetCharacteristicValueHandle(*eventParam, locServIndex,
                                                                                               locCharIndex))
                {
                    cy_stc_ble_hids_char_value_t locCharValue;
                    locCharValue.connHandle   = *eventParam;
                    locCharValue.serviceIndex = locServIndex;
                    locCharValue.charIndex    = locCharIndex;
                    locCharValue.value        = NULL;

                    cy_ble_hidscReqHandle[discIdx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
                    Cy_BLE_HIDS_ApplCallback((uint32_t)CY_BLE_EVT_HIDSC_WRITE_CHAR_RESPONSE, &locCharValue);
                    locReqHandle = 1u;
                }
                if(locCharIndex >= CY_BLE_HIDS_REPORT_MAP)
                {
                    cy_en_ble_hids_descr_t locDescIndex;

                    for(locDescIndex = CY_BLE_HIDS_REPORT_CCCD; (locDescIndex < CY_BLE_HIDS_DESCR_COUNT) &&
                        (locReqHandle == 0u); locDescIndex++)
                    {
                        if(cy_ble_hidscReqHandle[discIdx] ==
                           Cy_BLE_HIDSC_GetCharacteristicDescriptorHandle(*eventParam, locServIndex, locCharIndex,
                                                                          locDescIndex))
                        {
                            cy_stc_ble_hids_descr_value_t locDescrValue;
                            locDescrValue.connHandle   = *eventParam;
                            locDescrValue.serviceIndex = locServIndex;
                            locDescrValue.charIndex    = locCharIndex;
                            locDescrValue.descrIndex   = locDescIndex;
                            locDescrValue.value        = NULL;

                            cy_ble_hidscReqHandle[discIdx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
                            Cy_BLE_HIDS_ApplCallback((uint32_t)CY_BLE_EVT_HIDSC_WRITE_DESCR_RESPONSE, &locDescrValue);
                            locReqHandle = 1u;
                        }
                    }
                }
            }
        }
        if(locReqHandle != 0u)
        {
            cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
        }
    }
}


/******************************************************************************
* Function Name: Cy_BLE_HIDSC_GetCharacteristicValueHandle
***************************************************************************//**
*
*  Returns the discovered peer device characteristic value handle.
*
*  \param connHandle:   The connection handle.
*  \param serviceIndex: The index of the service instance.
*  \param charIndex:    The index of a service characteristic.
*
*  \return
*   Returns characteristic value handle of type cy_ble_gatt_db_attr_handle_t.
*   * CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE: when a peer device doesn't have an optional characteristic
*
******************************************************************************/
cy_ble_gatt_db_attr_handle_t Cy_BLE_HIDSC_GetCharacteristicValueHandle(cy_stc_ble_conn_handle_t connHandle,
                                                                       uint8_t serviceIndex,
                                                                       cy_en_ble_hids_char_index_t charIndex)
{
    cy_ble_gatt_db_attr_handle_t returnHandle;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(connHandle);

    /* Get pointer (with offset) to HIDS client structure with attribute handles */
    cy_stc_ble_hidsc_t *hidscPtr = (cy_stc_ble_hidsc_t *)&cy_ble_hidscConfigPtr->
                                    attrInfo[(discIdx * cy_ble_hidscConfigPtr->serviceCount) + serviceIndex];

    if((serviceIndex >= cy_ble_hidscConfigPtr->serviceCount) || ((uint8_t)charIndex >= CY_BLE_HIDSC_CHAR_COUNT))
    {
        returnHandle = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
    }
    else
    {
        if(charIndex == CY_BLE_HIDS_PROTOCOL_MODE)
        {
            returnHandle = hidscPtr->protocolMode.valueHandle;
        }
        else if(charIndex == CY_BLE_HIDS_INFORMATION)
        {
            returnHandle = hidscPtr->information.valueHandle;
        }
        else if(charIndex == CY_BLE_HIDS_CONTROL_POINT)
        {
            returnHandle = hidscPtr->controlPoint.valueHandle;
        }
        else if(charIndex == CY_BLE_HIDS_REPORT_MAP)
        {
            returnHandle = hidscPtr->reportMap.valueHandle;
        }
        else if(charIndex <= CY_BLE_HIDS_BOOT_MOUSE_IN_REP)
        {
            returnHandle = hidscPtr->bootReport[charIndex - CY_BLE_HIDS_BOOT_KYBRD_IN_REP].valueHandle;
        }
        else if(((uint8_t)charIndex - (uint8_t)CY_BLE_HIDS_REPORT) < hidscPtr->reportCount)
        {
            returnHandle = hidscPtr->report[charIndex - CY_BLE_HIDS_REPORT].valueHandle;
        }
        else
        {
            returnHandle = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
        }
    }

    return(returnHandle);
}


/******************************************************************************
* Function Name: Cy_BLE_HIDSC_GetCharacteristicDescriptorHandle
***************************************************************************//**
*
*  Returns the discovered peer device characteristic descriptor handle.
*
*  \param connHandle:   The connection handle.
*  \param serviceIndex: The index of the service instance.
*  \param charIndex:    The index of a service characteristic.
*  \param descrIndex:   The index of the descriptor.
*
* \return
*  Returns characteristic value handle of type cy_ble_gatt_db_attr_handle_t.
*  * CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE: when a peer device doesn't have an optional descriptor
*
******************************************************************************/
cy_ble_gatt_db_attr_handle_t Cy_BLE_HIDSC_GetCharacteristicDescriptorHandle(cy_stc_ble_conn_handle_t connHandle,
                                                                            uint8_t serviceIndex,
                                                                            cy_en_ble_hids_char_index_t charIndex,
                                                                            cy_en_ble_hids_descr_t descrIndex)
{
    cy_ble_gatt_db_attr_handle_t returnHandle;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(connHandle);
    /* Get pointer (with offset) to HIDS client structure with attribute handles */
    cy_stc_ble_hidsc_t *hidscPtr = (cy_stc_ble_hidsc_t *)&cy_ble_hidscConfigPtr->
                                    attrInfo[(discIdx * cy_ble_hidscConfigPtr->serviceCount) + serviceIndex];

    if((serviceIndex >= cy_ble_hidscConfigPtr->serviceCount) || ((uint8_t)charIndex >= CY_BLE_HIDSC_CHAR_COUNT) ||
       (charIndex < CY_BLE_HIDS_REPORT_MAP))
    {
        returnHandle = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
    }
    else
    {
        if(charIndex == CY_BLE_HIDS_REPORT_MAP)
        {
            if(descrIndex == CY_BLE_HIDS_REPORT_MAP_ERRD)
            {
                returnHandle = hidscPtr->reportMap.errdHandle;
            }
            else
            {
                returnHandle = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
            }
        }
        else if(charIndex <= CY_BLE_HIDS_BOOT_MOUSE_IN_REP)
        {
            if(descrIndex == CY_BLE_HIDS_REPORT_CCCD)
            {
                returnHandle = hidscPtr->bootReport[charIndex - CY_BLE_HIDS_BOOT_KYBRD_IN_REP].cccdHandle;
            }
            else if(descrIndex == CY_BLE_HIDS_REPORT_RRD)
            {
                returnHandle = hidscPtr->bootReport[charIndex - CY_BLE_HIDS_BOOT_KYBRD_IN_REP].rrdHandle;
            }
            else
            {
                returnHandle = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
            }
        }
        else if(((uint8_t)charIndex - (uint8_t)CY_BLE_HIDS_REPORT) < hidscPtr->reportCount)
        {
            if(descrIndex == CY_BLE_HIDS_REPORT_CCCD)
            {
                returnHandle = hidscPtr->report[charIndex - CY_BLE_HIDS_REPORT].cccdHandle;
            }
            else if(descrIndex == CY_BLE_HIDS_REPORT_RRD)
            {
                returnHandle = hidscPtr->report[charIndex - CY_BLE_HIDS_REPORT].rrdHandle;
            }
            else
            {
                returnHandle = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
            }
        }
        else
        {
            returnHandle = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
        }
    }

    return(returnHandle);
}


/******************************************************************************
* Function Name: Cy_BLE_HIDSC_ErrorResponseEventHandler
***************************************************************************//**
*
*  Handles the Error Response event.
*
*  \param eventParam:  the pointer to the data structure specified by the event.
*
******************************************************************************/
static void Cy_BLE_HIDSC_ErrorResponseEventHandler(const cy_stc_ble_gatt_err_param_t *eventParam)
{
    if(eventParam != NULL)
    {
        if(cy_ble_hidscReqHandle[Cy_BLE_GetDiscoveryIdx(eventParam->connHandle)] == eventParam->errInfo.attrHandle)
        {
            cy_ble_hidscReqHandle[Cy_BLE_GetDiscoveryIdx(eventParam->connHandle)] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
        }
    }
}


/******************************************************************************
* Function Name: Cy_BLE_HIDS_EventHandler
***************************************************************************//**
*
*  Handles the events from the BLE Stack for the HID service.
*
*  \param eventCode:  the event code
*  \param eventParam: the event parameters
*
* \return
*  A return value of type cy_en_ble_gatt_err_code_t.
*
******************************************************************************/
static cy_en_ble_gatt_err_code_t Cy_BLE_HIDS_EventHandler(uint32_t eventCode,
                                                          void *eventParam)
{
    cy_en_ble_gatt_err_code_t gattErr = CY_BLE_GATT_ERR_NONE;

    /************************************
     * Handling GATT Server events
     ***********************************/
    if(Cy_BLE_HIDSS_EventHandlerCallback != NULL)
    {
        gattErr = Cy_BLE_HIDSS_EventHandlerCallback(eventCode, eventParam);
    }

    /************************************
     * Handling GATT Client events
     ***********************************/
    if(Cy_BLE_HIDSC_EventHandlerCallback != NULL)
    {
        (void)Cy_BLE_HIDSC_EventHandlerCallback(eventCode, eventParam);
    }

    return(gattErr);
}


/******************************************************************************
* Function Name: Cy_BLE_HIDSS_EventHandler
***************************************************************************//**
*
*  Handles the server events from the BLE Stack for the HID service
*
*  \param eventCode:  the event code
*  \param eventParam:  the event parameters
*
* \return
*  A return value of type cy_en_ble_gatt_err_code_t.
*
******************************************************************************/
static cy_en_ble_gatt_err_code_t Cy_BLE_HIDSS_EventHandler(uint32_t eventCode, void *eventParam)
{
    cy_en_ble_gatt_err_code_t gattErr = CY_BLE_GATT_ERR_NONE;

    /* Handling GATT Server events */
    switch((cy_en_ble_event_t)eventCode)
    {
        case CY_BLE_EVT_GATT_CONNECT_IND:
            Cy_BLE_HIDSS_OnDeviceConnected();
            break;

        case CY_BLE_EVT_GATTS_WRITE_REQ:
            gattErr = Cy_BLE_HIDSS_WriteEventHandler((cy_stc_ble_gatts_write_cmd_req_param_t*)eventParam);
            break;

        case CY_BLE_EVT_GATTS_WRITE_CMD_REQ:
            (void)Cy_BLE_HIDSS_WriteEventHandler((cy_stc_ble_gatts_write_cmd_req_param_t*)eventParam);
            break;

        default:
            break;
    }

    return (gattErr);
}


/******************************************************************************
* Function Name: Cy_BLE_HIDSC_EventHandler
***************************************************************************//**
*
*  Handles the client events from the BLE Stack for the HID service
*
*  \param eventCode:  the event code
*  \param eventParam:  the event parameters
*
* \return
*  A return value of type cy_en_ble_gatt_err_code_t.
*
******************************************************************************/
static cy_en_ble_gatt_err_code_t Cy_BLE_HIDSC_EventHandler(uint32_t eventCode, void *eventParam)
{
    cy_en_ble_gatt_err_code_t gattErr = CY_BLE_GATT_ERR_NONE;

    if(eventCode > (uint32_t)CY_BLE_EVT_MAX)
    {
        /* Handling Client service-specific events */
        switch((cy_en_ble_evt_t)eventCode)
        {
            /* Discovery events */
            case CY_BLE_EVT_GATTC_DISC_CHAR:
                Cy_BLE_HIDSC_DiscoverCharacteristicsEventHandler((cy_stc_ble_disc_char_info_t*)eventParam);
                break;

            case CY_BLE_EVT_GATTC_DISC_INCL:
                Cy_BLE_HIDSC_InclDiscoveryEventHandler((cy_stc_ble_disc_incl_info_t*)eventParam);
                break;

            case CY_BLE_EVT_GATTC_DISC_DESCR:
                Cy_BLE_HIDSC_DiscoverCharDescriptorsEventHandler((cy_stc_ble_disc_descr_info_t*)eventParam);
                break;

            case CY_BLE_EVT_GATTC_DISC_DESCR_GET_RANGE:
                Cy_BLE_HIDSC_GetCharRange((cy_stc_ble_disc_range_info_t*)eventParam);
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
                if((discIdx < cy_ble_configPtr->params->maxClientCount) &&
                   (cy_ble_configPtr->context->discovery[discIdx].autoDiscoveryFlag == 0u) &&
                   (((cy_stc_ble_gatt_err_param_t*)eventParam)->errInfo.errorCode !=
                        CY_BLE_GATT_ERR_ATTRIBUTE_NOT_FOUND))
                {
                    Cy_BLE_HIDSC_ErrorResponseEventHandler((cy_stc_ble_gatt_err_param_t*)eventParam);
                }
            }
                break;

            case CY_BLE_EVT_GATTC_HANDLE_VALUE_NTF:
                Cy_BLE_HIDSC_NotificationEventHandler((cy_stc_ble_gattc_handle_value_ntf_param_t*)eventParam);
                break;

            case CY_BLE_EVT_GATTC_READ_RSP:
                Cy_BLE_HIDSC_ReadResponseEventHandler((cy_stc_ble_gattc_read_rsp_param_t*)eventParam);
                break;

            case CY_BLE_EVT_GATTC_WRITE_RSP:
                Cy_BLE_HIDSC_WriteResponseEventHandler((cy_stc_ble_conn_handle_t*)eventParam);
                break;

            default:
                break;
        }
    }
    return (gattErr);
}

#endif /* CY_BLE_LIB_HOST_CORE */
#endif /* CY_IP_MXBLESS */


/* [] END OF FILE */
