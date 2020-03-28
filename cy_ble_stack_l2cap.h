/*******************************************************************************
* \file cy_ble_stack_l2cap.h
* \version 3.40
*
* \brief
*  This file contains declarations of public BLE APIs of L2CAP.
*  Also specifies the defines, constants, and data structures required for the APIs.
* 
* Related Document:
*  BLE Standard Spec - CoreV4.2, CSS, CSAs, ESR05, ESR06
* 
********************************************************************************
* \copyright
* Copyright 2017-2020, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/


#ifndef CY_BLE_STACK_L2CAP_H_
#define CY_BLE_STACK_L2CAP_H_


/***************************************
* Common BLE Stack includes
***************************************/

#include "cy_ble_stack_gap.h"

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/***************************************
** L2CAP Constants
***************************************/
/** LE IPSP PSM */
#define CY_BLE_L2CAP_PSM_LE_PSM_IPSP        0x0023u
/** L2CAP Invalid PSM */
#define CY_BLE_L2CAP_PSM_INVALID             0x0000u

/** L2CAP Header Size */
#define CY_BLE_L2CAP_HDR_LEN                 0x04u /**< L2CAP header len */
#define CY_BLE_L2CAP_CREDIT_MODE_HDR_LEN     0x06u /**< L2CAP Credit mode header len */

/** L2CAP Connect Results */
/** L2CAP connection pending */
#define CY_BLE_L2CAP_CONNECTION_SUCCESSFUL                               0x0000u
 /** L2CAP connection pending */
#define CY_BLE_L2CAP_CONNECTION_PENDING                                  0x0001u
/** L2CAP connection refused due to unsupported psm */
#define CY_BLE_L2CAP_CONNECTION_REFUSED_PSM_UNSUPPORTED                  0x0002u
/** L2CAP connection refused due to security block */
#define CY_BLE_L2CAP_CONNECTION_REFUSED_SECURITY_BLOCK                   0x0003u
/** L2CAP connection refused due to no resources */
#define CY_BLE_L2CAP_CONNECTION_REFUSED_NO_RESOURCE                      0x0004u
/** L2CAP connection refused due to insufficient authentication */
#define CY_BLE_L2CAP_CONNECTION_REFUSED_AUTHENTICATION_INSUFFICIENT      0x0005u
/** L2CAP connection refused due to insufficient authorization */
#define CY_BLE_L2CAP_CONNECTION_REFUSED_AUTHORIZATION_INSUFFICIENT       0x0006u
/** L2CAP connection refused due to insufficient enc key size */
#define CY_BLE_L2CAP_CONNECTION_REFUSED_ENC_KEY_SIZE_INSUFFICIENT        0x0007u
/** L2CAP connection refused due to insufficient encryption */
#define CY_BLE_L2CAP_CONNECTION_REFUSED_ENCRYPTION_INSUFFICIENT          0x0008u
/** L2CAP connection refused due to source cid is invalid */
#define CY_BLE_L2CAP_CONNECTION_REFUSED_SRC_CID_INVALID                  0x0009u
/** L2CAP connection refused due to source cid already allocated */
#define CY_BLE_L2CAP_CONNECTION_REFUSED_SRC_CID_ALREADY_ALLOCATED        0x000Au
/** L2CAP connection refused due to unacceptable parameters */
#define CY_BLE_L2CAP_CONNECTION_REFUSED_UNACCEPTABLE_PARAMETERS          0x000Bu

/**Response codes for Connection parameter update request */
/** Connection Parameters accepted */
#define CY_BLE_L2CAP_CONN_PARAM_ACCEPTED            0x0000u
/** Connection Parameters rejected */
#define CY_BLE_L2CAP_CONN_PARAM_REJECTED            0x0001u

/** Config Parameter index for PSM */
#define CY_BLE_L2CAP_CONFIG_POS_PSM                 0x00u

/** Config Parameter index for CBFC Channel */
#define CY_BLE_L2CAP_CONFIG_POS_CHANNEL                0x01u

/** Config Parameter index for L2CAP Signalling timeout */
#define CY_BLE_L2CAP_CONFIG_POS_SIG_TO                0x02u

/***************************************
** Enumerated Types
***************************************/

/**
 \addtogroup group_ble_common_api_l2cap_definitions
 @{
*/

/** Reason for command reject event - CY_BLE_EVT_L2CAP_COMMAND_REJ */
typedef enum
{
    /** Command Not Understood */
    CY_BLE_L2CAP_COMMAND_NOT_UNDERSTOOD = 0x0000u,
    
    /** Signaling L2CAP MTU exceeded */
    CY_BLE_L2CAP_SIGNALLING_MTU_EXCEEDED,
    
    /** Invalid Connection Identifier in request */
    CY_BLE_L2CAP_INVALID_CID_IN_REQUEST

}cy_en_ble_l2cap_command_rej_reason_t;

/** The result code of call back structures for L2CAP */
typedef enum
{
    /** Operation Successful */
    CY_BLE_L2CAP_RESULT_SUCCESS = 0x0000u,

    /** Command timeout,  if L2CAP signaling channel timeout occurs, app should disconnect.*/
    CY_BLE_L2CAP_RESULT_COMMAND_TIMEOUT = 0x2318u,

    /** Invalid SDU length */
    CY_BLE_L2CAP_RESULT_INCORRECT_SDU_LENGTH = 0x2347u,

    /** Not enough Credit to perform this operation */
    CY_BLE_L2CAP_RESULT_NOT_ENOUGH_CREDITS = 0x2371u,
    
    /** Credit overflow. Total Credit exceeded 65535 (maximum) */
    CY_BLE_L2CAP_RESULT_CREDIT_OVERFLOW = 0x2373u,
    
    /** Invalid Credit value, receive Credit is Zero */
    CY_BLE_L2CAP_RESULT_UNACCEPTABLE_CREDIT_VALUE = 0x2374u

}cy_en_ble_l2cap_result_param_t;

/***************************************
** Exported data structures
***************************************/

/** L2CAP Credit based flow Connection parameter */
typedef struct 
{
    /** PSM value of the higher-level protocol */
    uint16_t   l2capPsm;

    /** Upper Layer defined Receive Credit Low Mark */
    uint16_t   creditLwm;

}cy_stc_ble_l2cap_cbfc_psm_info_t;

/** L2CAP Credit based flow Connection parameter */
typedef struct 
{
    /** L2CAP MTU - Maximum SDU Size
 
       The L2CAP MTU field specifies the maximum SDU size (in octets) that the L2CAP
       layer entity sending the LE Credit Based Connection Request can receive
       on this channel. L2CAP implementations shall support a minimum L2CAP MTU size
       of 23 octets. */
    uint16_t   mtu;

    /** MPS - Maximum PDU Size 
 
       The MPS field specifies the maximum payload size (in octets) that the
       L2CAP layer entity sending the LE Credit Based Connection Request is
       capable of receiving on this channel. L2CAP implementations shall support
       a minimum MPS of 23 octets and may support a maximum MPS up to 65488 octets. */
    uint16_t   mps;

    /** Initial Number of Credits
    
       The initial credit value indicates the number of LE-frames that the peer
       device can send to the L2CAP layer entity sending the LE Credit Based
       Connection Request. The initial credit value shall be in the range of 0 to
       65535. */
    uint16_t   credit;

}cy_stc_ble_l2cap_cbfc_connection_info_t;

/** Connect request parameter*/
typedef struct
{
    /** L2CAP Credit based flow Connection parameter */
    cy_stc_ble_l2cap_cbfc_connection_info_t  connParam;

    /**  Remote PSM value */
    uint16_t                              remotePsm;

    /** Local PSM value */
    uint16_t                              localPsm;

    /** bd handle of the remote device */
    uint8_t                               bdHandle;

}cy_stc_ble_l2cap_cbfc_conn_req_info_t;

/** L2cap flow control state */
typedef struct
{
    /**
    * States indicated by flowState variable
    *
    * CY_BLE_STACK_STATE_BUSY (0x01) = CY_BLE_STACK_STATE_BUSY indicates to the application that the BLE Stack's internal buffers
    *                    are about to be filled, and the remaining buffers are required to respond to the peer BLE device.
    *                    After this event, the application shall not initiate GATT, GAP Security, or L2CAP data transactions.
    *                    However the application shall respond to peer initiated transactions to prevent BLE protocol timeouts
    *                    from occurring.
    *                    Application-initiated data transactions can be resumed after the CY_BLE_EVT_STACK_BUSY_STATUS
    *                    event when parameter 'CY_BLE_STACK_STATE_FREE' is received.
    *
    * CY_BLE_STACK_STATE_FREE (0x00) = CY_BLE_STACK_STATE_FREE indicates application that pending transactions are completed
    *                    and sufficient buffers are available to process application-initiated transactions.
    *                    The 'CY_BLE_EVT_STACK_BUSY_STATUS' event with 'CY_BLE_STACK_STATE_FREE' indicates to the
    *                    application whether the BLE Stack's internal buffer state has transitioned from 'CY_BLE_STACK_STATE_BUSY'
    *                    to 'CY_BLE_STACK_STATE_FREE'.
    */
    uint8_t                              flowState;

    /** bd handle of the remote device */
    uint8_t                               bdHandle;

}cy_stc_ble_l2cap_state_info_t;

/** Connect response parameter*/
typedef struct
{
    /** L2CAP Credit based flow Connection parameter */
    cy_stc_ble_l2cap_cbfc_connection_info_t     connParam;

    /**  This parameter specifies the local L2CAP channel end-point for this new L2CAP channel. 
           On receipt of L2CAP Connect Request command from the peer, the local L2CAP will temporarily create a channel. 
              This parameter identifies the new channel. If the upper-layer PSM chooses to reject this connection, 
              this temporary channel will be closed. */
    uint16_t                                localCid;

    /** This parameter specifies the response of the upper layer for the new L2CAP channel establishment 
           request from the peer. It must be set to a value as specified in L2CAP Connect Result Codes. Refer
              to Bluetooth 5.0 core specification, Volume 3, Part A, section 4.23 for more details. */
    uint16_t                                response;

}cy_stc_ble_l2cap_cbfc_conn_resp_info_t;

/** Connect response parameter*/
typedef struct
{
    /**  This parameter specifies the local L2CAP channel end-point for this new L2CAP channel. 
           On receipt of L2CAP Connect Request command from the peer, the local L2CAP will temporarily create a channel. 
              This parameter identifies the new channel. If the upper-layer PSM chooses to reject this connection, 
              this temporary channel will be closed. */
    uint16_t   localCid;

    /** The credit value field represents number of Credits the receiving device can increment. 
          The credit value field is a number between 1 and 65535.*/
    uint16_t   credit;
    
}cy_stc_ble_l2cap_cbfc_credit_info_t;

/** Tx Data parameter*/
typedef struct
{
    /** Transmit data */
    uint8_t     * buffer;

    /** L2CAP Data Packet length. It shall be the minimum of local L2CAP 
        MTU and peer L2CAP MTU size. */
    uint16_t      bufferLength;    

    /**This parameter specifies the local channel end-point for the L2CAP
             channel. For the initiator of L2CAP channel establishment, this 
             must be set to the value indicated by the 
             CY_BLE_EVT_L2CAP_CBFC_CONN_CNF event. For the responder, the upper-
             layer protocol obtains this value when it receives the event
             CY_BLE_EVT_L2CAP_CBFC_CONN_IND.*/
    uint16_t      localCid; 

}cy_stc_ble_l2cap_cbfc_tx_data_info_t;

/** Disconnect request parameter */
typedef struct
{
    /**This parameter specifies the local channel end-point for the L2CAP
             channel. For the initiator of L2CAP channel establishment, this 
             must be set to the value indicated by the 
             CY_BLE_EVT_L2CAP_CBFC_CONN_CNF event. For the responder, the upper-
             layer protocol obtains this value when it receives the event
             CY_BLE_EVT_L2CAP_CBFC_CONN_IND.*/
    uint16_t      localCid; 

}cy_stc_ble_l2cap_cbfc_disconn_req_info_t;

/** Connection parameter update response parameter */
typedef struct
{
    /**This field indicates the response to the Connection Parameter Update Request*/
    uint16_t   result; 

    /** bd handle of the remote device */
    uint8_t    bdHandle;

}cy_stc_ble_l2cap_conn_update_param_rsp_info_t;

/** L2CAP queue flow control with water level marking */
typedef struct
{
    /** bd handle of remote device*/
    uint8_t    bdHandle;
    
    /** low water mark level*/
    uint8_t    lowWaterMark;
    
    /** high water level */
    uint8_t    highWaterMark;
    
} cy_stc_ble_l2cap_queue_flow_control_info_t;

/* --------------------------Structure corresponding to events-------------------- */

/** Connect indication parameter*/
typedef struct
{
    /** L2CAP Credit based flow Connection parameter */
    cy_stc_ble_l2cap_cbfc_connection_info_t     connParam;

    /** Local CID */
    uint16_t                                 lCid; 

    /** Local PSM value */
    uint16_t                                 psm; 

    /** bd handle of the remote device */
    uint8_t                               bdHandle;

}cy_stc_ble_l2cap_cbfc_conn_ind_param_t;

/** Connect confirmation parameter*/
typedef struct
{
    /** L2CAP Credit based flow Connection parameter */
    cy_stc_ble_l2cap_cbfc_connection_info_t     connParam;

    /** Local CID*/
    uint16_t                                 lCid; 

    /** Response codes for Connection parameter update request */
    uint16_t                                 response; 

    /** bd handle of the remote device */
    uint8_t                               bdHandle;    

}cy_stc_ble_l2cap_cbfc_conn_cnf_param_t;

/** Disconnect confirmation parameter */
typedef struct
{
    /** Local CID */
    uint16_t                             lCid; 

    /** The result field indicates the outcome of the connection request. The result
       value of 0x0000 indicates success while a non-zero value indicates the connection
       request failed or is pending. */
    cy_en_ble_l2cap_result_param_t         result; 

}cy_stc_ble_l2cap_cbfc_disconn_cnf_param_t;

/** Receive Data parameter */
typedef struct
{
    /** Received L2cap Data */
    uint8_t                            * rxData;

    /** Received L2cap Data Length */
    uint16_t                            rxDataLength;

    /** Local CID */
    uint16_t                            lCid; 

    /** A result value of 0x0000 indicates success, while a non-zero value
       indicates an error condition (e.g. peer device violating Credit flow, or L2CAP MTU size limit). */
    cy_en_ble_l2cap_result_param_t      result; 

}cy_stc_ble_l2cap_cbfc_rx_param_t;

/** Rx Credit info parameter*/
typedef struct
{
    /** Local CID */
    uint16_t         lCid; 

   /** The number of Credits (LE-frames) */
    uint16_t         credit; 

}cy_stc_ble_l2cap_cbfc_low_rx_credit_param_t;

/** Tx Credit info parameter */
typedef struct
{
    /** Local CID */
    uint16_t                             lCid; 

    /** The number of Credits (LE-frames) */
    uint16_t                         credit; 

    /** A result value of 0x0000 indicates success, while a non-zero value
       indicates an error condition (e.g. Credit overflow, or if total number of Credits
       crosses specification defined maximum limit of 0xFFFF). */
    cy_en_ble_l2cap_result_param_t           result; 

}cy_stc_ble_l2cap_cbfc_low_tx_credit_param_t;

/** Data Write parameter*/
typedef struct
{
    /**Local CID*/
    uint16_t                             lCid; 

    /** A result value of 0x0000 indicates success, 
            while a non-zero value indicates an error condition */
    cy_en_ble_l2cap_result_param_t       result; 

    /** Currently NULL. For future usage */
    uint8_t                             * buffer;

    /** Currently 0. For future usage */
    uint16_t                             bufferLength;    

}cy_stc_ble_l2cap_cbfc_rx_data_param_t;

/** L2CAP Connection parameters  Update response*/
typedef struct
{
    /** connection parameter update response received from the master.
        * Accepted = 0x0000
        * Rejected  = 0x0001 */
    uint16_t      result;

    /** Peer bdHandle */
    uint8_t       bdHandle;

}cy_stc_ble_l2cap_conn_update_rsp_param_t;

/** L2CAP command reject parameter */
typedef struct
{
    /** Reason for command reject  */
    cy_en_ble_l2cap_command_rej_reason_t        result;

    /** Peer bdHandle */
    uint8_t                                   bdHandle;

}cy_stc_ble_l2cap_cmd_rej_param_t;

/** @} */

/***************************************
** Exported APIs
***************************************/

/**
 \addtogroup group_ble_common_api_l2cap_functions
 @{
*/

/******************************************************************************
* Function Name: Cy_BLE_L2CAP_CbfcRegisterPsm
***************************************************************************//**
* 
*  This function is used in L2CAP LE Credit Based Flow Control mode. This function registers a
*  new PSM with L2CAP module.
*  This is a blocking function. No event is generated on calling this function.
* 
*  Refer to Bluetooth 5.0 core specification, Volume 3, Part A, section 3.4 
*  for more details about Credit based flow control mode of operation.
*  Refer to Bluetooth core specification, Volume 3, Part A, section 4.22 for more
*  details about LE PSM.
* 
*  \param param: Parameter is of type  'cy_stc_ble_l2cap_cbfc_psm_info_t'
* 
* \return
*  cy_en_ble_api_result_t : Return value indicates whether the function succeeded or
*  failed. Following are the possible error codes.
*
*  Errors codes                               | Description
*  ------------                               | -----------
*   CY_BLE_SUCCESS                            | On successful operation.
*   CY_BLE_ERROR_INVALID_PARAMETER            | Input param is NULL or 'l2capPsm' is 0.
*   CY_BLE_ERROR_INSUFFICIENT_RESOURCES       | Cannot register more than one PSM.
*   CY_BLE_ERROR_L2CAP_PSM_NOT_IN_RANGE       | The PSM is not in range of 0x0001 - 0x00FF.
*   CY_BLE_ERROR_L2CAP_PSM_ALREADY_REGISTERED | PSM already Registered.
* 
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_L2CAP_CbfcRegisterPsm
(
    cy_stc_ble_l2cap_cbfc_psm_info_t  * param
);

/******************************************************************************
* Function Name: Cy_BLE_L2CAP_CbfcUnregisterPsm
***************************************************************************//**
* 
*  This function de-registers previously registered PSM from the L2CAP for 
*  the L2CAP Credit Based Flow Control mode. This is a blocking function. No 
*  event is generated on calling this function.
* 
*  \param param: Parameter is of type  'cy_stc_ble_l2cap_cbfc_psm_info_t'
*              param->creditLwm : ignored
*
* \return
*  cy_en_ble_api_result_t : Return value indicates whether the function succeeded or
*  failed. Following are the possible error codes.
*
*  Errors codes                           | Description
*  ------------                           | -----------
*   CY_BLE_SUCCESS                        | On successful operation.
*   CY_BLE_ERROR_INVALID_PARAMETER        | On specifying NULL for input parameter.
*   CY_BLE_ERROR_INVALID_OPERATION        | Active CID is present with given PSM.
*   CY_BLE_ERROR_L2CAP_PSM_NOT_REGISTERED | PSM is not registered.
* 
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_L2CAP_CbfcUnregisterPsm 
(
    cy_stc_ble_l2cap_cbfc_psm_info_t  * param
);


/******************************************************************************
* Function Name: Cy_BLE_L2CAP_CbfcConnectReq
***************************************************************************//**
* 
*  This function is used to send LE Credit Based Connection Request packet to create and configure 
*  an L2CAP CBFC channel between two devices. This request is sent to the specified
*  peer Bluetooth device, with specific peer PSM. This is a non-blocking function.
*  Calling this function triggers CY_BLE_EVT_L2CAP_CBFC_CONN_IND event at receiver's end if
*  the receiver is a PSoC 6 device.
*  Following events are generated at the sender's end:
*   - CY_BLE_EVT_L2CAP_CBFC_CONN_CNF - Generated when the receiver responds to the request. 
*     The response field in the event parameter returned along with this event indicates
*     the status of the request; if the response is CY_BLE_L2CAP_CONNECTION_SUCCESSFUL, 
*     it indicates that the L2CAP channel was successfully created.
*   - CY_BLE_EVT_L2CAP_COMMAND_REJ- Generated when the receiver rejects the connection request.
* 
*  Refer to Bluetooth 5.0 core specification, Volume 3, Part A, section 4.22 for
*  more details about this operation.
* 
* 
*  \param param: Parameter is of type 'cy_stc_ble_l2cap_cbfc_conn_req_info_t'.
*     
* \return
*  cy_en_ble_api_result_t : Return value indicates whether the function succeeded or
*  failed. Following are the possible error codes.
*
*  Errors codes                           | Description
*  ------------                              | -----------
*   CY_BLE_SUCCESS                        | On successful operation.
*   CY_BLE_ERROR_INVALID_PARAMETER        | If "param" is NULL.
*   CY_BLE_ERROR_INSUFFICIENT_RESOURCES   | Insufficient resources.
*   CY_BLE_ERROR_NO_DEVICE_ENTITY         | 'bdHandle' does not represent known device entity.
*   CY_BLE_ERROR_L2CAP_PSM_NOT_REGISTERED | PSM not Registered.
* 
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_L2CAP_CbfcConnectReq
(
    cy_stc_ble_l2cap_cbfc_conn_req_info_t  * param
);

/******************************************************************************
* Function Name: Cy_BLE_L2CAP_CbfcConnectRsp
***************************************************************************//**
* 
*  This function should be called by the application to send the LE Credit Based Connection Response packet 
*  to the peer Bluetooth device. Application should call this function on receiving the L2CAP CBFC 
*  connection request indication through the CY_BLE_EVT_L2CAP_CBFC_CONN_IND event.
*  This is a non-blocking function.
* 
*  Refer to Bluetooth 5.0 core specification, Volume 3, Part A, section 4.23
*  for more details about this operation.
*  
*  If the application has set param->response as CY_BLE_L2CAP_CONNECTION_SUCCESSFUL and
*  the return value is CY_BLE_SUCCESS, L2CAP CBFC Channel creation is successful.
*
*  \param param: Parameter is of type 'cy_stc_ble_l2cap_cbfc_conn_resp_info_t'.
*     
* \return
*  cy_en_ble_api_result_t : Return value indicates whether the function succeeded or
*  failed. Following are the possible error codes.
*
*  Errors codes                                      | Description
*  ------------                                      | -----------
*   CY_BLE_SUCCESS                                   | On successful operation.
*   CY_BLE_ERROR_INVALID_PARAMETER                   | If "param" is NULL.
*   CY_BLE_ERROR_L2CAP_CONNECTION_ENTITY_NOT_FOUND   | Connection entity is not found.
* 
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_L2CAP_CbfcConnectRsp
(
    cy_stc_ble_l2cap_cbfc_conn_resp_info_t     * param
);

/******************************************************************************
* Function Name: Cy_BLE_L2CAP_CbfcSendFlowControlCredit
***************************************************************************//**
* 
*  This function enables an application to send a LE Flow Control 
*  Credit packet to the peer Bluetooth device when it is capable of receiving
*  additional LE-frames. This is a non-blocking function.
*   
*  This function is invoked when the device is expecting more data from the peer
*  device. If the application gets CY_BLE_EVT_L2CAP_CBFC_RX_CREDIT_IND event, it indicates 
*  that the peer device is low on Credits to send more data. The application can send more Credits 
*  to the peer device by calling this function.
* 
*  When a peer PSoC 6 MCU receives this packet, it generates the CY_BLE_EVT_L2CAP_CBFC_TX_CREDIT_IND
*  event to indicate that the receiver can send more LE-frames event. It is
*  the responsibility of the application layer of the device sending the Credit
*  to keep track of the total number of Credits and ensure that it does
*  not exceed 65535.
* 
*  Refer to Bluetooth 5.0 core specification, Volume 3, Part A, section 4.24 for
*  more details about this operation.
* 
*  \param param: Parameter is of type 'cy_stc_ble_l2cap_cbfc_credit_info_t'.
*     
* \return
*  cy_en_ble_api_result_t : Return value indicates whether the function succeeded or
*  failed. Following are the possible error codes.
*
*  Errors codes                                    | Description
*  ------------                                    | -----------
*   CY_BLE_SUCCESS                                 | On successful operation.
*   CY_BLE_ERROR_MEMORY_ALLOCATION_FAILED          | Memory allocation failed.
*   CY_BLE_ERROR_L2CAP_CONNECTION_ENTITY_NOT_FOUND | L2CAP connection instance is not present.
*   CY_BLE_ERROR_INSUFFICIENT_RESOURCES            | Insufficient resources.
* 
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_L2CAP_CbfcSendFlowControlCredit
(
    cy_stc_ble_l2cap_cbfc_credit_info_t  * param
);

/******************************************************************************
* Function Name: Cy_BLE_L2CAP_ChannelDataWrite
***************************************************************************//**
* 
*  This function is used to send a data packet on the L2CAP CBFC channel. This is a 
*  blocking function. This function should be used after successful creation of 
*  L2CAP CBFC channel.
* 
*  This function generates a 'CY_BLE_EVT_L2CAP_CBFC_DATA_WRITE_IND' event that 
*  is kept for backward compatibility with previous Cypress' applications and the 
*  user should handle cy_en_ble_api_result_t to determine whether the last data packet was sent properly.
* 
*  If the peer device is PSoC6 device, the CY_BLE_EVT_L2CAP_CBFC_DATA_READ event is used 
*  to inform the application about the data received over the L2CAP CBFC channel.
*
*  Refer to Bluetooth 5.0 core specification, Volume 3, Part A, section 3.4 for 
*  more details about this operation.
* 
*  \param param: Parameter is of type 'cy_stc_ble_l2cap_cbfc_tx_data_info_t'.
*     
* \return
*  cy_en_ble_api_result_t : Return value indicates whether the function succeeded or
*  failed. Following are the possible error codes.
*
*  Errors codes                                    | Description
*  ------------                                    | -----------
*   CY_BLE_SUCCESS                                 | On successful operation.
*   CY_BLE_ERROR_INVALID_PARAMETER                 | If 'param' or 'buffer' is NULL.
*   CY_BLE_ERROR_MEMORY_ALLOCATION_FAILED          | Memory allocation failed.
*   CY_BLE_ERROR_NO_CONNECTION                     | No Link Layer connection is present.
*   CY_BLE_ERROR_L2CAP_CONNECTION_ENTITY_NOT_FOUND | No L2CAP CBFC channel found corresponding to CID.
* 
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_L2CAP_ChannelDataWrite
(
    cy_stc_ble_l2cap_cbfc_tx_data_info_t  * param
);

/******************************************************************************
* Function Name: Cy_BLE_L2CAP_DisconnectReq
***************************************************************************//**
* 
*  This function is used to send L2CAP Disconnect Request packet to terminate the specified 
*  L2CAP CBFC channel on a peer device. This is a non-blocking function.
*  Upon calling this function from local device, CY_BLE_EVT_L2CAP_CBFC_DISCONN_IND event 
*  is generated on the peer side if the peer is a PSoC 6 device.
* 
*  Disconnection of the L2CAP CBFC channel always succeeds - either by reception of 
*  the L2CAP Disconnect Response from the peer, or by timeout. In either case, 
*  L2CAP will confirm disconnection of the channel by generating the 
*  CY_BLE_EVT_L2CAP_CBFC_DISCONN_CNF event.
* 
*  Refer to Bluetooth 5.0 core specification, Volume 3, Part A, section 4.6 for
*  more details about this operation.
* 
*  \param param: Parameter is of type 'cy_stc_ble_l2cap_cbfc_disconn_req_info_t'.
* 
* \return
*  cy_en_ble_api_result_t : Return value indicates whether the function succeeded or
*  failed. Following are the possible error codes.
*
*  Errors codes                                    | Description
*  ------------                                    | -----------
*   CY_BLE_SUCCESS                                 | On successful operation.
*   CY_BLE_ERROR_INVALID_OPERATION                 | No Link Layer connection is present.
*   CY_BLE_ERROR_MEMORY_ALLOCATION_FAILED          | Memory allocation failed.
*   CY_BLE_ERROR_L2CAP_CONNECTION_ENTITY_NOT_FOUND | No connection entity found that can be disconnected.    
*   CY_BLE_ERROR_INSUFFICIENT_RESOURCES            | Insufficient resources.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_L2CAP_DisconnectReq
(
     cy_stc_ble_l2cap_cbfc_disconn_req_info_t  * param
);


/******************************************************************************
* Function Name: Cy_BLE_L2CAP_LeConnectionParamUpdateRequest
***************************************************************************//**
* 
*  This function enables an application to send Connection Parameter Update Request packet. 
*  This function should be used only in LE Peripheral role. This function is used to request 
*  the peer Central device to update the connection parameters.
*
*  This is a non-blocking function. If the peer Central device is PSoC6 device, then this
*  function results in a CY_BLE_EVT_L2CAP_CONN_PARAM_UPDATE_REQ event at the Peer's end.
*
*  If peer Central device accepts and sends Connection Parameter Update Response, then 
*  CY_BLE_EVT_L2CAP_CONN_PARAM_UPDATE_RSP event is generated at Peripheral side.
* 
*  To update the connection parameters from the Central side use the
*  Cy_BLE_GAPC_ConnectionParamUpdateRequest() function. 
* 
*  Refer to Bluetooth 5.0 core specification, Volume 3, Part A, section 4.20 for
*  more details about this operation.
* 
*  \param param: Parameter is of type 'cy_stc_ble_gap_conn_update_param_info_t'.
* 
* \return
*  cy_en_ble_api_result_t : Return value indicates whether the function succeeded or
*  failed. Following are the possible error codes.
*
*  Errors codes                           | Description
*  ------------                           | -----------
*   CY_BLE_SUCCESS                        | On successful operation.
*   CY_BLE_ERROR_INVALID_PARAMETER        | If "connParam" is NULL.
*   CY_BLE_ERROR_INVALID_OPERATION        | Connection Parameter Update Request is not allowed.
*   CY_BLE_ERROR_MEMORY_ALLOCATION_FAILED | Memory allocation failed.
*   CY_BLE_ERROR_NO_CONNECTION            | No Link Layer connection is  present.
*   CY_BLE_ERROR_NO_DEVICE_ENTITY         | 'bdHandle' does not represent a known device entity.
*
*   Note: Please refer the function documentation of Cy_BLE_GAPC_InitConnection() for recommended
*   Connection Interval values.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_L2CAP_LeConnectionParamUpdateRequest
(
    cy_stc_ble_gap_conn_update_param_info_t  * param
);


/******************************************************************************
* Function Name: Cy_BLE_L2CAP_LeConnectionParamUpdateResponse
***************************************************************************//**
* 
*  This function is used to send Connection Parameter Update Response packet. 
*  This function should be used in LE Central role. Application should call this function on 
*  receiving the Connection Parameter Update Request through
*  CY_BLE_EVT_L2CAP_CONN_PARAM_UPDATE_REQ event. This is a non-blocking function.
* 
*  If the peer Peripheral device is PSoC6 device, this function results in 
*  CY_BLE_EVT_L2CAP_CONN_PARAM_UPDATE_RSP event at the Peripheral side.
*
*  \param param: Parameter is of type 'cy_stc_ble_l2cap_conn_update_param_rsp_info_t'.
* 
* \return
*  cy_en_ble_api_result_t : Return value indicates whether the function succeeded or
*  failed. Following are the possible error codes.
*
*  Errors codes                           | Description
*  ------------                           |    -----------
*   CY_BLE_SUCCESS                        | On successful operation.
*   CY_BLE_ERROR_INVALID_PARAMETER        | If 'result' is invalid (greater than connection parameter reject code i.e., 0x0001).
*   CY_BLE_ERROR_MEMORY_ALLOCATION_FAILED | Memory allocation failed.
*   CY_BLE_ERROR_NO_CONNECTION            | No Link Layer connection is present.
*   CY_BLE_ERROR_NO_DEVICE_ENTITY         | 'bdHandle' does not represent known device entity.
* 
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_L2CAP_LeConnectionParamUpdateResponse
(
    cy_stc_ble_l2cap_conn_update_param_rsp_info_t  * param
);

/******************************************************************************
* Function Name: Cy_BLE_L2CAP_SetFlowControlLimits
***************************************************************************//**
* 
*  This function allows the application to set L2CAP flow control limits using high
*  and low water marking for the L2CAP queue. This function should be used one time after 
*  successful LE Connection with peer device.
*
*  HighWaterMark is used to configure the maximum number of L2CAP queue entries that can be
*  used for the data (GATT, SMP, and L2CAP CBFC data) transmission.
*
*  During data transmission, when the number of available L2CAP queue entries reaches the 
*  low water mark level, Stack generates a CY_BLE_EVT_STACK_BUSY_STATUS event with 
*  status as busy, so that the application can stop queuing data and wait until Stack 
*  generates a CY_BLE_EVT_STACK_BUSY_STATUS event with free status.
* 
*  \param param: Parameter is of type 'cy_stc_ble_l2cap_queue_flow_control_info_t'.
*   Min HighWater mark: 1    Max HighWater mark: L2CAP queue depth
*   Min LowWater mark: 0     Max LowWater mark: (HighWaterMark -1)
*
*   Default values set by BLE Stack for better throughput are: LowWaterMark = 3, 
*   HighWaterMark = (L2CAP queue depth - 1).
*
*   Note: If the LowWaterMark is set to less than 3, then do not initiate GATT data 
*   transmission while pairing procedure is going on.
*   
*   For GATT data transmission, the difference between HighWaterMark and 
*   LowWaterMark must not be greater than number of GATT buffers provided during Stack-init
*   to avoid receiving a memory allocation error for GATT transaction.
*
* \return
*  cy_en_ble_api_result_t : Return value indicates whether the function succeeded or
*  failed. Following are the possible error codes.
*
*  Errors codes                           | Description
*  ------------                           | -----------
*   CY_BLE_SUCCESS                        | On successful operation.
*   CY_BLE_ERROR_INVALID_PARAMETER        | If 'param' is NULL or water mark levels are not proper.
*   CY_BLE_ERROR_NO_DEVICE_ENTITY         | 'bdHandle' does not represent known device entity.
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_L2CAP_SetFlowControlLimits
(
    cy_stc_ble_l2cap_queue_flow_control_info_t  * param
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

/** @} */

#endif /*CY_BLE_STACK_L2CAP_H_*/


/*EOF*/
