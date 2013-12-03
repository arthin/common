/**
 ******************************************************************************
 * @file    usbd_rndis.c
 * @author  arthin
 * @version V0.0.1
 * @date    23-Nov-2013
 * @brief   Generic media access Layer.
 ******************************************************************************
 */

#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED 
#pragma     data_alignment = 4 
#endif /* USB_OTG_HS_INTERNAL_DMA_ENABLED */

/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include "usbd_cdc_core.h"
#include "usbd_rndis.h"
#include "stm32f4xx_conf.h"

#define VENDOR_DESCRIPTION "RNDIS Demo Adapter"
#define ADAPTER_MAC_ADDRESS "\x02\x00\x02\x00\x02\0x00"

/* These are external variables imported from CDC core to be used for IN 
 transfer management. */
extern uint8_t APP_Rx_Buffer[]; /* Write CDC received data in this buffer.
 These data will be sent over USB IN endpoint
 in the CDC core functions. */
extern uint32_t APP_Rx_ptr_in; /* Increment this pointer or roll it back to
 start address when writing received data
 in the buffer APP_Rx_Buffer. */

/* Private variables */
static uint8_t  ResponseReady = 0; 
/**< Internal flag indicating if a RNDIS message is waiting to be returned to the host. */
static uint8_t  CurrRNDISState = RNDIS_Uninitialized;
/**< Current RNDIS state of the adapter, a value from the \ref RNDIS_States_t enum. */
static uint32_t CurrPacketFilter = 0;
/**< Current packet filter mode, used internally by the class driver. */

static const uint32_t AdapterSupportedOIDList[]  =
{
	OID_GEN_SUPPORTED_LIST,
	OID_GEN_PHYSICAL_MEDIUM,
	OID_GEN_HARDWARE_STATUS,
	OID_GEN_MEDIA_SUPPORTED,
	OID_GEN_MEDIA_IN_USE,
	OID_GEN_MAXIMUM_FRAME_SIZE,
	OID_GEN_MAXIMUM_TOTAL_SIZE,
	OID_GEN_LINK_SPEED,
	OID_GEN_TRANSMIT_BLOCK_SIZE,
	OID_GEN_RECEIVE_BLOCK_SIZE,
	OID_GEN_VENDOR_ID,
	OID_GEN_VENDOR_DESCRIPTION,
	OID_GEN_CURRENT_PACKET_FILTER,
	OID_GEN_MAXIMUM_TOTAL_SIZE,
	OID_GEN_MEDIA_CONNECT_STATUS,
	OID_GEN_XMIT_OK,
	OID_GEN_RCV_OK,
	OID_GEN_XMIT_ERROR,
	OID_GEN_RCV_ERROR,
	OID_GEN_RCV_NO_BUFFER,
	OID_802_3_PERMANENT_ADDRESS,
	OID_802_3_CURRENT_ADDRESS,
	OID_802_3_MULTICAST_LIST,
	OID_802_3_MAXIMUM_LIST_SIZE,
	OID_802_3_RCV_ERROR_ALIGNMENT,
	OID_802_3_XMIT_ONE_COLLISION,
	OID_802_3_XMIT_MORE_COLLISIONS,
};


/* Private function prototypes -----------------------------------------------*/
static uint16_t RNDIS_Init(void);
static uint16_t RNDIS_DeInit(void);
static uint16_t RNDIS_Ctrl(uint32_t Cmd, uint8_t* Buf, uint32_t Len);
static uint16_t RNDIS_DataTx(uint8_t* Buf, uint32_t Len);
static uint16_t RNDIS_DataRx(uint8_t* Buf, uint32_t Len);


void RNDIS_Device_ProcessRNDISControlMessage(uint8_t* Buf);
static uint8_t RNDIS_Device_ProcessNDISQuery(uint8_t* const Buf,
                                          const uint32_t OId,
                                          void* const QueryData,
                                          const uint16_t QuerySize,
                                          void* ResponseData,
                                          uint16_t* const ResponseSize);
static uint8_t RNDIS_Device_ProcessNDISSet(uint8_t* const Buf,
                                        const uint32_t OId,
                                        const void* SetData,
                                        const uint16_t SetSize);


CDC_IF_Prop_TypeDef RNDIS_fops = { RNDIS_Init, RNDIS_DeInit, RNDIS_Ctrl, RNDIS_DataTx,
		RNDIS_DataRx };

/* Private functions ---------------------------------------------------------*/
/**
 * @brief  RNDIS_Init
 *         Initializes the Media on the STM32
 * @param  None
 * @retval Result of the opeartion (USBD_OK in all cases)
 */
static uint16_t RNDIS_Init(void) {

        ResponseReady = 0;
        CurrRNDISState = RNDIS_Uninitialized;
        CurrPacketFilter = 0;

	return USBD_OK;
}

/**
 * @brief  RNDIS_DeInit
 *         DeInitializes the Media on the STM32
 * @param  None
 * @retval Result of the opeartion (USBD_OK in all cases)
 */
static uint16_t RNDIS_DeInit(void) {
	return USBD_OK;
}

/**
 * @brief  RNDIS_Ctrl
 *         Manage the CDC class requests
 * @param  Cmd: Command code
 * @param  Buf: Buffer containing command data (request parameters)
 * @param  Len: Number of data to be sent (in bytes)
 * @retval Result of the opeartion (USBD_OK in all cases)
 */
static uint16_t RNDIS_Ctrl(uint32_t Cmd, uint8_t* Buf, uint32_t Len) {

	switch (Cmd)
	{
		case SEND_ENCAPSULATED_COMMAND:
		{
			RNDIS_Device_ProcessRNDISControlMessage(Buf);
		}
		break;
		case GET_ENCAPSULATED_RESPONSE:
		{
			RNDIS_Message_Header_t* MessageHeader = (RNDIS_Message_Header_t*)Buf;
			if (!(MessageHeader->MessageLength))
			{
				Buf[0] = 0;
				MessageHeader->MessageLength                    = 1;
			}
			//todo clear endpoint setup
			//Endpoint_Write_Control_Stream_LE(Buf, MessageHeader->MessageLength);
			//todo clear endpoint out
			MessageHeader->MessageLength = 0;
		}
		break;
	}
	return USBD_OK;
}

void RNDIS_Device_ProcessRNDISControlMessage(uint8_t* Buf)
{
	/* Note: Only a single buffer is used for both the received message and its response to save SRAM. Because of
	         this, response bytes should be filled in order so that they do not clobber unread data in the buffer. */

	RNDIS_Message_Header_t* MessageHeader = (RNDIS_Message_Header_t*)Buf;

	switch (MessageHeader->MessageType)
	{
		case REMOTE_NDIS_INITIALIZE_MSG:
			ResponseReady     = 1;

			RNDIS_Initialize_Message_t*  INITIALIZE_Message  =
			               (RNDIS_Initialize_Message_t*)Buf;
			RNDIS_Initialize_Complete_t* INITIALIZE_Response =
			               (RNDIS_Initialize_Complete_t*)Buf;

			INITIALIZE_Response->MessageType            = REMOTE_NDIS_INITIALIZE_CMPLT;
			INITIALIZE_Response->MessageLength          = sizeof(RNDIS_Initialize_Complete_t);
			INITIALIZE_Response->RequestId              = INITIALIZE_Message->RequestId;
			INITIALIZE_Response->Status                 = REMOTE_NDIS_STATUS_SUCCESS;

			INITIALIZE_Response->MajorVersion           = REMOTE_NDIS_VERSION_MAJOR;
			INITIALIZE_Response->MinorVersion           = REMOTE_NDIS_VERSION_MINOR;
			INITIALIZE_Response->DeviceFlags            = REMOTE_NDIS_DF_CONNECTIONLESS;
			INITIALIZE_Response->Medium                 = REMOTE_NDIS_MEDIUM_802_3;
			INITIALIZE_Response->MaxPacketsPerTransfer  = 1;
			INITIALIZE_Response->MaxTransferSize        = sizeof(RNDIS_Packet_Message_t) + ETHERNET_FRAME_SIZE_MAX;
			INITIALIZE_Response->PacketAlignmentFactor  = 0;
			INITIALIZE_Response->AFListOffset           = 0;
			INITIALIZE_Response->AFListSize             = 0;

			CurrRNDISState    = RNDIS_Initialized;
			break;
		case REMOTE_NDIS_HALT_MSG:
			ResponseReady     = 0;

			MessageHeader->MessageLength                = 0;

			CurrRNDISState    = RNDIS_Uninitialized;
			break;
		case REMOTE_NDIS_QUERY_MSG:
			ResponseReady     = 1;

			RNDIS_Query_Message_t*  QUERY_Message       = (RNDIS_Query_Message_t*)Buf;
			RNDIS_Query_Complete_t* QUERY_Response      = (RNDIS_Query_Complete_t*)Buf;
			uint32_t                Query_Oid           = QUERY_Message->Oid;

			void*    QueryData    = &Buf[sizeof(RNDIS_Message_Header_t) + QUERY_Message->InformationBufferOffset];
			void*    ResponseData = &Buf[sizeof(RNDIS_Query_Complete_t)];
			uint16_t ResponseSize;

			QUERY_Response->MessageType                 = REMOTE_NDIS_QUERY_CMPLT;

			if (RNDIS_Device_ProcessNDISQuery(Buf, Query_Oid, QueryData, QUERY_Message->InformationBufferLength,
			                                  ResponseData, &ResponseSize))
			{
				QUERY_Response->Status                  = REMOTE_NDIS_STATUS_SUCCESS;
				QUERY_Response->MessageLength           = sizeof(RNDIS_Query_Complete_t) + ResponseSize;

				QUERY_Response->InformationBufferLength = ResponseSize;
				QUERY_Response->InformationBufferOffset = sizeof(RNDIS_Query_Complete_t) - sizeof(RNDIS_Message_Header_t);
			}
			else
			{
				QUERY_Response->Status                  = REMOTE_NDIS_STATUS_NOT_SUPPORTED;
				QUERY_Response->MessageLength           = sizeof(RNDIS_Query_Complete_t);

				QUERY_Response->InformationBufferLength = 0;
				QUERY_Response->InformationBufferOffset = 0;
			}

			break;
		case REMOTE_NDIS_SET_MSG:
			ResponseReady     = 1;

			RNDIS_Set_Message_t*  SET_Message           = (RNDIS_Set_Message_t*)Buf;
			RNDIS_Set_Complete_t* SET_Response          = (RNDIS_Set_Complete_t*)Buf;
			uint32_t              SET_Oid               = SET_Message->Oid;

			SET_Response->MessageType                   = REMOTE_NDIS_SET_CMPLT;
			SET_Response->MessageLength                 = sizeof(RNDIS_Set_Complete_t);
			SET_Response->RequestId                     = SET_Message->RequestId;

			void* SetData = &Buf[sizeof(RNDIS_Message_Header_t) + SET_Message->InformationBufferOffset];

			SET_Response->Status = RNDIS_Device_ProcessNDISSet(Buf, SET_Oid, SetData,
			                                                   SET_Message->InformationBufferLength) ?
			                                                   REMOTE_NDIS_STATUS_SUCCESS : REMOTE_NDIS_STATUS_NOT_SUPPORTED;
			break;
		case REMOTE_NDIS_RESET_MSG:
			ResponseReady     = 1;

			RNDIS_Reset_Complete_t* RESET_Response      = (RNDIS_Reset_Complete_t*)Buf;

			RESET_Response->MessageType                 = REMOTE_NDIS_RESET_CMPLT;
			RESET_Response->MessageLength               = sizeof(RNDIS_Reset_Complete_t);
			RESET_Response->Status                      = REMOTE_NDIS_STATUS_SUCCESS;
			RESET_Response->AddressingReset             = 0;

			break;
		case REMOTE_NDIS_KEEPALIVE_MSG:
			ResponseReady     = 1;

			RNDIS_KeepAlive_Message_t*  KEEPALIVE_Message  =
			                (RNDIS_KeepAlive_Message_t*)Buf;
			RNDIS_KeepAlive_Complete_t* KEEPALIVE_Response =
			                (RNDIS_KeepAlive_Complete_t*)Buf;

			KEEPALIVE_Response->MessageType             = REMOTE_NDIS_KEEPALIVE_CMPLT;
			KEEPALIVE_Response->MessageLength           = sizeof(RNDIS_KeepAlive_Complete_t);
			KEEPALIVE_Response->RequestId               = KEEPALIVE_Message->RequestId;
			KEEPALIVE_Response->Status                  = REMOTE_NDIS_STATUS_SUCCESS;

			break;
	}
}

static uint8_t RNDIS_Device_ProcessNDISQuery(uint8_t* const Buf,
                                          const uint32_t OId,
                                          void* const QueryData,
                                          const uint16_t QuerySize,
                                          void* ResponseData,
                                          uint16_t* const ResponseSize)
{
	(void)QueryData;
	(void)QuerySize;

	switch (OId)
	{
		case OID_GEN_SUPPORTED_LIST:
			*ResponseSize = sizeof(AdapterSupportedOIDList);

			memcpy(ResponseData, AdapterSupportedOIDList, sizeof(AdapterSupportedOIDList));

			return 1;
		case OID_GEN_PHYSICAL_MEDIUM:
			*ResponseSize = sizeof(uint32_t);

			/* Indicate that the device is a true ethernet link */
			*((uint32_t*)ResponseData) = 0;

			return 1;
		case OID_GEN_HARDWARE_STATUS:
			*ResponseSize = sizeof(uint32_t);

			*((uint32_t*)ResponseData) = NDIS_HardwareStatus_Ready;

			return 1;
		case OID_GEN_MEDIA_SUPPORTED:
		case OID_GEN_MEDIA_IN_USE:
			*ResponseSize = sizeof(uint32_t);

			*((uint32_t*)ResponseData) = REMOTE_NDIS_MEDIUM_802_3;

			return 1;
		case OID_GEN_VENDOR_ID:
			*ResponseSize = sizeof(uint32_t);
			/* Vendor ID 0x0xFFFFFF is reserved for vendors who have not purchased a NDIS VID */
			*((uint32_t*)ResponseData) = 0x00FFFFFF;
			return 1;
		case OID_GEN_MAXIMUM_FRAME_SIZE:
		case OID_GEN_TRANSMIT_BLOCK_SIZE:
		case OID_GEN_RECEIVE_BLOCK_SIZE:
			*ResponseSize = sizeof(uint32_t);
			*((uint32_t*)ResponseData) = ETHERNET_FRAME_SIZE_MAX;
			return 1;
		case OID_GEN_VENDOR_DESCRIPTION:
			memcpy(ResponseData, VENDOR_DESCRIPTION, sizeof(VENDOR_DESCRIPTION));
			return 1;
		case OID_GEN_MEDIA_CONNECT_STATUS:
			*ResponseSize = sizeof(uint32_t);
			*((uint32_t*)ResponseData) = REMOTE_NDIS_MEDIA_STATE_CONNECTED;
			return 1;
		case OID_GEN_LINK_SPEED:
			*ResponseSize = sizeof(uint32_t);
			/* Indicate 10Mb/s link speed */
			*((uint32_t*)ResponseData) = 100000;
			return 1;
		case OID_802_3_PERMANENT_ADDRESS:
		case OID_802_3_CURRENT_ADDRESS:
			*ResponseSize = 6;
			memcpy(ResponseData, ADAPTER_MAC_ADDRESS, 6);
			return 1;
		case OID_802_3_MAXIMUM_LIST_SIZE:
			*ResponseSize = sizeof(uint32_t);
			/* Indicate only one multicast address supported */
			*((uint32_t*)ResponseData) = 1;
			return 1;
		case OID_GEN_CURRENT_PACKET_FILTER:
			*ResponseSize = sizeof(uint32_t);
			*((uint32_t*)ResponseData) = CurrPacketFilter;
			return 1;
		case OID_GEN_XMIT_OK:
		case OID_GEN_RCV_OK:
		case OID_GEN_XMIT_ERROR:
		case OID_GEN_RCV_ERROR:
		case OID_GEN_RCV_NO_BUFFER:
		case OID_802_3_RCV_ERROR_ALIGNMENT:
		case OID_802_3_XMIT_ONE_COLLISION:
		case OID_802_3_XMIT_MORE_COLLISIONS:
			*ResponseSize = sizeof(uint32_t);
			/* Unused statistic OIDs - always return 0 for each */
			*((uint32_t*)ResponseData) = 0;
			return 1;
		case OID_GEN_MAXIMUM_TOTAL_SIZE:
			*ResponseSize = sizeof(uint32_t);

			/* Indicate maximum overall buffer (Ethernet frame and RNDIS header) the adapter can handle */
			*((uint32_t*)ResponseData) = RNDIS_MESSAGE_BUFFER_SIZE + ETHERNET_FRAME_SIZE_MAX;

			return 1;
		default:
			return 0;
	}
}


static uint8_t RNDIS_Device_ProcessNDISSet(uint8_t* const Buf,
                                        const uint32_t OId,
                                        const void* SetData,
                                        const uint16_t SetSize)
{
	(void)SetSize;
	switch (OId)
	{
		case OID_GEN_CURRENT_PACKET_FILTER:
			CurrPacketFilter = *((uint32_t*)SetData);
			CurrRNDISState   = CurrPacketFilter ? RNDIS_Data_Initialized : RNDIS_Initialized;
			return 1;
		case OID_802_3_MULTICAST_LIST:
			/* Do nothing - throw away the value from the host as it is unused */
			return 1;
		default:
			return 0;
	}
}


/**
 * @brief  RNDIS_DataTx
 *         CDC received data to be send over USB IN endpoint are managed in
 *         this function.
 * @param  Buf: Buffer of data to be sent
 * @param  Len: Number of data to be sent (in bytes)
 * @retval Result of the opeartion: USBD_OK if all operations are OK else VCP_FAIL
 */
static uint16_t RNDIS_DataTx(uint8_t* Buf, uint32_t Len) {
	uint32_t i = 0;

	while (i < Len) {
		APP_Rx_Buffer[APP_Rx_ptr_in] = *(Buf + i);
		APP_Rx_ptr_in++;
		i++;
		/* To avoid buffer overflow */
		if (APP_Rx_ptr_in == APP_RX_DATA_SIZE) {
			APP_Rx_ptr_in = 0;
		}
	}

	return USBD_OK;
}

/**
 * @brief  RNDIS_DataRx
 *         Data received over USB OUT endpoint are sent over CDC interface
 *         through this function.
 *
 *         @note
 *         This function will block any OUT packet reception on USB endpoint
 *         until exiting this function. If you exit this function before transfer
 *         is complete on CDC interface (ie. using DMA controller) it will result
 *         in receiving more data while previous ones are still not sent.
 *
 * @param  Buf: Buffer of data to be received
 * @param  Len: Number of data received (in bytes)
 * @retval Result of the opeartion: USBD_OK if all operations are OK else VCP_FAIL
 */

#define APP_TX_BUF_SIZE 128
uint8_t APP_Tx_Buffer[APP_TX_BUF_SIZE];
uint32_t APP_tx_ptr_head;
uint32_t APP_tx_ptr_tail;

static uint16_t RNDIS_DataRx(uint8_t* Buf, uint32_t Len) {
	uint32_t i;
	RNDIS_Packet_Message_t* packet = (RNDIS_Packet_Message_t*)Buf; 
	Ethernet_Frame_Header_t* header = (Ethernet_Frame_Header_t*)&Buf[sizeof(RNDIS_Packet_Message_t)];

	for (i = 0; i < Len; i++) {
		APP_Tx_Buffer[APP_tx_ptr_head] = *(Buf + i);
		APP_tx_ptr_head++;
		if (APP_tx_ptr_head == APP_TX_BUF_SIZE)
			APP_tx_ptr_head = 0;

		if (APP_tx_ptr_head == APP_tx_ptr_tail)
			return USBD_FAIL;
	}


	return USBD_OK;
}

/*
 *
 * @param  None.
 * @retval None.
 */
void EVAL_COM_IRQHandler(void) {

}

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
