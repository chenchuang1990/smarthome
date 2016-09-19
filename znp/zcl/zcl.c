#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "zcl.h"
#include "commands.h"
#include "mtAf.h"
#include "mtParser.h"
#include "zcl_ss.h"
#include "zcl_general.h"
#include "gateway.h"
#include "bytebuffer.h"
#include "sqlitedb.h"
#include "zcl_down_cmd.h"
#include "zcl_datatype.h"
#include "zcl_ha.h"
#include "toolkit.h"


#define SEC_KEY_LEN 16 // ???
void *zcl_mem_alloc( uint16 size ){
	return malloc(size);
};

void *zcl_memset( void *dest, uint8 value, int len ){
	return memset(dest, value, len);
};

void *zcl_memcpy( void *dst, void *src, unsigned int len ){
	return memcpy(dst, src, len);
};

void zcl_mem_free(void *ptr){
	free(ptr);
};

uint8* zcl_buffer_uint32( uint8 *buf, uint32 val ){ 
	return memcpy(buf, ((uint8 *)(&val)), 4);
};
int zcl_handle_basic(unsigned char * buf, unsigned short buflen, struct device * d); 

static zclAttrRecsList *attrList = (zclAttrRecsList *)NULL;

/*********************************************************************
 * @fn          zcl_registerAttrList
 *
 * @brief       Register an Attribute List with ZCL Foundation
 *
 * @param       endpoint - endpoint the attribute list belongs to
 * @param       numAttr - number of attributes in list
 * @param       newAttrList - array of Attribute records.
 *                            NOTE: THE ATTRIBUTE IDs (FOR A CLUSTER) MUST BE IN
 *                            ASCENDING ORDER. OTHERWISE, THE DISCOVERY RESPONSE
 *                            COMMAND WILL NOT HAVE THE RIGHT ATTRIBUTE INFO
 *
 * @return      ZSuccess if OK
 */
ZStatus_t zcl_registerAttrList( uint8 endpoint, uint8 numAttr, const zclAttrRec_t newAttrList[] )
{
	zclAttrRecsList *pNewItem;
	zclAttrRecsList *pLoop;

	// Fill in the new profile list
	pNewItem = zcl_mem_alloc( sizeof( zclAttrRecsList ) );
	if ( pNewItem == NULL )
	{
		return (ZMemError);
	}

	pNewItem->next = (zclAttrRecsList *)NULL;
	pNewItem->endpoint = endpoint;
	pNewItem->pfnReadWriteCB = NULL;
	pNewItem->numAttributes = numAttr;
	pNewItem->attrs = newAttrList;

	// Find spot in list
	if ( attrList == NULL )
	{
		attrList = pNewItem;
	}
	else
	{
		// Look for end of list
		pLoop = attrList;
		while ( pLoop->next != NULL )
		{
			pLoop = pLoop->next;
		}

		// Put new item at end of list
		pLoop->next = pNewItem;
	}

	return ( ZSuccess );
}

/*********************************************************************
 * @fn      zclCalcHdrSize
 *
 * @brief   Calculate the number of bytes needed for an outgoing
 *          ZCL header.
 *
 * @param   hdr - outgoing header information
 *
 * @return  returns the number of bytes needed
 */
static uint8 zclCalcHdrSize( struct zclframehdr *hdr ) {
	uint8 needed = (1 + 1 + 1); // frame control + transaction seq num + cmd ID

	// Add the manfacturer code
	if ( hdr->control.manuspecific )
	{
		needed += 2;
	}

	return ( needed );
}



/*********************************************************************
 * @fn      zclBuildHdr
 *
 * @brief   Build header of the ZCL format
 *
 * @param   hdr - outgoing header information
 * @param   pData - outgoing header space
 *
 * @return  pointer past the header
 */
static uint8 *zclBuildHdr( struct zclframehdr *hdr, uint8 *pData )
{
	// Build the Frame Control byte
	*pData = hdr->control.type;
	*pData |= hdr->control.manuspecific << 2;
	*pData |= hdr->control.direction << 3;
	*pData |= hdr->control.disabledefaultrsp << 4;
	pData++;  // move past the frame control field

	// Add the manfacturer code
	if ( hdr->control.manuspecific )
	{
		*pData++ = LO_UINT16( hdr->manucode );
		*pData++ = HI_UINT16( hdr->manucode );
	}

	// Add the Transaction Sequence Number
	*pData++ = hdr->transseqnum;

	// Add the Cluster's command ID
	*pData++ = hdr->commandid;

	// Should point to the frame payload
	return ( pData );
}
/*********************************************************************
 * @fn      zcl_sendcommand
 *
 * @brief   Used to send Profile and Cluster Specific Command messages.
 *
 *          NOTE: The calling application is responsible for incrementing
 *                the Sequence Number.
 *
 * @param   srcEp - source endpoint
 * @param   destAddr - destination address
 * @param   clusterID - cluster ID
 * @param   cmd - command ID
 * @param   specific - whether the command is Cluster Specific
 * @param   direction - client/server direction of the command
 * @param   disableDefaultRsp - disable Default Response command
 * @param   manuCode - manufacturer code for proprietary extensions to a profile
 * @param   seqNumber - identification number for the transaction
 * @param   cmdFormatLen - length of the command to be sent
 * @param   cmdFormat - command to be sent
 *
 * @return  ZSuccess if OK
 */
ZStatus_t zcl_sendcommand( uint8 srcEP, uint8 dstEp, uint16 dstaddr,
		uint16 clusterID, uint8 cmd, uint8 specific, uint8 direction,
		uint8 disableDefaultRsp, uint16 manuCode, uint8 seqNum,
		uint16 cmdFormatLen, uint8 *cmdFormat )
{
	//endPointDesc_t *epDesc;
	struct zclframehdr hdr;
	unsigned char * msgBuf;
	uint16 msgLen;
	uint8 *pBuf;
	//uint8 options;
	ZStatus_t status;

	//  epDesc = afFindEndPointDesc( srcEP );
	//  if ( epDesc == NULL )
	//  {
	//    return ( ZInvalidParameter ); // EMBEDDED RETURN
	//  }
	//
	//#if defined ( INTER_PAN )
	//  if ( StubAPS_InterPan( destAddr->panId, destAddr->endPoint ) )
	//  {
	//    options = AF_TX_OPTIONS_NONE;
	//  }
	//  else
	//#endif
	//  {
	//    options = zclGetClusterOption( srcEP, clusterID );
	//
	//    // The cluster might not have been defined to use security but if this message
	//    // is in response to another message that was using APS security this message
	//    // will be sent with APS security
	//    if ( !( options & AF_EN_SECURITY ) )
	//    {
	//      afIncomingMSGPacket_t *origPkt = zcl_getRawAFMsg();
	//
	//      if ( ( origPkt != NULL ) && ( origPkt->SecurityUse == TRUE ) )
	//      {
	//        options |= AF_EN_SECURITY;
	//      }
	//    }
	//  }

	memset( &hdr, 0, sizeof( struct zclframehdr ) );

	// Not Profile wide command (like READ, WRITE)
	if ( specific )
	{
		hdr.control.type = ZCL_FRAME_TYPE_SPECIFIC_CMD;
	}
	else
	{
		hdr.control.type = ZCL_FRAME_TYPE_PROFILE_CMD;
	}

	//  if ( ( epDesc->simpleDesc == NULL ) ||
	//       ( zcl_DeviceOperational( srcEP, clusterID, hdr.fc.type,
	//                                cmd, epDesc->simpleDesc->AppProfId ) == FALSE ) )
	//  {
	//    return ( ZFailure ); // EMBEDDED RETURN
	//  }

	// Fill in the Maufacturer Code
	if ( manuCode != 0 )
	{
		hdr.control.manuspecific = 1;
		hdr.manucode = manuCode;
	}

	// Set the Command Direction
	if ( direction )
	{
		hdr.control.direction = ZCL_FRAME_SERVER_CLIENT_DIR;
	}
	else
	{
		hdr.control.direction = ZCL_FRAME_CLIENT_SERVER_DIR;
	}

	// Set the Disable Default Response field
	if ( disableDefaultRsp )
	{
		hdr.control.disabledefaultrsp = 1;
	}
	else
	{
		hdr.control.disabledefaultrsp = 0;
	}

	// Fill in the Transaction Sequence Number
	hdr.transseqnum = seqNum;

	// Fill in the command
	hdr.commandid = cmd;

	// calculate the needed buffer size
	msgLen = zclCalcHdrSize( &hdr );
	msgLen += cmdFormatLen;

	// Allocate the buffer needed
	msgBuf = (unsigned char *)malloc( msgLen );
	memset(msgBuf, 0, msgLen);
	if ( msgBuf != NULL )
	{
		// Fill in the ZCL Header
		pBuf = zclBuildHdr( &hdr, msgBuf );

		// Fill in the command frame
		memcpy( pBuf, cmdFormat, cmdFormatLen );

		DataRequestFormat_t req;
		memset(&req, 0, sizeof(DataRequestFormat_t));
		req.DstAddr = dstaddr;
		req.DstEndpoint = dstEp;
		req.SrcEndpoint = srcEP;
		req.ClusterID = clusterID;
		//	uint8_t TransID;
		//	uint8_t Options;
		//	uint8_t Radius;     
		req.Len = msgLen;
		memcpy(req.Data, msgBuf, msgLen);
		free(msgBuf);
		sendcmd((unsigned char *)&req, AF_DATA_REQUEST);

	}
	else
	{
		status = ZMemError;
	}

	return ( status );
}


ZStatus_t zcl_SendCommand( uint8 srcEP, afAddrType_t *destAddr,
                           uint16 clusterID, uint8 cmd, uint8 specific, uint8 direction,
                           uint8 disableDefaultRsp, uint16 manuCode, uint8 seqNum,
                           uint16 cmdFormatLen, uint8 *cmdFormat ){
                           
	return zcl_sendcommand(srcEP, destAddr->endPoint, destAddr->addr.shortAddr,clusterID, cmd, specific, direction, disableDefaultRsp, manuCode, seqNum, cmdFormatLen, cmdFormat);
}

/*********************************************************************
 * @fn      zclGetDataTypeLength
 *
 * @brief   Return the length of the datatype in octet.
 *
 *          NOTE: Should not be called for ZCL_DATATYPE_OCTECT_STR or
 *                ZCL_DATATYPE_CHAR_STR data types.
 *
 * @param   dataType - data type
 *
 * @return  length of data
 */
uint8 zclGetDataTypeLength( uint8 dataType )
{
	uint8 len;

	switch ( dataType )
	{
		case ZCL_DATATYPE_DATA8:
		case ZCL_DATATYPE_BOOLEAN:
		case ZCL_DATATYPE_BITMAP8:
		case ZCL_DATATYPE_INT8:
		case ZCL_DATATYPE_UINT8:
		case ZCL_DATATYPE_ENUM8:
			len = 1;
			break;

		case ZCL_DATATYPE_DATA16:
		case ZCL_DATATYPE_BITMAP16:
		case ZCL_DATATYPE_UINT16:
		case ZCL_DATATYPE_INT16:
		case ZCL_DATATYPE_ENUM16:
		case ZCL_DATATYPE_SEMI_PREC:
		case ZCL_DATATYPE_CLUSTER_ID:
		case ZCL_DATATYPE_ATTR_ID:
			len = 2;
			break;

		case ZCL_DATATYPE_DATA24:
		case ZCL_DATATYPE_BITMAP24:
		case ZCL_DATATYPE_UINT24:
		case ZCL_DATATYPE_INT24:
			len = 3;
			break;

		case ZCL_DATATYPE_DATA32:
		case ZCL_DATATYPE_BITMAP32:
		case ZCL_DATATYPE_UINT32:
		case ZCL_DATATYPE_INT32:
		case ZCL_DATATYPE_SINGLE_PREC:
		case ZCL_DATATYPE_TOD:
		case ZCL_DATATYPE_DATE:
		case ZCL_DATATYPE_UTC:
		case ZCL_DATATYPE_BAC_OID:
			len = 4;
			break;

		case ZCL_DATATYPE_UINT40:
		case ZCL_DATATYPE_INT40:
			len = 5;
			break;

		case ZCL_DATATYPE_UINT48:
		case ZCL_DATATYPE_INT48:
			len = 6;
			break;

		case ZCL_DATATYPE_UINT56:
		case ZCL_DATATYPE_INT56:
			len = 7;
			break;

		case ZCL_DATATYPE_DOUBLE_PREC:
		case ZCL_DATATYPE_IEEE_ADDR:
		case ZCL_DATATYPE_UINT64:
		case ZCL_DATATYPE_INT64:
			len = 8;
			break;

		case ZCL_DATATYPE_128_BIT_SEC_KEY:
			len = SEC_KEY_LEN;
			break;

		case ZCL_DATATYPE_NO_DATA:
		case ZCL_DATATYPE_UNKNOWN:
			// Fall through

		default:
			len = 0;
			break;
	}

	return ( len );
}

/*********************************************************************
 * @fn      zclGetAttrDataLength
 *
 * @brief   Return the length of the attribute.
 *
 * @param   dataType - data type
 * @param   pData - pointer to data
 *
 * @return  returns atrribute length
 */
uint16 zclGetAttrDataLength( uint8 dataType, uint8 *pData )
{
	uint16 dataLen = 0;

	if ( dataType == ZCL_DATATYPE_LONG_CHAR_STR || dataType == ZCL_DATATYPE_LONG_OCTET_STR )
	{
		dataLen = BUILD_UINT16( pData[0], pData[1] ) + 2; // long string length + 2 for length field
	}
	else if ( dataType == ZCL_DATATYPE_CHAR_STR || dataType == ZCL_DATATYPE_OCTET_STR )
	{
		dataLen = *pData + 1; // string length + 1 for length field
	}
	else
	{
		dataLen = zclGetDataTypeLength( dataType );
	}

	return ( dataLen );
}

/*********************************************************************
 * @fn      zclAnalogDataType
 *
 * @brief   Checks to see if Data Type is Analog
 *
 * @param   dataType - data type
 *
 * @return  TRUE if data type is analog
 */
uint8 zclAnalogDataType( uint8 dataType )
{
  uint8 analog;

  switch ( dataType )
  {
    case ZCL_DATATYPE_UINT8:
    case ZCL_DATATYPE_UINT16:
    case ZCL_DATATYPE_UINT24:
    case ZCL_DATATYPE_UINT32:
    case ZCL_DATATYPE_UINT40:
    case ZCL_DATATYPE_UINT48:
    case ZCL_DATATYPE_UINT56:
    case ZCL_DATATYPE_UINT64:
    case ZCL_DATATYPE_INT8:
    case ZCL_DATATYPE_INT16:
    case ZCL_DATATYPE_INT24:
    case ZCL_DATATYPE_INT32:
    case ZCL_DATATYPE_INT40:
    case ZCL_DATATYPE_INT48:
    case ZCL_DATATYPE_INT56:
    case ZCL_DATATYPE_INT64:
    case ZCL_DATATYPE_SEMI_PREC:
    case ZCL_DATATYPE_SINGLE_PREC:
    case ZCL_DATATYPE_DOUBLE_PREC:
    case ZCL_DATATYPE_TOD:
    case ZCL_DATATYPE_DATE:
    case ZCL_DATATYPE_UTC:
      analog = TRUE;
      break;

    default:
      analog = FALSE;
      break;
  }

  return ( analog );
}


/*********************************************************************
 * @fn      zclSerializeData
 *
 * @brief   Builds a buffer from the attribute data to sent out over
 *          the air.
 *          NOTE - Not compatible with application's attributes callbacks.
 *
 * @param   dataType - data types defined in zcl.h
 * @param   attrData - pointer to the attribute data
 * @param   buf - where to put the serialized data
 *
 * @return  pointer to end of destination buffer
 */
uint8 *zclSerializeData( uint8 dataType, void *attrData, uint8 *buf )
{
	uint8 *pStr;
	uint16 len;

	if ( attrData == NULL )
	{
		return ( buf );
	}

	switch ( dataType )
	{
		case ZCL_DATATYPE_DATA8:
		case ZCL_DATATYPE_BOOLEAN:
		case ZCL_DATATYPE_BITMAP8:
		case ZCL_DATATYPE_INT8:
		case ZCL_DATATYPE_UINT8:
		case ZCL_DATATYPE_ENUM8:
			*buf++ = *((uint8 *)attrData);
			break;

		case ZCL_DATATYPE_DATA16:
		case ZCL_DATATYPE_BITMAP16:
		case ZCL_DATATYPE_UINT16:
		case ZCL_DATATYPE_INT16:
		case ZCL_DATATYPE_ENUM16:
		case ZCL_DATATYPE_SEMI_PREC:
		case ZCL_DATATYPE_CLUSTER_ID:
		case ZCL_DATATYPE_ATTR_ID:
			*buf++ = LO_UINT16( *((uint16*)attrData) );
			*buf++ = HI_UINT16( *((uint16*)attrData) );
			break;

		case ZCL_DATATYPE_DATA24:
		case ZCL_DATATYPE_BITMAP24:
		case ZCL_DATATYPE_UINT24:
		case ZCL_DATATYPE_INT24:
			*buf++ = BREAK_UINT32( *((uint32*)attrData), 0 );
			*buf++ = BREAK_UINT32( *((uint32*)attrData), 1 );
			*buf++ = BREAK_UINT32( *((uint32*)attrData), 2 );
			break;

		case ZCL_DATATYPE_DATA32:
		case ZCL_DATATYPE_BITMAP32:
		case ZCL_DATATYPE_UINT32:
		case ZCL_DATATYPE_INT32:
		case ZCL_DATATYPE_SINGLE_PREC:
		case ZCL_DATATYPE_TOD:
		case ZCL_DATATYPE_DATE:
		case ZCL_DATATYPE_UTC:
		case ZCL_DATATYPE_BAC_OID:
			buf = zcl_buffer_uint32( buf, *((uint32*)attrData) );
			break;

		case ZCL_DATATYPE_UINT40:
		case ZCL_DATATYPE_INT40:
			pStr = (uint8*)attrData;
			buf = zcl_memcpy( buf, pStr, 5 );
			break;

		case ZCL_DATATYPE_UINT48:
		case ZCL_DATATYPE_INT48:
			pStr = (uint8*)attrData;
			buf = zcl_memcpy( buf, pStr, 6 );
			break;

		case ZCL_DATATYPE_UINT56:
		case ZCL_DATATYPE_INT56:
			pStr = (uint8*)attrData;
			buf = zcl_memcpy( buf, pStr, 7 );
			break;

		case ZCL_DATATYPE_DOUBLE_PREC:
		case ZCL_DATATYPE_IEEE_ADDR:
		case ZCL_DATATYPE_UINT64:
		case ZCL_DATATYPE_INT64:
			pStr = (uint8*)attrData;
			buf = zcl_memcpy( buf, pStr, 8 );
			break;

		case ZCL_DATATYPE_CHAR_STR:
		case ZCL_DATATYPE_OCTET_STR:
			pStr = (uint8*)attrData;
			len = *pStr;
			buf = zcl_memcpy( buf, pStr, len+1 ); // Including length field
			break;

		case ZCL_DATATYPE_LONG_CHAR_STR:
		case ZCL_DATATYPE_LONG_OCTET_STR:
			pStr = (uint8*)attrData;
			len = BUILD_UINT16( pStr[0], pStr[1] );
			buf = zcl_memcpy( buf, pStr, len+2 ); // Including length field
			break;

		case ZCL_DATATYPE_128_BIT_SEC_KEY:
			pStr = (uint8*)attrData;
			buf = zcl_memcpy( buf, pStr, SEC_KEY_LEN );
			break;

		case ZCL_DATATYPE_NO_DATA:
		case ZCL_DATATYPE_UNKNOWN:
			// Fall through

		default:
			break;
	}

	return ( buf );
}
/*********************************************************************
 * @fn      zcl_SendRead
 *
 * @brief   Send a Read command
 *
 * @param   srcEP - Application's endpoint
 * @param   dstAddr - destination address
 * @param   clusterID - cluster ID
 * @param   readCmd - read command to be sent
 * @param   direction - direction of the command
 * @param   seqNum - transaction sequence number
 *
 * @return  ZSuccess if OK
 */
ZStatus_t zcl_SendRead( uint8 srcEP, uint8 dstEp, uint16 dstAddr,
		uint16 clusterID, zclReadCmd_t *readCmd,
		uint8 direction, uint8 disableDefaultRsp, uint8 seqNum)
{
	uint16 dataLen;
	uint8 *buf;
	uint8 *pBuf;
	ZStatus_t status;

	dataLen = readCmd->numAttr * 2; // Attribute ID

	buf = (uint8 *)malloc( dataLen );
	if ( buf != NULL )
	{
		uint8 i;

		// Load the buffer - serially
		pBuf = buf;
		for (i = 0; i < readCmd->numAttr; i++)
		{
			*pBuf++ = LO_UINT16( readCmd->attrID[i] );
			*pBuf++ = HI_UINT16( readCmd->attrID[i] );
		}

		status = zcl_sendcommand( srcEP, dstEp, dstAddr, clusterID, ZCL_CMD_READ, FALSE,
				direction, disableDefaultRsp, 0, seqNum, dataLen, buf );
		free( buf );
	}
	else
	{
		status = ZMemError;
	}

	return ( status );
}

/*********************************************************************
 * @fn      zclFindAttrRecsList
 *
 * @brief   Find the right attribute record list for an endpoint
 *
 * @param   clusterID - endpointto look for
 *
 * @return  pointer to record list, NULL if not found
 */
static zclAttrRecsList *zclFindAttrRecsList( uint8 endpoint )
{
	zclAttrRecsList *pLoop = attrList;

	while ( pLoop != NULL )
	{
		if ( pLoop->endpoint == endpoint )
		{
			return ( pLoop );
		}

		pLoop = pLoop->next;
	}

	return ( NULL );
}

/*********************************************************************
 * @fn      zclGetReadWriteCB
 *
 * @brief   Get the Read/Write callback function pointer for a given endpoint.
 *
 * @param   endpoint - Application's endpoint
 *
 * @return  Read/Write CB, NULL if not found
 */
static zclReadWriteCB_t zclGetReadWriteCB( uint8 endpoint )
{
	zclAttrRecsList *pRec = zclFindAttrRecsList( endpoint );

	if ( pRec != NULL )
	{
		return ( pRec->pfnReadWriteCB );
	}

	return ( NULL );
}

/*********************************************************************
 * @fn      zclReadAttrDataUsingCB
 *
 * @brief   Use application's callback to read the attribute's current
 *          value stored in the database.
 *
 * @param   endpoint - application's endpoint
 * @param   clusterId - cluster that attribute belongs to
 * @param   attrId - attribute id
 * @param   pAttrData - where to put attribute data
 * @param   pDataLen - where to put attribute data length
 *
 * @return  Successful if data was read
 */
static ZStatus_t zclReadAttrDataUsingCB( uint8 endpoint, uint16 clusterId, uint16 attrId,
                                         uint8 *pAttrData, uint16 *pDataLen )
{
  zclReadWriteCB_t pfnReadWriteCB = zclGetReadWriteCB( endpoint );

  if ( pDataLen != NULL )
  {
    *pDataLen = 0; // Always initialize it to 0
  }

  if ( pfnReadWriteCB != NULL )
  {
    // Read the attribute value and its length
    return ( (*pfnReadWriteCB)( clusterId, attrId, ZCL_OPER_READ, pAttrData, pDataLen ) );
  }

  return ( ZCL_STATUS_SOFTWARE_FAILURE );
}

/*********************************************************************
 * @fn      zclGetAttrDataLengthUsingCB
 *
 * @brief   Use application's callback to get the length of the attribute's
 *          current value stored in the database.
 *
 * @param   endpoint - application's endpoint
 * @param   clusterId - cluster that attribute belongs to
 * @param   attrId - attribute id
 *
 * @return  returns attribute length
 */
static uint16 zclGetAttrDataLengthUsingCB( uint8 endpoint, uint16 clusterId, uint16 attrId )
{
	uint16 dataLen = 0;
	zclReadWriteCB_t pfnReadWriteCB = zclGetReadWriteCB( endpoint );

	if ( pfnReadWriteCB != NULL )
	{
		// Only get the attribute length
		(*pfnReadWriteCB)( clusterId, attrId, ZCL_OPER_LEN, NULL, &dataLen );
	}

	return ( dataLen );
}

/*********************************************************************
 * @fn      zcl_SendReadRsp
 *
 * @brief   Send a Read Response command.
 *
 * @param   srcEP - Application's endpoint
 * @param   dstAddr - destination address
 * @param   clusterID - cluster ID
 * @param   readRspCmd - read response command to be sent
 * @param   direction - direction of the command
 * @param   seqNum - transaction sequence number
 *
 * @return  ZSuccess if OK
 */
ZStatus_t zcl_SendReadRsp( uint8 srcEP, uint8 dstEp, uint16 dstAddr,
		uint16 clusterID, zclReadRspCmd_t *readRspCmd,
		uint8 direction, uint8 disableDefaultRsp, uint8 seqNum )
{
	uint8 *buf;
	uint16 len = 0;
	ZStatus_t status;
	uint8 i;

	// calculate the size of the command
	for ( i = 0; i < readRspCmd->numAttr; i++ )
	{
		zclReadRspStatus_t *statusRec = &(readRspCmd->attrList[i]);

		len += 2 + 1; // Attribute ID + Status

		if ( statusRec->status == ZCL_STATUS_SUCCESS )
		{
			len++; // Attribute Data Type length

			// Attribute Data length
			if ( statusRec->data != NULL )
			{
				len += zclGetAttrDataLength( statusRec->dataType, statusRec->data );
			}
			else
			{

				len += zclGetAttrDataLengthUsingCB( srcEP, clusterID, statusRec->attrID );
			}
		}
	}

	buf = (uint8 *)malloc( len );
	if ( buf != NULL )
	{
		// Load the buffer - serially
		uint8 *pBuf = buf;

		for ( i = 0; i < readRspCmd->numAttr; i++ )
		{
			zclReadRspStatus_t *statusRec = &(readRspCmd->attrList[i]);

			*pBuf++ = LO_UINT16( statusRec->attrID );
			*pBuf++ = HI_UINT16( statusRec->attrID );
			*pBuf++ = statusRec->status;

			if ( statusRec->status == ZCL_STATUS_SUCCESS )
			{
				*pBuf++ = statusRec->dataType;

				if ( statusRec->data != NULL )
				{
					// Copy attribute data to the buffer to be sent out
					pBuf = zclSerializeData( statusRec->dataType, statusRec->data, pBuf );
				}
				else
				{
					uint16 dataLen;

					// Read attribute data directly into the buffer to be sent out
					zclReadAttrDataUsingCB( srcEP, clusterID, statusRec->attrID, pBuf, &dataLen );
					pBuf += dataLen;
				}
			}
		} // for loop

		status = zcl_sendcommand( srcEP, dstEp, dstAddr, clusterID, ZCL_CMD_READ_RSP, FALSE,
				direction, disableDefaultRsp, 0, seqNum, len, buf );
		zcl_mem_free( buf );
	}
	else
	{
		status = ZMemError;
	}

	return ( status );
}

unsigned char * zcl_parsehdr(struct zclframehdr * framehdr,unsigned char * data){ 
	memcpy(&framehdr->control, data, 1); 
	data++;
	if(framehdr->control.manuspecific){
		framehdr->manucode = BUILD_UINT16(data[0],data[1]);
		data+=2;
	}
	framehdr->transseqnum = *data++;
	framehdr->commandid = *data++;

	return data;
}


/*add the code that report fucking devicename*/

#define min(a,b) a>b?b:a

enum dn {
	SWITCH,
	SOCKET,
	CONTACT,
	MOTION,
	GAS,
	FIRE,
	KEY,
	WARN,
	SHADE,
	BELL,
	UNKNOWN
};

extern char utf8_table[][32];

void handle_outlet_devicename(struct device *d)
{
	struct endpoint *ep;
	char name[MAXNAMELEN] = {0};
	int slen = 0;

	ep = list_entry(d->eplisthead.next, struct endpoint, list); 
	if(ep && (ZCL_HA_DEVICEID_MAINS_POWER_OUTLET == ep->simpledesc.simpledesc.DeviceID)) {
		if(!strncasecmp(d->modelidentifier, "Z809", 4)) {
			snprintf(name, sizeof(name), "%s", utf8_table[SOCKET]);
		}
		else if('F' == d->modelidentifier[0]) {
			int i;
			char *substrp[2];
			char *out_ptr = NULL;
			char sa[33] = {0};
			char *str = sa;			
			slen = strlen(d->modelidentifier);
			memcpy(str, d->modelidentifier, min(slen, 33));
			str[32] = 0;
			
			for(i = 0; i < 2; str = NULL, i++) {
				substrp[i] = strtok_r(str, "-", &out_ptr);
				if(NULL == substrp[i])
					break;
				printf("--> %s\n", substrp[i]);
			}
			if(!strncasecmp(substrp[1], "SKT", 3))
				snprintf(name, sizeof(name), "%s", utf8_table[SOCKET]);
		}
		else
			snprintf(name, sizeof(name), "%d%s", d->activeep.ActiveEPCount, utf8_table[SWITCH]);
		
		
		slen = strlen(name); 
		memcpy(d->devicename, name, min(slen, MAXNAMELEN-1));
		//toolkit_printbytes((unsigned char *)d->devicename, slen);
		d->devicename[MAXNAMELEN-1] = 0;
		sqlitedb_update_devicename(d->ieeeaddr, d->devicename);
	}
}

int zcl_proccessincomingmessage(IncomingMsgFormat_t * message){ 
	int result = -1;
	if(message->Len == 0){
		return 1;
	}

	struct zclincomingmsg zclmessage;
	memset(&zclmessage, 0, sizeof(struct zclincomingmsg));
	zclmessage.message = message;
	unsigned char * zclpayload = zcl_parsehdr(&zclmessage.zclframehdr, message->Data);
	zclmessage.data = zclpayload;
	zclmessage.datalen = message->Len;
	zclmessage.datalen -= zclmessage.data - message->Data;

	// to get attribute 
	struct device * d = gateway_getdevice_shortaddr(message->SrcAddr);
	if(!d) {
		printf("no such device\n");
		return -1;
	}
	#if 0
	if(d && !device_check_status(d, DEVICE_SEND_ATTR)){ 
		zclReadCmd_t readcmd; 
		readcmd.numAttr = 8;
		readcmd.attrID[0] = ATTRID_BASIC_ZCL_VERSION;
		readcmd.attrID[1] = ATTRID_BASIC_APPL_VERSION;
		readcmd.attrID[2] = ATTRID_BASIC_STACK_VERSION;
		readcmd.attrID[3] = ATTRID_BASIC_HW_VERSION;
		readcmd.attrID[4] = ATTRID_BASIC_MANUFACTURER_NAME;
		readcmd.attrID[5] = ATTRID_BASIC_MODEL_ID;
		readcmd.attrID[6] = ATTRID_BASIC_DATE_CODE;         
		readcmd.attrID[7] = ATTRID_BASIC_POWER_SOURCE;

		//zcl_SendRead(message->DstEndpoint, message->SrcEndpoint, message->SrcAddr, ZCL_CLUSTER_ID_GEN_BASIC, &readcmd, ZCL_CLUSTER_ID_GEN_BASIC,0,0);
		printf("zcl_proccessincomingmessage:send read attribute cmd\n");
		zcl_SendRead(message->DstEndpoint, message->SrcEndpoint, message->SrcAddr, ZCL_CLUSTER_ID_GEN_BASIC, &readcmd, ZCL_CLUSTER_ID_GEN_BASIC,0,get_sequence());
		device_set_status(d, DEVICE_SEND_ATTR);
	}
	#endif

	d->timestamp = time(NULL);
		
	switch(zclmessage.message->ClusterId) { 
		case ZCL_CLUSTER_ID_SS_IAS_ZONE: 
			result = zclss_handleincoming(&zclmessage);
			break;
		case ZCL_CLUSTER_ID_GEN_BASIC:
			zcl_handle_basic(zclmessage.data,zclmessage.datalen, d);
			if(0 == strlen(d->devicename)) {
				handle_outlet_devicename(d);
			}
			sqlitedb_update_device_attr(d);
			//printf("receive basic req response\n");
			//printf("data[0]:%02x, data[1]:%02x\n", zclmessage.data[0], zclmessage.data[1]);
			/*if((0 == zclmessage.data[0]) && (0 == zclmessage.data[1]))
				handle_basic_status(&zclmessage);*/
				//report_basic_status(&zclmessage);
			break;
		case ZCL_CLUSTER_ID_GEN_ON_OFF:
			handle_onoff_state(&zclmessage);
			break;
		case ZCL_CLUSTER_ID_GEN_LEVEL_CONTROL:
			handle_levelctl_state(&zclmessage);
			//handle_levelctr_rsp(&zclmessage);
			break;
		case ZCL_CLUSTER_ID_SS_IAS_WD:
			//handle_warning_rsp(&zclmessage);
			break;
		default:
			//zclss_handle_default(&zclmessage);
			break;
	}

	return result;
}

// ------------------------------ZCL_WRITE---------------------------
/*********************************************************************
 * @fn      sendWriteRequest
 *
 * @brief   Send a Write command
 *
 * @param   dstAddr - destination address
 * @param   clusterID - cluster ID
 * @param   writeCmd - write command to be sent
 * @param   cmd - ZCL_CMD_WRITE, ZCL_CMD_WRITE_UNDIVIDED or ZCL_CMD_WRITE_NO_RSP
 * @param   direction - direction of the command
 * @param   seqNum - transaction sequence number
 *
 * @return  ZSuccess if OK
 */
ZStatus_t zcl_SendWriteRequest( uint8 srcEP, afAddrType_t *dstAddr, uint16 clusterID,
                                zclWriteCmd_t *writeCmd, uint8 cmd, uint8 direction,
                                uint8 disableDefaultRsp, uint8 seqNum )
{
  uint8 *buf;
  uint16 dataLen = 0;
  ZStatus_t status;
  uint8 i;

  for ( i = 0; i < writeCmd->numAttr; i++ )
  {
    zclWriteRec_t *statusRec = &(writeCmd->attrList[i]);

    dataLen += 2 + 1; // Attribute ID + Attribute Type

    // Attribute Data
    dataLen += zclGetAttrDataLength( statusRec->dataType, statusRec->attrData );
  }

  buf = zcl_mem_alloc( dataLen );
  if ( buf != NULL )
  {
    // Load the buffer - serially
    uint8 *pBuf = buf;
    for ( i = 0; i < writeCmd->numAttr; i++ )
    {
      zclWriteRec_t *statusRec = &(writeCmd->attrList[i]);

      *pBuf++ = LO_UINT16( statusRec->attrID );
      *pBuf++ = HI_UINT16( statusRec->attrID );
      *pBuf++ = statusRec->dataType;

      pBuf = zclSerializeData( statusRec->dataType, statusRec->attrData, pBuf );
    }

    status = zcl_SendCommand( srcEP, dstAddr, clusterID, cmd, FALSE,
                              direction, disableDefaultRsp, 0, seqNum, dataLen, buf );
    zcl_mem_free( buf );
  }
  else
  {
    status = ZMemError;
  }

  return ( status);
}

/*********************************************************************
 * @fn      zcl_SendWriteRsp
 *
 * @brief   Send a Write Response command
 *
 * @param   dstAddr - destination address
 * @param   clusterID - cluster ID
 * @param   wrtieRspCmd - write response command to be sent
 * @param   direction - direction of the command
 * @param   seqNum - transaction sequence number
 *
 * @return  ZSuccess if OK
 */
ZStatus_t zcl_SendWriteRsp( uint8 srcEP, afAddrType_t *dstAddr,
                            uint16 clusterID, zclWriteRspCmd_t *writeRspCmd,
                            uint8 direction, uint8 disableDefaultRsp, uint8 seqNum )
{
  uint16 dataLen;
  uint8 *buf;
  ZStatus_t status;

  dataLen = writeRspCmd->numAttr * ( 1 + 2 ); // status + attribute id

  buf = zcl_mem_alloc( dataLen );
  if ( buf != NULL )
  {
    // Load the buffer - serially
    uint8 i;
    uint8 *pBuf = buf;
    for ( i = 0; i < writeRspCmd->numAttr; i++ )
    {
      *pBuf++ = writeRspCmd->attrList[i].status;
      *pBuf++ = LO_UINT16( writeRspCmd->attrList[i].attrID );
      *pBuf++ = HI_UINT16( writeRspCmd->attrList[i].attrID );
    }

    // If there's only a single status record and its status field is set to
    // SUCCESS then omit the attribute ID field.
    if ( writeRspCmd->numAttr == 1 && writeRspCmd->attrList[0].status == ZCL_STATUS_SUCCESS )
    {
      dataLen = 1;
    }

    status = zcl_SendCommand( srcEP, dstAddr, clusterID, ZCL_CMD_WRITE_RSP, FALSE,
                              direction, disableDefaultRsp, 0, seqNum, dataLen, buf );
    zcl_mem_free( buf );
  }
  else
  {
    status = ZMemError;
  }

  return ( status );
}

// ------------------------------ZCL_WRITE---------------------------
//
struct zcl_basic_attr{
	unsigned short attrid;
	unsigned char status;
	unsigned char datatype;
	union{
		unsigned char uint8_value;
		unsigned short uint16_value;
		unsigned int uint32_value;
		unsigned long long uint_value;
		unsigned char bytes[256];
	}value;
	
};

int zcl_handle_basic(unsigned char * buf, unsigned short buflen, struct device * d){ 
	struct zcl_basic_attr attr; 
	const unsigned char * cursor = buf;

	unsigned char datalen;
	
	while(cursor < buf+buflen){
		bytebuffer_readwordl(&cursor, &attr.attrid);
		bytebuffer_readbyte(&cursor, &attr.status);
		if(attr.status == ZCL_STATUS_SUCCESS){
			bytebuffer_readbyte(&cursor, &attr.datatype);

			switch(attr.attrid){
				case ATTRID_BASIC_ZCL_VERSION: 
					bytebuffer_readbyte(&cursor, &d->zclversion);
					break;
				case ATTRID_BASIC_APPL_VERSION:
					bytebuffer_readbyte(&cursor, &d->applicationversion);
					break;
				case ATTRID_BASIC_STACK_VERSION:
					bytebuffer_readbyte(&cursor, &d->stackversion);
					break;
				case ATTRID_BASIC_HW_VERSION:
					bytebuffer_readbyte(&cursor, &d->hwversion);
					break;
				case ATTRID_BASIC_MANUFACTURER_NAME: 
					bytebuffer_readbyte(&cursor, &datalen);
					bytebuffer_readbytes(&cursor, d->manufacturername, datalen);
					d->manufacturername[datalen] = 0;
					break;
				case ATTRID_BASIC_MODEL_ID:
					bytebuffer_readbyte(&cursor, &datalen);
					bytebuffer_readbytes(&cursor, d->modelidentifier, datalen);
					d->modelidentifier[datalen] = 0;
					break;
				case ATTRID_BASIC_DATE_CODE:
					bytebuffer_readbyte(&cursor, &datalen);
					bytebuffer_readbytes(&cursor, d->datecode, datalen);
					d->datecode[datalen] = 0;
					break;
				case ATTRID_BASIC_POWER_SOURCE:
					bytebuffer_readbyte(&cursor, &d->powersource);
					break;
			}
		}
	}


	return 0;
}
// ------------------------------ZCL_REPORT---------------------------

/*********************************************************************
 * @fn      zcl_SendConfigReportCmd
 *
 * @brief   Send a Configure Reporting command
 *
 * @param   dstAddr - destination address
 * @param   clusterID - cluster ID
 * @param   cfgReportCmd - configure reporting command to be sent
 * @param   direction - direction of the command
 * @param   seqNum - transaction sequence number
 *
 * @return  ZSuccess if OK
 */
ZStatus_t zcl_SendConfigReportCmd( uint8 srcEP, uint8 dstEp, uint16 dstAddr,
                          uint16 clusterID, zclCfgReportCmd_t *cfgReportCmd,
                          uint8 direction, uint8 disableDefaultRsp, uint8 seqNum )
{
  uint8 *buf;
  uint16 dataLen = 0;
  ZStatus_t status;
  uint8 i;

  // Find out the data length
  printf("numAttr is %d\n", cfgReportCmd->numAttr);
  for ( i = 0; i < cfgReportCmd->numAttr; i++ )
  {
    zclCfgReportRec_t *reportRec = &(cfgReportCmd->attrList[i]);

    dataLen += 1 + 2; // Direction + Attribute ID

    if ( reportRec->direction == ZCL_SEND_ATTR_REPORTS )
    {
    printf("ZCL_SEND_ATTR_REPORTS\n");
      dataLen += 1 + 2 + 2; // Data Type + Min + Max Reporting Intervals

      // Find out the size of the Reportable Change field (for Analog data types)
      if ( zclAnalogDataType( reportRec->dataType ) )
      {
        dataLen += zclGetDataTypeLength( reportRec->dataType );
      }
    }
    else
    {
      dataLen += 2; // Timeout Period
    }
  }

  buf = zcl_mem_alloc( dataLen );
  if ( buf != NULL )
  {
    // Load the buffer - serially
    printf("load buffer\n");
    uint8 *pBuf = buf;

    for ( i = 0; i < cfgReportCmd->numAttr; i++ )
    {
      zclCfgReportRec_t *reportRec = &(cfgReportCmd->attrList[i]);

      *pBuf++ = reportRec->direction;
      *pBuf++ = LO_UINT16( reportRec->attrID );
      *pBuf++ = HI_UINT16( reportRec->attrID );

      if ( reportRec->direction == ZCL_SEND_ATTR_REPORTS )
      {
        *pBuf++ = reportRec->dataType;
        *pBuf++ = LO_UINT16( reportRec->minReportInt );
        *pBuf++ = HI_UINT16( reportRec->minReportInt );
        *pBuf++ = LO_UINT16( reportRec->maxReportInt );
        *pBuf++ = HI_UINT16( reportRec->maxReportInt );

        if ( zclAnalogDataType( reportRec->dataType ) )
        {
          pBuf = zclSerializeData( reportRec->dataType, reportRec->reportableChange, pBuf );
        }
      }
      else
      {
        *pBuf++ = LO_UINT16( reportRec->timeoutPeriod );
        *pBuf++ = HI_UINT16( reportRec->timeoutPeriod );
      }
    } // for loop

    status = zcl_sendcommand( srcEP, dstEp, dstAddr, clusterID, ZCL_CMD_CONFIG_REPORT, FALSE,
                              direction, disableDefaultRsp, 0, seqNum, dataLen, buf );
    zcl_mem_free( buf );
  }
  else
  {
    status = ZMemError;
  }

  return ( status );
}


/*********************************************************************
 * @fn      zcl_SendReadReportCfgCmd
 *
 * @brief   Send a Read Reporting Configuration command
 *
 * @param   dstAddr - destination address
 * @param   clusterID - cluster ID
 * @param   readReportCfgCmd - read reporting configuration command to be sent
 * @param   direction - direction of the command
 * @param   seqNum - transaction sequence number
 *
 * @return  ZSuccess if OK
 */
ZStatus_t zcl_SendReadReportCfgCmd( uint8 srcEP, uint8 dstEp, uint16 dstAddr,
                  uint16 clusterID, zclReadReportCfgCmd_t *readReportCfgCmd,
                  uint8 direction, uint8 disableDefaultRsp, uint8 seqNum )
{
  uint16 dataLen;
  uint8 *buf;
  ZStatus_t status;

  dataLen = readReportCfgCmd->numAttr * ( 1 + 2 ); // Direction + Atrribute ID

  buf = zcl_mem_alloc( dataLen );
  if ( buf != NULL )
  {
    // Load the buffer - serially
    uint8 *pBuf = buf;
    uint8 i;

    for ( i = 0; i < readReportCfgCmd->numAttr; i++ )
    {
      *pBuf++ = readReportCfgCmd->attrList[i].direction;
      *pBuf++ = LO_UINT16( readReportCfgCmd->attrList[i].attrID );
      *pBuf++ = HI_UINT16( readReportCfgCmd->attrList[i].attrID );
    }

    status = zcl_sendcommand( srcEP, dstEp, dstAddr, clusterID, ZCL_CMD_READ_REPORT_CFG, FALSE,
                              direction, disableDefaultRsp, 0, seqNum, dataLen, buf );
    zcl_mem_free( buf );
  }
  else
  {
    status = ZMemError;
  }

  return ( status );
}


