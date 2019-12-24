/*******************************************************************************
* \file cy_ble_stack_gatt_client.h
* \version 3.30
*
* \brief
*  This file contains declarations of public BLE APIs of Generic Attribute Profile - Client Role.
*  Also specifies the defines, constants, and data structures required for the APIs.
* 
* Related Document:
*  BLE Standard Spec - CoreV4.2, CSS, CSAs, ESR05, ESR06
* 
********************************************************************************
* \copyright
* Copyright 2017-2019, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#ifndef CY_BLE_STACK_GATT_CLIENT_H_
#define CY_BLE_STACK_GATT_CLIENT_H_


/***************************************
* Common BLE Stack includes
***************************************/

#include "cy_ble_stack_gatt.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/***************************************
** Exported data structures
***************************************/
/**
 \addtogroup group_ble_common_api_gatt_definitions
 @{
*/

/** Stop command parameter */
typedef struct
{
    /**Connection handle*/
    cy_stc_ble_conn_handle_t     connHandle; 
    
}cy_stc_ble_gattc_stop_cmd_param_t;

/** GATT Discover primary service request parameter */
typedef struct
{
    /** Handle Range */
    cy_stc_ble_gatt_attr_handle_range_t      range;     

    /**Connection handle*/
    cy_stc_ble_conn_handle_t                 connHandle; 
    
}cy_stc_ble_gattc_read_by_group_req_t;

/** GATT Discover primary service by UUID request parameter */
typedef struct
{
    /** Attribute Value to Find */
    cy_stc_ble_gatt_value_t                   value; 

    /** Handle Range - Start and End Handle */
    cy_stc_ble_gatt_attr_handle_range_t         range;  

    /** 16-bit UUID to Find */
    cy_ble_uuid16_t                           uuid; 
    
    /**Connection handle*/
    cy_stc_ble_conn_handle_t                 connHandle; 

}cy_stc_ble_gattc_find_by_type_value_req_t;

/** GATT read by type request parameter */
typedef struct
{
    /** Handle Range */
    cy_stc_ble_gatt_attr_handle_range_t      range;

    /** GATT UUID type */
    cy_ble_uuid_t                            uuid;

   /** Format indicating, 16-bit or 128-bit UUIDs
       * For 16-bit UUID format - CY_BLE_GATT_16_BIT_UUID_FORMAT (0x01)
       * For 128-bit UUID format - CY_BLE_GATT_128_BIT_UUID_FORMAT (0x02)
    */
    uint8_t                                 uuidFormat;   
    
    /**Connection handle*/
    cy_stc_ble_conn_handle_t                 connHandle; 
    
}cy_stc_ble_gattc_read_by_type_req_t;

/** GATT find info request parameter */
typedef cy_stc_ble_gattc_read_by_group_req_t cy_stc_ble_gattc_find_info_req_t;

/** Read request parameter */
typedef struct
{
    /** Handle on which Read Blob is requested */
    cy_ble_gatt_db_attr_handle_t        attrHandle;    

    /**Connection handle*/
    cy_stc_ble_conn_handle_t             connHandle; 
    
}cy_stc_ble_gattc_read_req_t;

/** Handle-offset pair parameter */
typedef struct
{
    /** Handle on which Read Blob is requested */
    cy_ble_gatt_db_attr_handle_t        attrHandle;    

    /** Value Offset from which the Read is Requested */
    uint16_t                             offset;    
    
}cy_stc_ble_gattc_handle_offset_pair_t;

/** Read blob request parameter */
typedef struct
{
    /** Handle-offset pair parameter */
    cy_stc_ble_gattc_handle_offset_pair_t    handleOffset;    
    
    /**Connection handle*/
    cy_stc_ble_conn_handle_t                 connHandle; 
    
}cy_stc_ble_gattc_read_blob_req_t;

/** GATT handle list type */
typedef struct
{
    /** Handle list where the UUID with value indicated is found */
    uint16_t    * handleList;

    /** Number of Handles in the list */
    uint16_t    listCount;

    /** Actual Number of Handles Packed. This is a output parameter */
    uint16_t    actualCount;

}cy_stc_ble_gattc_handle_list_t;

/** Read multiple request parameter */
typedef struct
{
    /** GATT handle list type */
    cy_stc_ble_gattc_handle_list_t  handleListType;
    
    /**Connection handle*/
    cy_stc_ble_conn_handle_t        connHandle; 

}cy_stc_ble_gattc_read_mult_req_t;

/** Write request parameter */
typedef cy_stc_ble_gatt_write_param_t  cy_stc_ble_gattc_write_req_t;

/** Prepare Write request parameter */
typedef cy_stc_ble_gatt_prep_write_param_t  cy_stc_ble_gattc_prep_write_req_t;

/** Prepare Write request parameter for reliable write request */
typedef struct
{
    /** handle value offset pair */
    cy_stc_ble_gatt_handle_value_offset_param_t  * handleValOffsetPair; 

    /** Connection handle */
    cy_stc_ble_conn_handle_t                     connHandle;  
    
    /** Number of requests. That is, the count of array of structures 
             of type 'cy_stc_ble_gatt_handle_value_offset_param_t'. Each array element 
             represents a value and the attribute to which the value has 
             to be written */
    uint8_t                                   numOfRequests;
    
}cy_stc_ble_gattc_reliable_write_req_t;

/** Execute Write request parameter */
typedef struct
{
    /** Connection handle */
    cy_stc_ble_conn_handle_t     connHandle;             

    /** Indicates whether Queued Write is to be executed (0x01) or canceled (0x00) */
    uint8_t                   flag;
    
}cy_stc_ble_gattc_exec_write_req_t;

/** Write command request to be sent to Server */
typedef cy_stc_ble_gattc_write_req_t  cy_stc_ble_gattc_write_cmd_req_t;

/** Signed Write command request to be sent to Server */
typedef cy_stc_ble_gattc_write_req_t  cy_stc_ble_gattc_signed_write_cmd_req_t;

/** Signed Write command request to be sent to Server */
typedef cy_stc_ble_gattc_stop_cmd_param_t  cy_stc_ble_gattc_confirmation_req_t;

/* --------------------------Structure corresponding to events-------------------- */

/** Read response parameter type received from server*/
typedef struct
{
    /** Attribute Value  */
    cy_stc_ble_gatt_value_t      value;     

    /** Connection handle */
    cy_stc_ble_conn_handle_t     connHandle; 
    
}cy_stc_ble_gattc_read_rsp_param_t;

/** Handle value notification data received from server */
typedef cy_stc_ble_gattc_write_req_t  cy_stc_ble_gattc_handle_value_ntf_param_t;

/** GATT handle value indication parameter received from server type */
typedef cy_stc_ble_gattc_write_req_t  cy_stc_ble_gattc_handle_value_ind_param_t;

/** Data Element for Group Response */
typedef struct
{
    /** attribute handle value pair */
    uint8_t       * attrValue;

    /** Length of each Attribute Data Element including the Handle Range */
    uint16_t      length;

    /** Total Length of Attribute Data */
    uint16_t      attrLen;
}cy_stc_ble_gattc_grp_attr_data_list_t;

/** Read By Group Response received from Server*/
typedef struct
{
    /** Group attribute data list */
    cy_stc_ble_gattc_grp_attr_data_list_t   attrData;

    /** Connection handle */
    cy_stc_ble_conn_handle_t                connHandle; 
    
}cy_stc_ble_gattc_read_by_grp_rsp_param_t;

/** GATT read by type response received from server*/
typedef cy_stc_ble_gattc_read_by_grp_rsp_param_t cy_stc_ble_gattc_read_by_type_rsp_param_t;

/** GATT find by type value response received from server*/
typedef struct
{
    /** Handle Range list */
    cy_stc_ble_gatt_attr_handle_range_t        * range; 

     /** Size of list */
    uint8_t                                  count;  

    /** Connection handle */
    cy_stc_ble_conn_handle_t                 connHandle; 
     
}cy_stc_ble_gattc_find_by_type_rsp_param_t;

/** GATT list of Handle UUID pair parameter type */
typedef struct
{
    /** Handle - UUID Pair list
       This is a packed byte stream, hence it needs to be unpacked and decoded. */
    uint8_t      *list;     

    /** Number of elements in the list in bytes */
    uint16_t  byteCount;
    
}cy_stc_ble_gattc_handle_uuid_list_param_t;

/** GATT find info response received from Server*/
typedef struct
{    
    /** Handle Value list */    
    cy_stc_ble_gattc_handle_uuid_list_param_t       handleValueList;  

    /** Connection handle */
    cy_stc_ble_conn_handle_t                     connHandle;

    /** Format indicating, 16-bit (0x01) or 128-bit (0x02) UUIDs */    
    uint8_t                                   uuidFormat;         
    
}cy_stc_ble_gattc_find_info_rsp_param_t;

/** Execute Write result */
typedef struct
{
    /**Connection handle*/
    cy_stc_ble_conn_handle_t             connHandle; 
    
    /** Result of the execute write request */
    uint8_t                             result;       

}cy_stc_ble_gattc_exec_write_rsp_param_t;

/** Long procedure end indication event parameter */
typedef struct
{
    /**Connection handle*/
    cy_stc_ble_conn_handle_t             connHandle; 
    
    /** Result of the execute write request */
    uint8_t                             opcode;       

}cy_stc_ble_gattc_long_procedure_end_param_t;

/** Execute Write response parameter */
typedef cy_stc_ble_gattc_exec_write_req_t  cy_stc_ble_gattc_exec_write_rsp_t;
/** @} */
/***************************************
** Exported APIs
***************************************/
/**
 \addtogroup group_ble_common_api_gatt_client_functions
 @{
*/

/******************************************************************************
* Function Name: Cy_BLE_GATTC_StopCmd
***************************************************************************//**
* 
*  This function is used by the GATT Client application to stop any of the following ongoing 
*  GATT procedures:
*   1.  Cy_BLE_GATTC_DiscoverPrimaryServices()
*   2.  Cy_BLE_GATTC_DiscoverPrimaryServiceByUuid()
*   3.  Cy_BLE_GATTC_FindIncludedServices()
*   4.  Cy_BLE_GATTC_DiscoverCharacteristics()
*   5.  Cy_BLE_GATTC_DiscoverCharacteristicByUuid()
*   6.  Cy_BLE_GATTC_DiscoverCharacteristicDescriptors()
*   7.  Cy_BLE_GATTC_ReadLongCharacteristicValues()
*   8.  Cy_BLE_GATTC_WriteLongCharacteristicValues()
*   9.  Cy_BLE_GATTC_ReliableWrites()
*   10. Cy_BLE_GATTC_ReadLongCharacteristicDescriptors()
*   11. Cy_BLE_GATTC_WriteLongCharacteristicDescriptors()
*   12. Cy_BLE_GATTC_ReadByTypeReq()
*   .
*  If none of the above procedures is ongoing, then this command will be ignored.
*  This function has no effect on ATT procedures other than those listed above.
*  If the procedure is ongoing, this function will stop it and inform that the operation  
*  is successful through the event 'CY_BLE_EVT_GATTC_STOP_CMD_COMPLETE'. 
*  This is a non-blocking function.
*
*  If the user intends to start a new GATT procedure including those listed above
*  and there is an ongoing GATT procedure (any one from the above list), the user
*  needs to call this function to stop the ongoing GATT procedure and then invoke
*  the desired GATT procedure.
* 
*  \param param: parameter is of type cy_stc_ble_gattc_stop_cmd_param_t.
*   param->connHandle: Connection handle to identify the peer GATT entity of type
*              cy_stc_ble_conn_handle_t.
*
* \return
*  cy_en_ble_api_result_t : Return value indicates whether the function succeeded or
*  failed. Following are the possible error codes.
*
*  Error codes                           | Description
*  ------------                          | -----------
*  CY_BLE_SUCCESS                        | On successful operation.
*  CY_BLE_ERROR_INVALID_PARAMETER        | If 'param' is NULL or 'connHandle' is invalid.
* 
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_GATTC_StopCmd
(
    cy_stc_ble_gattc_stop_cmd_param_t  * param
);


/******************************************************************************
* Function Name: Cy_BLE_GATTC_ExchangeMtuReq
***************************************************************************//**
* 
*  This function is used by the GATT Client application to send Exchange MTU Request PDU.
*  This function is used to send maximum size of packet (GATT MTU) that cab be received by
*  GATT Client application. This is a non-blocking function.
* 
*  Default GATT MTU size as per Bluetooth 5.0 core specification is 23 bytes. If
*  the GATT Client application supports a size greater than the default, it has to invoke
*  this function with the desired GATT MTU size. This function should be
*  initiated only once during a connection.
* 
*  Refer to  Bluetooth 5.0 core specification, Volume 3, Part G, section 4.3.1
*  for more details on GATT MTU exchange operation.
* 
*  If the peer GATT Server device is a PSoC 6 MCU, Exchange MTU Request PDU results in 
*  CY_BLE_EVT_GATTS_XCNHG_MTU_REQ event at the GATT Server's end, in response 
*  to which the GATT Server is expected to send its maximum size of packet (GATT MTU).
* 
*  The CY_BLE_EVT_GATTC_XCHNG_MTU_RSP event is generated at the
*  GATT Client application on receiving GATT MTU response from the GATT Server.
* 
*  \param param: parameter is of type CY_BLE_GATT_XCHG_MTU_REQ_PARAM_T.
*   param->mtu: Size of GATT MTU.
* 
* \return
*  cy_en_ble_api_result_t : Return value indicates whether the function succeeded or
*  failed. Following are the possible error codes.
*
*  Error codes                          | Description
*  ------------                         | -----------
*  CY_BLE_SUCCESS                       | On successful operation.
*  CY_BLE_ERROR_INVALID_PARAMETER       | If 'param' is NULL or 'connHandle' is invalid or, 'mtu' value is greater than that set on calling Cy_BLE_StackInit().
*  CY_BLE_ERROR_INVALID_OPERATION       | This operation is not permitted.
*  CY_BLE_ERROR_MEMORY_ALLOCATION_FAILED| Memory allocation failed.
* 
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_GATTC_ExchangeMtuReq
(
    cy_stc_ble_gatt_xchg_mtu_param_t  * param
);


/******************************************************************************
* Function Name: Cy_BLE_GATTC_DiscoverPrimaryServices
***************************************************************************//**
* 
* This function is used by the GATT Client application to discover the primary services as
* per the handle-range provided, on a peer GATT Server. 
* This is a non-blocking function.
*
* Internally, this function initiates multiple Read By Group Type Requests to
* the peer device, in response to which it receives Read By Group Type Responses.
* Each Read By Group Type Response results in 
* CY_BLE_EVT_GATTC_READ_BY_GROUP_TYPE_RSP event, which is propagated to the 
* application layer for handling.
*
* Primary service discovery is complete when Error Response 
* (CY_BLE_EVT_GATTC_ERROR_RSP) is received with Error Code set to Attribute
* Not Found or when the End Group Handle in the Read by Group Type Response is
* 0xFFFF. Completion of this operation is notified to the upper layer(s) using
* CY_BLE_EVT_GATTC_ERROR_RSP or CY_BLE_EVT_GATTC_LONG_PROCEDURE_END event.
*
* Application can stop the primary service discovery if the
* desired primary service is found prior to discovering all the primary services
* on the GATT Server. This can be achieved by using the Cy_BLE_GATTC_StopCmd() 
* function.
*
* Refer to Bluetooth 5.0 core specification, Volume 3, Part G, section 4.5.0 for
* more details on this sequence of operations.
*
*  \param param: parameter is of type cy_stc_ble_gattc_read_by_group_req_t.
*   param->connHandle: Connection handle of type cy_stc_ble_conn_handle_t  
*              to identify the peer GATT entity.
*   param->range: Parameter is of type cy_stc_ble_gatt_attr_handle_range_t where,
*        1. 'range.startHandle' can be set to the start handle of the desired
*            primary service.
*        2. 'range.endHandle' can be set to the end handle of the desired 
*            primary service.
*   \note
*   'startHandle' & 'endHandle' should not be zero and 'startHandle' shall be less than or equal to the 'endHandle'.
*
* \return
* cy_en_ble_api_result_t : Return value indicates whether the function succeeded or
* failed. Following are the possible error codes.
*
*  Error codes                          | Description
*  ------------                         | -----------
*  CY_BLE_SUCCESS                       | On successful operation.
*  CY_BLE_ERROR_INVALID_PARAMETER       | If 'param' is NULL or if any of the input parameters is invalid.
*  CY_BLE_ERROR_INVALID_OPERATION       | This operation is not permitted.
*  CY_BLE_ERROR_MEMORY_ALLOCATION_FAILED| Memory allocation failed.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_GATTC_DiscoverPrimaryServices
(
    cy_stc_ble_gattc_read_by_group_req_t  * param
);


/******************************************************************************
* Function Name: Cy_BLE_GATTC_DiscoverPrimaryServiceByUuid
***************************************************************************//**
* 
*  This function is used by the GATT Client application to discover a specific primary
*  service on a peer GATT Server, when only the Service UUID is known. 
*  This is a non-blocking function.
* 
*  Internally, this function initiates multiple Find By Type Value Requests with
*  the Attribute Type parameter set to the UUID for Primary Service and the
*  Attribute Value set to the 16-bit Bluetooth UUID or 128-bit UUID for the
*  specific primary service. Each Find By Type Value Response received from the
*  peer device is passed to the application as 
*  CY_BLE_EVT_GATTC_FIND_BY_TYPE_VALUE_RSP event.
* 
*  The sequence of operations is complete when the Error Response is received
*  and the Error Code is set to Attribute Not Found or when the End Group 
*  Handle in the Find By Type Value Response is 0xFFFF. Completion of this 
*  function is notified to the upper layer using CY_BLE_EVT_GATTC_ERROR_RSP or 
*  CY_BLE_EVT_GATTC_LONG_PROCEDURE_END event.
* 
*  Application can stop this procedure if a desired primary service is found 
*  prior to the discovery of all the primary services of the specified service UUID supported 
*  on the GATT Server. This can be achieved by using the Cy_BLE_GATTC_StopCmd() 
*  function. 
* 
*  Refer to Bluetooth 5.0 core specification, Volume 3, Part G, section 4.4.2 
*  for more details on this sequence of operations.
*     
*  \param param: parameter is of type cy_stc_ble_gattc_find_by_type_value_req_t.
*   param->connHandle: Connection handle of type cy_stc_ble_conn_handle_t   
*               to identify the peer GATT entity.
*   param->range and param->uuid are ignored in this function.
*   param->value: Parameter is of type cy_stc_ble_gatt_value_t, where,
*          1. 'value.val' should point to uint8_t array containing the UUID to 
*              look for. UUID can be 16 or 128 bit.
*          2. 'value.len' should be set to 2 if the 16-bit UUID is to be found.
*              The length should be set to 16 if 128-bit UUID is to be found.
*          3. 'value.actualLen' is an unused parameter and should be ignored.
* 
* \return
*  cy_en_ble_api_result_t : Return value indicates whether the function succeeded or
*  failed. Following are the possible error codes.
*
*  Error codes                          | Description
*  ------------                         | -----------
*  CY_BLE_SUCCESS                       | On successful operation.
*  CY_BLE_ERROR_INVALID_PARAMETER       | If 'param' is NULL or 'connHandle' is invalid.
*  CY_BLE_ERROR_INVALID_OPERATION       | This operation is not permitted.
*  CY_BLE_ERROR_MEMORY_ALLOCATION_FAILED| Memory allocation failed.
* 
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_GATTC_DiscoverPrimaryServiceByUuid
(
    cy_stc_ble_gattc_find_by_type_value_req_t  * param
);


/******************************************************************************
* Function Name: Cy_BLE_GATTC_DiscoverCharacteristics
***************************************************************************//**
* 
*  This function is used by the GATT Client application to find all characteristic
*  declarations within a service definition on a peer GATT Server, when
*  only the service handle range is known. 
*  This is a non-blocking function.
* 
*  Internally, multiple Read By Type Requests are sent to the GATT Server in
*  response to which Read By Type Responses are received. Each response results
*  in the event CY_BLE_EVT_GATTC_READ_BY_TYPE_RSP, which is passed to the 
*  application layer for handling.
*
*  The sequence of operations is complete when the Error Response is received and the
*  Error Code is set to Attribute Not Found or the Read By Type Response has an
*  Attribute Handle that is equal to the Ending Handle of the request.
*  Completion of this function is notified to the upper layer using
*  CY_BLE_EVT_GATTC_ERROR_RSP or with the CY_BLE_EVT_GATTC_LONG_PROCEDURE_END
*  event.
* 
*  Application can stop this procedure if a desired characteristic is found 
*  prior to the discovering all the characteristics of the specified service supported 
*  on the GATT Server. This can be achieved by using the Cy_BLE_GATTC_StopCmd() 
*  function. 
* 
*  Refer to Bluetooth 5.0 core specification, Volume 3, Part G, section 4.6.1 for
*  more details on the sequence of operations.
* 
*  \param param: Pointer to a variable of type cy_stc_ble_gattc_read_by_type_req_t,
*                The following must be set:
*                     param->range.startHandle <br>
*                     param->range.endHandle <br>
*                     param->connHandle <br>
*                The following must be ignored:
*                     param->uuidFormat (CY_BLE_GATT_16_BIT_UUID_FORMAT or
*                                                     CY_BLE_GATT_128_BIT_UUID_FORMAT) <br>
*                     param->uuid.uuid16 or param->uuid.uuid128 
*                     based on the uuidFormat
*  \note
*   'startHandle' & 'endHandle' should not be zero, and 'startHandle' shall be less than or equal to the 'endHandle'.
*
* \return
*  cy_en_ble_api_result_t : Return value indicates whether the function succeeded or
*  failed. Following are the possible error codes.
*
*  Error codes                          | Description
*  ------------                         | -----------
*  CY_BLE_SUCCESS                       | On successful operation.
*  CY_BLE_ERROR_INVALID_PARAMETER       | If 'param' is NULL or if any of the input parameters is invalid.
*  CY_BLE_ERROR_INVALID_OPERATION       | This operation is not permitted.
*  CY_BLE_ERROR_MEMORY_ALLOCATION_FAILED| Memory allocation failed.
* 
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_GATTC_DiscoverCharacteristics
(
    cy_stc_ble_gattc_read_by_type_req_t  * param
);


/******************************************************************************
* Function Name: Cy_BLE_GATTC_DiscoverCharacteristicByUuid
***************************************************************************//**
* 
*  This function is used by the GATT Client application to discover service characteristics
*  on a peer GATT Server, when only the service handle ranges are known and the
*  characteristic UUID is known. 
*  This is a non-blocking function.
* 
*  Internally, multiple Read By Type Requests are sent to the peer device in
*  response to which Read By Type Responses are received. Each of these responses
*  results in the event CY_BLE_EVT_GATTC_READ_BY_TYPE_RSP, which is passed to the
*  application layer for further processing.
* 
*  The sequence of operations is complete when the Error Response is received and the
*  Error Code is set to Attribute Not Found or the Read By Type Response has an
*  Attribute Handle that is equal to the Ending Handle of the request.
*  Completion of this function is notified to the upper layer using
*  CY_BLE_EVT_GATTC_ERROR_RSP or CY_BLE_EVT_GATTC_LONG_PROCEDURE_END event.
* 
*  Application can stop this procedure if a desired characteristic is found 
*  prior to the discovering all the characteristics of the specified service supported 
*  on the GATT Server. This can be achieved by using the Cy_BLE_GATTC_StopCmd() 
*  function. 
*
*  Refer to Bluetooth 5.0 core specification, Volume 3, Part G, section 4.6.2
*  for more details on the sequence of operations.
* 
*  \param param: Pointer to a variable of type cy_stc_ble_gattc_read_by_type_req_t,
*                     The following must be set:
*                     param->range.startHandle
*                     param->range.endHandle
*                                 param->connHandle  
*                     param->uuidFormat (CY_BLE_GATT_16_BIT_UUID_FORMAT or
*                                                     CY_BLE_GATT_128_BIT_UUID_FORMAT)
*                     param->uuid.uuid16 or readByTypeReqParam->uuid.uuid128 
*                     based on the uuidFormat
*  \note
*   'startHandle' & 'endHandle' should not be zero, and 'startHandle' shall be less than or equal to the 'endHandle'.
*
* \return
*  cy_en_ble_api_result_t : Return value indicates whether the function succeeded or
*  failed. Following are the possible error codes.
*
*  Error codes                          | Description
*  ------------                         | -----------
*  CY_BLE_SUCCESS                       | On successful operation.
*  CY_BLE_ERROR_INVALID_PARAMETER       | If 'param' is NULL or if any of the input parameters is invalid.
*  CY_BLE_ERROR_INVALID_OPERATION       | This operation is not permitted.
*  CY_BLE_ERROR_MEMORY_ALLOCATION_FAILED| Memory allocation failed.
* 
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_GATTC_DiscoverCharacteristicByUuid
(
    cy_stc_ble_gattc_read_by_type_req_t  * param
);


/******************************************************************************
* Function Name: Cy_BLE_GATTC_DiscoverCharacteristicDescriptors
***************************************************************************//**
* 
*  This function is used by the GATT Client application to find all the characteristic
*  descriptors on a peer GATT Server. This is a non-blocking function.
* 
*  Internally, multiple Find Information Requests are sent to the peer device in
*  response to which Find Information Responses are received by the GATT Client application.
*  Each of these responses generate CY_BLE_EVT_GATTC_FIND_INFO_RSP event at the
*  GATT Client application end.
* 
*  The sequence of operations is complete when the Error Response is received and the
*  Error Code is set to Attribute Not Found or the Find Information Response has
*  an Attribute Handle that is equal to the Ending Handle of the request.
*  Completion of this function is notified to the upper layer using CY_BLE_EVT_GATTC_ERROR_RSP or 
*  CY_BLE_EVT_GATTC_LONG_PROCEDURE_END event.
*
*  Application can stop this procedure if the desired Characteristic Descriptor 
*  is found prior to the discovering all the characteristic descriptors of the 
*  specified characteristic. This can be achieved by using the Cy_BLE_GATTC_StopCmd() 
*  function. 
*
* 
*  Refer to Bluetooth 5.0 core specification, Volume 3, Part G, section 4.7.1 for
*  more details on the sequence of operations.
* 
*  \param param: Pointer to a variable of type cy_stc_ble_gattc_find_info_req_t.
*                     The following must be set:
*                     param->range.startHandle <br>
*                     param->range.endHandle <br>
*                     param->connHandle <br>
*  \note
*   'startHandle' & 'endHandle' should not be zero, and 'startHandle' shall be less than or equal to the 'endHandle'.
*
* \return
*  cy_en_ble_api_result_t : Return value indicates whether the function succeeded or
*  failed. Following are the possible error codes.
*
*  Error codes                          | Description
*  ------------                         | -----------
*  CY_BLE_SUCCESS                       | On successful operation.
*  CY_BLE_ERROR_INVALID_PARAMETER       | If 'param' is NULL or if any of the input parameters is invalid.
*  CY_BLE_ERROR_INVALID_OPERATION       | This operation is not permitted.
*  CY_BLE_ERROR_MEMORY_ALLOCATION_FAILED| Memory allocation failed.
* 
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_GATTC_DiscoverCharacteristicDescriptors
(
    cy_stc_ble_gattc_find_info_req_t  * param
);


/******************************************************************************
* Function Name: Cy_BLE_GATTC_FindIncludedServices
***************************************************************************//**
* 
*  This function is used by the GATT Client application to find Included Service
*  declarations within a GATT Service on a peer GATT Server. This is a 
*  non-blocking function.
* 
*  Internally, multiple Read By Type Requests are sent to the peer device in
*  response to which Read By Type Responses are received 
*  (CY_BLE_EVT_GATTC_READ_BY_TYPE_RSP) and passed to the application layer.
* 
*  When Read By Type Response data does not contain the service UUID,
*  indicating the service uses a 128-bit UUID, the application layer can
*  choose to get the service UUID by performing the following steps:
*  1. Stop ongoing GATT operation by invoking Cy_BLE_GATTC_StopCmd()
*  2. Send Read Request by invoking the function 
*      Cy_BLE_GATTC_ReadCharacteristicValue() with the read request attribute handle set to
*      the included service attribute handle. Application should handle CY_BLE_EVT_GATTC_READ_RSP event.
*  3. Include service discovery can further be continued by calling Cy_BLE_GATTC_FindIncludedServices() function with 
*     start handle (param->range.startHandle) equals to (included service attribute handle + 1).
* 
*  The sequence of operations is complete when either the Error Response is received
*  with the Error Code set to Attribute Not Found or the Read By Type Response
*  has an Attribute Handle of the included service declaration that is equal to the
*  Ending Handle of the request. Completion of
*  this function is notified to the upper layer using CY_BLE_EVT_GATTC_ERROR_RSP 
*  or CY_BLE_EVT_GATTC_LONG_PROCEDURE_END event.

*  Application can stop this procedure if the desired included service is 
*  found prior to the discovering all the included services of the 
*  specified service. This can be achieved by using the Cy_BLE_GATTC_StopCmd() 
*  function.
* 
*  Refer to Bluetooth 5.0 core specification, Volume 3, Part G, section 4.5.1 
*  for more details on the sequence of operations.
* 
*  \param param: Pointer to a variable of type cy_stc_ble_gattc_read_by_type_req_t,
*                The following must be set:
*                     param->range.startHandle <br>
*                     param->range.endHandle <br>
*                     param->connHandle <br>
*                The following must be ignored:
*                     param->uuidFormat (CY_BLE_GATT_16_BIT_UUID_FORMAT or
*                                                     CY_BLE_GATT_128_BIT_UUID_FORMAT) <br>
*                     param->uuid.uuid16 or readByTypeReqParam->uuid.uuid128 
*                     based on the uuidFormat
*  \note
*   'startHandle' & 'endHandle' should not be zero, and 'startHandle' shall be less than or equal to the 'endHandle'.
*
*  \return
*  cy_en_ble_api_result_t : Return value indicates whether the function succeeded or
*  failed. Following are the possible error codes.
*
*  Error codes                          | Description
*  ------------                         | -----------
*  CY_BLE_SUCCESS                       | On successful operation.
*  CY_BLE_ERROR_INVALID_PARAMETER       | If 'param' is NULL or if any of the input parameters is invalid.
*  CY_BLE_ERROR_INVALID_OPERATION       | This operation is not permitted.
*  CY_BLE_ERROR_MEMORY_ALLOCATION_FAILED| Memory allocation failed.
*     
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_GATTC_FindIncludedServices
(
    cy_stc_ble_gattc_read_by_type_req_t  * param
);


/******************************************************************************
* Function Name: Cy_BLE_GATTC_ReadByTypeReq
***************************************************************************//**
* 
*  This function allows the user to send Read by type request to a peer GATT server.
*  This function results in CY_BLE_EVT_GATTC_READ_BY_TYPE_RSP event or 
*  CY_BLE_EVT_GATTC_ERROR_RSP event at the GATT client depending on whether the GATT server 
*  sends read by type response or GATT Error response.
*
*  Refer to Bluetooth 5.0 core specification, Volume 3, Part G, section 4.5.1 
*  for more details on the sequence of operations.
* 
*  \param param: Pointer to a variable of type cy_stc_ble_gattc_read_by_type_req_t,
*                     Where, the following needs to be set:
*                     param->range.startHandle
*                     param->range.endHandle
*                                 param->connHandle  
*                     param->uuidFormat (CY_BLE_GATT_16_BIT_UUID_FORMAT or
*                                                     CY_BLE_GATT_128_BIT_UUID_FORMAT)
*                     param->uuid.uuid16 or readByTypeReqParam->uuid.uuid128 
*                     based on the uuidFormat
* 
*  \note
*   'startHandle' & 'endHandle' should not be zero, and 'startHandle' shall be less than or equal to the 'endHandle'.
*
* \return
*  cy_en_ble_api_result_t : Return value indicates whether the function succeeded or
*  failed. Following are the possible error codes.
*
*  Error codes                          | Description
*  ------------                         | -----------
*  CY_BLE_SUCCESS                       | On successful operation.
*  CY_BLE_ERROR_INVALID_PARAMETER       | If 'param' is NULL or if any of the input parameters is invalid.
*  CY_BLE_ERROR_INVALID_OPERATION       | This operation is not permitted.
*  CY_BLE_ERROR_MEMORY_ALLOCATION_FAILED| Memory allocation failed.
*     
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_GATTC_ReadByTypeReq
(
    cy_stc_ble_gattc_read_by_type_req_t  * param
);


/******************************************************************************
* Function Name: Cy_BLE_GATTC_ReadCharacteristicValue
***************************************************************************//**
* 
*  This function is used by the GATT Client application to read a Characteristic Value 
*  from a peer GATT Server, when the GATT Client application knows the Characteristic 
*  Value Handle. This is a non-blocking function.
* 
*  Internally, Read Request is sent to the peer GATT Server. If the read operation
*  is successful, GATT Server sends a Read Response. This response results in 
*  CY_BLE_EVT_GATTC_READ_RSP event at the GATT Client's end, and response is passed to the application.
*  
*  
*  In response to the Read Request GATT server shall send an Error Response if
*  1. GATT Client has insufficient authentication to read requested characteristic value OR
*  2. GATT Client has insufficient authorization to read requested characteristic value OR
*  3. GATT Client has used insufficient encryption key size OR 
*  4. GATT Client has insufficient encryption to read requested characteristic value OR
*  5. If GATT Client requested handle is invalid OR
*  6. If a read operation is not permitted on the requested Characteristic Value. 
*  The Error Code parameter is set as specified in the Attribute Protocol. 
*  This Error response results in CY_BLE_EVT_GATTC_ERROR_RSP event at the GATT client application.
*
*  If the peer GATT Server is a PSoC 6 MCU, Read Request PDU results in 
*  CY_BLE_EVT_GATTS_READ_CHAR_VAL_ACCESS_REQ event at the GATT server's end, if the corresponding 
*  characteristic's attribute permission is set to CY_BLE_GATT_DB_ATTR_CHAR_VAL_RD_EVENT.
* 
* 
*  Refer to Bluetooth 5.0 core specification, Volume 3, Part G, section 4.8.1
*  for more details on the sequence of operations.
* 
*  \param param: Pointer to a variable of type cy_stc_ble_gattc_read_req_t.
*                     The following must be set:
*                     param->attrHandle: Attribute handle to be read <br>
*                     param->connHandle  
*
* \return
*  cy_en_ble_api_result_t : Return value indicates whether the function succeeded or
*  failed. Following are the possible error codes.
*
*  Error codes                          | Description
*  ------------                         | -----------
*  CY_BLE_SUCCESS                       | On successful operation.
*  CY_BLE_ERROR_INVALID_PARAMETER       | If 'param' is NULL or 'connHandle' is invalid, or if 'attrHandle' is zero. 
*  CY_BLE_ERROR_INVALID_OPERATION       | This operation is not permitted.
*  CY_BLE_ERROR_MEMORY_ALLOCATION_FAILED| Memory allocation failed.
* 
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_GATTC_ReadCharacteristicValue
(
    cy_stc_ble_gattc_read_req_t  * param
); 


/******************************************************************************
* Function Name: Cy_BLE_GATTC_ReadUsingCharacteristicUuid
***************************************************************************//**
* 
*  This function is used by the GATT Client application to read a Characteristic Value from a peer 
*  GATT Server when the application knows only the characteristic UUID and 
*  does not know the handle of the characteristic. This is a non-blocking function.
* 
*  Internally, Read By Type Request is sent to the peer GATT server. If the read operation
*  is successful, GATT Server sends a Read By Type Response. This response results in 
*  CY_BLE_EVT_GATTC_READ_RSP event at the GATT Client's end, and response is passed to the application.
*
*  In response to this Request GATT server shall send an Error Response if
*  1. GATT Client has insufficient authentication to read the requested characteristic value OR
*  2. GATT Client has insufficient authorization to read the requested characteristic value OR
*  3. GATT Client has insufficient encryption to read the requested characteristic value OR
*  3. GATT Client has used insufficient encryption key size OR 
*  4. Read operation is not permitted on the requested Characteristic Value OR
*  5. characteristic does not exist on the GATT server within the handle OR
*     range provided OR
*  6. Starting Handle parameter greater than the Ending Handle parameter or the 
*     Starting Handle parameter is 0x0000
*  The Error Code parameter is set as specified in the Attribute Protocol. 
*  This Error response results in CY_BLE_EVT_GATTC_ERROR_RSP event at the GATT client application.
*
*  If the peer GATT Server is a PSoC 6 MCU, Read By Type Request PDU results in the 
*  CY_BLE_EVT_GATTS_READ_CHAR_VAL_ACCESS_REQ event at the GATT server's end if the 
*  corresponding characteristic's attribute permission is set to 
*  CY_BLE_GATT_DB_ATTR_CHAR_VAL_RD_EVENT.
* 
* 
*  Refer to Bluetooth 5.0 core specification, Volume 3, Part G, section 4.8.2 for
*  more details on the sequence of operations.
* 
*  \param param: Pointer to a variable of type cy_stc_ble_gattc_read_by_type_req_t,
*                     Where, the following needs to be set:
*                     param->range.startHandle
*                     param->range.endHandle
*                                 param->connHandle  
*                     param->uuidFormat (CY_BLE_GATT_16_BIT_UUID_FORMAT or
*                                                     CY_BLE_GATT_128_BIT_UUID_FORMAT)
*                     param->uuid.uuid16 or readByTypeReqParam->uuid.uuid128 
*                     based on the uuidFormat
*  \note
*   'startHandle' & 'endHandle' should not be zero, and 'startHandle' shall be less than or equal to the 'endHandle'.
* 
* \return
*  cy_en_ble_api_result_t : Return value indicates whether the function succeeded or
*  failed. Following are the possible error codes.
*
*  Error codes                          | Description
*  ------------                         | -----------
*  CY_BLE_SUCCESS                       | On successful operation.
*  CY_BLE_ERROR_INVALID_PARAMETER       | If 'param' is NULL or if any of the input parameters is invalid.
*  CY_BLE_ERROR_INVALID_OPERATION       | This operation is not permitted.
*  CY_BLE_ERROR_MEMORY_ALLOCATION_FAILED| Memory allocation failed.
* 
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_GATTC_ReadUsingCharacteristicUuid
(
    cy_stc_ble_gattc_read_by_type_req_t  * param
);


/******************************************************************************
* Function Name: Cy_BLE_GATTC_ReadLongCharacteristicValues
***************************************************************************//**
* 
*  This function is used by the GATT Client application to read a Characteristic Value from a peer 
*  GATT Server when the GATT Client knows the Characteristic Value Handle and the length 
*  of the Characteristic Value is longer than that can be sent in a single Read Response
*  Attribute Protocol message. This is a non-blocking function.
* 
*  Internally multiple Read Blob Requests are sent to the peer device in response
*  to which Read Blob Responses are received. For each Read Blob Request, a Read
*  Blob Response event is received (CY_BLE_EVT_GATTC_READ_BLOB_RSP) with a
*  portion of the Characteristic Value contained in the Part Attribute Value
*  parameter. These events are propagated to the application layer
*  for further processing. Each read blob response will return up to (GATT MTU-1) bytes of
*  data. If the size of characteristic value field is an integral multiple of (GATT MTU-1)
*  then the operation terminates with an error response event, where the error code is
*  CY_BLE_GATT_ERR_INVALID_OFFSET. If the size of the characteristic value field is
*  not an integral multiple of (GATT MTU-1), the last read blob response will return
*  data bytes that are less than (GATT MTU-1). The application must monitor these two
*  conditions before proceeding with the initiation of any other GATT operation.
* 
*  In response to the this Request GATT server shall send an Error Response if
*  1. GATT Client has insufficient authentication to read the requested characteristic value OR
*  2. GATT Client has insufficient authorization to read the requested characteristic value OR
*  3. GATT Client has insufficient encryption to read the requested characteristic value OR
*  4. GATT Client has used insufficient encryption key size OR 
*  5. Read operation is not permitted on the requested Characteristic Value OR
*  6. The Characteristic Value is not longer than (GATT MTU - 1) OR
*  7. The requested handle is invalid OR
*  8. Value offset is greater than length attribute value.
*  The Error Code parameter is set as specified in the Attribute Protocol. 
*  This Error response results in CY_BLE_EVT_GATTC_ERROR_RSP event at the GATT client application.
* 
*  If the peer GATT Server is a PSoC 6 MCU, Read Blob Request PDU results in the 
*  CY_BLE_EVT_GATTS_READ_CHAR_VAL_ACCESS_REQ event at the GATT server's end if the 
*  corresponding characteristic's attribute permission is set to 
*  CY_BLE_GATT_DB_ATTR_CHAR_VAL_RD_EVENT.
* 
*  Refer to Bluetooth 5.0 core specification, Volume 3, Part G, section 4.8.3
*  for more details on the sequence of operations.
* 
*  \param param: Pointer to a variable of type cy_stc_ble_gattc_read_blob_req_t.
*                     Where, the following needs to be set:
*                     param->attrHandle
*                     param->offset
*                                 param->connHandle  
*
* \return
*  cy_en_ble_api_result_t : Return value indicates whether the function succeeded or
*  failed. Following are the possible error codes.
*
*  Error codes                          | Description
*  ------------                         | -----------
*  CY_BLE_SUCCESS                       | On successful operation.
*  CY_BLE_ERROR_INVALID_PARAMETER       | If 'param' is NULL or 'connHandle' is invalid, or if 'attrHandle' is zero. 
*  CY_BLE_ERROR_INVALID_OPERATION       | This operation is not permitted.
*  CY_BLE_ERROR_MEMORY_ALLOCATION_FAILED| Memory allocation failed.
* 
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_GATTC_ReadLongCharacteristicValues
(
    cy_stc_ble_gattc_read_blob_req_t  * param
);


/******************************************************************************
* Function Name: Cy_BLE_GATTC_ReadMultipleCharacteristicValues
***************************************************************************//**
* 
*  This function is used by the GATT Client application to read multiple Characteristic Values 
*  from a peer GATT Server when the GATT Client application knows the Characteristic 
*  Value Handles. This is a non-blocking function.
* 
*  Internally, Read Multiple Request is sent to the peer GATT Server. If the read operation
*  is successful, GATT Server sends a Read Multiple Response. This response results in 
*  CY_BLE_EVT_GATTC_READ_RSP event at the GATT Client's end, and response is passed to the application.
* 
*  In response to the this Request, the GATT server shall send an Error Response if
*  1. GATT Client has insufficient authentication to read the requested characteristic value OR
*  2. GATT Client has insufficient authorization to read the requested characteristic value OR
*  3. GATT Client has insufficient encryption to read the requested characteristic value OR
*  4. GATT Client has used insufficient encryption key size OR 
*  5. Read operation is not permitted on any of the requested Characteristic Value OR
*  6. Any of requested handle are invalid.
*  The Error Code parameter is set as specified in the Attribute Protocol. 
*  This Error response results in CY_BLE_EVT_GATTC_ERROR_RSP event at the GATT client application.
* 
*  If the peer GATT Server is a PSoC 6 MCU, Read Multiple Request PDU results in the 
*  CY_BLE_EVT_GATTS_READ_CHAR_VAL_ACCESS_REQ event at the GATT server's end if the 
*  corresponding characteristic's attribute permission is set to 
*  CY_BLE_GATT_DB_ATTR_CHAR_VAL_RD_EVENT.
* 
*  Refer to Bluetooth 5.0 core specification, Volume 3, Part G, section 4.8.4
*  for more details on the sequence of operations.
* 
*  \param param: Pointer to a variable of type cy_stc_ble_gattc_read_mult_req_t.
*                     Where, the following needs to be set:
*                     param->handleList
*                     param->listCount
*                                 param->connHandle  
*
* \return
*  cy_en_ble_api_result_t : Return value indicates whether the function succeeded or
*  failed. Following are the possible error codes.
*
*  Error codes                          | Description
*  ------------                         | -----------
*  CY_BLE_SUCCESS                       | On successful operation.
*  CY_BLE_ERROR_INVALID_PARAMETER       | If 'param' is NULL or if any of the input parameters is invalid.
*  CY_BLE_ERROR_INVALID_OPERATION       | This operation is not permitted.
*  CY_BLE_ERROR_MEMORY_ALLOCATION_FAILED| Memory allocation failed.
* 
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_GATTC_ReadMultipleCharacteristicValues
(
    cy_stc_ble_gattc_read_mult_req_t            * param
);


/******************************************************************************
* Function Name: Cy_BLE_GATTC_ReadCharacteristicDescriptors
***************************************************************************//**
* 
*  This function is used by the GATT Client application to read a characteristic descriptor 
*  from a peer GATT Server when the GATT Client application knows the Attribute handle 
*  from the characteristic descriptor declaration. This is a non-blocking function.
* 
*  Internally, Read Request is sent to the peer GATT Server. If the read operation
*  is successful, GATT Server sends a Read Response. This response results in 
*  CY_BLE_EVT_GATTC_READ_RSP event at the GATT Client's end, and response is passed to the application.
* 
*  In response to the this Request GATT server shall send an Error Response if
*  1. GATT Client has insufficient authentication to read the requested descriptor value OR
*  2. GATT Client has insufficient authorization to read the requested descriptor value OR
*  3. GATT Client has used insufficient encryption key size OR 
*  4. GATT Client has insufficient encryption to read the requested descriptor value OR
*  5. GATT Client requested handle is invalid OR
*  6. Read operation is not permitted on the requested descriptor.
*  The Error Code parameter is set as specified in the Attribute Protocol. 
*  This Error response results in CY_BLE_EVT_GATTC_ERROR_RSP event at the GATT client application.
* 
*  If the peer GATT Server is a PSoC 6 MCU, Read Request PDU results in the 
*  CY_BLE_EVT_GATTS_READ_CHAR_VAL_ACCESS_REQ event at the GATT server's end if the 
*  corresponding characteristic's attribute permission is set to 
*  CY_BLE_GATT_DB_ATTR_CHAR_VAL_RD_EVENT.
*
*  Refer to Bluetooth 5.0 core specification, Volume 3, Part G, section 4.12.1
*  for more details on the sequence of operations.
* 
*  \param param: Pointer to a variable of type cy_stc_ble_gattc_read_req_t.
*                     Where, the following needs to be set:
*                     param->attrHandle: Attribute handle to be read
*                                 param->connHandle  
* 
* \return
*  cy_en_ble_api_result_t : Return value indicates whether the function succeeded or
*  failed. Following are the possible error codes.
*
*  Error codes                          | Description
*  ------------                         | -----------
*  CY_BLE_SUCCESS                       | On successful operation.
*  CY_BLE_ERROR_INVALID_PARAMETER       | If 'param' is NULL or 'connHandle' is invalid, or if 'attrHandle' is zero.
*  CY_BLE_ERROR_INVALID_OPERATION       | This operation is not permitted.
*  CY_BLE_ERROR_MEMORY_ALLOCATION_FAILED| Memory allocation failed.
* 
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_GATTC_ReadCharacteristicDescriptors
(
    cy_stc_ble_gattc_read_req_t  * param
);


/******************************************************************************
* Function Name: Cy_BLE_GATTC_ReadLongCharacteristicDescriptors
***************************************************************************//**
* 
*  This function is used by the GATT Client application to read a characteristic descriptor 
*  from a peer GATT Server when the GATT Client application knows the Attribute handle 
*  from the characteristic descriptor declaration and the length of the characteristic 
*  descriptor declaration is longer than what can be sent in a single Read Response 
*  Attribute Protocol message. This is a non-blocking function.
* 
*  Internally multiple Read Blob Requests are sent to the peer device in response
*  to which Read Blob Responses are received. For each Read Blob Request, a Read
*  Blob Response event is received (CY_BLE_EVT_GATTC_READ_BLOB_RSP) with a portion
*  of the Characteristic Value contained in the Part Attribute Value parameter.
*  These events are propagated to the application layer for further processing.
*  Each read blob response will return up to (GATT MTU-1) bytes of data. If the size of 
*  characteristic descriptor field is an integral multiple of (GATT MTU-1) then the 
*  operation terminates with an error response event, where the error code is
*  CY_BLE_GATT_ERR_INVALID_OFFSET. If the size of the characteristic descriptor
*  field is not an integral multiple of (GATT MTU-1), the last read blob response will
*  return data bytes that are less than (GATT MTU-1). The application must monitor 
*  these two conditions before proceeding with the initiation of any other GATT
*  operation.
* 
*  In response to the this Request GATT server shall send an Error Response if
*  1. GATT Client has insufficient authentication to read the requested descriptor value OR
*  2. GATT Client has insufficient authorization to read the requested descriptor value OR
*  3. GATT Client has insufficient encryption to read the requested descriptor value OR
*  4. GATT Client has used insufficient encryption key size OR 
*  5. Read operation is not permitted on the requested descriptor Value. OR
*  6. The Characteristic descriptor value is not longer than (GATT MTU - 1)
*  7. The requested handle is invalid OR
*  8. The value offset is greater than length of attribute value.
*  The Error Code parameter is set as specified in the Attribute Protocol. 
*  This Error response results in CY_BLE_EVT_GATTC_ERROR_RSP event at the GATT client application.
* 
*  If the peer GATT Server is a PSoC 6 MCU, Read Blob Request PDU results in the 
*  CY_BLE_EVT_GATTS_READ_CHAR_VAL_ACCESS_REQ event at the GATT server's end if the 
*  corresponding characteristic's attribute permission is set to 
*  CY_BLE_GATT_DB_ATTR_CHAR_VAL_RD_EVENT.
* 
*  Refer to Bluetooth 5.0 core specification,
*  Volume 3, Part G, section 4.12.2 for more details on the sequence of operations.
* 
*  \param param: Pointer to a variable of type cy_stc_ble_gattc_read_blob_req_t.
*                     Where, the following needs to be set:
*                     param->attrHandle
*                     param->offset
*                                 param->connHandle  
* 
* \return
*  cy_en_ble_api_result_t : Return value indicates whether the function succeeded or
*  failed. Following are the possible error codes.
*
*  Error codes                          | Description
*  ------------                         | -----------
*  CY_BLE_SUCCESS                       | On successful operation.
*  CY_BLE_ERROR_INVALID_PARAMETER       | If 'param' is NULL or 'connHandle' is invalid, or if 'attrHandle' is zero. 
*  CY_BLE_ERROR_INVALID_OPERATION       | This operation is not permitted.
*  CY_BLE_ERROR_MEMORY_ALLOCATION_FAILED| Memory allocation failed.
* 
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_GATTC_ReadLongCharacteristicDescriptors
(
    cy_stc_ble_gattc_read_blob_req_t  * param
);


/******************************************************************************
* Function Name: Cy_BLE_GATTC_WriteCharacteristicValue
***************************************************************************//**
* 
*  This function is used by the GATT Client application to write a Characteristic Value to
*  a peer GATT Server when the GATT Client knows the Characteristic Value Handle. 
*  This is a non-blocking function.
* 
*  Internally, Write Request is sent to the peer GATT Server. If write operation is
*  successful GATT server sends Write Response. This Response results in the event
*  CY_BLE_EVT_GATTC_WRITE_RSP at the GATT Client application.
* 
*  In response to the this Request GATT server shall send an Error Response if
*  1. GATT Client has insufficient authentication to write the requested Characteristic value OR
*  2. GATT Client has insufficient authorization to write the requested Characteristic value OR
*  3. GATT Client has insufficient encryption to write the requested Characteristic value OR
*  4. GATT Client has used insufficient encryption key size OR 
*  5. Write operation is not permitted on the requested Characteristic Value OR
*  6. The requested handle is invalid OR
*  7. The requested value length is greater than attribute value length.
*  The Error Code parameter is set as specified in the Attribute Protocol. 
*  This Error response results in CY_BLE_EVT_GATTC_ERROR_RSP event at the 
*  GATT client application.
*
*  If the peer GATT Server is a PSoC 6 MCU, Write Request PDU results in the 
*  CY_BLE_EVT_GATTS_WRITE_REQ event at the GATT server's end.
* 
*  Refer to Bluetooth 5.0 core specification, Volume 3, Part G, section
*  4.9.3 for more details on the sequence of operations.
*
*  \param param: Pointer to a variable of type cy_stc_ble_gattc_write_req_t.
*                     Where, the following needs to be set:
*                     param->handleValPair
*                                 param->connHandle  
* 
* \return
*  cy_en_ble_api_result_t : Return value indicates whether the function succeeded or
*  failed. Following are the possible error codes.
*
*  Error codes                          | Description
*  ------------                         | -----------
*  CY_BLE_SUCCESS                       | On successful operation.
*  CY_BLE_ERROR_INVALID_PARAMETER       | If 'param' is NULL or 'connHandle' is invalid, or if 'attrHandle' is zero or if 'param->handleValPair.value.len' value is greater than (Effective GATT MTU-3).
*  CY_BLE_ERROR_INVALID_OPERATION       | This operation is not permitted.
*  CY_BLE_ERROR_MEMORY_ALLOCATION_FAILED| Memory allocation failed.
* 
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_GATTC_WriteCharacteristicValue
(
    cy_stc_ble_gattc_write_req_t  * param
);


/******************************************************************************
* Function Name: Cy_BLE_GATTC_WriteCharacteristicDescriptors
***************************************************************************//**
* 
*  This function is used by the GATT client application to write a characteristic descriptor value 
*  to a peer GATT Server when the GATT Client application knows the characteristic 
*  descriptor handle. This is a non-blocking function.
* 
*  Internally, Write Request is sent to the peer GATT Server. If write operation is
*  successful GATT server sends Write Response. This Response results in the event
*  CY_BLE_EVT_GATTC_WRITE_RSP at the GATT Client application.
* 
*  In response to the this Request, the GATT server shall send an Error Response if
*  1. GATT Client has insufficient authentication to write the requested descriptor value OR
*  2. GATT Client has insufficient authorization to write the requested descriptor value OR
*  3. GATT Client has insufficient encryption to write the requested descriptor value OR
*  4. GATT Client has used insufficient encryption key size OR 
*  5. Write operation is not permitted on the requested descriptor Value OR
*  6. The requested handle is invalid OR
*  7. The requested value length is greater than attribute value length.
*  The Error Code parameter is set as specified in the Attribute Protocol. 
*  This Error response results in CY_BLE_EVT_GATTC_ERROR_RSP event at the 
*  GATT client application.
*
*  If the peer GATT Server is a PSoC 6 MCU, Write Request PDU results in the 
*  CY_BLE_EVT_GATTS_WRITE_REQ event at the GATT server's end.
* 
*  Refer to Bluetooth 5.0 core specification, Volume 3, Part G, section 4.12.3
*  for more details on the sequence of operations.
* 
*  \param param: Pointer to a variable of type cy_stc_ble_gattc_write_req_t.
*                     Where, the following needs to be set:
*                     param->handleValPair
*                                 param->connHandle  
* 
* \return
*  cy_en_ble_api_result_t : Return value indicates whether the function succeeded or
*  failed. Following are the possible error codes.
*
*  Error codes                          | Description
*  ------------                         | -----------
*  CY_BLE_SUCCESS                       | On successful operation.
*  CY_BLE_ERROR_INVALID_PARAMETER       | If 'param' is NULL or 'connHandle' is invalid, or if 'attrHandle' is zero or if 'param->handleValPair.value.len' value is greater than (Effective GATT MTU-3).
*  CY_BLE_ERROR_INVALID_OPERATION       | This operation is not permitted.
*  CY_BLE_ERROR_MEMORY_ALLOCATION_FAILED| Memory allocation failed.
* 
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_GATTC_WriteCharacteristicDescriptors
(
    cy_stc_ble_gattc_write_req_t  * param
);


/******************************************************************************
* Function Name: Cy_BLE_GATTC_WriteLongCharacteristicValues
***************************************************************************//**
* 
*  This function is used by the GATT Client application to write a Characteristic Value to a 
*  peer GATT Server when the GATT Client knows the Characteristic Value Handle but the 
*  length of the Characteristic Value is longer than GATT MTU size and cannot be sent in
*  a single Write Request Attribute Protocol message. This is a non-blocking function.
* 
*  Internally, multiple Prepare Write Requests are sent to the GATT Server,
*  in response to which Prepare Write Responses are received. No events are generated 
*  by the BLE Stack for these Responses.
* 
*  Prepare Write Requests are repeated until the complete Characteristic Value
*  has been transferred to the GATT Server, after which an Execute Write Request
*  is sent to the GATT Server to write the initially transferred value at the
*  GATT Server's end. 
* 
*  If the write operation is successful, GATT Server sends an Execute Write Response.
*  This response results in CY_BLE_EVT_GATTC_EXEC_WRITE_RSP event at the GATT Client 
*  application. The value associated with this event has to be checked by the 
*  application layer to confirm that the long write operation is succeeded.
* 
*  In response to the Prepare Write Request, GATT server shall send an Error Response if
*  1. GATT Client has insufficient authentication to write the requested characteristic value OR
*  2. GATT Client has insufficient authorization to write the requested characteristic value OR
*  3. GATT Client has insufficient encryption to write the requested characteristic value OR
*  4. GATT Client has used insufficient encryption key size OR 
*  5. Write operation is not permitted on the requested Characteristic Value OR
*  6. The requested handle is invalid.
*
*  In response to Execute Write Request, GATT server shall send an Error Response if
*  1. If the prepared attribute value exceeds the maximum length of attribute value OR
*  2. If the prepare value offset is greater than the current length of the attribute
*     value
*  The Error Code parameter is set as specified in the Attribute Protocol. 
*  This Error response results in CY_BLE_EVT_GATTC_ERROR_RSP event at the GATT client application.
* 
*  If the peer GATT Server is a PSoC 6 MCU, this function results in following events at the GATT Server's end.
*  1. CY_BLE_EVT_GATTS_PREP_WRITE_REQ - event for Prepare Write Request.
*  2. CY_BLE_EVT_GATTS_EXEC_WRITE_REQ - event for Execute Write Request.
*  3. CY_BLE_EVT_MEMORY_REQUEST - Need confirmation from NPAL.
*    
*  Refer to Bluetooth 5.0 core specification, Volume 3, Part G, section 4.9.4
*  for more details on the sequence of operations.
*  
*  \param param: Pointer to a variable of type cy_stc_ble_gattc_prep_write_req_t,
*                      where 'param->value.val' points to the actual
*                      data to be written. 'param' and all associated
*                      variables need to be retained in memory by the calling
*                      application until the GATT Write Long Characteristic Value
*                      operation is completed successfully.
* 
* \return
*  cy_en_ble_api_result_t : Return value indicates whether the function succeeded or
*  failed. Following are the possible error codes.
*
*  Error codes                          | Description
*  ------------                         | -----------
*  CY_BLE_SUCCESS                       | On successful operation.
*  CY_BLE_ERROR_INVALID_PARAMETER       | If 'param' is NULL or 'connHandle' is invalid, or if 'attrHandle' is zero.
*  CY_BLE_ERROR_INVALID_OPERATION       | This operation is not permitted.
*  CY_BLE_ERROR_MEMORY_ALLOCATION_FAILED| Memory allocation failed.
* 
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_GATTC_WriteLongCharacteristicValues
(
    cy_stc_ble_gattc_prep_write_req_t  * param
);


/******************************************************************************
* Function Name: Cy_BLE_GATTC_ReliableWrites
***************************************************************************//**
* 
*  This function is used by the GATT Client application to write a Characteristic Value to 
*  a peer GATT Server when the GATT Client knows the Characteristic Value Handle, 
*  and assurance is required that the correct Characteristic Value is going to be 
*  written by transferring the Characteristic Value to be written in both directions 
*  before the write is performed. This is a non-blocking function.
* 
*  Internally, multiple Prepare Write Requests are sent to the GATT Server,
*  in response to which Prepare Write Responses are received. No events are generated 
*  by the BLE Stack for these Responses.
* 
*  Prepare Write Requests are repeated until the complete Characteristic Value
*  has been transferred to the GATT Server, after which an Execute Write Request
*  is sent to the GATT Server to write the initially transferred value at the
*  GATT Server's end. 
* 
*  If the write operation is successful, GATT Server sends an Execute Write Response.
*  This response results in CY_BLE_EVT_GATTC_EXEC_WRITE_RSP event at the GATT Client 
*  application. The value associated with this event has to be checked by the 
*  application layer to confirm that the long write operation is succeeded.
* 
*  In response to the Prepare Write Request GATT server shall send an Error Response if
*  1. GATT Client has insufficient authentication to write the requested characteristic value OR
*  2. GATT Client has insufficient authorization to write the requested characteristic value OR
*  3. GATT Client has insufficient encryption to write the requested characteristic value OR
*  4. GATT Client has used insufficient encryption key size OR 
*  5. Write operation is not permitted on the requested Characteristic Value OR
*  6. GATT server does not have sufficient space to queue this request.
*
*  In response to Execute Write Request, GATT server shall send an Error Response if
*  1. The prepared attribute value exceeds the maximum length of attribute value OR
*  2. The prepare value offset is greater than the current length of the attribute
*     value
*  The Error Code parameter is set as specified in the Attribute Protocol. 
*  This Error response results in CY_BLE_EVT_GATTC_ERROR_RSP event at the GATT client application.
* 
*  If the peer GATT Server is a PSoC 6 MCU, this function results in following events at the GATT Server's end.
*  1. CY_BLE_EVT_GATTS_PREP_WRITE_REQ - event for Prepare Write Request.
*  2. CY_BLE_EVT_GATTS_EXEC_WRITE_REQ - event for Execute Write Request.
*  3. CY_BLE_EVT_MEMORY_REQUEST - Need confirmation from NPAL.
* 
*  Refer to Bluetooth 5.0 core specification, Volume 3, Part G, section 4.9.5
*  for more details on the sequence of operations.
* 
*  \param param: Pointer to a variable of type cy_stc_ble_gattc_reliable_write_req_t.
*                      Since more than one writes are performed  'param' and 
*                      all associated variables need to be retained in memory by
*                      the calling application until the GATT Reliable Write 
*                      operation is completed successfully.
*  param->handleValOffsetPair: array of type 'cy_stc_ble_gatt_handle_value_offset_param_t', which 
*                                           contains the values to be written, has to be specified.
*  param->numOfRequests: Number of requests. That is, the count of array of structures 
*                  of type cy_stc_ble_gatt_handle_value_offset_param_t. Each array element 
*                  represents a value and the attribute to which the value has 
*                  to be written.
* 
* \return
*  cy_en_ble_api_result_t : Return value indicates whether the function succeeded or
*  failed. Following are the possible error codes.
*
*  Error codes                          | Description
*  ------------                         | -----------
*  CY_BLE_SUCCESS                       | On successful operation.
*  CY_BLE_ERROR_INVALID_PARAMETER       | If 'param' or 'handleValOffsetPair' is NULL or 'connHandle' is invalid, or if 'numOfRequests' is zero.
*  CY_BLE_ERROR_INVALID_OPERATION       | This operation is not permitted.
*  CY_BLE_ERROR_MEMORY_ALLOCATION_FAILED| Memory allocation failed.
* 
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_GATTC_ReliableWrites
(
    cy_stc_ble_gattc_reliable_write_req_t  * param
);


/******************************************************************************
* Function Name: Cy_BLE_GATTC_WriteLongCharacteristicDescriptors
***************************************************************************//**
* 
*  This function is used by the GATT Client application to write a characteristic descriptor value 
*  to a GATT Server when the GATT Client application knows the characteristic descriptor 
*  handle but the length of the characteristic descriptor value is longer than what can be
*  sent in a single Write Request Attribute Protocol message. This is a non-blocking function.
* 
*  Internally, multiple Prepare Write Requests are sent to the GATT Server,
*  in response to which Prepare Write Responses are received. No events are generated 
*  by the BLE Stack for these Responses.
* 
*  Prepare Write Requests are repeated until the complete descriptor value
*  has been transferred to the GATT Server, after which an Execute Write Request
*  is sent to the GATT Server to write the initially transferred value at the
*  GATT Server's end. 
* 
*  If the write operation is successful, GATT Server sends an Execute Write Response.
*  This response results in CY_BLE_EVT_GATTC_EXEC_WRITE_RSP event at the GATT Client 
*  application. The value associated with this event has to be checked by the 
*  application layer to confirm that the long write operation is succeeded.
* 
*  In response to the Prepare Write Request GATT server shall send an Error Response if
*  1. GATT Client has insufficient authentication to write the requested descriptor value OR
*  2. GATT Client has insufficient authorization to write the requested descriptor value OR
*  3. GATT Client has insufficient encryption to write the requested descriptor value OR
*  4. GATT Client has used insufficient encryption key size OR 
*  5. Write operation is not permitted on the requested descriptor value OR
*  6. GATT server does not have sufficient space to queue this request.
*
*  In response to Execute Write Request, GATT server shall send an Error Response if
*  1. The prepared attribute value exceeds the maximum length of attribute value OR
*  2. The prepare value offset is greater than the current length of the attribute
*     value
*  The Error Code parameter is set as specified in the Attribute Protocol. 
*  This Error response results in CY_BLE_EVT_GATTC_ERROR_RSP event at the GATT client application.
* 
*  If the peer GATT Server is a PSoC 6 MCU, this function results in following events at the GATT Server's end.
*  1. CY_BLE_EVT_GATTS_PREP_WRITE_REQ - event for Prepare Write Request.
*  2. CY_BLE_EVT_GATTS_EXEC_WRITE_REQ - event for Execute Write Request.
*  3. CY_BLE_EVT_MEMORY_REQUEST - Need confirmation from NPAL.
*    
* 
*  Refer to Bluetooth 5.0 core specification, Volume 3,
*  Part G, section 4.12.4 for more details on the sequence of operations.
* 
*  \param param: Pointer to a variable of type cy_stc_ble_gattc_prep_write_req_t,
*                      where 'param->value.val' points to the actual
*                      data to be written. 'param' and all associated
*                      variables must be retained in memory by the calling
*                      application until the GATT Write Long Characteristic Value
*                      operation is completed successfully.
* 
* \return
*  cy_en_ble_api_result_t : Return value indicates whether the function succeeded or
*  failed. Following are the possible error codes.
*
*  Error codes                          | Description
*  ------------                         | -----------
*  CY_BLE_SUCCESS                       | On successful operation.
*  CY_BLE_ERROR_INVALID_PARAMETER       | If 'param' is NULL or 'connHandle' is invalid, or if 'attrHandle' is zero. 
*  CY_BLE_ERROR_INVALID_OPERATION       | This operation is not permitted.
*  CY_BLE_ERROR_MEMORY_ALLOCATION_FAILED| Memory allocation failed.
* 
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_GATTC_WriteLongCharacteristicDescriptors
(
    cy_stc_ble_gattc_prep_write_req_t  * param
);


/******************************************************************************
* Function Name: Cy_BLE_GATTC_SendExecuteWriteReq
***************************************************************************//**
* 
*  This function is used by the GATT Client application to send execute write request to peer 
*  GATT Server. This function should be called if client has previously initiated long/reliable 
*  write operation but GATT Client has received error response from GATT Server. Using this 
*  function GATT client application can request to cancel all the pending prepare write requests 
*  on GATT Server.
*  
*  In response to this request, GATT server sends Execute Write Response. This response
*  results in CY_BLE_EVT_GATTC_EXEC_WRITE_RSP event at the GATT Client application.
*
*  If the peer GATT Server is a PSoC 6 MCU, Execute Write Request PDU results in 
*  CY_BLE_EVT_GATTS_EXEC_WRITE_REQ event at the GATT Server's end.
* 
*  \param param: Pointer to a variable of type cy_stc_ble_gattc_exec_write_req_t.
*   param->connHandle: Connection handle to identify the peer GATT entity of type 
*               cy_stc_ble_conn_handle_t.
*   param->flag: Indicates whether Queued Write is to be executed (0x01) or canceled (0x00)
* 
* \return
*  cy_en_ble_api_result_t : Return value indicates whether the function succeeded or
*  failed. Following are the possible error codes.
*
*  Error codes                          | Description
*  ------------                         | -----------
*  CY_BLE_SUCCESS                       | On successful operation.
*  CY_BLE_ERROR_INVALID_PARAMETER       | If 'param' is NULL or 'connHandle' is invalid.
*  CY_BLE_ERROR_INVALID_OPERATION       | This operation is not permitted.
*  CY_BLE_ERROR_MEMORY_ALLOCATION_FAILED| Memory allocation failed.
*     
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_GATTC_SendExecuteWriteReq
(
    cy_stc_ble_gattc_exec_write_req_t  * param
);


/******************************************************************************
* Function Name: Cy_BLE_GATTC_WriteWithoutResponse
***************************************************************************//**
* 
*  This function is used by the GATT Client application to do a Write Without Response procedure 
*  on a Characteristic in the peer GATT Server. This is a blocking function. No event is 
*  generated on calling this function.
* 
*  Internally, Write Command is sent to the GATT Server and nothing is
*  received in response from the GATT Server.
* 
*  Refer to Bluetooth 5.0 core specification, Volume 3, Part G, section 4.9.1
*  for more details on the sequence of operations.
*
*  If the peer GATT server is a PSoC 6 MCU, Write Without Response PDU results in 
*  CY_BLE_EVT_GATTS_WRITE_CMD_REQ event GATT Server's end.
* 
*  \param param: Pointer to a variable of type cy_stc_ble_gattc_write_req_t.
*                     Where, the following needs to be set:
*                     param->handleValPair
*                                 param->connHandle  
* 
* \return
*  cy_en_ble_api_result_t : Return value indicates whether the function succeeded or
*  failed. Following are the possible error codes.
*
*  Error codes                          | Description
*  ------------                         | -----------
*  CY_BLE_SUCCESS                       | On successful operation.
*  CY_BLE_ERROR_INVALID_PARAMETER       | If 'param' is NULL or 'connHandle' is invalid, or if 'attrHandle' is zero or if 'param->handleValPair.value.len' value is greater than (Effective GATT MTU-3).
*  CY_BLE_ERROR_INVALID_OPERATION       | This operation is not permitted.
*  CY_BLE_ERROR_MEMORY_ALLOCATION_FAILED| Memory allocation failed.
* 
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_GATTC_WriteWithoutResponse
(
    cy_stc_ble_gattc_write_cmd_req_t  * param
);


/******************************************************************************
* Function Name: Cy_BLE_GATTC_SignedWriteWithoutRsp
***************************************************************************//**
* 
*  This function is used by the GATT Client application to write a specific characteristic value
*  when the ATT bearer is not encrypted. This procedure shall be used only if the 
*  Characteristic Properties authenticated bit is enabled and the GATT client and GATT server
*  devices share a bond as defined in Bluetooth Spec 5.0 [Vol. 3] Part C, Generic Access Profile.
*  This is a blocking function. No event is generated on calling this function.
* 
*  This function only writes the first (GATT_MTU - 15) octets of an 
*  Attribute Value. This function cannot be used to write a long Attribute.
* 
*  Internally, Signed Write Command is used.
*  Refer to Bluetooth Spec 5.0 Security Manager [Vol. 3] Part H, Section 2.4.5.
* 
*  If the authenticated Characteristic Value that is written is the wrong size, 
*  has an invalid value as defined by the profile, or the signed value does not
*  authenticate the client, then the write shall not succeed and no error shall
*  be generated by the server.
*  
*  If peer GATT server is a PSoC 6 device, Signed Write Without Response PDU results in
*  CY_BLE_EVT_GATTS_DATA_SIGNED_CMD_REQ event at the GATT Server's end.
* 
*  \param param: Pointer to a variable of type cy_stc_ble_gattc_write_req_t.
*                     Where, the following needs to be set:
*                     param->handleValPair
*                                 param->connHandle  
* 
* \return
*  cy_en_ble_api_result_t : Return value indicates whether the function succeeded or
*  failed. Following are the possible error codes.
*
*  Error codes                          | Description
*  ------------                         | -----------
*  CY_BLE_SUCCESS                       | On successful operation.
*  CY_BLE_ERROR_INVALID_PARAMETER       | If 'param' is NULL or if 'attrHandle' is zero.
*  CY_BLE_ERROR_NO_DEVICE_ENTITY        | 'connHandle.bdHandle' value does not represent any existing entry in the Stack.
*  CY_BLE_ERROR_MEMORY_ALLOCATION_FAILED| Memory allocation failed.
*  CY_BLE_ERROR_INSUFFICIENT_RESOURCES  | BLE Stack resources are unavailable.
* 
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_GATTC_SignedWriteWithoutRsp
(
    cy_stc_ble_gattc_signed_write_cmd_req_t  * param
);


/** \cond IGNORE */
/******************************************************************************
* Function Name: Cy_BLE_GATTC_Confirmation
***************************************************************************//**
* 
*  This function is used by the GATT Client application to send Handle Value confirmation PDU 
*  to the GATT Server. The application should use this function on receiving Handle Value
*  Indication through CY_BLE_EVT_GATTC_HANDLE_VALUE_IND event at the GATT 
*  Client application's end. This is a non-blocking function.
* 
*  If the peer GATT Server is a PSoC 6 MCU, Handle Value Confirmation PDU results in 
*  CY_BLE_EVT_GATTS_HANDLE_VALUE_CNF event at the GATT Server's end.
* 
*  Refer to the Bluetooth 5.0 core specification, Volume 3, Part G, section 4.11.1
*  for more details on the sequence of operations.
*     
*  \param param: Pointer to a variable of type cy_stc_ble_gattc_confirmation_req_t.
*                     Where, the following needs to be set:
*                                 param->connHandle  
* 
* \return
*  cy_en_ble_api_result_t : Return value indicates whether the function succeeded or
*  failed. Following are the possible error codes.
*
*  Error codes                          | Description
*  ------------                         | -----------
*  CY_BLE_SUCCESS                       | On successful operation.
*  CY_BLE_ERROR_INVALID_PARAMETER       | If 'param' is NULL or 'connHandle' is invalid.
*  CY_BLE_ERROR_INVALID_OPERATION       | This operation is not permitted.
*  CY_BLE_ERROR_MEMORY_ALLOCATION_FAILED| Memory allocation failed.
* 
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_GATTC_Confirmation
(
    cy_stc_ble_gattc_confirmation_req_t  * param
);
/** \endcond */

#ifdef __cplusplus
}
#endif /* __cplusplus */

/** @} */

#endif /* CY_BLE_STACK_GATT_CLIENT_H_ */

/* EOF */
