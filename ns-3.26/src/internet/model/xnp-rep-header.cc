/*
 * xnp-rep-header.cc
 *
 *  Created on: 2018年5月14日
 *      Author: Odie
 */
#include "xnp-rep-header.h"

#include "ns3/assert.h"
#include "ns3/address-utils.h"
#include "ns3/log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("XnpRepHeader");

NS_OBJECT_ENSURE_REGISTERED (XnpRepHeader);

void
XnpRepHeader::x_SetVersionNum(uint8_t num)
{
	x_versionNum = num;
}

void
XnpRepHeader::x_SetId(uint8_t id)
{
	x_identifier = id;
}

TypeId
XnpRepHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::XnpRepHeader")
    .SetParent<Header> ()
    .SetGroupName ("Internet")
    .AddConstructor<XnpRepHeader> ()
  ;
  return tid;
}

TypeId
XnpRepHeader::GetInstanceTypeId (void) const
{
  NS_LOG_FUNCTION (this);
  return GetTypeId ();
}

void
XnpRepHeader::Print (std::ostream &os) const
{
  NS_LOG_FUNCTION (this << &os);

}

uint8_t
XnpRepHeader::x_GetVersionNum(void)
{
	return x_versionNum;
}

uint8_t
XnpRepHeader::x_GetId(void)
{
	return x_identifier;
}

uint32_t
XnpRepHeader::GetSerializedSize (void) const
{
  return 48;
}

void
XnpRepHeader::Serialize (Buffer::Iterator start) const
{
  NS_LOG_FUNCTION (this << &start);
  Buffer::Iterator i = start;
  i.WriteU8(x_versionNum);
  i.WriteU8(x_identifier);
  //****** Messeage Filed************OD
  i.WriteU8(0x10);
  i.WriteU8(0x0);
  i.WriteU8(0x0);
  i.WriteU8(0x0);
  i.WriteU8(0x20);
  i.WriteU8(0x30);
  i.WriteU8(0x40);
  i.WriteU8(0x08);
  i.WriteU8(0x0);
  i.WriteU8(0x30);
  i.WriteU8(0x0);
  i.WriteU8(0x0);
  i.WriteU8(0xff);
  i.WriteU8(0xff);
  i.WriteU8(0xff);
  i.WriteU8(0x7f);
  i.WriteU8(0x40);
  i.WriteU8(0x80);
  i.WriteU8(0xb0);
  i.WriteU8(0x88);
  i.WriteU8(0x0);
  i.WriteU8(0x14);
  i.WriteU8(0x20);
  i.WriteU8(0xa0);
  i.WriteU8(0xc0);
  i.WriteU8(0x0);
  i.WriteU8(0x3c);
  i.WriteU8(0x60);
  i.WriteU8(0xe0);
  i.WriteU8(0x20);
  i.WriteU8(0x80);
  i.WriteU8(0x10);
  i.WriteU8(0xc0);
  i.WriteU8(0x26);
  i.WriteU8(0x30);
  i.WriteU8(0xd0);
  i.WriteU8(0x70);
  i.WriteU8(0x70);
  i.WriteU8(0x0);
  i.WriteU8(0x0);
  i.WriteU8(0x26);
  i.WriteU8(0x0);
  i.WriteU8(0x26);
  i.WriteU8(0xc0);
  i.WriteU8(0xc0);
  i.WriteU8(0xff);
  //****** Messeage Filed************OD

}

uint32_t
XnpRepHeader::Deserialize (Buffer::Iterator start)
{
  NS_LOG_FUNCTION (this << &start);
  Buffer::Iterator i = start;
  x_versionNum = i.ReadU8();
  x_identifier = i.ReadU8();
  for(uint32_t cnt = 0; cnt < 9; cnt++)
  i.ReadU8();
  return GetSerializedSize ();
}

} // namespace ns3




