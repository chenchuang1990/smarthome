
#include <stdlib.h>
#include "zcl.h"
#include "commands.h"
#include "mtAf.h"
#include "mtParser.h"
#include "zcl_ss.h"
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
static uint8 zclCalcHdrSize( struct zclframehdr *hdr )
{
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
 * @fn      zcl_SendCommand
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
ZStatus_t zcl_SendCommand( uint8 srcEP, uint8 dstEp, uint16 dstaddr,
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
	switch(zclmessage.message->ClusterId){ 
		case ZCL_CLUSTER_ID_SS_IAS_ZONE: 
			result = zclss_handleincoming(&zclmessage);
			break;
	}

	return result;
}
