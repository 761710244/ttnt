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

#include "ns3/assert.h"
#include "ns3/abort.h"
#include "ns3/log.h"
#include "ns3/header.h"
#include <sstream>
#include <iostream>
#include <fstream>
#include "ns3/address-utils.h"
#include <sstream>
#include <string>

#include "ipv4-header1.h"
using namespace std;

namespace ns3 {
namespace sincgars {

NS_LOG_COMPONENT_DEFINE ("Ipv4Header1");

NS_OBJECT_ENSURE_REGISTERED (Ipv4Header);

Ipv4Header::Ipv4Header ()
  : m_calcChecksum (false),
    m_payloadSize (0),
    m_identification (0),
    m_tos (0),
    m_ttl (0),
    m_protocol (0),
    m_flags (0),
    m_fragmentOffset (0),
    m_checksum (0),
    m_goodChecksum (true),
    m_headerSize(5*4 -14 + 8+1+2/*+(getReadAddrFile()-2)*4*/ /*+4*(getRouteSize())*/) //ODD
{
}

//************* IntraHeader is nested in front of Ipv4 *************OD
uint32_t Ipv4Header::getRouteSize()
{
	std::vector<uint32_t> odie;
	std::stringstream od,ss;
	ss<<"dsrRoutingTable_"<<m_source<<"_"<<m_destination<<".txt";
	std::ifstream in;
	in.open(ss.str());
	if(!in) std::cout<<"Can't Open DSR File :("<<std::endl;

	std::string t1;
  	std::vector<std::string> ve;
  	while(in>>t1)
  	{
  		ve.push_back(t1);
  	}

  	std::string ods_size = ve[0];
  	uint32_t od_size =  atoi(ods_size.c_str());
  	return od_size;
}


std::vector<Ipv4Address> Ipv4Header::getReadAddrFile(void)
{
    std::vector<uint32_t> odie;
    std::stringstream od,ss;
    ss<<"dsrRoutingTable_"<<m_source<<"_"<<m_destination<<".txt";
    std::ifstream in;
    in.open(ss.str());
    if(!in) std::cout<<"Can't Open DSR File :("<<std::endl;

    std::string t1;
    std::vector<std::string> ve;
    while(in>>t1)
    {
        ve.push_back(t1);
    }

    char a[NOdie];
    char b1[NOdie];
    char b2[NOdie];
    char b3[NOdie];
    char b4[NOdie];

    std::vector<std::string> c;
    for(uint32_t iod = 1; iod < ve.size(); iod++)
    {
        uint32_t iii =0;
        uint32_t bi_1 =0;
        uint32_t bi_2 =0;
        uint32_t bi_3 =0;
        uint32_t bi_4 =0;

        memset(b1, 0, sizeof(char)*NOdie);
        memset(b2, 0, sizeof(char)*NOdie);
        memset(b3, 0, sizeof(char)*NOdie);
        memset(b4, 0, sizeof(char)*NOdie);

        strcpy(a, ve[iod].c_str());

        for(;a[iii] != '.';)
        {
            b1[bi_1] = a[iii];
              iii++;
              bi_1++;
        }
        c.push_back(b1);
        iii++;

        for(;a[iii] != '.';)
        {
            b2[bi_2] = a[iii];
            iii++;
            bi_2++;
        }
          c.push_back(b2);
          iii++;

          for(;a[iii] != '.';)
          {
              b3[bi_3] = a[iii];
              iii++;
              bi_3++;
          }
          c.push_back(b3);
          iii++;

          for(;a[iii] != '\0';)
          {
              b4[bi_4] = a[iii];
              iii++;
              bi_4++;
          }
          c.push_back(b4);
          }

          for(uint32_t t = 0; t < c.size(); t++)
              odie.push_back(atoi(c[t].c_str()));

          uint32_t i_add=0;
          for(uint16_t p=0;p<odie.size();p++)
          {
              i_add|= (odie[p]<<(32-8))&(0xff<<(32-8));
              p++;
              i_add|= (odie[p]<<(32-16))&(0xff<<(32-16));
              p++;
              i_add|= (odie[p]<<(32-24))&(0xff<<(32-24));
              p++;
              i_add|= (odie[p]<<(32-32))&(0xff<<(32-32));
              if((p+1)%4==0)
              {
                  Ipv4Address a(i_add);
                  i_addlist.push_back(a);
                  i_add=0;
//                  std::cout<<"mie:! "<<a<<std::endl;
              }
          }
//          return odie;
          return i_addlist;
}


void Ipv4Header::Set_i_MessageType(void)
{
	i_messageType = 0x1;
}

void Ipv4Header::Set_i_Version(void)
{
	i_version = 0x1;
}

void Ipv4Header::Set_i_HeaderLength(void)
{
	i_headerLength = 0x2b;
}

void Ipv4Header::Set_i_ServeType(void)
{
	i_serveType = 0x3b;
}

void Ipv4Header::Set_i_InfoRecognize(void)
{
	i_infoRecognize = 0x1;
}

void Ipv4Header::Set_i_FrameNum(void)
{
	i_frameNum = 0x5;
}

void Ipv4Header::Set_i_frameCounts(void)
{
	i_frameCounts = 0xb;
}

void Ipv4Header::Set_i_MaxHoop(void)
{
	i_maxHoop = 0xf;
}

void Ipv4Header::Set_i_Field_1(uint8_t field_1)
{
	i_version = field_1 & 0x1;
	m_protocol = (field_1) & (0xfe);
//	std::cout<<"i_version = "<<std::hex<<(uint16_t)i_version<<" m_protocol = "<<(uint16_t)m_protocol<<std::endl;
}

void Ipv4Header::Set_i_Field_2(uint8_t field_2)
{
	i_frameCounts = field_2 & 0xf;
	i_frameNum = (field_2 >> 4) & (0xf);
}

void Ipv4Header::Set_i_Field_3(uint8_t field_3)
{
	i_vacancy = field_3 & 0xf;
	i_maxHoop = (field_3 >> 4) & (0xf);
}

uint8_t Ipv4Header::Get_i_Field_1(void) const
{
	uint8_t val = 0;
	val |= i_version  &  0x1;
	val |= (m_protocol)  &  (0xfe); //ODDDDD
//	std::cout<<"m_protocol: "<<std::hex<<(uint16_t)m_protocol<<std::endl;
//	std::cout<<"GetField_1: "<<std::hex<<(uint16_t)val<<std::endl;
	return val;
}

uint8_t Ipv4Header::Get_i_Field_2(void) const
{
	uint8_t val1 = 0;
	val1 |= i_frameCounts  &  0xf;
	val1 |= (i_frameNum << 4)  &  (0xf << 4);
//	std::cout<<"GetField_2: "<<std::hex<<(uint16_t)val1<<std::endl;
	return val1;
}

uint8_t Ipv4Header::Get_i_Field_3(void) const
{
	uint8_t val2 = 0;
	val2 |= i_vacancy  &  0xf;
	val2 |= (i_maxHoop << 4)  &  (0xf << 4);
//	std::cout<<"GetField_3: "<<std::hex<<(uint16_t)val2<<std::endl;
	return val2;
}
//************* IntraHeader is nested in front of Ipv4 *************OD


void
Ipv4Header::EnableChecksum (void)
{
  NS_LOG_FUNCTION (this);
  m_calcChecksum = true;
}

void
Ipv4Header::SetPayloadSize (uint16_t size)
{
  NS_LOG_FUNCTION (this << size);
  m_payloadSize = size;
}

uint16_t
Ipv4Header::GetPayloadSize (void) const
{
  NS_LOG_FUNCTION (this);
  return m_payloadSize;
}

uint16_t
Ipv4Header::GetIdentification (void) const
{
  NS_LOG_FUNCTION (this);
  return m_identification;
}
void
Ipv4Header::SetIdentification (uint16_t identification)
{
  NS_LOG_FUNCTION (this << identification);
  m_identification = identification;
}

void
Ipv4Header::SetTos (uint8_t tos)
{
  NS_LOG_FUNCTION (this << static_cast<uint32_t> (tos));
  m_tos = tos;
}

void
Ipv4Header::SetDscp (DscpType dscp)
{
  NS_LOG_FUNCTION (this << dscp);
  m_tos &= 0x3; // Clear out the DSCP part, retain 2 bits of ECN
  m_tos |= (dscp << 2);
}

void
Ipv4Header::SetEcn (EcnType ecn)
{
  NS_LOG_FUNCTION (this << ecn);
  m_tos &= 0xFC; // Clear out the ECN part, retain 6 bits of DSCP
  m_tos |= ecn;
}

Ipv4Header::DscpType
Ipv4Header::GetDscp (void) const
{
  NS_LOG_FUNCTION (this);
  // Extract only first 6 bits of TOS byte, i.e 0xFC
  return DscpType ((m_tos & 0xFC) >> 2);
}

std::string
Ipv4Header::DscpTypeToString (DscpType dscp) const
{
  NS_LOG_FUNCTION (this << dscp);
  switch (dscp)
    {
      case DscpDefault:
        return "Default";
      case DSCP_CS1:
        return "CS1";
      case DSCP_AF11:
        return "AF11";
      case DSCP_AF12:
        return "AF12";
      case DSCP_AF13:
        return "AF13";
      case DSCP_CS2:
        return "CS2";
      case DSCP_AF21:
        return "AF21";
      case DSCP_AF22:
        return "AF22";
      case DSCP_AF23:
        return "AF23";
      case DSCP_CS3:
        return "CS3";
      case DSCP_AF31:
        return "AF31";
      case DSCP_AF32:
        return "AF32";
      case DSCP_AF33:
        return "AF33";
      case DSCP_CS4:
        return "CS4";
      case DSCP_AF41:
        return "AF41";
      case DSCP_AF42:
        return "AF42";
      case DSCP_AF43:
        return "AF43";
      case DSCP_CS5:
        return "CS5";
      case DSCP_EF:
        return "EF";
      case DSCP_CS6:
        return "CS6";
      case DSCP_CS7:
        return "CS7";
      default:
        return "Unrecognized DSCP";
    };
}


Ipv4Header::EcnType
Ipv4Header::GetEcn (void) const
{
  NS_LOG_FUNCTION (this);
  // Extract only last 2 bits of TOS byte, i.e 0x3
  return EcnType (m_tos & 0x3);
}

std::string
Ipv4Header::EcnTypeToString (EcnType ecn) const
{
  NS_LOG_FUNCTION (this << ecn);
  switch (ecn)
    {
      case ECN_NotECT:
        return "Not-ECT";
      case ECN_ECT1:
        return "ECT (1)";
      case ECN_ECT0:
        return "ECT (0)";
      case ECN_CE:
        return "CE";
      default:
        return "Unknown ECN";
    };
}

uint8_t
Ipv4Header::GetTos (void) const
{
  NS_LOG_FUNCTION (this);
  return m_tos;
}
void
Ipv4Header::SetMoreFragments (void)
{
  NS_LOG_FUNCTION (this);
  m_flags |= MORE_FRAGMENTS;
}
void
Ipv4Header::SetLastFragment (void)
{
  NS_LOG_FUNCTION (this);
  m_flags &= ~MORE_FRAGMENTS;
}
bool
Ipv4Header::IsLastFragment (void) const
{
  NS_LOG_FUNCTION (this);
  return !(m_flags & MORE_FRAGMENTS);
}

void
Ipv4Header::SetDontFragment (void)
{
  NS_LOG_FUNCTION (this);
  m_flags |= DONT_FRAGMENT;
}
void
Ipv4Header::SetMayFragment (void)
{
  NS_LOG_FUNCTION (this);
  m_flags &= ~DONT_FRAGMENT;
}
bool
Ipv4Header::IsDontFragment (void) const
{
  NS_LOG_FUNCTION (this);
  return (m_flags & DONT_FRAGMENT);
}

void
Ipv4Header::SetFragmentOffset (uint16_t offsetBytes)
{
  NS_LOG_FUNCTION (this << offsetBytes);
  // check if the user is trying to set an invalid offset
  NS_ABORT_MSG_IF ((offsetBytes & 0x7), "offsetBytes must be multiple of 8 bytes");
  m_fragmentOffset = offsetBytes;
}
uint16_t
Ipv4Header::GetFragmentOffset (void) const
{
  NS_LOG_FUNCTION (this);
  // -fstrict-overflow sensitive, see bug 1868
  if ( m_fragmentOffset + m_payloadSize > 65535 - 5*4 )
    {
      NS_LOG_WARN("Fragment will exceed the maximum packet size once reassembled");
    }

  return m_fragmentOffset;
}

void
Ipv4Header::SetTtl (uint8_t ttl)
{
  NS_LOG_FUNCTION (this << static_cast<uint32_t> (ttl));
  m_ttl = ttl;
}
uint8_t
Ipv4Header::GetTtl (void) const
{
  NS_LOG_FUNCTION (this);
  return m_ttl;
}

uint8_t
Ipv4Header::GetProtocol (void) const
{
  NS_LOG_FUNCTION (this);
  return m_protocol;
}
void
Ipv4Header::SetProtocol (uint8_t protocol)
{
  NS_LOG_FUNCTION (this << static_cast<uint32_t> (protocol));
  m_protocol = protocol;
}

void
Ipv4Header::SetSource (Ipv4Address source)
{
  NS_LOG_FUNCTION (this << source);
  //******** Write that Source address to file ************OD
   /* std::ofstream addrFile("m_source.txt");
    if(addrFile.is_open())
    {
  	  addrFile << source <<std::endl;
    }
    else std::cout<<"Cannot open the file!"<<std::endl;
    addrFile.close();*/
  m_source = source;
}

Ipv4Address
Ipv4Header::GetSource (void) const
{
  NS_LOG_FUNCTION (this);
 /* std::ifstream in;
    in.open("m_source.txt");
    if(!in) std::cout<<"Wrong!!"<<std::endl;

    Ipv4Address od;
    in>>od;
  return od;*/
  return m_source;
}

void
Ipv4Header::SetDestination (Ipv4Address dst)
{
  NS_LOG_FUNCTION (this << dst);
  //******** Write that Destination address to file ************OD
   /* std::ofstream addrFile("m_destination.txt");
    if(addrFile.is_open())
    {
  	  addrFile << dst <<std::endl;
    }
    else std::cout<<"Cannot open the file!"<<std::endl;
    addrFile.close();*/
  m_destination = dst;
}

Ipv4Address
Ipv4Header::GetDestination (void) const
{
  NS_LOG_FUNCTION (this);
  /*std::ifstream in;
    in.open("m_destination.txt");
    if(!in) std::cout<<"Wrong!!"<<std::endl;

    Ipv4Address od;
    in>>od;
  return od;*/
  return m_destination;
}


bool
Ipv4Header::IsChecksumOk (void) const
{
  NS_LOG_FUNCTION (this);
  return m_goodChecksum;
}

TypeId
Ipv4Header::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::Ipv4Header1")
    .SetParent<Header> ()
    .SetGroupName ("Internet")
    .AddConstructor<Ipv4Header> ()
  ;
  return tid;
}
TypeId
Ipv4Header::GetInstanceTypeId (void) const
{
  NS_LOG_FUNCTION (this);
  return GetTypeId ();
}
void
Ipv4Header::Print (std::ostream &os) const
{
std::stringstream od;

  NS_LOG_FUNCTION (this << &os);
  // ipv4, right ?
  std::string flags;
  if (m_flags == 0)
    {
      flags = "none";
    }
  else if ((m_flags & MORE_FRAGMENTS) &&
           (m_flags & DONT_FRAGMENT))
    {
      flags = "MF|DF";
    }
  else if (m_flags & DONT_FRAGMENT)
    {
      flags = "DF";
    }
  else if (m_flags & MORE_FRAGMENTS)
    {
      flags = "MF";
    }
  else
    {
      flags = "XX";
    }
//  od << "tos 0x" << std::hex << m_tos << std::dec << " "
//     << "DSCP " << DscpTypeToString (GetDscp ()) << " "
//     << "ECN " << EcnTypeToString (GetEcn ()) << " "
//     << "ttl " << m_ttl << " "
//     << "id " << m_identification << " "
//     << "protocol " << m_protocol << " "
//     << "offset (bytes) " << m_fragmentOffset << " "
//     << "flags [" << flags << "] "
//	 //*********************************************OD
//     << "length: " << (m_payloadSize + 5 * 4 + 6)
//     << " "
//     << m_source << " > " << m_destination
//  ;

  os << "tos 0x" << std::hex << m_tos << std::dec << " "
     << "DSCP " << DscpTypeToString (GetDscp ()) << " "
     << "ECN " << EcnTypeToString (GetEcn ()) << " "
     << "ttl " << m_ttl << " "
     << "id " << m_identification << " "
     << "protocol " << m_protocol << " "
     << "offset (bytes) " << m_fragmentOffset << " "
     << "flags [" << flags << "] "
	 //*********************************************OD
     << "length: " << (m_payloadSize + 5 * 4 + 6)
     << " "
     << m_source << " > " << m_destination
  ;
}
uint32_t
Ipv4Header::GetSerializedSize (void) const
{
  NS_LOG_FUNCTION (this);
  //return 5 * 4 + 6;
	return m_headerSize + 2;
}

void
Ipv4Header::Serialize (Buffer::Iterator start) const
{
  NS_LOG_FUNCTION (this << &start);
  Buffer::Iterator i = start;
//******************* IntranetHeader ************************OD
    //i.WriteHtonU16 (m_payloadSize + m_headerSize);
  	i.WriteU8(Get_i_Field_1());
  	i.WriteU8(m_headerSize);
  	i.WriteU8(m_tos);
  	i.WriteU16(m_identification);
  	i.WriteU8(Get_i_Field_2());
  	i.WriteU8(m_ttl);
  	i.WriteU16(m_payloadSize);
    i.WriteHtonU32 (m_source.Get ());
  	//**** Route ********************************************ODD
  	/*Ipv4Header ipv4od;
  	std::vector<Ipv4Address> a = ipv4od.getReadAddrFile();
  	 for(uint16_t m=1;m<a.size()-1;m++)
  	  {
  		   WriteTo (i, a[m]);
  	  }*/
  	//**** Route ********************************************ODD
     i.WriteHtonU32 (m_destination.Get ());
//******************* IntranetHeader ************************OD

  if (m_calcChecksum)
    {
      i = start;
      uint16_t checksum = i.CalculateIpChecksum (20);
      NS_LOG_LOGIC ("checksum=" <<checksum);
      i = start;
      i.Next (10);
      i.WriteU16 (checksum);
    }
}
uint32_t
Ipv4Header::Deserialize (Buffer::Iterator start)
{
  NS_LOG_FUNCTION (this << &start);
  Buffer::Iterator i = start;

//******************* IntranetHeader ************************OD
 // uint16_t size = i.ReadNtohU16 ();
//  m_payloadSize = size - m_headerSize;
  uint8_t i_field1 = i.ReadU8 ();
  	Set_i_Field_1 (i_field1);
  	m_headerSize = i.ReadU8();
  	i_serveType = i.ReadU8();
  	m_identification = i.ReadU16();
  	uint8_t i_field2 = i.ReadU8 ();
  	Set_i_Field_2 (i_field2);
    m_ttl = i.ReadU8 ();
    m_payloadSize = i.ReadU16();
    m_source.Set (i.ReadNtohU32 ());
  	//**** Route ******************************************ODD

	Ipv4Header ipv4od;
	 /*uint16_t a =m_headerSize-15 ;
    for(uint16_t m=0;m<a;m++)
    {
    	i.ReadLsbtohU32();
    }*/

    m_destination.Set (i.ReadNtohU32 ());
    //**** Route ******************************************ODD

//******************* IntranetHeader ************************OD

  uint16_t headerSize = 5 * 4 -14 + 8+1/*+a + 4(ipv4od.getRouteSize())*/;//ODD

//  if ((verIhl >> 4) != 4)
//    {
//      NS_LOG_WARN ("Trying to decode a non-IPv4 header, refusing to do it.");
//      return 0;
//    }

 // m_headerSize = headerSize;

  if (m_calcChecksum)
    {
      i = start;
      uint16_t checksum = i.CalculateIpChecksum (headerSize);
      NS_LOG_LOGIC ("checksum=" <<checksum);

      m_goodChecksum = (checksum == 0);
    }
  return GetSerializedSize ();
}
}
} // namespace ns3
//
