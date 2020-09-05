#include "ns3/assert.h"
#include "ns3/address-utils.h"
#include "XNPRequest.h"

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (XNPRequest);

XNPRequest::XNPRequest ()
  : XNP_version (0)
{
}

XNPRequest::~XNPRequest ()
{
}
TypeId
XNPRequest::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::XNPRequest")
    .SetParent<Header> ()
    .SetGroupName ("XNPRequest")
    .AddConstructor<XNPRequest> ()
  ;
  return tid;
}

TypeId
XNPRequest::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

void
XNPRequest::SetVersion(uint8_t ver)
{
	XNP_version=ver;
}

void
XNPRequest::SetType(uint8_t type)
{
	XNP_type=type;
}

uint8_t
XNPRequest::GetVersion(void) const
{
	return XNP_version;
}

uint8_t
XNPRequest::GetType(void) const
{
	return XNP_type;
}

uint32_t
XNPRequest::GetSize (void) const
{
	return 11;
}



uint32_t
XNPRequest::GetSerializedSize (void) const
{
  return GetSize ();
}



void
XNPRequest::Serialize (Buffer::Iterator i) const
{
	i.WriteU8(GetVersion());
	i.WriteU8(GetType());
	uint8_t fill=0xaa;
	for(uint16_t m=0;m<9;m++)
	{
		i.WriteU8(fill);
	}
}

uint32_t
XNPRequest::Deserialize (Buffer::Iterator start)
{
	 Buffer::Iterator i = start;
	  uint8_t version = i.ReadU8();
	  SetVersion(version);
	  uint8_t type = i.ReadU8();
	  SetType( type);
		for(uint16_t m=0;m<9;m++)
		{
			i.ReadU8();
		}
		return i.GetDistanceFrom (start);
}

}



