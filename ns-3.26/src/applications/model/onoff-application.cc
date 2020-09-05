/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
//
// Copyright (c) 2006 Georgia Tech Research Corporation
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation;
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
// Author: George F. Riley<riley@ece.gatech.edu>
//

// ns3 - On/Off Data Source Application class
// George F. Riley, Georgia Tech, Spring 2007
// Adapted from ApplicationOnOff in GTNetS.

#include "ns3/log.h"
#include "ns3/address.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/packet-socket-address.h"
#include "ns3/node.h"
#include "ns3/nstime.h"
#include "ns3/data-rate.h"
#include "ns3/random-variable-stream.h"
#include "ns3/socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/trace-source-accessor.h"
#include "onoff-application.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/string.h"
#include "ns3/pointer.h"

#include "seq-ts-header.h"
#include "application-header.h"
#include "application-user-data.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("OnOffApplication");

NS_OBJECT_ENSURE_REGISTERED (OnOffApplication);

TypeId
OnOffApplication::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::OnOffApplication")
    .SetParent<Application> ()
    .SetGroupName("Applications")
    .AddConstructor<OnOffApplication> ()
    .AddAttribute ("DataRate", "The data rate in on state.",
                   DataRateValue (DataRate ("500kb/s")),
                   MakeDataRateAccessor (&OnOffApplication::m_cbrRate),
                   MakeDataRateChecker ())
    .AddAttribute ("PacketSize", "The size of packets sent in on state",
                   UintegerValue (1024),
                   MakeUintegerAccessor (&OnOffApplication::m_pktSize),
                   MakeUintegerChecker<uint32_t> (1,5000))
    .AddAttribute ("Remote", "The address of the destination",
                   AddressValue (),
                   MakeAddressAccessor (&OnOffApplication::m_peer),
                   MakeAddressChecker ())
    .AddAttribute ("OnTime", "A RandomVariableStream used to pick the duration of the 'On' state.",
                   StringValue ("ns3::ConstantRandomVariable[Constant=1.0]"),
                   MakePointerAccessor (&OnOffApplication::m_onTime),
                   MakePointerChecker <RandomVariableStream>())
    .AddAttribute ("OffTime", "A RandomVariableStream used to pick the duration of the 'Off' state.",
                   StringValue ("ns3::ConstantRandomVariable[Constant=1.0]"),
                   MakePointerAccessor (&OnOffApplication::m_offTime),
                   MakePointerChecker <RandomVariableStream>())
    .AddAttribute ("MaxBytes", 
                   "The total number of bytes to send. Once these bytes are sent, "
                   "no packet is sent again, even in on state. The value zero means "
                   "that there is no limit.",
                   UintegerValue (0),
                   MakeUintegerAccessor (&OnOffApplication::m_maxBytes),
                   MakeUintegerChecker<uint64_t> ())

				   .AddAttribute("Od_Interval",
		                   "The time to wait between packets", TimeValue (Seconds (1.0)),
		                   MakeTimeAccessor (&OnOffApplication::od_interval),
		                   MakeTimeChecker ())


    .AddAttribute ("Protocol", "The type of protocol to use.",
                   TypeIdValue (UdpSocketFactory::GetTypeId ()),
                   MakeTypeIdAccessor (&OnOffApplication::m_tid),
                   MakeTypeIdChecker ())
    .AddTraceSource ("Tx", "A new packet is created and is sent",
                     MakeTraceSourceAccessor (&OnOffApplication::m_txTrace),
                     "ns3::Packet::TracedCallback")
					 .AddTraceSource ("Rx", "A new packet is created and is sent",
					                     MakeTraceSourceAccessor (&OnOffApplication::m_rxTrace),
					                     "ns3::Packet::TracedCallback")
  ;
  return tid;
}


OnOffApplication::OnOffApplication ()
  : m_socket (0),
    m_connected (false),
    m_residualBits (0),
    m_lastStartTime (Seconds (0)),
    m_totBytes (0)
{
  NS_LOG_FUNCTION (this);
  m_sent = 0;
}

OnOffApplication::~OnOffApplication()
{
  NS_LOG_FUNCTION (this);
}


/*******************************************************************************************/
void
OnOffApplication::SetNodeID (uint32_t NodeID)   //添加一对Set和Get方法使得udp-client-server-helper.cc中的
{                                        //NodeID可以传递给UdpClient对象中的成员变量m_NodeID
	m_NodeID = NodeID;                   //以便application-user-data.cc对定位字节读写时可以获得对应节点
}
uint32_t
OnOffApplication::GetNodeID (void)
{
	return m_NodeID;
}

/*******************************************************************************************/



void 
OnOffApplication::SetMaxBytes (uint64_t maxBytes)
{
  NS_LOG_FUNCTION (this << maxBytes);
  m_maxBytes = maxBytes;
}

Ptr<Socket>
OnOffApplication::GetSocket (void) const
{
  NS_LOG_FUNCTION (this);
  return m_socket;
}

int64_t 
OnOffApplication::AssignStreams (int64_t stream)
{
  NS_LOG_FUNCTION (this << stream);
  m_onTime->SetStream (stream);
  m_offTime->SetStream (stream + 1);
  return 2;
}

void
OnOffApplication::DoDispose (void)
{
  NS_LOG_FUNCTION (this);

  m_socket = 0;
  // chain up
  Application::DoDispose ();
}

// Application Methods
void OnOffApplication::StartApplication () // Called at time specified by Start
{
  NS_LOG_FUNCTION (this);

  // Create the socket if not already
  if (!m_socket)
    {
      m_socket = Socket::CreateSocket (GetNode (), m_tid);
      if (Inet6SocketAddress::IsMatchingType (m_peer))
        {
          m_socket->Bind6 ();
        }
      else if (InetSocketAddress::IsMatchingType (m_peer) ||
               PacketSocketAddress::IsMatchingType (m_peer))
        {
          m_socket->Bind ();
        }
      m_socket->Connect (m_peer);
      m_socket->SetAllowBroadcast (true);
      m_socket->ShutdownRecv ();

      m_socket->SetConnectCallback (
        MakeCallback (&OnOffApplication::ConnectionSucceeded, this),
        MakeCallback (&OnOffApplication::ConnectionFailed, this));
    }
  m_cbrRateFailSafe = m_cbrRate;

  // Insure no pending event
  CancelEvents ();
  // If we are not yet connected, there is nothing to do here
  // The ConnectionComplete upcall will start timers at that time
  //if (!m_connected) return;
  ScheduleStartEvent ();
}

void OnOffApplication::StopApplication () // Called at time specified by Stop
{
  NS_LOG_FUNCTION (this);

  CancelEvents ();
  if(m_socket != 0)
    {
      m_socket->Close ();
    }
  else
    {
      NS_LOG_WARN ("OnOffApplication found null socket to close in StopApplication");
    }
}

void OnOffApplication::CancelEvents ()
{
  NS_LOG_FUNCTION (this);

  if (m_sendEvent.IsRunning () && m_cbrRateFailSafe == m_cbrRate )
    { // Cancel the pending send packet event
      // Calculate residual bits since last packet sent
      Time delta (Simulator::Now () - m_lastStartTime);
      int64x64_t bits = delta.To (Time::S) * m_cbrRate.GetBitRate ();
      m_residualBits += bits.GetHigh ();
    }
  m_cbrRateFailSafe = m_cbrRate;
  Simulator::Cancel (m_sendEvent);
  Simulator::Cancel (m_startStopEvent);
}

// Event handlers
void OnOffApplication::StartSending ()
{
  NS_LOG_FUNCTION (this);
  m_lastStartTime = Simulator::Now ();
  ScheduleNextTx ();  // Schedule the send packet event
  ScheduleStopEvent ();
}

void OnOffApplication::StopSending ()
{
  NS_LOG_FUNCTION (this);
  CancelEvents ();

  ScheduleStartEvent ();
}

// Private helpers
void OnOffApplication::ScheduleNextTx ()
{
  NS_LOG_FUNCTION (this);

  if (m_maxBytes == 0 || m_totBytes < m_maxBytes)
    {
      uint32_t bits = m_pktSize * 8 - m_residualBits;
      std::cout<<"m_pktSize = "<<m_pktSize<<" m_residualBits = "<<m_residualBits<<std::endl;
      NS_LOG_LOGIC ("bits = " << bits);
      Time nextTime (Seconds (bits /
                              static_cast<double>(m_cbrRate.GetBitRate ()))); // Time till next packet
      NS_LOG_LOGIC ("nextTime = " << nextTime);
      m_sendEvent = Simulator::Schedule (nextTime,
                                         &OnOffApplication::SendPacket, this); //od_interval
    }
  else
    { // All done, cancel any pending events
      StopApplication ();
    }
}

void OnOffApplication::ScheduleStartEvent ()
{  // Schedules the event to start sending data (switch to the "On" state)
  NS_LOG_FUNCTION (this);

  Time offInterval = Seconds (m_offTime->GetValue ());
  NS_LOG_LOGIC ("start at " << offInterval);
  m_startStopEvent = Simulator::Schedule (offInterval, &OnOffApplication::StartSending, this);
}

void OnOffApplication::ScheduleStopEvent ()
{  // Schedules the event to stop sending data (switch to "Off" state)
  NS_LOG_FUNCTION (this);

  Time onInterval = Seconds (m_onTime->GetValue ());
  NS_LOG_LOGIC ("stop at " << onInterval);
  m_startStopEvent = Simulator::Schedule (onInterval, &OnOffApplication::StopSending, this);
}


void OnOffApplication::SendPacket ()
{
  NS_LOG_FUNCTION (this);

  Address cpAddr = m_peer;
  InetSocketAddress transport_Cp = InetSocketAddress::ConvertFrom (cpAddr);
  std::cout << " OnOffApplication::SendPacket transport_Cp Ipv4 = " << transport_Cp.GetIpv4()
		  << " Port = " << transport_Cp.GetPort() << std::endl;

  NS_ASSERT (m_sendEvent.IsExpired ());
  ++m_sent;
  SeqTsHeader seqTs;
  seqTs.SetSeq(m_sent);

  uint32_t MessageNumber;
 uint32_t size_temp =m_pktSize;

     if (m_pktSize == 150)              // 512          //由于定位消息一般较短且需要定周期发送，因此对于小于512Byte的
   	  {                                       //用户数据在应用头中对应的bit位设置为0
     	MessageNumber = 0;
     	size_temp = 0;
   	  }
     else if (m_pktSize == 300)       //    1024        //用于将来语音和图像应用的数据发送
   	  {
     	MessageNumber = 1;
     	size_temp = 0;
   	  }
     else if ((m_pktSize == 10) || (m_pktSize == 100))       // 1500
   	  {
     	MessageNumber = 2;
     	size_temp = 0;
   	  }
     else if(m_pktSize== 1000) // ***************
     {
     	MessageNumber = 2;
     	size_temp = 0;
     }
     else if(m_pktSize==1) // This type is ready for building route
     {
     	MessageNumber = 2;
     	size_temp = 0;
     }

     std::ofstream write_type;
     write_type.open("AppType.txt");
     if(write_type.good())
     {
   	  write_type<<MessageNumber;
   	  write_type.close();
     }
     else
     {
   	  std::cout<<"Open AppType.txt Failed ! "<<std::endl;
     }

     Ptr<Packet> packet;
     if(MessageNumber == 0)
     {
    	 packet = Create<Packet> (size_temp);
     }
     else
     {
    	 packet = Create<Packet> (m_pktSize);
     }

     time_t timeStamp = time(NULL);                                //将系统时间进行读取并写入应用头中
     tm* Time1= localtime(&timeStamp);
       uint8_t time_year_val = 128 | Time1->tm_year;
       uint32_t time_month_val = 0;
       time_month_val += (Time1->tm_mon + 1) << 28;
       time_month_val += (Time1->tm_mday) << 23;
       time_month_val += (Time1->tm_hour) << 18;
       time_month_val += (Time1->tm_min) << 12;
       time_month_val += (Time1->tm_sec) << 6;
     ApplicationHeader AppHdr;
     AppHdr.SetVMFmessageIdentificationGroup_MessageNumber (MessageNumber);//4
     AppHdr.SetOriginatorDataTimeGroup_GPI_Year (time_year_val);//1
     AppHdr.SetOriginatorDataTimeGroup_Complementation (time_month_val);//4

     AppHdr.SetPacketId(od_PID[InetSocketAddress::ConvertFrom (m_peer).GetPort ()]);
     od_PID[InetSocketAddress::ConvertFrom (m_peer).GetPort ()]++;

     ApplicationUserData AppUD;
     AppUD.SetNodeID(m_NodeID);    //将对应的NodeID传递给定位消息的发送端
     if(MessageNumber==0)
     {
    	 packet->AddHeader(AppUD);          //将包含有定位信息的负载加入packet
     }

     packet->AddHeader (AppHdr);

     packet->AddHeader (seqTs);
     if(InetSocketAddress::ConvertFrom (m_peer).GetPort () == 10)
     m_txTrace (packet);
  //**********************************************ODD
  od_TimestampTag timestamp;
  timestamp.SetTimestamp (Simulator::Now ());
  packet->AddByteTag (timestamp);

  m_socket->Send (packet);
  m_totBytes += m_pktSize;
  if (InetSocketAddress::IsMatchingType (m_peer))
    {
      NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds ()
                   << "s on-off application sent "
                   <<  packet->GetSize () << " bytes to "
                   << InetSocketAddress::ConvertFrom(m_peer).GetIpv4 ()
                   << " port " << InetSocketAddress::ConvertFrom (m_peer).GetPort ()
                   << " total Tx " << m_totBytes << " bytes");
    }
  else if (Inet6SocketAddress::IsMatchingType (m_peer))
    {
      NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds ()
                   << "s on-off application sent "
                   <<  packet->GetSize () << " bytes to "
                   << Inet6SocketAddress::ConvertFrom(m_peer).GetIpv6 ()
                   << " port " << Inet6SocketAddress::ConvertFrom (m_peer).GetPort ()
                   << " total Tx " << m_totBytes << " bytes");
    }
  m_lastStartTime = Simulator::Now ();
  m_residualBits = 0;
  ScheduleNextTx ();
}


void OnOffApplication::ConnectionSucceeded (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  m_connected = true;
}

void OnOffApplication::ConnectionFailed (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
}


} // Namespace ns3
