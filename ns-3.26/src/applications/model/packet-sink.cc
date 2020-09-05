/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright 2007 University of Washington
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
 * Author:  Tom Henderson (tomhend@u.washington.edu)
 */
#include "ns3/address.h"
#include "ns3/address-utils.h"
#include "ns3/log.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/node.h"
#include "ns3/socket.h"
#include "ns3/udp-socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/udp-socket-factory.h"
#include "packet-sink.h"
#include <numeric>
#include "seq-ts-header.h"
#include "application-header.h"
#include "application-user-data.h"


using namespace std;
namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("PacketSink");

NS_OBJECT_ENSURE_REGISTERED (PacketSink);
vector<uint32_t> PacketSink::packetSizeVec;

TypeId 
PacketSink::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::PacketSink")
    .SetParent<Application> ()
    .SetGroupName("Applications")
    .AddConstructor<PacketSink> ()
    .AddAttribute ("Local",
                   "The Address on which to Bind the rx socket.",
                   AddressValue (),
                   MakeAddressAccessor (&PacketSink::m_local),
                   MakeAddressChecker ())
    .AddAttribute ("Protocol",
                   "The type id of the protocol to use for the rx socket.",
                   TypeIdValue (UdpSocketFactory::GetTypeId ()),
                   MakeTypeIdAccessor (&PacketSink::m_tid),
                   MakeTypeIdChecker ())
    .AddTraceSource ("Rx",
                     "A packet has been received",
                     MakeTraceSourceAccessor (&PacketSink::m_rxTrace),
                     "ns3::Packet::AddressTracedCallback")
					 .AddTraceSource ("Rxx", "A new packet is created and is sent",
					    		MakeTraceSourceAccessor (&PacketSink::m_rxxTrace),
								"ns3::Packet::TracedCallback")
							    .AddTraceSource ("Delay", "A new packet is created and is sent",
							    		MakeTraceSourceAccessor (&PacketSink::m_delay))
  ;
  return tid;
}

PacketSink::PacketSink ()
{
  NS_LOG_FUNCTION (this);
  m_socket = 0;
  m_totalRx = 0;
}

PacketSink::~PacketSink()
{
  NS_LOG_FUNCTION (this);
}

uint64_t PacketSink::GetTotalRx () const
{
  NS_LOG_FUNCTION (this);
  return m_totalRx;
}

Ptr<Socket>
PacketSink::GetListeningSocket (void) const
{
  NS_LOG_FUNCTION (this);
  return m_socket;
}

std::list<Ptr<Socket> >
PacketSink::GetAcceptedSockets (void) const
{
  NS_LOG_FUNCTION (this);
  return m_socketList;
}

void PacketSink::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  m_socket = 0;
  m_socketList.clear ();

  // chain up
  Application::DoDispose ();
}


// Application Methods
void PacketSink::StartApplication ()    // Called at time specified by Start
{
  NS_LOG_FUNCTION (this);
  // Create the socket if not already
  if (!m_socket)
    {
      m_socket = Socket::CreateSocket (GetNode (), m_tid);
      m_socket->Bind (m_local);
      m_socket->Listen ();
      m_socket->ShutdownSend ();
      if (addressUtils::IsMulticast (m_local))
        {
          Ptr<UdpSocket> udpSocket = DynamicCast<UdpSocket> (m_socket);
          if (udpSocket)
            {
              // equivalent to setsockopt (MCAST_JOIN_GROUP)
              udpSocket->MulticastJoinGroup (0, m_local);
            }
          else
            {
              NS_FATAL_ERROR ("Error: joining multicast on a non-UDP socket");
            }
        }
    }

  m_socket->SetRecvCallback (MakeCallback (&PacketSink::HandleRead, this));
  m_socket->SetAcceptCallback (
    MakeNullCallback<bool, Ptr<Socket>, const Address &> (),
    MakeCallback (&PacketSink::HandleAccept, this));
  m_socket->SetCloseCallbacks (
    MakeCallback (&PacketSink::HandlePeerClose, this),
    MakeCallback (&PacketSink::HandlePeerError, this));
}

void PacketSink::StopApplication ()     // Called at time specified by Stop
{
  NS_LOG_FUNCTION (this);
  while(!m_socketList.empty ()) //these are accepted sockets, close them
    {
      Ptr<Socket> acceptedSocket = m_socketList.front ();
      m_socketList.pop_front ();
      acceptedSocket->Close ();
    }
  if (m_socket) 
    {
      m_socket->Close ();
      m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
    }
}

void PacketSink::HandleRead (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  Ptr<Packet> packet;
  Address from;
  uint64_t aaa;

  while ((packet = socket->RecvFrom (from)))
    {
      if (packet->GetSize () == 0)
        { //EOF
          break;
        }

	  static uint16_t rxcnt = 1;
	  static Time firstRx;
	  Ptr<Packet> packetOdcp;
	  packetOdcp = packet->Copy();

      od_TimestampTag timestamp;
      std::cout<<"m_localm_local = "<<InetSocketAddress::ConvertFrom (m_local).GetPort ()<<std::endl;
      if(/*Simulator::Now()>=Seconds(0.0)&&Simulator::Now()<=Seconds(51.0)&&*/
    		  InetSocketAddress::ConvertFrom (m_local).GetPort () == 10)
      {
       	  if(packet->FindFirstMatchingByteTag (timestamp))
        	  {
        			Time tx = timestamp.GetTimestamp();
        			Time dx = Simulator::Now() - tx;
        			aaa = dx.GetMicroSeconds();
        			m_delay(aaa);
        	  }
          m_rxxTrace (packet);
      }

      SeqTsHeader seqTs;
       packet->RemoveHeader (seqTs);
       uint32_t currentSequenceNumber = seqTs.GetSeq ();
       ApplicationHeader AppHdr;       //调用packet对象中的RemoveHeader（）方法
       ApplicationUserData AppUD;      //使得header对象中的Desirelized()方法被触发
       packet->RemoveHeader(AppHdr);

      if(InetSocketAddress::ConvertFrom (m_local).GetPort () == 10)
      {

          static int a = 0;
          std::cout<<"-------> PacketSink Rev Count = "<<++a<<std::endl;

          ofstream actuallyRevCntFile("actuallyRevCnt.txt");
                     if(actuallyRevCntFile.good())
                     {
                     	actuallyRevCntFile << a;
                     }
                     actuallyRevCntFile.close();

          PidSet.insert(AppHdr.ReadPacketId());




          if(rxcnt == 1) //记录吞吐量的时间
          {
          	firstRx = Simulator::Now();
          	++rxcnt;
          }

          std::cout<<"first arrived time = "<<firstRx.GetSeconds()<<std::endl;
          std::cout<<"pureAppPayLoadSize = "<<packetOdcp->GetSize ()-39<<std::endl;
          packetSizeVec.push_back((packetOdcp->GetSize ()-39)/*purePacketSize*/ * 8);//应用层负载

          //Odie get throughput
          if(Simulator::Now()>Seconds(495.0))
          {
          	uint32_t sumPacketSize = accumulate(packetSizeVec.begin(), packetSizeVec.end(), 0.0);
          	double packetThroughput = sumPacketSize / (Seconds(500.0).GetSeconds() - firstRx.GetSeconds()) / 1024 / 1024;
          	cout<<"Throughput: "<<packetThroughput<<" Mbps\n";
          	ofstream udpThoughputFile("tcpThroughput.txt");
          	if(udpThoughputFile.good())
          	{
          		cout<<"tcpThroughput is good!\n"<<endl;
          		udpThoughputFile << packetThroughput;
          		udpThoughputFile <<" Mbps\n";
          		udpThoughputFile.close();
          	}
          	else
          	{
          		cout<<"Cannot create udpThroughput.txt !\n";
          	}
          }
      }

      if(Simulator::Now()>Seconds(495.0))
      {
    	  std::ofstream PidSizeFile("PidSetSize.txt");
    	  if(PidSizeFile.good())
    	  {
    		  PidSizeFile << PidSet.size();
    	  }
          PidSizeFile.close();

          std::ofstream PidFile("Pid.txt", std::ios::app);
          if(PidFile.good())
          {
        	  for(std::set<uint32_t>::iterator i = PidSet.begin(); i != PidSet.end(); i++)
        	  {
        		  PidFile << *i <<" ";
        	  }
        	  PidFile << "\n-----------------------------------------------------------------------\n";
          }
      }

      m_totalRx += packet->GetSize ();
      if (InetSocketAddress::IsMatchingType (from))
        {
          NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds ()
                       << "s packet sink received "
                       <<  packet->GetSize () << " bytes from "
                       << InetSocketAddress::ConvertFrom(from).GetIpv4 ()
                       << " port " << InetSocketAddress::ConvertFrom (from).GetPort ()
                       << " total Rx " << m_totalRx << " bytes" << " currentSequenceNumber "<<currentSequenceNumber);
        }
      else if (Inet6SocketAddress::IsMatchingType (from))
        {
          NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds ()
                       << "s packet sink received "
                       <<  packet->GetSize () << " bytes from "
                       << Inet6SocketAddress::ConvertFrom(from).GetIpv6 ()
                       << " port " << Inet6SocketAddress::ConvertFrom (from).GetPort ()
                       << " total Rx " << m_totalRx << " bytes");
        }
      m_rxTrace (packet, from);
    }
}


void PacketSink::HandlePeerClose (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
}
 
void PacketSink::HandlePeerError (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
}
 

void PacketSink::HandleAccept (Ptr<Socket> s, const Address& from)
{
  NS_LOG_FUNCTION (this << s << from);
  s->SetRecvCallback (MakeCallback (&PacketSink::HandleRead, this));
  m_socketList.push_back (s);
}

} // Namespace ns3
