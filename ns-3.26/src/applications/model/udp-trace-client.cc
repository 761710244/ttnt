/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 *  Copyright (c) 2007,2008, 2009 INRIA, UDcast
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
 * Author: Mohamed Amine Ismail <amine.ismail@sophia.inria.fr>
 *                              <amine.ismail@udcast.com>
 */
#include "ns3/log.h"
#include "ns3/ipv4-address.h"
#include "ns3/nstime.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/string.h"
#include "seq-ts-header.h"
#include "application-header.h"
#include "udp-trace-client.h"
#include <cstdlib>
#include <cstdio>
#include <fstream>
#include <time.h>
#include <iostream>
namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("UdpTraceClient");

NS_OBJECT_ENSURE_REGISTERED (UdpTraceClient);

/**
 * \brief Default trace to send
 */
struct UdpTraceClient::TraceEntry UdpTraceClient::g_defaultEntries[] = {
  { 0, 534, 'I'},
  { 40, 1542, 'P'},
  { 120, 134, 'B'},
  { 80, 390, 'B'},
  { 240, 765, 'P'},
  { 160, 407, 'B'},
  { 200, 504, 'B'},
  { 360, 903, 'P'},
  { 280, 421, 'B'},
  { 320, 587, 'B'}
};

TypeId
UdpTraceClient::GetTypeId (void)
{
	  static TypeId tid = TypeId ("ns3::UdpTraceClient")
	    .SetParent<Application> ()
	    .SetGroupName("Applications")
	    .AddConstructor<UdpTraceClient> ()
	    .AddAttribute ("RemoteAddress",
	                   "The destination Address of the outbound packets",
	                   AddressValue (),
	                   MakeAddressAccessor (&UdpTraceClient::m_peerAddress),
	                   MakeAddressChecker ())
	    .AddAttribute ("RemotePort",
	                   "The destination port of the outbound packets",
	                   UintegerValue (100),
	                   MakeUintegerAccessor (&UdpTraceClient::m_peerPort),
	                   MakeUintegerChecker<uint16_t> ())
	    .AddAttribute ("MaxPacketSize",
	                   "The maximum size of a packet (including the SeqTsHeader, 12 bytes).",
	                   UintegerValue (1024),
	                   MakeUintegerAccessor (&UdpTraceClient::m_maxPacketSize),
	                   MakeUintegerChecker<uint32_t> ())
	    .AddAttribute ("TraceFilename",
	                   "Name of file to load a trace from. By default, uses a hardcoded trace.",
	                   StringValue (""),
	                   MakeStringAccessor (&UdpTraceClient::SetTraceFile),
	                   MakeStringChecker ())
//		.AddTraceSource ("Tx", "A new packet is created and is sent",
//					                        MakeTraceSourceAccessor (&UdpTraceClient::m_txTrace),
//					                        "ns3::Packet::TracedCallback")
		 .AddTraceSource ("Tx", "A new packet is created and is sent",
						 MakeTraceSourceAccessor (&UdpTraceClient::m_txTrace),
						"ns3::Packet::TracedCallback")
	//    .AddTraceSource ("Delay", "A new packet is created and is sent",
	//    		MakeTraceSourceAccessor (&UdpTraceClient::m_txDelay), "ns3::Packet::TracedCallback")

	  ;
	  return tid;
}

UdpTraceClient::UdpTraceClient ()
{
  NS_LOG_FUNCTION (this);
  m_sent = 0;
  m_socket = 0;
  m_sendEvent = EventId ();
  m_maxPacketSize = 1400;
}

UdpTraceClient::UdpTraceClient (Ipv4Address ip, uint16_t port,
                                char *traceFile)
{
  NS_LOG_FUNCTION (this);
  m_sent = 0;
  m_socket = 0;
  m_sendEvent = EventId ();
  m_peerAddress = ip;
  m_peerPort = port;
  m_currentEntry = 0;
  m_maxPacketSize = 1400;
  if (traceFile != NULL)
    {
      SetTraceFile (traceFile);
    }
}

UdpTraceClient::~UdpTraceClient ()
{
  NS_LOG_FUNCTION (this);
  m_entries.clear ();
}

void
UdpTraceClient::SetRemote (Address ip, uint16_t port)
{
  NS_LOG_FUNCTION (this << ip << port);
  m_entries.clear ();
  m_peerAddress = ip;
  m_peerPort = port;
}

void
UdpTraceClient::SetRemote (Address addr)
{
  NS_LOG_FUNCTION (this << addr);
  m_entries.clear ();
  m_peerAddress = addr;
}

void
UdpTraceClient::SetTraceFile (std::string traceFile)
{
  NS_LOG_FUNCTION (this << traceFile);
  if (traceFile == "")
    {
      LoadDefaultTrace ();
    }
  else
    {
      LoadTrace (traceFile);
    }
}

void
UdpTraceClient::SetMaxPacketSize (uint16_t maxPacketSize)
{
  NS_LOG_FUNCTION (this << maxPacketSize);
  m_maxPacketSize = maxPacketSize;
}


uint16_t UdpTraceClient::GetMaxPacketSize (void)
{
  NS_LOG_FUNCTION (this);
  return m_maxPacketSize;
}


void
UdpTraceClient::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  Application::DoDispose ();
}

void
UdpTraceClient::LoadTrace (std::string filename)
{
  NS_LOG_FUNCTION (this << filename);
  uint32_t time, index, size, prevTime = 0;
  char frameType;
  TraceEntry entry;
  std::ifstream ifTraceFile;
  ifTraceFile.open (filename.c_str (), std::ifstream::in);
  m_entries.clear ();
  if (!ifTraceFile.good ())
    {
      LoadDefaultTrace ();
    }
  while (ifTraceFile.good ())
    {
      ifTraceFile >> index >> frameType >> time >> size;
      if (frameType == 'B')
        {
          entry.timeToSend = 0;
        }
      else
        {
          entry.timeToSend = time - prevTime;
          prevTime = time;
        }
      entry.packetSize = size;
      entry.frameType = frameType;
      m_entries.push_back (entry);
    }
  ifTraceFile.close ();
  m_currentEntry = 0;
}

void
UdpTraceClient::LoadDefaultTrace (void)
{
  NS_LOG_FUNCTION (this);
  uint32_t prevTime = 0;
  for (uint32_t i = 0; i < (sizeof (g_defaultEntries) / sizeof (struct TraceEntry)); i++)
    {
      struct TraceEntry entry = g_defaultEntries[i];
      if (entry.frameType == 'B')
        {
          entry.timeToSend = 0;
        }
      else
        {
          uint32_t tmp = entry.timeToSend;
          entry.timeToSend -= prevTime;
          prevTime = tmp;
        }
      m_entries.push_back (entry);
    }
  m_currentEntry = 0;
}

void
UdpTraceClient::StartApplication (void)
{
  NS_LOG_FUNCTION (this);

  if (m_socket == 0)
    {
      TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
      m_socket = Socket::CreateSocket (GetNode (), tid);
      if (Ipv4Address::IsMatchingType(m_peerAddress) == true)
        {
          m_socket->Bind ();
          m_socket->Connect (InetSocketAddress (Ipv4Address::ConvertFrom (m_peerAddress), m_peerPort));
        }
      else if (Ipv6Address::IsMatchingType(m_peerAddress) == true)
        {
          m_socket->Bind6 ();
          m_socket->Connect (Inet6SocketAddress (Ipv6Address::ConvertFrom (m_peerAddress), m_peerPort));
        }
      else if (InetSocketAddress::IsMatchingType (m_peerAddress) == true)
        {
          m_socket->Bind ();
          m_socket->Connect (m_peerAddress);
        }
      else if (Inet6SocketAddress::IsMatchingType (m_peerAddress) == true)
        {
          m_socket->Bind6 ();
          m_socket->Connect (m_peerAddress);
        }
      else
        {
          NS_ASSERT_MSG (false, "Incompatible address type: " << m_peerAddress);
        }
    }
  m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
  m_socket->SetAllowBroadcast (true);
  m_sendEvent = Simulator::Schedule (Seconds (0.0), &UdpTraceClient::Send, this);
}

void
UdpTraceClient::StopApplication ()
{
  NS_LOG_FUNCTION (this);
  Simulator::Cancel (m_sendEvent);
}

void
UdpTraceClient::SendPacket (uint32_t size)
{
  NS_LOG_FUNCTION (this << size);
  Ptr<Packet> p;
  uint32_t packetSize;
  if (size>35)
    {
      packetSize = size - 35; // 35 is the 12 byte size of the SeqTsHeader and 19 + 4 byte of ApplicationHeader
    }
  else
    {
      packetSize = 0;
    }
  p = Create<Packet> (packetSize);

  uint32_t AppTypeVal;
  if (size == 500)
	  {
		  AppTypeVal = 0;

	  }
  else if (size == 1000)
	  {
		  AppTypeVal = 1;

	  }
  else
	  {
		  AppTypeVal = 2;

	  }
  time_t timeStamp = time(NULL);
  tm* Time= localtime(&timeStamp);
    uint8_t time_year_val = 128 | Time->tm_year;
    uint32_t time_month_val = 0;
    time_month_val += (Time->tm_mon + 1) << 28;
    time_month_val += (Time->tm_mday) << 23;
    time_month_val += (Time->tm_hour) << 18;
    time_month_val += (Time->tm_min) << 12;
    time_month_val += (Time->tm_sec) << 6;
  ApplicationHeader AppHdr;
  AppHdr.SetVMFmessageIdentificationGroup_MessageNumber (AppTypeVal);
  AppHdr.SetOriginatorDataTimeGroup_GPI_Year (time_year_val);
  AppHdr.SetOriginatorDataTimeGroup_Complementation (time_month_val);
  p->AddHeader (AppHdr);

  std::ofstream write_type;
  write_type.open("AppType.txt");
  if(write_type.good())
  {
	  write_type<<AppTypeVal;
	  write_type.close();
  }
  else
  {
	  std::cout<<"Open AppType.txt Failed ! "<<std::endl;
  }

  SeqTsHeader seqTs;
  seqTs.SetSeq (m_sent);
  p->AddHeader (seqTs);

  //**********************************************ODD
   	   od_TimestampTag timestamp;
   	   timestamp.SetTimestamp (Simulator::Now ());
   	   p->AddByteTag (timestamp);


  std::stringstream addressString;
  if (Ipv4Address::IsMatchingType(m_peerAddress) == true)
    {
      addressString << Ipv4Address::ConvertFrom (m_peerAddress);
    }
  else if (Ipv6Address::IsMatchingType(m_peerAddress) == true)
    {
      addressString << Ipv6Address::ConvertFrom (m_peerAddress);
    }
  else
    {
      addressString << m_peerAddress;
    }

  if ((m_socket->Send (p)) >= 0)
    {
	  m_txTrace (p);
      ++m_sent;
      NS_LOG_INFO ("Sent " << size << " bytes to "
                           << addressString.str ());
    }
  else
    {
//	  static uint32_t a = 1;
//	  std::cout<<"UdpTraceClient::SendPacket @@ "<<++a<<std::endl;
      NS_LOG_INFO ("Error while sending " << size << " bytes to "
                                          << addressString.str ());
    }
}

void
UdpTraceClient::Send (void)
{
  NS_LOG_FUNCTION (this);

  NS_ASSERT (m_sendEvent.IsExpired ());
  Ptr<Packet> p;
  struct TraceEntry *entry = &m_entries[m_currentEntry];
  do
    {
      for (uint32_t i = 0; i < entry->packetSize / m_maxPacketSize; i++)
        {
          SendPacket (m_maxPacketSize);
        }

      uint16_t sizetosend = entry->packetSize % m_maxPacketSize;
      SendPacket (sizetosend);

      m_currentEntry++;
      m_currentEntry %= m_entries.size ();
      entry = &m_entries[m_currentEntry];
    }
  while (entry->timeToSend == 0);
  m_sendEvent = Simulator::Schedule (MilliSeconds (entry->timeToSend), &UdpTraceClient::Send, this);
}


TypeId
od_TimestampTag::GetTypeId (void)
{
  static TypeId tid = TypeId ("od_TimestampTag")
    .SetParent<Tag> ()
    .AddConstructor<od_TimestampTag> ()
    .AddAttribute ("Timestamp",
                   "Some momentous point in time!",
                   EmptyAttributeValue (),
                   MakeTimeAccessor (&od_TimestampTag::GetTimestamp),
                   MakeTimeChecker ())
  ;
  return tid;
}

TypeId
od_TimestampTag::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
od_TimestampTag::GetSerializedSize (void) const
{
  return 8;
}
void
od_TimestampTag::Serialize (TagBuffer i) const
{
  int64_t t = m_timestamp.GetNanoSeconds ();
  i.Write ((const uint8_t *)&t, 8);
}
void
od_TimestampTag::Deserialize (TagBuffer i)
{
  int64_t t;
  i.Read ((uint8_t *)&t, 8);
  m_timestamp = NanoSeconds (t);
}

void
od_TimestampTag::SetTimestamp (Time time)
{
  m_timestamp = time;
}
Time
od_TimestampTag::GetTimestamp (void) const
{
  return m_timestamp;
}

void
od_TimestampTag::Print (std::ostream &os) const
{
  os << "t=" << m_timestamp;
}

} // Namespace ns3
