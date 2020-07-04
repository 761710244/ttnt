/*application-header.h
 *Created on: May 9, 2018
 *    Author: liang
 */

#include "application-header.h"
#include <bitset>
#include <iostream>
#include <time.h>
#include <stdio.h>
#include <fstream>
#include "udp-client.h"
namespace ns3 {


ApplicationHeader::ApplicationHeader()
{
}

ApplicationHeader::~ApplicationHeader()
{
}

//void
//ApplicationHeader::DispPacketVec(void)
//{
//	std::ofstream PacketVectorFile("PacketVector.txt", std::ios::app);
//	if(PacketVectorFile.is_open())
//	{
//		for(uint16_t i = 0; i < od_packetVec.size(); i++)
//		{
//			PacketVectorFile << od_packetVec[i] << " + ";
//		}
//	}
//	PacketVectorFile.close();
//}

void
ApplicationHeader::SetPacketId (uint32_t od_Pid)
{
	od_packetId = od_Pid;
}

uint32_t
ApplicationHeader::ReadPacketId(void)
{
	//std::cout<<"-r od_packetId = "<<od_packetId<<std::endl;
	return od_packetId;
}

TypeId
ApplicationHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::ApplicationHeader")
    .SetParent<Header> ()
    .SetGroupName("Applications")
    .AddConstructor<ApplicationHeader> ();
  return tid;
}

TypeId
ApplicationHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

bool
ApplicationHeader::IsImage (void) const
{
  return (m_AppType == APPLICATION_IMAGE);

}

bool
ApplicationHeader::IsVoice (void) const
{
  return (m_AppType == APPLICATION_VOICE);

}


bool
ApplicationHeader::IsPosition (void) const
{
  return (m_AppType == APPLICATION_POSITION);

}


void
ApplicationHeader::SetAppType (uint32_t Appt)
{
  m_AppType = Appt;
}
uint32_t
ApplicationHeader::GetAppType (void) const
{
  return m_AppType;
}

void
ApplicationHeader::SetVersionCompression (uint32_t val)
{
	m_version = (val >> 28)  & 0x0000000f;
	m_compressionType = (val >> 27) & 0x00000001;
}

uint32_t
ApplicationHeader::GetVersionCompression (void) const
{
  uint32_t val = 0;
  val += (m_version << 28)  ;
  val += (m_compressionType << 27);
  return val;
}

void
ApplicationHeader::SetOriginatorAddressGroup (uint32_t val)
{
	m_originatorAddressGroup_GPI = (val >> 31)  & 0x00000001;
	m_originatorAddressGroup_URN_FPI = (val >> 30) & 0x00000001;
	m_originatorAddressGroup_URN = (val >> 6) & 0x00ffffff;
	m_originatorAddressGroup_UnitName_FPI = (val >> 5) & 0x00000001;
}

uint32_t
ApplicationHeader::GetOriginatorAddressGroup (void) const
{
  uint32_t val = 0;
  val += (m_originatorAddressGroup_GPI << 31);
  val += (m_originatorAddressGroup_URN_FPI << 30);
  val += (m_originatorAddressGroup_URN << 6);
  val += (m_originatorAddressGroup_UnitName_FPI << 5);
  return val;
}

void
ApplicationHeader::SetRecipientAddressGroup (uint32_t val)
{
	m_recipientAddressGroup_GPI = (val >> 31) & 0x00000001;
	m_recipientAddressGroup_GRI = (val >> 30) & 0x00000001;
	m_recipientAddressGroup_URN_FPI = (val >> 29) & 0x00000001;
	m_originatorAddressGroup_URN = (val >> 5) & 0x00ffffff;
	m_originatorAddressGroup_UnitName_FPI = (val >> 4) & 0x00000001;
}

uint32_t
ApplicationHeader::GetRecipientAddressGroup (void) const                                      //28bit
{
  uint32_t val = 0;
  val += (m_recipientAddressGroup_GPI << 31);
  val += (m_recipientAddressGroup_GRI << 30);
  val += (m_recipientAddressGroup_URN_FPI << 29);
  val += (m_originatorAddressGroup_URN << 5);
  val += (m_originatorAddressGroup_UnitName_FPI << 4);
  return val;
}

void
ApplicationHeader::SetInformationAddressGroup (uint32_t val)                                  //2bit
{
	m_informationAddressGroup_GPI = (val >> 31) & 0x00000001;
	m_informationAddressGroup_HeaderSize_FPI = (val >> 30) & 0x00000001;
}

uint32_t
ApplicationHeader::GetInformationAddressGroup (void) const
{
  uint32_t val = 0;
  val += (m_informationAddressGroup_GPI << 31);
  val += (m_informationAddressGroup_HeaderSize_FPI << 30);
  return val;
}

void
ApplicationHeader::SetFutureUseGroup12 (uint32_t val)
{
	m_futureUse1 = (val >> 15) & 0x0001;
	m_futureUse2 = (val >> 14) & 0x0001;
}

uint32_t
ApplicationHeader::GetFutureUseGroup12 (void) const
{
  uint32_t val = 0;
  val += (m_futureUse1 << 15);
  val += (m_futureUse2 << 14);
  return val;
}

void
ApplicationHeader::SetFutureUseGroup345 (uint32_t val)
{
	m_futureUse3 = (val >> 31) & 0x00000001;
	m_futureUse4 = (val >> 30) & 0x00000001;
	m_futureUse5 = (val >> 29) & 0x00000001;
}

uint32_t
ApplicationHeader::GetFutureUseGroup345 (void) const
{
  uint32_t val = 0;
  val += (m_futureUse3 << 31);
  val += (m_futureUse4 << 30);
  val += (m_futureUse5 << 29);
  return val;
}

void
ApplicationHeader::SetMessageHandlingGroup (uint32_t val)
{
	m_messageHandlingGroup_GPI = (val >> 31) & 0x00000001;
	m_messageHandlingGroup_UMF = (val >> 27) & 0x0000000f;
	m_messageHandlingGroup_MessageStandardVersionFPI = (val >> 26) & 0x00000001;
}

uint32_t
ApplicationHeader::GetMessageHandlingGroup (void) const
{
  uint32_t val = 0;
  val += (m_messageHandlingGroup_GPI << 31);
  val += (m_messageHandlingGroup_UMF << 27);
  val += (m_messageHandlingGroup_MessageStandardVersionFPI << 26);
  return val;
}

void
ApplicationHeader::SetVMFmessageIdentificationGroup (uint32_t val)
{
	m_VMFmessageIdentificationGroup_GPI = (val >> 31) & 0x00000001;
	m_VMFmessageIdentificationGroup_FAD = (val >> 27) & 0x0000000f;
	m_VMFmessageIdentificationGroup_MessageNumber = (val >> 20) & 0x0000007f;
	m_VMFmessageIdentificationGroup_MessageSubtype_FPI = (val >> 19) & 0x00000001;
	m_VMFmessageIdentificationGroup_FileName_FPI = (val >> 18) & 0x00000001;
	m_VMFmessageIdentificationGroup_MessageSize_FPI = (val >> 17) & 0x00000001;
	m_VMFmessageIdentificationGroup_OperationIndicator = (val >> 16) & 0x00000001;
	m_VMFmessageIdentificationGroup_RetransmitIndicator = (val >> 15) & 0x00000001;
	m_VMFmessageIdentificationGroup_MessagePrecedenceCode = (val >> 12) & 0x00000007;
	m_VMFmessageIdentificationGroup_SecurityClassification = (val >> 10) & 0x00000003;
	m_VMFmessageIdentificationGroup_ControlReleaseMaking_FPI = (val >> 9) & 0x00000001;
}

uint32_t
ApplicationHeader::GetVMFmessageIdentificationGroup (void) const
{
  uint32_t val = 0;
  val += (m_VMFmessageIdentificationGroup_GPI << 31);
  val += (m_VMFmessageIdentificationGroup_FAD << 27);
  val += (m_VMFmessageIdentificationGroup_MessageNumber << 20);
  val += (m_VMFmessageIdentificationGroup_MessageSubtype_FPI << 19);
  val += (m_VMFmessageIdentificationGroup_FileName_FPI << 18);
  val += (m_VMFmessageIdentificationGroup_MessageSize_FPI << 17);
  val += (m_VMFmessageIdentificationGroup_OperationIndicator << 16);
  val += (m_VMFmessageIdentificationGroup_RetransmitIndicator << 15);
  val += (m_VMFmessageIdentificationGroup_MessagePrecedenceCode << 12);
  val += (m_VMFmessageIdentificationGroup_SecurityClassification << 10);
  val += (m_VMFmessageIdentificationGroup_ControlReleaseMaking_FPI << 9);
  return val;
}

void
ApplicationHeader::SetVMFmessageIdentificationGroup_MessageNumber (uint32_t val)
{
	m_VMFmessageIdentificationGroup_MessageNumber = val;
}

uint32_t
ApplicationHeader::GetVMFmessageIdentificationGroup_MessageNumber (void) const
{
	uint32_t val = 0;
  val = m_VMFmessageIdentificationGroup_MessageNumber;
  return val;
}


void
ApplicationHeader::SetOriginatorDataTimeGroup_GPI_Year (uint8_t val)
{
	m_OriginatorDataTimeGroup_GPI = (val >> 7) & 0x01;
	m_OriginatorDataTimeGroup_Year = val & 0x7f;
}

uint8_t
ApplicationHeader::GetOriginatorDataTimeGroup_GPI_Year (void) const
{
  uint8_t val = 0;
  val += (m_OriginatorDataTimeGroup_GPI << 7);
  val += m_OriginatorDataTimeGroup_Year;
  return val;
}

void
ApplicationHeader::SetOriginatorDataTimeGroup_Complementation (uint32_t val)
{
	m_OriginatorDataTimeGroup_Month = (val >> 28) & 0x0000000f;
	m_OriginatorDataTimeGroup_Day = (val >> 23) & 0x0000001f;
	m_OriginatorDataTimeGroup_Hour = (val >> 18) & 0x0000001f;
	m_OriginatorDataTimeGroup_Minute = (val >> 12) & 0x0000003f;
	m_OriginatorDataTimeGroup_Second = (val >> 6) & 0x0000003f;
	m_PerishabilityDataTimeGroup_GPI = (val >> 5) & 0x00000001;
    m_AcknowledgmentRequestGroup_GPI = (val >> 4) & 0x00000001;
	m_ResponseDataGroup_GPI = (val >> 3) & 0x00000001;
	m_PeferenceMessageDataGroup_GPI = (val >> 2) & 0x00000001;
	m_futureUse6 = (val >> 1) & 0x00000001;
	m_futureUse7 = val & 0x00000001;
}

uint32_t
ApplicationHeader::GetOriginatorDataTimeGroup_Complementation (void) const
{
  uint32_t val = 0;
  val += (m_OriginatorDataTimeGroup_Month << 28);
  val += (m_OriginatorDataTimeGroup_Day << 23);
  val += (m_OriginatorDataTimeGroup_Hour << 18);
  val += (m_OriginatorDataTimeGroup_Minute << 12);
  val += (m_OriginatorDataTimeGroup_Second << 6);
  val += (m_PerishabilityDataTimeGroup_GPI << 5);
  val += (m_AcknowledgmentRequestGroup_GPI << 4);
  val += (m_ResponseDataGroup_GPI << 3);
  val += (m_PeferenceMessageDataGroup_GPI << 2);
  val += (m_futureUse6 << 1);
  val += m_futureUse7;
  return val;
}

void
ApplicationHeader::SetRestGroup (uint16_t val)
{
	m_futureUse8 = (val >> 15) & 0x00000001;
	m_futureUse9 = (val >> 14) & 0x00000001;
	m_futureUse10 = (val >> 13) & 0x00000001;
	m_MessageSecurityGroup_GPI = (val >> 12) & 0x00000001;
	m_futureUse11 = (val >> 11) & 0x00000001;
	m_futureUse12 = (val >> 10) & 0x00000001;
	m_futureUse13 = (val >> 9) & 0x00000001;
	m_futureUse14 = (val >> 8) & 0x00000001;
	m_futureUse15 = (val >> 7) & 0x00000001;
}

uint16_t
ApplicationHeader::GetRestGroup (void) const
{
  uint16_t val = 0;
  val += (m_futureUse8 << 15);
  val += (m_futureUse9 << 14);
  val += (m_futureUse10 << 13);
  val += (m_MessageSecurityGroup_GPI << 12);
  val += (m_futureUse11 << 11);
  val += (m_futureUse12 << 10);
  val += (m_futureUse13 << 9);
  val += (m_futureUse14 << 8);
  val += (m_futureUse15 << 7);
  return val;
}

uint32_t
ApplicationHeader::GetSize (void) const
{
	  uint32_t size = 23;
	  return size + 4;
}


void
ApplicationHeader::Print (std::ostream &os) const
{
}

uint32_t
ApplicationHeader::GetSerializedSize (void) const
{
  return GetSize ();
}

uint32_t
ApplicationHeader::GetGroupAggregation1 (void) const
{
	uint32_t val = 0;
	val += (GetVersionCompression ());
	val += (GetOriginatorAddressGroup () >> 5);
	return val;
}

void
ApplicationHeader::SetGroupAggregation1 (uint32_t val)
{
	SetVersionCompression (val & 0xf800000000);
	SetOriginatorAddressGroup ((val << 5) & 0xffffffe0);
}

uint32_t
ApplicationHeader::GetGroupAggregation2 (void) const
{
	uint32_t val = 0;
	val += (GetRecipientAddressGroup ());
	val += (GetInformationAddressGroup () >> 28);
	val += (GetFutureUseGroup12 () >> 30);
	return val;
}

void
ApplicationHeader::SetGroupAggregation2 (uint32_t val)
{
	SetRecipientAddressGroup (val & 0xfffffff0);
	SetInformationAddressGroup ((val << 28) & 0xc0000000);
	SetFutureUseGroup12 ((val << 30) & 0xc00000000);
}

uint32_t
ApplicationHeader::GetGroupAggregation3 (void) const
{
	uint32_t val = 0;
	val += (GetFutureUseGroup345 ());
	val += (GetMessageHandlingGroup () >> 3);
	val += (GetVMFmessageIdentificationGroup () >> 9);
	return val;
}

void
ApplicationHeader::SetGroupAggregation3 (uint32_t val)
{
	SetFutureUseGroup345 (val & 0xe0000000);
	SetMessageHandlingGroup ((val << 3) & 0xfc000000);
	SetVMFmessageIdentificationGroup ((val << 9) & 0xfffffe00);
}



void
ApplicationHeader::Serialize (Buffer::Iterator i) const
{
  i.WriteHtonU32 (GetGroupAggregation1 ());
  i.WriteHtonU32 (GetGroupAggregation2 ());
  i.WriteHtonU32 (GetGroupAggregation3 ());
  i.WriteU8 (GetOriginatorDataTimeGroup_GPI_Year (), 1);
  i.WriteHtonU32 (GetOriginatorDataTimeGroup_Complementation ());
  i.WriteHtonU16 (GetRestGroup ());
  i.WriteHtonU32 (m_string2);
  i.WriteU32(od_packetId);//odd
}

uint32_t
ApplicationHeader::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;
  SetGroupAggregation1 (i.ReadU32());
  SetGroupAggregation2 (i.ReadU32());
  SetGroupAggregation3 (i.ReadU32());
  SetOriginatorDataTimeGroup_GPI_Year (i.ReadU8());
  SetOriginatorDataTimeGroup_Complementation (i.ReadU32());
  SetRestGroup (i.ReadU16 ());
  m_string2 = i.ReadU32 ();
  od_packetId = i.ReadU32();//odd
  return i.GetDistanceFrom (start);
}

} /* namespace ns3 */
