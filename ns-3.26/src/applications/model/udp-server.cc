/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 *  Copyright (c) 2007,2008,2009 INRIA, UDCAST
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
 * Author: Amine Ismail <amine.ismail@sophia.inria.fr>
 *                      <amine.ismail@udcast.com>
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
#include "packet-loss-counter.h"

#include "seq-ts-header.h"
#include "udp-server.h"

#include "vector"
#include "set"
#include "fstream"
#include "numeric"
#include "udp-trace-client.h"
using namespace std;

namespace ns3 {

    NS_LOG_COMPONENT_DEFINE ("UdpServer");

    NS_OBJECT_ENSURE_REGISTERED (UdpServer);

    TypeId
    UdpServer::GetTypeId(void) {
        static TypeId tid = TypeId("ns3::UdpServer")
                .SetParent<Application>()
                .SetGroupName("Applications")
                .AddConstructor<UdpServer>()
                .AddAttribute("Port",
                              "Port on which we listen for incoming packets.",
                              UintegerValue(100),
                              MakeUintegerAccessor(&UdpServer::m_port),
                              MakeUintegerChecker<uint16_t>())
                .AddAttribute("PacketWindowSize",
                              "The size of the window used to compute the packet loss. This value should be a multiple of 8.",
                              UintegerValue(32),
                              MakeUintegerAccessor(&UdpServer::GetPacketWindowSize,
                                                   &UdpServer::SetPacketWindowSize),
                              MakeUintegerChecker<uint16_t>(8, 256));
        return tid;
    }

    UdpServer::UdpServer()
            : m_lossCounter(0) {
        NS_LOG_FUNCTION(this);
        m_received = 0;
    }

    UdpServer::~UdpServer() {
        NS_LOG_FUNCTION(this);
    }

    uint16_t
    UdpServer::GetPacketWindowSize() const {
        NS_LOG_FUNCTION(this);
        return m_lossCounter.GetBitMapSize();
    }

    void
    UdpServer::SetPacketWindowSize(uint16_t size) {
        NS_LOG_FUNCTION(this << size);
        m_lossCounter.SetBitMapSize(size);
    }

    uint32_t
    UdpServer::GetLost(void) const {
        NS_LOG_FUNCTION(this);
        return m_lossCounter.GetLost();
    }

    uint64_t
    UdpServer::GetReceived(void) const {
        NS_LOG_FUNCTION(this);
        return m_received;
    }

    void
    UdpServer::DoDispose(void) {
        NS_LOG_FUNCTION(this);
        Application::DoDispose();
    }

    void
    UdpServer::StartApplication(void) {
        NS_LOG_FUNCTION(this);

        if (m_socket == 0) {
            TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
            m_socket = Socket::CreateSocket(GetNode(), tid);
            InetSocketAddress local = InetSocketAddress(Ipv4Address::GetAny(),
                                                        m_port);
            m_socket->Bind(local);
        }

        m_socket->SetRecvCallback(MakeCallback(&UdpServer::HandleRead, this));

        if (m_socket6 == 0) {
            TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
            m_socket6 = Socket::CreateSocket(GetNode(), tid);
            Inet6SocketAddress local = Inet6SocketAddress(Ipv6Address::GetAny(),
                                                          m_port);
            m_socket6->Bind(local);
        }

        m_socket6->SetRecvCallback(MakeCallback(&UdpServer::HandleRead, this));

    }

    void
    UdpServer::StopApplication() {
        NS_LOG_FUNCTION(this);

        if (m_socket != 0) {
            m_socket->SetRecvCallback(MakeNullCallback < void, Ptr < Socket > > ());
        }
    }

    /**************************************************************************************/
    vector <uint32_t> UdpServer::packetSizeVec21;

    void
    UdpServer::HandleRead(Ptr <Socket> socket) {
        NS_LOG_FUNCTION(this << socket);
        Ptr <Packet> packet;
        Address from;

        uint16_t ttnt = 2;
        uint16_t record_start[30] = {0};
        uint16_t record_end[30] = {0};

        for (uint16_t i = 1; i <= (ttnt / 2); i++) {

            record_start[i] = (i - 1) * 57 + 6;
            record_end[i] = record_start[i] + 50;
        }

        /**************************************************************************************/
        static uint16_t rxcnt21 = 1;

        static Time firstRx21;

        vector <uint64_t> delay21;


        /**************************************************************************************/
        while ((packet = socket->RecvFrom(from))) {

            cout << "TCP listen:" << Simulator::Now() << endl;
            Ptr <Packet> packetOdcp;
            packetOdcp = packet->Copy();

            if (packet->GetSize() > 0) {
                SeqTsHeader seqTs;
                packet->RemoveHeader(seqTs);
                uint32_t currentSequenceNumber = seqTs.GetSeq();

                /**
                 * workflow[1]
                 */
                if (UdpServer::m_port == 21) {

                    static int a21 = 0;

                    cout << "pinganzhang:: UdpSever Rev Count = " << ++a21 << " Now ********* " << Simulator::Now()
                         << std::endl;

                    PidSet21.insert(a21);

                    if (rxcnt21 == 1) //记录吞吐量的时间
                    {
                        firstRx21 = Simulator::Now();
                        ++rxcnt21;
                    }

                    cout << "port :" << m_port << " first arrived time = " << firstRx21.GetSeconds() << std::endl;
                    cout << "pureAppPayLoadSize = " << packetOdcp->GetSize() - 39 << std::endl;
                    packetSizeVec21.push_back((packetOdcp->GetSize() - 39)/*purePacketSize*/ * 8);//应用层负载

                    //  get throughput
                    if (1) {

                        if (Simulator::Now() > Seconds(record_start[1])) {
                            uint32_t sumPacketSize = accumulate(packetSizeVec21.begin(), packetSizeVec21.end(), 0.0);
                            double packetThroughput =
                                    sumPacketSize / (Seconds(record_end[1]).GetSeconds() - firstRx21.GetSeconds()) /
                                    1024;
                            cout << "Throughput: " << packetThroughput << " Kbps\n";
                            ofstream udpThoughputFile21("udpThroughput21.txt");
                            if (udpThoughputFile21.good()) {
                                cout << "udpThroughput is good!\n" << endl;
                                udpThoughputFile21 << packetThroughput << " Kbps\n";
                                udpThoughputFile21.close();
                            } else {
                                cout << "Cannot create udpThroughput.txt !\n";
                            }
                        }

                        if (Simulator::Now() > Seconds(record_start[1])) {
                            std::ofstream PidSizeFile21("PidSetSize21.txt");
                            if (PidSizeFile21.good()) {
                                PidSizeFile21 << PidSet21.size() << endl;
                            }
                            PidSizeFile21.close();
                        }
                    }
                }


                /**************************************************************************************/
                if (InetSocketAddress::IsMatchingType(from)) {
                    NS_LOG_INFO("TraceDelay: RX " << packet->GetSize() <<
                                                  " bytes from " << InetSocketAddress::ConvertFrom(from).GetIpv4() <<
                                                  " Sequence Number: " << currentSequenceNumber <<
                                                  " Uid: " << packet->GetUid() <<
                                                  " TXtime: " << seqTs.GetTs() <<
                                                  " RXtime: " << Simulator::Now() <<
                                                  " Delay: " << Simulator::Now() - seqTs.GetTs());
                } else if (Inet6SocketAddress::IsMatchingType(from)) {
                    NS_LOG_INFO("TraceDelay: RX " << packet->GetSize() <<
                                                  " bytes from " << Inet6SocketAddress::ConvertFrom(from).GetIpv6() <<
                                                  " Sequence Number: " << currentSequenceNumber <<
                                                  " Uid: " << packet->GetUid() <<
                                                  " TXtime: " << seqTs.GetTs() <<
                                                  " RXtime: " << Simulator::Now() <<
                                                  " Delay: " << Simulator::Now() - seqTs.GetTs());
                }

                m_lossCounter.NotifyReceived(currentSequenceNumber);
                m_received++;
            }
        }
    }

} // Namespace ns3