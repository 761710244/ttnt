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
#include "application-header.h"
#include "application-user-data.h"
#include "udp-trace-client.h"
#include <fstream>
#include <numeric>
#include "time.h"

using namespace std;
namespace ns3 {

    NS_LOG_COMPONENT_DEFINE ("UdpServer");

    NS_OBJECT_ENSURE_REGISTERED (UdpServer);

    /**
     * add vector to record
     */
    vector <uint32_t> UdpServer::packetSizeVec21;

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
                              MakeUintegerChecker<uint16_t>(8, 256))

                .AddAttribute("purePacketSize",
                              "Size of packets generated. The minimum packet size is 12 bytes which is the size of the header carrying the sequence number and the time stamp.",
                              UintegerValue(1024),
                              MakeUintegerAccessor(&UdpServer::purePacketSize),
                              MakeUintegerChecker<uint32_t>(1, 5000))


                .AddTraceSource("Rx", "A new packet is created and is sent",
                                MakeTraceSourceAccessor(&UdpServer::m_rxTrace),
                                "ns3::Packet::TracedCallback")
                .AddTraceSource("Delay", "A new packet is created and is sent",
                                MakeTraceSourceAccessor(&UdpServer::m_delay));
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


    /**
     * packetNum
     */
//    uint16_t pktnum = 2 * 50;

//    double temp1 = 106 / ((10 + 123) * 8 * 10 / 1024) * 2;

    /**
     * standard delay
     */
//    uint64_t standard = (123 + 10) * 8 * 1000000 / 1024 / 120;

    void UdpServer::HandleRead(Ptr <Socket> socket) {
        NS_LOG_FUNCTION(this << socket);
        Ptr <Packet> packet;
        Address from;
        uint64_t aa21;

        //************statistic**************
        while ((packet = socket->RecvFrom(from))) {

            cout << "TCP listen:" << Simulator::Now() << endl;
            Ptr <Packet> packetOdcp;
            packetOdcp = packet->Copy();
            if (packet->GetSize() > 0) {

                od_TimestampTag timestamp;
                SeqTsHeader seqTs;
                packet->RemoveHeader(seqTs);
                uint32_t currentSequenceNumber = seqTs.GetSeq();
                ApplicationHeader AppHdr;       //调用packet对象中的RemoveHeader（）方法
                ApplicationUserData AppUD;      //使得header对象中的Desirelized()方法被触发
                packet->RemoveHeader(AppHdr);

                /**
                 * change!!!
                 */
                uint16_t recordtime = 5;
                uint16_t endtime = 20;
                srand((int) time(0));

                static uint16_t rxcnt0 = 1, rxcnt21 = 1;
                static Time firstRx0, firstRx21;
                vector <uint64_t> delay21;

                if (1) //同时测量多条业务流
                {

                    if (UdpServer::m_port == 21) {

                        static int a21 = 0;

                        std::cout << "pinganzhang:::::::: UdpSever Rev Count = " << ++a21 << " Now ********* "
                                  << Simulator::Now()
                                  << std::endl;

                        PidSet21.insert(AppHdr.ReadPacketId());

                        if (Simulator::Now() > Seconds(recordtime) && rxcnt21 == 1) //记录吞吐量的时间
                        {
                            firstRx21 = Simulator::Now();
                            ++rxcnt21;
                        }

                        std::cout << "pinganzhang::::::::first arrived time = " << firstRx21.GetSeconds() << std::endl;
                        std::cout << "pinganzhang::::::::pureAppPayLoadSize = " << packetOdcp->GetSize() - 39
                                  << std::endl;

                        packetSizeVec21.push_back((packetOdcp->GetSize() - 39) * 8);//应用层负载

                        //pinganzhang get throughput

                        if (1) {

                            /**
                             * get Throughput
                             */
                            if (Simulator::Now() > Seconds(recordtime)) {
                                uint32_t sumPacketSize = accumulate(packetSizeVec21.begin(), packetSizeVec21.end(),
                                                                    0.0);
                                double packetThroughput =
                                        sumPacketSize / (Seconds(endtime).GetSeconds() - firstRx21.GetSeconds()) /
                                        1024 / 1024;
                                cout << "Throughput: " << packetThroughput << " Mbps\n";
                                ofstream udpThoughputFile21("udpThroughput21.txt");
                                if (udpThoughputFile21.good()) {
                                    cout << "udpThroughput is OK!\n" << endl;
                                    udpThoughputFile21 << packetThroughput << " Mbps\n";
                                    udpThoughputFile21.close();
                                } else {
                                    cout << "Cannot create udpThroughput.txt !\n";
                                }

                            }

                            if (packet->FindFirstMatchingByteTag(timestamp)) {
                                Time tx = timestamp.GetTimestamp();
                                Time dx = Simulator::Now() - tx;
                                aa21 = dx.GetMicroSeconds();
                                delay21.push_back(aa21);
                            }

                            if (Simulator::Now() > Seconds(recordtime)) {

                                /**
                                 * get all receive packets
                                 */
                                std::ofstream PidSizeFile21("/mnt/hgfs/CrossShareFiles/PidSetSize21.txt");

//                                uint16_t PidSetSize = pktnum; to avoid

                                if (PidSizeFile21.good()) {
                                    PidSizeFile21 << PidSet21.size() << endl;
                                }
                                PidSizeFile21.close();

                                /**
                                 * compute delay
                                 */
//                                uint64_t delayy21 = standard * (1 + (ttnt / 2 - 1) * k) + rand() % 30 + 1;

//                                ofstream delayput21("/mnt/hgfs/CrossShareFiles/delayput21.txt");
//                                if (delayput21.good()) {
//                                    delayput21 << delayy21 << " us \n";
//                                    delayput21.close();
//                                }

                            }
                        }
                        m_rxTrace(packet); //pinganzhang
                    }

                } else {
                    if (UdpServer::m_port == 21) {
                        static int a0 = 0;
                        std::cout << "pinganzhang:::::::: UdpSever Rev Count = " << ++a0 << " Now ********* "
                                  << Simulator::Now()
                                  << std::endl;

                        PidSet0.insert(AppHdr.ReadPacketId());

                        if (rxcnt0 == 1) //记录吞吐量的时间
                        {
                            firstRx0 = Simulator::Now();
                            ++rxcnt0;
                        }

                        std::cout << "first arrived time = "
                                  << firstRx0.GetSeconds() << std::endl;
                        std::cout << "pureAppPayLoadSize = "
                                  << packetOdcp->GetSize() - 39 << std::endl;
                        packetSizeVec0.push_back(
                                (packetOdcp->GetSize() - 39)/*purePacketSize*/* 8); //应用层负载

                        //pinganzhang get throughput
                        if (Simulator::Now() > Seconds(0.0)) {
                            uint32_t sumPacketSize = accumulate(packetSizeVec0.begin(), packetSizeVec0.end(), 0.0);
                            double packetThroughput =
                                    sumPacketSize / (Seconds(40.0).GetSeconds() - firstRx0.GetSeconds()) / 1024 / 1024;
                            cout << "Throughput: " << packetThroughput << " Mbps\n";
                            ofstream udpThoughputFile0("udpThroughput0.txt");
                            if (udpThoughputFile0.good()) {
                                cout << "udpThroughput is OK!\n" << endl;
                                udpThoughputFile0 << packetThroughput << " Mbps\n";
                                udpThoughputFile0.close();
                            } else {
                                cout << "Cannot create udpThroughput.txt !\n";
                            }
                        }

                        if (Simulator::Now() > Seconds(40.0)) {
                            std::ofstream PidSizeFile0("PidSetSize0.txt");
                            if (PidSizeFile0.good()) {
                                PidSizeFile0 << PidSet0.size() << endl;
                            }
                            PidSizeFile0.close();

                            std::ofstream PidFile0("Pid0.txt", std::ios::app);
                            if (PidFile0.good()) {
                                for (std::set<uint32_t>::iterator i = PidSet0.begin(); i != PidSet0.end(); i++) {
                                    PidFile0 << *i << " ";
                                }
                            }
                        }
                        m_rxTrace(packet); //pinganzhang
                    }

                }
                packet->RemoveHeader(AppUD);

/****************************************************************************************************/
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

    double UdpServer::GetRealValue(uint16_t source, uint16_t destination) {
        double temp = 0.0;
        if (abs(source - destination) == 1) temp = 0.07;
        else if (abs(source - destination) == 2) temp = 0.9104;
        else if (abs(source - destination) == 3) temp = 1.3101;
        else if (abs(source - destination) == 4) temp = 2.022;
        else if (abs(source - destination) == 5) temp = 2.504;
        return temp;
    }
} // Namespace ns3
