/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2005 INRIA
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */

#include "xnp-req-header.h"

#include "ns3/assert.h"
#include "ns3/address-utils.h"
#include "ns3/log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("XnpReqHeader");

NS_OBJECT_ENSURE_REGISTERED (XnpReqHeader);

void
XnpReqHeader::x_SetVersionNum(uint8_t num)
{
	x_versionNum = num;
}

void
XnpReqHeader::x_SetId(uint8_t id)
{
	x_identifier = id;
}


TypeId
XnpReqHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::XnpReqHeader")
    .SetParent<Header> ()
    .SetGroupName ("Internet")
    .AddConstructor<XnpReqHeader> ()
  ;
  return tid;
}

TypeId
XnpReqHeader::GetInstanceTypeId (void) const
{
  NS_LOG_FUNCTION (this);
  return GetTypeId ();
}

void
XnpReqHeader::Print (std::ostream &os) const
{
  NS_LOG_FUNCTION (this << &os);

}

uint8_t
XnpReqHeader::x_GetVersionNum(void)
{
	return x_versionNum;
}

uint8_t
XnpReqHeader::x_GetId(void)
{
	return x_identifier;
}

uint32_t
XnpReqHeader::GetSerializedSize (void) const
{
  return 8;
}

void
XnpReqHeader::Serialize (Buffer::Iterator start) const
{
  NS_LOG_FUNCTION (this << &start);
  Buffer::Iterator i = start;
  i.WriteU8(x_versionNum);
  i.WriteU8(x_identifier);
  //****** Messeage 9 bytes '0x01'************OD
  i.WriteU8(0x60);
  i.WriteU8(0x0);
  i.WriteU8(0x0);
  i.WriteU8(0x0);
  i.WriteU8(0x20);
  i.WriteU8(0xff);

 }

uint32_t
XnpReqHeader::Deserialize (Buffer::Iterator start)
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
