/*application-header.h
 *Created on: May 9, 2018
 *    Author: liang
 */

#ifndef SRC_APPLICATIONS_MODEL_APPLICATION_HEADER_H_
#define SRC_APPLICATIONS_MODEL_APPLICATION_HEADER_H_

#include "ns3/header.h"
#include "ns3/nstime.h"
#include <stdint.h>
#include <bitset>

namespace ns3 {


enum ApplicationType
{
  APPLICATION_IMAGE = 0,
  APPLICATION_VOICE = 1,
  APPLICATION_POSITION = 2 ,

};

class ApplicationHeader: public Header
{
public:
	ApplicationHeader();
    ~ApplicationHeader();

	static TypeId GetTypeId (void);
	virtual TypeId GetInstanceTypeId (void) const;

	bool IsImage (void) const;
	bool IsVoice (void) const;
	bool IsPosition (void) const;

	void SetAppType (uint32_t ctrl);
	uint32_t GetAppType (void) const;
	void SetVersionCompression (uint32_t field);
	uint32_t GetVersionCompression (void) const;
	void SetOriginatorAddressGroup (uint32_t val);
	uint32_t GetOriginatorAddressGroup (void) const;
	void SetRecipientAddressGroup (uint32_t val);
	uint32_t GetRecipientAddressGroup (void) const;
	void SetInformationAddressGroup (uint32_t val);
	uint32_t GetInformationAddressGroup (void) const;
	void SetFutureUseGroup12 (uint32_t val);
	uint32_t GetFutureUseGroup12 (void) const;
	void SetFutureUseGroup345 (uint32_t val);
	uint32_t GetFutureUseGroup345 (void) const;
	void SetMessageHandlingGroup (uint32_t val);
	uint32_t GetMessageHandlingGroup (void) const;
	void SetVMFmessageIdentificationGroup (uint32_t val);
	uint32_t GetVMFmessageIdentificationGroup (void) const;

	void SetVMFmessageIdentificationGroup_MessageNumber (uint32_t val);
	uint32_t GetVMFmessageIdentificationGroup_MessageNumber (void) const;

	void SetOriginatorDataTimeGroup_GPI_Year (uint8_t val);
	uint8_t GetOriginatorDataTimeGroup_GPI_Year (void) const;

	void SetOriginatorDataTimeGroup_Complementation (uint32_t val);
	uint32_t GetOriginatorDataTimeGroup_Complementation (void) const;
	void SetRestGroup (uint16_t val);
	uint16_t GetRestGroup (void) const;

	void SetGroupAggregation1 (uint32_t val);
	uint32_t GetGroupAggregation1 (void) const;
	void SetGroupAggregation2 (uint32_t val);
	uint32_t GetGroupAggregation2 (void) const;
	void SetGroupAggregation3 (uint32_t val);
	uint32_t GetGroupAggregation3 (void) const;

	//***********************od
	void SetPacketId(uint32_t od_Pid);
	uint32_t ReadPacketId(void);
//	void DispPacketVec(void);


	uint32_t GetSize (void) const;
	virtual void Print (std::ostream &os) const;
	uint32_t GetSerializedSize (void) const;
	void Serialize (Buffer::Iterator start) const;
	uint32_t Deserialize (Buffer::Iterator start);

	uint32_t od_packetId = 0;

	uint32_t m_AppType;
	uint32_t m_string2 = 0xabcdabcd;

	uint32_t m_version = 3;                                               //4bit
	uint32_t m_compressionType = 0;                                       //1bit

	uint32_t m_originatorAddressGroup_GPI = 1;					         //1bit
	uint32_t m_originatorAddressGroup_URN_FPI = 1;				         //1bit
	uint32_t m_originatorAddressGroup_URN = 0xffffff;                    //24bit****Changeable
	uint32_t m_originatorAddressGroup_UnitName_FPI = 0;			         //1bit

	uint32_t m_recipientAddressGroup_GPI = 1;			   		         //1bit
	uint32_t m_recipientAddressGroup_GRI = 0;			                 //1bit
	uint32_t m_recipientAddressGroup_URN_FPI = 1;			             //1bit
	uint32_t m_recipientAddressGroup_URN = 0xffffff;                     //24bit****Changeable
	uint32_t m_recipientAddressGroup_UnitName_FPI = 0;			         //1bit

	uint32_t m_informationAddressGroup_GPI = 0;			                 //1bit
	uint32_t m_informationAddressGroup_HeaderSize_FPI = 0;		         //1bit

	uint32_t m_futureUse1 = 0;			                                 //1bit
	uint32_t m_futureUse2 = 0;			                                 //1bit
	uint32_t m_futureUse3 = 0;			                                 //1bit
	uint32_t m_futureUse4 = 0;			                                 //1bit
	uint32_t m_futureUse5 = 0;			                                 //1bit

	uint32_t m_messageHandlingGroup_GPI = 0;			                     //1bit
	uint32_t m_messageHandlingGroup_UMF = 2;                              //4bit
	uint32_t m_messageHandlingGroup_MessageStandardVersionFPI = 0;        //1bit

	uint32_t m_VMFmessageIdentificationGroup_GPI = 1;                     //1bit
	uint32_t m_VMFmessageIdentificationGroup_FAD = 15;                    //4bit****Changeable
	uint32_t m_VMFmessageIdentificationGroup_MessageNumber = 99;          //7bit****Changeable
	uint32_t m_VMFmessageIdentificationGroup_MessageSubtype_FPI = 0;      //1bit
	uint32_t m_VMFmessageIdentificationGroup_FileName_FPI = 0;            //1bit
	uint32_t m_VMFmessageIdentificationGroup_MessageSize_FPI = 0;         //1bit
	uint32_t m_VMFmessageIdentificationGroup_OperationIndicator = 0;      //1bit
	uint32_t m_VMFmessageIdentificationGroup_RetransmitIndicator = 0;     //1bit
	uint32_t m_VMFmessageIdentificationGroup_MessagePrecedenceCode = 7;   //3bit
	uint32_t m_VMFmessageIdentificationGroup_SecurityClassification = 0;  //2bit
	uint32_t m_VMFmessageIdentificationGroup_ControlReleaseMaking_FPI = 0;//1bit

	uint32_t m_OriginatorDataTimeGroup_GPI = 1;                           //1bit
	uint32_t m_OriginatorDataTimeGroup_Year = 4;                          //7bit****Changeable
	uint32_t m_OriginatorDataTimeGroup_Month = 2;                         //4bit****Changeable
	uint32_t m_OriginatorDataTimeGroup_Day = 14;                          //5bit****Changeable
	uint32_t m_OriginatorDataTimeGroup_Hour = 8;                          //5bit****Changeable
	uint32_t m_OriginatorDataTimeGroup_Minute = 32;                       //6bit****Changeable
	uint32_t m_OriginatorDataTimeGroup_Second = 16;                       //6bit****Changeable
	uint32_t m_OriginatorDataTimeGroup_Extension_FPI = 0;                 //1bit

	uint32_t m_PerishabilityDataTimeGroup_GPI = 0;                        //1bit
	uint32_t m_AcknowledgmentRequestGroup_GPI = 0;                        //1bit
	uint32_t m_ResponseDataGroup_GPI = 0;                                 //1bit
	uint32_t m_PeferenceMessageDataGroup_GPI = 0;                         //1bit
	uint32_t m_futureUse6 = 0;			                                 //1bit
	uint32_t m_futureUse7 = 0;			                                 //1bit
	uint32_t m_futureUse8 = 0;			                                 //1bit
	uint32_t m_futureUse9 = 0;			                                 //1bit
	uint32_t m_futureUse10 = 0;			                                 //1bit
	uint32_t m_MessageSecurityGroup_GPI = 0;                              //1bit
	uint32_t m_futureUse11 = 0;			                                 //1bit
	uint32_t m_futureUse12 = 0;			                                 //1bit
	uint32_t m_futureUse13 = 0;			                                 //1bit
	uint32_t m_futureUse14 = 0;			                                 //1bit
	uint32_t m_futureUse15 = 0;			                                 //1bit,total = 141bit
};
} /* namespace ns3 */

#endif /* SRC_APPLICATIONS_MODEL_APPLICATION_HEADER_H_ */
