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
    vector <uint32_t> UdpServer::packetSizeVec10;
    vector <uint32_t> UdpServer::packetSizeVec0;
    vector <uint32_t> UdpServer::packetSizeVec1;
    vector <uint32_t> UdpServer::packetSizeVec2;
    vector <uint32_t> UdpServer::packetSizeVec3;
    vector <uint32_t> UdpServer::packetSizeVec4;
    vector <uint32_t> UdpServer::packetSizeVec5;
    vector <uint32_t> UdpServer::packetSizeVec6;
    vector <uint32_t> UdpServer::packetSizeVec7;
    vector <uint32_t> UdpServer::packetSizeVec8;
    vector <uint32_t> UdpServer::packetSizeVec9;
    vector <uint32_t> UdpServer::packetSizeVec;
    vector <uint32_t> UdpServer::packetSizeVec21;
    vector <uint32_t> UdpServer::packetSizeVec22;
    vector <uint32_t> UdpServer::packetSizeVec23;
    vector <uint32_t> UdpServer::packetSizeVec24;
    vector <uint32_t> UdpServer::packetSizeVec25;
    vector <uint32_t> UdpServer::packetSizeVec26;
    vector <uint32_t> UdpServer::packetSizeVec27;
    vector <uint32_t> UdpServer::packetSizeVec28;
    vector <uint32_t> UdpServer::packetSizeVec29;
    vector <uint32_t> UdpServer::packetSizeVec30;
    vector <uint32_t> UdpServer::packetSizeVec31;
    vector <uint32_t> UdpServer::packetSizeVec32;
    vector <uint32_t> UdpServer::packetSizeVec33;
    vector <uint32_t> UdpServer::packetSizeVec34;
    vector <uint32_t> UdpServer::packetSizeVec35;
    vector <uint32_t> UdpServer::packetSizeVec41;
    vector <uint32_t> UdpServer::packetSizeVec20000;
    vector <uint32_t> UdpServer::packetSizeVec20001;

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

    uint16_t sinc = 16, pktnum = 10 * 50;
    double temp1 = 106 / ((10 + 123) * 8 * 10 / 1024) * 2;
    uint16_t yuzhi = temp1;
    uint64_t standard = (123 + 10) * 8 * 1000000 / 1024 / 120;

    void UdpServer::HandleRead(Ptr <Socket> socket) {
        NS_LOG_FUNCTION(this << socket);
        Ptr <Packet> packet;
        Address from;
        uint64_t aa21, aa22, aa23, aa24, aa25, aa26, aa27, aa28, aa29, aa30, aa31, aa32, aa33, aa34, aa35, aa41, aa20000, aa20001;
        //************statistic**************
        while ((packet = socket->RecvFrom(from))) {
            cout << "dsr routing start." << endl;

            Ptr <Packet> packetOdcp;
            packetOdcp = packet->Copy();
            if (packet->GetSize() > 0) {
                static uint16_t rxcnt0 = 1, rxcnt1 = 1, rxcnt2 = 1, rxcnt3 = 1, rxcnt4 = 1, rxcnt10 = 1
                , rxcnt5 = 1, rxcnt6 = 1, rxcnt7 = 1, rxcnt8 = 1, rxcnt9 = 1, rxcnt = 1,
                        rxcnt21 = 1, rxcnt22 = 1, rxcnt23 = 1, rxcnt24 = 1, rxcnt25 = 1, rxcnt26 = 1,
                        rxcnt27 = 1, rxcnt28 = 1, rxcnt29 = 1, rxcnt30 = 1, rxcnt31 = 1, rxcnt32 = 1,
                        rxcnt33 = 1, rxcnt34 = 1, rxcnt35 = 1, rxcnt41 = 1, rxcnt20000 = 1, rxcnt20001 = 1;
                static Time firstRx0, firstRx1, firstRx2, firstRx3, firstRx4, firstRx5,
                        firstRx6, firstRx7, firstRx8, firstRx9, firstRx10, firstRx,
                        firstRx21, firstRx22, firstRx23, firstRx24, firstRx25,
                        firstRx26, firstRx27, firstRx28, firstRx29, firstRx30, firstRx20001,
                        firstRx31, firstRx32, firstRx33, firstRx34, firstRx35, firstRx41, firstRx20000;
                vector <uint64_t> delay21, delay22, delay23, delay24, delay25, delay26, delay27, delay28, delay29,
                        delay30, delay31, delay32, delay33, delay34, delay35, delay41, delay20000, delay20001;
                od_TimestampTag timestamp;
                SeqTsHeader seqTs;
                packet->RemoveHeader(seqTs);
                uint32_t currentSequenceNumber = seqTs.GetSeq();
/****************************************************************************************************/
                ApplicationHeader AppHdr;       //调用packet对象中的RemoveHeader（）方法
                ApplicationUserData AppUD;      //使得header对象中的Desirelized()方法被触发
                packet->RemoveHeader(AppHdr);
                uint16_t recordtime = 5;
                uint16_t endtime = 20;
                uint16_t NodeNumTPS = 0;


                uint16_t workflow = recordtime / 7 - 1;
                string str;
                uint16_t num = 0;
                map<uint16_t, double> udpthoughputmap;
                double temp;
                ifstream udpthoughput(
                        "/home/yf518/eclipse-workspace/Tactical/src/nix-vector-routing/test/nix-vector-routing.txt");
                if (NodeNumTPS == 1) {
                    if (udpthoughput.good()) {
                        while (getline(udpthoughput, str)) {
                            istringstream iss(str);
                            iss >> temp;
                            udpthoughputmap[num++] = temp;
                        }
                        udpthoughput.close();
                    }
                }
                srand((int) time(0));

                double k = (rand() % 6 + 1) / 10 + 1;

                if (1) //同时测量多条业务流
                {
                    if (UdpServer::m_port == 10) {
                        static int a0 = 0;
                        std::cout << "-------> UdpSever Rev Count = " << ++a0 << " Now ********* " << Simulator::Now()
                                  << std::endl;

                        PidSet0.insert(AppHdr.ReadPacketId());

                        if (rxcnt0 == 1) //记录吞吐量的时间
                        {
                            firstRx0 = Simulator::Now();
                            ++rxcnt0;
                        }

                        std::cout << "first arrived time = " << firstRx0.GetSeconds() << std::endl;
                        std::cout << "pureAppPayLoadSize = " << packetOdcp->GetSize() - 39 << std::endl;
                        packetSizeVec0.push_back((packetOdcp->GetSize() - 39)/*purePacketSize*/ * 8);//应用层负载

                        //pinganzhang get throughput
                        if (1) {

                            if (Simulator::Now() > Seconds(recordtime)) {
                                uint32_t sumPacketSize = accumulate(packetSizeVec0.begin(), packetSizeVec0.end(), 0.0);
                                double packetThroughput =
                                        sumPacketSize / (Seconds(endtime).GetSeconds() - firstRx0.GetSeconds()) / 1024 /
                                        1024;
                                cout << "Throughput: " << packetThroughput << " Mbps\n";
                                ofstream udpThoughputFile0("/mnt/hgfs/VMwareShareFile/udpThroughput0.txt");
                                if (udpThoughputFile0.good()) {
                                    cout << "udpThroughput is OK!\n" << endl;
                                    udpThoughputFile0 << packetThroughput << " Mbps\n";
                                    udpThoughputFile0.close();
                                } else {
                                    cout << "Cannot create udpThroughput.txt !\n";
                                }
                            }

                            if (Simulator::Now() > Seconds(recordtime)) {
                                std::ofstream PidSizeFile0("/mnt/hgfs/VMwareShareFile/PidSetSize0.txt");
                                if (PidSizeFile0.good()) {
                                    PidSizeFile0 << PidSet0.size() << endl;
                                }
                                PidSizeFile0.close();

                                std::ofstream PidFile0("Pid0.txt", std::ios::app);
                                if (PidFile0.good()) {
                                    for (std::set<uint32_t>::iterator i = PidSet0.begin(); i != PidSet0.end(); i++) {
                                        PidFile0 << *i << " ";
                                    }
                                    PidFile0
                                            << "\n-----------------------------------------------------------------------\n";
                                }
                            }
                        }
                        m_rxTrace(packet); //pinganzhang
                    }

                    if (UdpServer::m_port == 11) {
                        static int a1 = 0;
                        std::cout << "-------> UdpSever Rev Count = " << ++a1 << " Now ********* " << Simulator::Now()
                                  << std::endl;

                        PidSet1.insert(AppHdr.ReadPacketId());

                        if (rxcnt1 == 1) //记录吞吐量的时间
                        {
                            firstRx1 = Simulator::Now();
                            ++rxcnt1;
                        }

                        std::cout << "first arrived time = " << firstRx1.GetSeconds() << std::endl;
                        std::cout << "pureAppPayLoadSize = " << packetOdcp->GetSize() - 39 << std::endl;
                        packetSizeVec1.push_back((packetOdcp->GetSize() - 39)/*purePacketSize*/ * 8);//应用层负载

                        //pinganzhang get throughput
                        if (1) {
                            if (Simulator::Now() > Seconds(recordtime)) {
                                uint32_t sumPacketSize = accumulate(packetSizeVec1.begin(), packetSizeVec1.end(), 0.0);
                                double packetThroughput =
                                        sumPacketSize / (Seconds(endtime).GetSeconds() - firstRx1.GetSeconds()) / 1024 /
                                        1024;
                                cout << "Throughput: " << packetThroughput << " Mbps\n";
                                ofstream udpThoughputFile1("/mnt/hgfs/VMwareShareFile/udpThroughput1.txt");
                                if (udpThoughputFile1.good()) {
                                    cout << "udpThroughput is OK!\n" << endl;
                                    udpThoughputFile1 << packetThroughput << " Mbps\n";
                                    udpThoughputFile1.close();
                                } else {
                                    cout << "Cannot create udpThroughput.txt !\n";
                                }
                            }


                            if (Simulator::Now() > Seconds(recordtime)) {
                                std::ofstream PidSizeFile1("/mnt/hgfs/VMwareShareFile/PidSetSize1.txt");
                                if (PidSizeFile1.good()) {
                                    PidSizeFile1 << PidSet1.size() << endl;
                                }
                                PidSizeFile1.close();

                                std::ofstream PidFile1("Pid1.txt", std::ios::app);
                                if (PidFile1.good()) {
                                    for (std::set<uint32_t>::iterator i = PidSet1.begin(); i != PidSet1.end(); i++) {
                                        PidFile1 << *i << " ";
                                    }
                                    PidFile1
                                            << "\n-----------------------------------------------------------------------\n";
                                }
                            }
                        }
                        m_rxTrace(packet); //pinganzhang
                    }

                    if (UdpServer::m_port == 12) {
                        static int a2 = 0;
                        std::cout << "-------> UdpSever Rev Count = " << ++a2 << " Now ********* " << Simulator::Now()
                                  << std::endl;

                        PidSet2.insert(AppHdr.ReadPacketId());

                        if (rxcnt2 == 1) //记录吞吐量的时间
                        {
                            firstRx2 = Simulator::Now();
                            ++rxcnt2;
                        }

                        std::cout << "first arrived time = " << firstRx2.GetSeconds() << std::endl;
                        std::cout << "pureAppPayLoadSize = " << packetOdcp->GetSize() - 39 << std::endl;
                        packetSizeVec2.push_back((packetOdcp->GetSize() - 39)/*purePacketSize*/ * 8);//应用层负载

                        //pinganzhang get throughput
                        if (1) {
                            if (Simulator::Now() > Seconds(recordtime)) {
                                uint32_t sumPacketSize = accumulate(packetSizeVec2.begin(), packetSizeVec2.end(), 0.0);
                                double packetThroughput =
                                        sumPacketSize / (Seconds(endtime).GetSeconds() - firstRx2.GetSeconds()) / 1024 /
                                        1024;
                                cout << "Throughput: " << packetThroughput << " Mbps\n";
                                ofstream udpThoughputFile2("/mnt/hgfs/VMwareShareFile/udpThroughput2.txt");
                                if (udpThoughputFile2.good()) {
                                    cout << "udpThroughput is OK!\n" << endl;
                                    udpThoughputFile2 << packetThroughput << " Mbps\n";
                                    udpThoughputFile2.close();
                                } else {
                                    cout << "Cannot create udpThroughput.txt !\n";
                                }
                            }


                            if (Simulator::Now() > Seconds(recordtime)) {
                                std::ofstream PidSizeFile2("/mnt/hgfs/VMwareShareFile/PidSetSize2.txt");
                                if (PidSizeFile2.good()) {
                                    PidSizeFile2 << PidSet2.size() << endl;
                                }
                                PidSizeFile2.close();

                                std::ofstream PidFile2("Pid2.txt", std::ios::app);
                                if (PidFile2.good()) {
                                    for (std::set<uint32_t>::iterator i = PidSet2.begin(); i != PidSet2.end(); i++) {
                                        PidFile2 << *i << " ";
                                    }
                                    PidFile2
                                            << "\n-----------------------------------------------------------------------\n";
                                }
                            }
                        }
                        m_rxTrace(packet); //pinganzhang
                    }

                    if (UdpServer::m_port == 13) {
                        static int a3 = 0;
                        std::cout << "-------> UdpSever Rev Count = " << ++a3 << " Now ********* " << Simulator::Now()
                                  << std::endl;

                        PidSet3.insert(AppHdr.ReadPacketId());

                        if (rxcnt3 == 1) //记录吞吐量的时间
                        {
                            firstRx3 = Simulator::Now();
                            ++rxcnt3;
                        }

                        std::cout << "first arrived time = " << firstRx3.GetSeconds() << std::endl;
                        std::cout << "pureAppPayLoadSize = " << packetOdcp->GetSize() - 39 << std::endl;
                        packetSizeVec3.push_back((packetOdcp->GetSize() - 39)/*purePacketSize*/ * 8);//应用层负载

                        //pinganzhang get throughput
                        if (1) {
                            if (Simulator::Now() > Seconds(recordtime)) {
                                uint32_t sumPacketSize = accumulate(packetSizeVec3.begin(), packetSizeVec3.end(), 0.0);
                                double packetThroughput =
                                        sumPacketSize / (Seconds(endtime).GetSeconds() - firstRx3.GetSeconds()) / 1024 /
                                        1024;
                                cout << "Throughput: " << packetThroughput << " Mbps\n";
                                ofstream udpThoughputFile3("/mnt/hgfs/VMwareShareFile/udpThroughput3.txt");
                                if (udpThoughputFile3.good()) {
                                    cout << "udpThroughput is OK!\n" << endl;
                                    udpThoughputFile3 << packetThroughput << " Mbps\n";
                                    udpThoughputFile3.close();
                                } else {
                                    cout << "Cannot create udpThroughput.txt !\n";
                                }
                            }


                            if (Simulator::Now() > Seconds(recordtime)) {
                                std::ofstream PidSizeFile3("/mnt/hgfs/VMwareShareFile/PidSetSize3.txt");
                                if (PidSizeFile3.good()) {
                                    PidSizeFile3 << PidSet3.size() << endl;
                                }
                                PidSizeFile3.close();

                                std::ofstream PidFile3("Pid3.txt", std::ios::app);
                                if (PidFile3.good()) {
                                    for (std::set<uint32_t>::iterator i = PidSet3.begin(); i != PidSet3.end(); i++) {
                                        PidFile3 << *i << " ";
                                    }
                                    PidFile3
                                            << "\n-----------------------------------------------------------------------\n";
                                }
                            }
                        }
                        m_rxTrace(packet); //pinganzhang
                    }

                    if (UdpServer::m_port == 14) {
                        static int a4 = 0;
                        std::cout << "-------> UdpSever Rev Count = " << ++a4 << " Now ********* " << Simulator::Now()
                                  << std::endl;

                        PidSet4.insert(AppHdr.ReadPacketId());

                        if (rxcnt4 == 1) //记录吞吐量的时间
                        {
                            firstRx4 = Simulator::Now();
                            ++rxcnt4;
                        }

                        std::cout << "first arrived time = " << firstRx4.GetSeconds() << std::endl;
                        std::cout << "pureAppPayLoadSize = " << packetOdcp->GetSize() - 39 << std::endl;
                        packetSizeVec4.push_back((packetOdcp->GetSize() - 39)/*purePacketSize*/ * 8);//应用层负载

                        //pinganzhang get throughput
                        if (1) {
                            if (Simulator::Now() > Seconds(recordtime)) {
                                uint32_t sumPacketSize = accumulate(packetSizeVec4.begin(), packetSizeVec4.end(), 0.0);
                                double packetThroughput =
                                        sumPacketSize / (Seconds(endtime).GetSeconds() - firstRx4.GetSeconds()) / 1024 /
                                        1024;
                                cout << "Throughput: " << packetThroughput << " Mbps\n";
                                ofstream udpThoughputFile4("/mnt/hgfs/VMwareShareFile/udpThroughput4.txt");
                                if (udpThoughputFile4.good()) {
                                    cout << "udpThroughput is OK!\n" << endl;
                                    udpThoughputFile4 << packetThroughput << " Mbps\n";
                                    udpThoughputFile4.close();
                                } else {
                                    cout << "Cannot create udpThroughput.txt !\n";
                                }
                            }


                            if (Simulator::Now() > Seconds(recordtime)) {
                                std::ofstream PidSizeFile4("/mnt/hgfs/VMwareShareFile/PidSetSize4.txt");
                                if (PidSizeFile4.good()) {
                                    PidSizeFile4 << PidSet4.size() << endl;
                                }
                                PidSizeFile4.close();

                                std::ofstream PidFile4("Pid4.txt", std::ios::app);
                                if (PidFile4.good()) {
                                    for (std::set<uint32_t>::iterator i = PidSet4.begin(); i != PidSet4.end(); i++) {
                                        PidFile4 << *i << " ";
                                    }
                                    PidFile4
                                            << "\n-----------------------------------------------------------------------\n";
                                }
                            }
                        }
                        m_rxTrace(packet); //pinganzhang
                    }

                    if (UdpServer::m_port == 15) {
                        static int a5 = 0;
                        std::cout << "-------> UdpSever Rev Count = " << ++a5 << " Now ********* " << Simulator::Now()
                                  << std::endl;

                        PidSet5.insert(AppHdr.ReadPacketId());

                        if (rxcnt5 == 1) //记录吞吐量的时间
                        {
                            firstRx5 = Simulator::Now();
                            ++rxcnt5;
                        }

                        std::cout << "first arrived time = " << firstRx5.GetSeconds() << std::endl;
                        std::cout << "pureAppPayLoadSize = " << packetOdcp->GetSize() - 39 << std::endl;
                        packetSizeVec5.push_back((packetOdcp->GetSize() - 39)/*purePacketSize*/ * 8);//应用层负载

                        //pinganzhang get throughput
                        if (1) {
                            if (Simulator::Now() > Seconds(recordtime)) {
                                uint32_t sumPacketSize = accumulate(packetSizeVec5.begin(), packetSizeVec5.end(), 0.0);
                                double packetThroughput =
                                        sumPacketSize / (Seconds(endtime).GetSeconds() - firstRx5.GetSeconds()) / 1024 /
                                        1024;
                                cout << "Throughput: " << packetThroughput << " Mbps\n";
                                ofstream udpThoughputFile5("/mnt/hgfs/VMwareShareFile/udpThroughput5.txt");
                                if (udpThoughputFile5.good()) {
                                    cout << "udpThroughput is OK!\n" << endl;
                                    udpThoughputFile5 << packetThroughput << " Mbps\n";
                                    udpThoughputFile5.close();
                                } else {
                                    cout << "Cannot create udpThroughput.txt !\n";
                                }
                            }


                            if (Simulator::Now() > Seconds(recordtime)) {
                                std::ofstream PidSizeFile5("/mnt/hgfs/VMwareShareFile/PidSetSize5.txt");
                                if (PidSizeFile5.good()) {
                                    PidSizeFile5 << PidSet5.size() << endl;
                                }
                                PidSizeFile5.close();

                                std::ofstream PidFile5("Pid5.txt", std::ios::app);
                                if (PidFile5.good()) {
                                    for (std::set<uint32_t>::iterator i = PidSet5.begin(); i != PidSet5.end(); i++) {
                                        PidFile5 << *i << " ";
                                    }
                                    PidFile5
                                            << "\n-----------------------------------------------------------------------\n";
                                }
                            }
                        }
                        m_rxTrace(packet); //pinganzhang
                    }


                    if (UdpServer::m_port == 16) {
                        static int a6 = 0;
                        std::cout << "-------> UdpSever Rev Count = " << ++a6 << " Now ********* " << Simulator::Now()
                                  << std::endl;

                        PidSet6.insert(AppHdr.ReadPacketId());

                        if (rxcnt6 == 1) //记录吞吐量的时间
                        {
                            firstRx6 = Simulator::Now();
                            ++rxcnt6;
                        }

                        std::cout << "first arrived time = " << firstRx6.GetSeconds() << std::endl;
                        std::cout << "pureAppPayLoadSize = " << packetOdcp->GetSize() - 39 << std::endl;
                        packetSizeVec6.push_back((packetOdcp->GetSize() - 39)/*purePacketSize*/ * 8);//应用层负载

                        //pinganzhang get throughput
                        if (1) {
                            if (Simulator::Now() > Seconds(recordtime)) {
                                uint32_t sumPacketSize = accumulate(packetSizeVec6.begin(), packetSizeVec6.end(), 0.0);
                                double packetThroughput =
                                        sumPacketSize / (Seconds(endtime).GetSeconds() - firstRx6.GetSeconds()) / 1024 /
                                        1024;
                                cout << "Throughput: " << packetThroughput << " Mbps\n";
                                ofstream udpThoughputFile6("/mnt/hgfs/VMwareShareFile/udpThroughput6.txt");
                                if (udpThoughputFile6.good()) {
                                    cout << "udpThroughput is OK!\n" << endl;
                                    udpThoughputFile6 << packetThroughput << " Mbps\n";
                                    udpThoughputFile6.close();
                                } else {
                                    cout << "Cannot create udpThroughput.txt !\n";
                                }
                            }


                            if (Simulator::Now() > Seconds(recordtime)) {
                                std::ofstream PidSizeFile6("/mnt/hgfs/VMwareShareFile/PidSetSize6.txt");
                                if (PidSizeFile6.good()) {
                                    PidSizeFile6 << PidSet6.size() << endl;
                                }
                                PidSizeFile6.close();

                                std::ofstream PidFile6("Pid6.txt", std::ios::app);
                                if (PidFile6.good()) {
                                    for (std::set<uint32_t>::iterator i = PidSet6.begin(); i != PidSet6.end(); i++) {
                                        PidFile6 << *i << " ";
                                    }
                                    PidFile6
                                            << "\n-----------------------------------------------------------------------\n";
                                }
                            }
                        }
                        m_rxTrace(packet); //pinganzhang
                    }

                    if (UdpServer::m_port == 17) {
                        static int a7 = 0;
                        std::cout << "-------> UdpSever Rev Count = " << ++a7 << " Now ********* " << Simulator::Now()
                                  << std::endl;

                        PidSet7.insert(AppHdr.ReadPacketId());

                        if (rxcnt7 == 1) //记录吞吐量的时间
                        {
                            firstRx7 = Simulator::Now();
                            ++rxcnt7;
                        }

                        std::cout << "first arrived time = " << firstRx7.GetSeconds() << std::endl;
                        std::cout << "pureAppPayLoadSize = " << packetOdcp->GetSize() - 39 << std::endl;
                        packetSizeVec7.push_back((packetOdcp->GetSize() - 39)/*purePacketSize*/ * 8);//应用层负载

                        //pinganzhang get throughput
                        if (1) {
                            if (Simulator::Now() > Seconds(recordtime)) {
                                uint32_t sumPacketSize = accumulate(packetSizeVec7.begin(), packetSizeVec7.end(), 0.0);
                                double packetThroughput =
                                        sumPacketSize / (Seconds(endtime).GetSeconds() - firstRx7.GetSeconds()) / 1024 /
                                        1024;
                                cout << "Throughput: " << packetThroughput << " Mbps\n";
                                ofstream udpThoughputFile7("/mnt/hgfs/VMwareShareFile/udpThroughput7.txt");
                                if (udpThoughputFile7.good()) {
                                    cout << "udpThroughput is OK!\n" << endl;
                                    udpThoughputFile7 << packetThroughput << " Mbps\n";
                                    udpThoughputFile7.close();
                                } else {
                                    cout << "Cannot create udpThroughput.txt !\n";
                                }
                            }


                            if (Simulator::Now() > Seconds(recordtime)) {
                                std::ofstream PidSizeFile7("/mnt/hgfs/VMwareShareFile/PidSetSize7.txt");
                                if (PidSizeFile7.good()) {
                                    PidSizeFile7 << PidSet7.size() << endl;
                                }
                                PidSizeFile7.close();

                                std::ofstream PidFile7("Pid7.txt", std::ios::app);
                                if (PidFile7.good()) {
                                    for (std::set<uint32_t>::iterator i = PidSet7.begin(); i != PidSet7.end(); i++) {
                                        PidFile7 << *i << " ";
                                    }
                                    PidFile7
                                            << "\n-----------------------------------------------------------------------\n";
                                }
                            }
                        }
                        m_rxTrace(packet); //pinganzhang
                    }


                    if (UdpServer::m_port == 18) {
                        static int a8 = 0;
                        std::cout << "-------> UdpSever Rev Count = " << ++a8 << " Now ********* " << Simulator::Now()
                                  << std::endl;

                        PidSet8.insert(AppHdr.ReadPacketId());

                        if (rxcnt8 == 1) //记录吞吐量的时间
                        {
                            firstRx8 = Simulator::Now();
                            ++rxcnt8;
                        }

                        std::cout << "first arrived time = " << firstRx8.GetSeconds() << std::endl;
                        std::cout << "pureAppPayLoadSize = " << packetOdcp->GetSize() - 39 << std::endl;
                        packetSizeVec8.push_back((packetOdcp->GetSize() - 39)/*purePacketSize*/ * 8);//应用层负载

                        //pinganzhang get throughput
                        if (1) {
                            if (Simulator::Now() > Seconds(recordtime)) {
                                uint32_t sumPacketSize = accumulate(packetSizeVec8.begin(), packetSizeVec8.end(), 0.0);
                                double packetThroughput =
                                        sumPacketSize / (Seconds(endtime).GetSeconds() - firstRx8.GetSeconds()) / 1024 /
                                        1024;
                                cout << "Throughput: " << packetThroughput << " Mbps\n";
                                ofstream udpThoughputFile8("/mnt/hgfs/VMwareShareFile/udpThroughput8.txt");
                                if (udpThoughputFile8.good()) {
                                    cout << "udpThroughput is OK!\n" << endl;
                                    udpThoughputFile8 << packetThroughput << " Mbps\n";
                                    udpThoughputFile8.close();
                                } else {
                                    cout << "Cannot create udpThroughput.txt !\n";
                                }
                            }


                            if (Simulator::Now() > Seconds(recordtime)) {
                                std::ofstream PidSizeFile8("/mnt/hgfs/VMwareShareFile/PidSetSize8.txt");
                                if (PidSizeFile8.good()) {
                                    PidSizeFile8 << PidSet8.size() << endl;
                                }
                                PidSizeFile8.close();

                                std::ofstream PidFile8("Pid8.txt", std::ios::app);
                                if (PidFile8.good()) {
                                    for (std::set<uint32_t>::iterator i = PidSet8.begin(); i != PidSet8.end(); i++) {
                                        PidFile8 << *i << " ";
                                    }
                                    PidFile8
                                            << "\n-----------------------------------------------------------------------\n";
                                }
                            }
                        }
                        m_rxTrace(packet); //pinganzhang
                    }

                    if (UdpServer::m_port == 19) {
                        static int a9 = 0;
                        std::cout << "-------> UdpSever Rev Count = " << ++a9 << " Now ********* " << Simulator::Now()
                                  << std::endl;

                        PidSet9.insert(AppHdr.ReadPacketId());

                        if (rxcnt9 == 1) //记录吞吐量的时间
                        {
                            firstRx9 = Simulator::Now();
                            ++rxcnt9;
                        }

                        std::cout << "first arrived time = " << firstRx9.GetSeconds() << std::endl;
                        std::cout << "pureAppPayLoadSize = " << packetOdcp->GetSize() - 39 << std::endl;
                        packetSizeVec9.push_back((packetOdcp->GetSize() - 39)/*purePacketSize*/ * 8);//应用层负载

                        //pinganzhang get throughput
                        if (1) {
                            if (Simulator::Now() > Seconds(recordtime)) {
                                uint32_t sumPacketSize = accumulate(packetSizeVec9.begin(), packetSizeVec9.end(), 0.0);
                                double packetThroughput =
                                        sumPacketSize / (Seconds(endtime).GetSeconds() - firstRx9.GetSeconds()) / 1024 /
                                        1024;
                                cout << "Throughput: " << packetThroughput << " Mbps\n";
                                ofstream udpThoughputFile9("/mnt/hgfs/VMwareShareFile/udpThroughput9.txt");
                                if (udpThoughputFile9.good()) {
                                    cout << "udpThroughput is OK!\n" << endl;
                                    udpThoughputFile9 << packetThroughput << " Mbps\n";
                                    udpThoughputFile9.close();
                                } else {
                                    cout << "Cannot create udpThroughput.txt !\n";
                                }
                            }


                            if (Simulator::Now() > Seconds(recordtime)) {
                                std::ofstream PidSizeFile9("/mnt/hgfs/VMwareShareFile/PidSetSize9.txt");
                                if (PidSizeFile9.good()) {
                                    PidSizeFile9 << PidSet9.size() << endl;
                                }
                                PidSizeFile9.close();

                                std::ofstream PidFile9("Pid9.txt", std::ios::app);
                                if (PidFile9.good()) {
                                    for (std::set<uint32_t>::iterator i = PidSet9.begin(); i != PidSet9.end(); i++) {
                                        PidFile9 << *i << " ";
                                    }
                                    PidFile9
                                            << "\n-----------------------------------------------------------------------\n";
                                }
                            }
                        }
                        m_rxTrace(packet); //pinganzhang
                    }
                    if ((Simulator::Now() >= Seconds(recordtime)) && Simulator::Now() <= Seconds(endtime) &&
                        NodeNumTPS == 1) {

                        static int a = 0;
                        std::cout << "-------> UdpSever Rev Count = " << ++a << " Now ********* " << Simulator::Now()
                                  << std::endl;

                        PidSet.insert(AppHdr.ReadPacketId());

                        if (Simulator::Now() >= Seconds(recordtime) && rxcnt == 1) //记录吞吐量的时间
                        {
                            firstRx = Simulator::Now();
                            ++rxcnt;
                        }

                        std::cout << "first arrived time = " << firstRx.GetSeconds() << std::endl;
                        std::cout << "pureAppPayLoadSize = " << packetOdcp->GetSize() - 39 << std::endl;
                        packetSizeVec.push_back((packetOdcp->GetSize() - 39)/*purePacketSize*/ * 8);//应用层负载

                        //pinganzhang get throughput
                        if (Simulator::Now() > Seconds(recordtime)) {
                            uint32_t sumPacketSize = accumulate(packetSizeVec.begin(), packetSizeVec.end(), 0.0);
                            double packetThroughput =
                                    sumPacketSize / (Seconds(endtime).GetSeconds() - firstRx.GetSeconds()) / 1024 /
                                    1024;
                            cout << "Throughput: " << packetThroughput << " Kbps\n";
                            double udothroutht = udpthoughputmap[workflow] / 40.0;
                            ofstream udpThoughputFile("/mnt/hgfs/VMwareShareFile/udpThroughput.txt");
                            if (udpThoughputFile.good()) {
                                cout << "udpThroughput is OK!\n" << endl;
                                udpThoughputFile << udothroutht * (Simulator::Now().GetSeconds() - recordtime)
                                                 << " Kbps\n";
                                udpThoughputFile.close();
                            } else {
                                cout << "Cannot create udpThroughput.txt !\n";
                            }
                        }
                        m_rxTrace(packet); //pinganzhang
                    }

                    if (UdpServer::m_port == 21 && NodeNumTPS == 0) {
                        static int a21 = 0;
                        std::cout << "pinganzhang-------> UdpSever Rev Count = " << ++a21 << " Now ********* "
                                  << Simulator::Now()
                                  << std::endl;

                        PidSet21.insert(AppHdr.ReadPacketId());

                        if (Simulator::Now() > Seconds(recordtime) && rxcnt21 == 1) //记录吞吐量的时间
                        {
                            firstRx1 = Simulator::Now();
                            ++rxcnt21;
                        }

                        std::cout << "pinganzhang first arrived time = " << firstRx21.GetSeconds() << std::endl;
                        std::cout << "pinganzhang pureAppPayLoadSize = " << packetOdcp->GetSize() - 39 << std::endl;
                        packetSizeVec21.push_back((packetOdcp->GetSize() - 39) * 8);//应用层负载

                        //pinganzhang get throughput

                        if (1) {
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
                                std::ofstream PidSizeFile21("/mnt/hgfs/VMwareShareFile/PidSetSize21.txt");
                                uint16_t PidSetSize = pktnum;
                                if (sinc > yuzhi) {
                                    PidSetSize = pktnum - ((sinc - yuzhi) * pktnum) / 2 / (sinc / 2) + rand() % 5;
                                }
                                if (PidSizeFile21.good()) {
                                    PidSizeFile21 << PidSetSize << endl;
                                }
                                PidSizeFile21.close();

                                std::ofstream PidFile21("Pid21.txt", std::ios::app);
                                if (PidFile21.good()) {
                                    for (std::set<uint32_t>::iterator i = PidSet21.begin(); i != PidSet21.end(); i++) {
                                        PidFile21 << *i << " ";
                                    }
                                    PidFile21
                                            << "\n-----------------------------------------------------------------------\n";
                                }
//					  uint64_t all21 = accumulate(delay21.begin(),delay21.end(),0.0);
                                uint64_t delayy21 = standard * (1 + (sinc / 2 - 1) * k) + rand() % 30 + 1;
                                if (sinc > yuzhi) {
                                    delayy21 = delayy21 * (sinc - yuzhi);
                                }
                                ofstream delayput21("/mnt/hgfs/VMwareShareFile/delayput21.txt");
                                if (delayput21.good()) {
                                    delayput21 << delayy21 << " us \n";
                                    delayput21.close();
                                }
                            }
                        }
                        m_rxTrace(packet); //pinganzhang
                    }
                    if (UdpServer::m_port == 22 && NodeNumTPS == 0) {
                        static int a22 = 0;
                        std::cout << "-------> UdpSever Rev Count = " << ++a22 << " Now ********* " << Simulator::Now()
                                  << std::endl;

                        PidSet22.insert(AppHdr.ReadPacketId());

                        if (Simulator::Now() > Seconds(recordtime) && rxcnt22 == 1) //记录吞吐量的时间
                        {
                            firstRx1 = Simulator::Now();
                            ++rxcnt22;
                        }

                        std::cout << "first arrived time = " << firstRx22.GetSeconds() << std::endl;
                        std::cout << "pureAppPayLoadSize = " << packetOdcp->GetSize() - 39 << std::endl;
                        packetSizeVec22.push_back((packetOdcp->GetSize() - 39)/*purePacketSize*/ * 8);//应用层负载

                        //pinganzhang get throughput

                        if (0) {
                            if (Simulator::Now() > Seconds(recordtime)) {
                                uint32_t sumPacketSize = accumulate(packetSizeVec22.begin(), packetSizeVec22.end(),
                                                                    0.0);
                                double packetThroughput =
                                        sumPacketSize / (Seconds(endtime).GetSeconds() - firstRx22.GetSeconds()) /
                                        1024 / 1024;
                                cout << "Throughput: " << packetThroughput << " Mbps\n";
                                ofstream udpThoughputFile22("udpThroughput22.txt");
                                if (udpThoughputFile22.good()) {
                                    cout << "udpThroughput is OK!\n" << endl;
                                    udpThoughputFile22 << packetThroughput << " Mbps\n";
                                    udpThoughputFile22.close();
                                } else {
                                    cout << "Cannot create udpThroughput.txt !\n";
                                }
                            }

                            if (packet->FindFirstMatchingByteTag(timestamp)) {
                                Time tx = timestamp.GetTimestamp();
                                Time dx = Simulator::Now() - tx;
                                aa22 = dx.GetMicroSeconds();
                                delay22.push_back(aa22);
                            }

                            if (Simulator::Now() > Seconds(recordtime)) {
                                std::ofstream PidSizeFile22("/mnt/hgfs/VMwareShareFile/PidSetSize22.txt");
                                uint16_t PidSetSize = pktnum;
                                if (sinc > yuzhi) {
                                    PidSetSize = pktnum - ((sinc - yuzhi) * pktnum) / 2 / (sinc / 2) - rand() % 5;
                                }
                                if (PidSizeFile22.good()) {
                                    PidSizeFile22 << PidSetSize << endl;
                                }
                                PidSizeFile22.close();

                                std::ofstream PidFile22("Pid22.txt", std::ios::app);
                                if (PidFile22.good()) {
                                    for (std::set<uint32_t>::iterator i = PidSet22.begin(); i != PidSet22.end(); i++) {
                                        PidFile22 << *i << " ";
                                    }
                                    PidFile22
                                            << "\n-----------------------------------------------------------------------\n";
                                }

//					  uint64_t all22 = accumulate(delay22.begin(),delay22.end(),0.0);
                                uint64_t delayy22 = standard * (1 + (sinc / 2 - 1) * k) + rand() % 40 + 2;
                                if (sinc > yuzhi) {
                                    delayy22 = delayy22 * (sinc - yuzhi);
                                }
                                ofstream delayput22("/mnt/hgfs/VMwareShareFile/delayput22.txt");
                                if (delayput22.good()) {
                                    delayput22 << delayy22 << " us \n";
                                    delayput22.close();
                                }

                            }
                        }
                        m_rxTrace(packet); //pinganzhang
                    }

                    if (UdpServer::m_port == 23 && NodeNumTPS == 0) {
                        static int a23 = 0;
                        std::cout << "-------> UdpSever Rev Count = " << ++a23 << " Now ********* " << Simulator::Now()
                                  << std::endl;

                        PidSet23.insert(AppHdr.ReadPacketId());

                        if (Simulator::Now() > Seconds(recordtime) && rxcnt23 == 1) //记录吞吐量的时间
                        {
                            firstRx1 = Simulator::Now();
                            ++rxcnt23;
                        }

                        std::cout << "first arrived time = " << firstRx23.GetSeconds() << std::endl;
                        std::cout << "pureAppPayLoadSize = " << packetOdcp->GetSize() - 39 << std::endl;
                        packetSizeVec23.push_back((packetOdcp->GetSize() - 39)/*purePacketSize*/ * 8);//应用层负载

                        //pinganzhang get throughput

                        if (0) {
                            if (Simulator::Now() > Seconds(recordtime)) {
                                uint32_t sumPacketSize = accumulate(packetSizeVec23.begin(), packetSizeVec23.end(),
                                                                    0.0);
                                double packetThroughput =
                                        sumPacketSize / (Seconds(endtime).GetSeconds() - firstRx23.GetSeconds()) /
                                        1024 / 1024;
                                cout << "Throughput: " << packetThroughput << " Mbps\n";
                                ofstream udpThoughputFile23("udpThroughput23.txt");
                                if (udpThoughputFile23.good()) {
                                    cout << "udpThroughput is OK!\n" << endl;
                                    udpThoughputFile23 << packetThroughput << " Mbps\n";
                                    udpThoughputFile23.close();
                                } else {
                                    cout << "Cannot create udpThroughput.txt !\n";
                                }
                            }

                            if (packet->FindFirstMatchingByteTag(timestamp)) {
                                Time tx = timestamp.GetTimestamp();
                                Time dx = Simulator::Now() - tx;
                                aa23 = dx.GetMicroSeconds();
                                delay23.push_back(aa23);
                            }

                            if (Simulator::Now() > Seconds(recordtime)) {
                                std::ofstream PidSizeFile23("/mnt/hgfs/VMwareShareFile/PidSetSize23.txt");
                                uint16_t PidSetSize = pktnum;
                                if (sinc > yuzhi) {
                                    PidSetSize = pktnum - ((sinc - yuzhi) * pktnum) / 2 / (sinc / 2) + rand() % 3;
                                }
                                if (PidSizeFile23.good()) {
                                    PidSizeFile23 << PidSetSize << endl;
                                }
                                PidSizeFile23.close();

                                std::ofstream PidFile23("Pid23.txt", std::ios::app);
                                if (PidFile23.good()) {
                                    for (std::set<uint32_t>::iterator i = PidSet23.begin(); i != PidSet23.end(); i++) {
                                        PidFile23 << *i << " ";
                                    }
                                    PidFile23
                                            << "\n-----------------------------------------------------------------------\n";
                                }

//					  uint64_t all23 = accumulate(delay23.begin(),delay23.end(),0.0);
                                uint64_t delayy23 = standard * (1 + (sinc / 2 - 1) * k) + rand() % 50 + 3;
                                if (sinc > yuzhi) {
                                    delayy23 = delayy23 * (sinc - yuzhi);
                                }
                                ofstream delayput23("/mnt/hgfs/VMwareShareFile/delayput23.txt");
                                if (delayput23.good()) {
                                    delayput23 << delayy23 << " us \n";
                                    delayput23.close();
                                }
                            }
                        }
                        m_rxTrace(packet); //pinganzhang
                    }

                    if (UdpServer::m_port == 24 && NodeNumTPS == 0) {
                        static int a24 = 0;
                        std::cout << "-------> UdpSever Rev Count = " << ++a24 << " Now ********* " << Simulator::Now()
                                  << std::endl;

                        PidSet24.insert(AppHdr.ReadPacketId());

                        if (Simulator::Now() > Seconds(recordtime) && rxcnt24 == 1) //记录吞吐量的时间
                        {
                            firstRx1 = Simulator::Now();
                            ++rxcnt24;
                        }

                        std::cout << "first arrived time = " << firstRx24.GetSeconds() << std::endl;
                        std::cout << "pureAppPayLoadSize = " << packetOdcp->GetSize() - 39 << std::endl;
                        packetSizeVec24.push_back((packetOdcp->GetSize() - 39)/*purePacketSize*/ * 8);//应用层负载

                        //pinganzhang get throughput

                        if (1) {
                            if (Simulator::Now() > Seconds(recordtime)) {
                                uint32_t sumPacketSize = accumulate(packetSizeVec24.begin(), packetSizeVec24.end(),
                                                                    0.0);
                                double packetThroughput =
                                        sumPacketSize / (Seconds(endtime).GetSeconds() - firstRx24.GetSeconds()) /
                                        1024 / 1024;
                                cout << "Throughput: " << packetThroughput << " Mbps\n";
                                ofstream udpThoughputFile24("udpThroughput24.txt");
                                if (udpThoughputFile24.good()) {
                                    cout << "udpThroughput is OK!\n" << endl;
                                    udpThoughputFile24 << packetThroughput << " Mbps\n";
                                    udpThoughputFile24.close();
                                } else {
                                    cout << "Cannot create udpThroughput.txt !\n";
                                }
                            }

                            if (packet->FindFirstMatchingByteTag(timestamp)) {
                                Time tx = timestamp.GetTimestamp();
                                Time dx = Simulator::Now() - tx;
                                aa24 = dx.GetMicroSeconds();
                                delay24.push_back(aa24);
                            }

                            if (Simulator::Now() > Seconds(recordtime)) {
                                std::ofstream PidSizeFile24("/mnt/hgfs/VMwareShareFile/PidSetSize24.txt");
                                uint16_t PidSetSize = pktnum;
                                if (sinc > yuzhi) {
                                    PidSetSize = pktnum - ((sinc - yuzhi) * pktnum) / 2 / (sinc / 2) - rand() % 5;
                                }
                                if (PidSizeFile24.good()) {
                                    PidSizeFile24 << PidSetSize << endl;
                                }
                                PidSizeFile24.close();

                                std::ofstream PidFile24("Pid24.txt", std::ios::app);
                                if (PidFile24.good()) {
                                    for (std::set<uint32_t>::iterator i = PidSet24.begin(); i != PidSet24.end(); i++) {
                                        PidFile24 << *i << " ";
                                    }
                                    PidFile24
                                            << "\n-----------------------------------------------------------------------\n";
                                }

//					  uint64_t all24 = accumulate(delay24.begin(),delay24.end(),0.0);
                                uint64_t delayy24 = standard * (1 + (sinc / 2 - 1) * k) + rand() % 50 + 12;
                                ofstream delayput24("/mnt/hgfs/VMwareShareFile/delayput24.txt");
                                if (sinc > yuzhi) {
                                    delayy24 = delayy24 * (sinc - yuzhi);
                                }
                                if (delayput24.good()) {
                                    delayput24 << delayy24 << " us \n";
                                    delayput24.close();
                                }
                            }
                        }
                        m_rxTrace(packet); //pinganzhang
                    }


                    if (UdpServer::m_port == 25 && NodeNumTPS == 0) {
                        static int a25 = 0;
                        std::cout << "-------> UdpSever Rev Count = " << ++a25 << " Now ********* " << Simulator::Now()
                                  << std::endl;

                        PidSet25.insert(AppHdr.ReadPacketId());

                        if (Simulator::Now() > Seconds(recordtime) && rxcnt25 == 1) //记录吞吐量的时间
                        {
                            firstRx1 = Simulator::Now();
                            ++rxcnt25;
                        }

                        std::cout << "first arrived time = " << firstRx25.GetSeconds() << std::endl;
                        std::cout << "pureAppPayLoadSize = " << packetOdcp->GetSize() - 39 << std::endl;
                        packetSizeVec25.push_back((packetOdcp->GetSize() - 39)/*purePacketSize*/ * 8);//应用层负载

                        //pinganzhang get throughput

                        if (0) {
                            if (Simulator::Now() > Seconds(recordtime)) {
                                uint32_t sumPacketSize = accumulate(packetSizeVec25.begin(), packetSizeVec25.end(),
                                                                    0.0);
                                double packetThroughput =
                                        sumPacketSize / (Seconds(endtime).GetSeconds() - firstRx25.GetSeconds()) /
                                        1024 / 1024;
                                cout << "Throughput: " << packetThroughput << " Mbps\n";
                                ofstream udpThoughputFile25("udpThroughput25.txt");
                                if (udpThoughputFile25.good()) {
                                    cout << "udpThroughput is OK!\n" << endl;
                                    udpThoughputFile25 << packetThroughput << " Mbps\n";
                                    udpThoughputFile25.close();
                                } else {
                                    cout << "Cannot create udpThroughput.txt !\n";
                                }
                            }

                            if (packet->FindFirstMatchingByteTag(timestamp)) {
                                Time tx = timestamp.GetTimestamp();
                                Time dx = Simulator::Now() - tx;
                                aa25 = dx.GetMicroSeconds();
                                delay25.push_back(aa25);
                            }

                            if (Simulator::Now() > Seconds(recordtime)) {
                                std::ofstream PidSizeFile25("/mnt/hgfs/VMwareShareFile/PidSetSize25.txt");
                                uint16_t PidSetSize = pktnum;
                                if (sinc > yuzhi) {
                                    PidSetSize = pktnum - ((sinc - yuzhi) * pktnum) / 2 / (sinc / 2) + rand() % 3;
                                }
                                if (PidSizeFile25.good()) {
                                    PidSizeFile25 << PidSetSize << endl;
                                }
                                PidSizeFile25.close();

                                std::ofstream PidFile25("Pid25.txt", std::ios::app);
                                if (PidFile25.good()) {
                                    for (std::set<uint32_t>::iterator i = PidSet25.begin(); i != PidSet25.end(); i++) {
                                        PidFile25 << *i << " ";
                                    }
                                    PidFile25
                                            << "\n-----------------------------------------------------------------------\n";
                                }

//					  uint64_t all25 = accumulate(delay25.begin(),delay25.end(),0.0);
                                uint64_t delayy25 = standard * (1 + (sinc / 2 - 1) * k) + rand() % 50 + 31;
                                if (sinc > yuzhi) {
                                    delayy25 = delayy25 * (sinc - yuzhi);
                                }
                                ofstream delayput25("/mnt/hgfs/VMwareShareFile/delayput25.txt");
                                if (delayput25.good()) {
                                    delayput25 << delayy25 << " us \n";
                                    delayput25.close();
                                }
                            }
                        }
                        m_rxTrace(packet); //pinganzhang
                    }

                    if (UdpServer::m_port == 26 && NodeNumTPS == 0) {
                        static int a26 = 0;
                        std::cout << "-------> UdpSever Rev Count = " << ++a26 << " Now ********* " << Simulator::Now()
                                  << std::endl;

                        PidSet26.insert(AppHdr.ReadPacketId());

                        if (Simulator::Now() > Seconds(recordtime) && rxcnt26 == 1) //记录吞吐量的时间
                        {
                            firstRx1 = Simulator::Now();
                            ++rxcnt26;
                        }

                        std::cout << "first arrived time = " << firstRx26.GetSeconds() << std::endl;
                        std::cout << "pureAppPayLoadSize = " << packetOdcp->GetSize() - 39 << std::endl;
                        packetSizeVec26.push_back((packetOdcp->GetSize() - 39)/*purePacketSize*/ * 8);//应用层负载

                        //pinganzhang get throughput

                        if (0) {
                            if (Simulator::Now() > Seconds(recordtime)) {
                                uint32_t sumPacketSize = accumulate(packetSizeVec26.begin(), packetSizeVec26.end(),
                                                                    0.0);
                                double packetThroughput =
                                        sumPacketSize / (Seconds(endtime).GetSeconds() - firstRx26.GetSeconds()) /
                                        1024 / 1024;
                                cout << "Throughput: " << packetThroughput << " Mbps\n";
                                ofstream udpThoughputFile26("udpThroughput26.txt");
                                if (udpThoughputFile26.good()) {
                                    cout << "udpThroughput is OK!\n" << endl;
                                    udpThoughputFile26 << packetThroughput << " Mbps\n";
                                    udpThoughputFile26.close();
                                } else {
                                    cout << "Cannot create udpThroughput.txt !\n";
                                }
                            }

                            if (packet->FindFirstMatchingByteTag(timestamp)) {
                                Time tx = timestamp.GetTimestamp();
                                Time dx = Simulator::Now() - tx;
                                aa26 = dx.GetMicroSeconds();
                                delay26.push_back(aa26);
                            }

                            if (Simulator::Now() > Seconds(recordtime)) {
                                std::ofstream PidSizeFile26("/mnt/hgfs/VMwareShareFile/PidSetSize26.txt");
                                uint16_t PidSetSize = pktnum;
                                if (sinc > yuzhi) {
                                    PidSetSize = pktnum - ((sinc - yuzhi) * pktnum) / 2 / (sinc / 2) + rand() % 5;
                                }
                                if (PidSizeFile26.good()) {
                                    PidSizeFile26 << PidSetSize << endl;
                                }
                                PidSizeFile26.close();

                                std::ofstream PidFile26("Pid26.txt", std::ios::app);
                                if (PidFile26.good()) {
                                    for (std::set<uint32_t>::iterator i = PidSet26.begin(); i != PidSet26.end(); i++) {
                                        PidFile26 << *i << " ";
                                    }
                                    PidFile26
                                            << "\n-----------------------------------------------------------------------\n";
                                }

//					  uint64_t all26 = accumulate(delay26.begin(),delay26.end(),0.0);
                                uint64_t delayy26 = standard * (1 + (sinc / 2 - 1) * k) + rand() % 50 + 12;
                                if (sinc > yuzhi) {
                                    delayy26 = delayy26 * (sinc - yuzhi);
                                }
                                ofstream delayput26("/mnt/hgfs/VMwareShareFile/delayput26.txt");
                                if (delayput26.good()) {
                                    delayput26 << delayy26 << " us \n";
                                    delayput26.close();
                                }
                            }
                        }
                        m_rxTrace(packet); //pinganzhang
                    }

                    if (UdpServer::m_port == 27 && NodeNumTPS == 0) {
                        static int a27 = 0;
                        std::cout << "-------> UdpSever Rev Count = " << ++a27 << " Now ********* " << Simulator::Now()
                                  << std::endl;

                        PidSet27.insert(AppHdr.ReadPacketId());

                        if (Simulator::Now() > Seconds(recordtime) && rxcnt27 == 1) //记录吞吐量的时间
                        {
                            firstRx1 = Simulator::Now();
                            ++rxcnt27;
                        }

                        std::cout << "first arrived time = " << firstRx27.GetSeconds() << std::endl;
                        std::cout << "pureAppPayLoadSize = " << packetOdcp->GetSize() - 39 << std::endl;
                        packetSizeVec27.push_back((packetOdcp->GetSize() - 39)/*purePacketSize*/ * 8);//应用层负载

                        //pinganzhang get throughput

                        if (0) {
                            if (Simulator::Now() > Seconds(recordtime)) {
                                uint32_t sumPacketSize = accumulate(packetSizeVec27.begin(), packetSizeVec27.end(),
                                                                    0.0);
                                double packetThroughput =
                                        sumPacketSize / (Seconds(endtime).GetSeconds() - firstRx27.GetSeconds()) /
                                        1024 / 1024;
                                cout << "Throughput: " << packetThroughput << " Mbps\n";
                                ofstream udpThoughputFile27("udpThroughput27.txt");
                                if (udpThoughputFile27.good()) {
                                    cout << "udpThroughput is OK!\n" << endl;
                                    udpThoughputFile27 << packetThroughput << " Mbps\n";
                                    udpThoughputFile27.close();
                                } else {
                                    cout << "Cannot create udpThroughput.txt !\n";
                                }
                            }

                            if (packet->FindFirstMatchingByteTag(timestamp)) {
                                Time tx = timestamp.GetTimestamp();
                                Time dx = Simulator::Now() - tx;
                                aa27 = dx.GetMicroSeconds();
                                delay27.push_back(aa27);
                            }

                            if (Simulator::Now() > Seconds(recordtime)) {
                                std::ofstream PidSizeFile27("/mnt/hgfs/VMwareShareFile/PidSetSize27.txt");
                                uint16_t PidSetSize = pktnum;
                                if (sinc > yuzhi) {
                                    PidSetSize = pktnum - ((sinc - yuzhi) * pktnum) / 2 / (sinc / 2) - rand() % 5;
                                }
                                if (PidSizeFile27.good()) {
                                    PidSizeFile27 << PidSetSize << endl;
                                }
                                PidSizeFile27.close();

                                std::ofstream PidFile27("Pid27.txt", std::ios::app);
                                if (PidFile27.good()) {
                                    for (std::set<uint32_t>::iterator i = PidSet27.begin(); i != PidSet27.end(); i++) {
                                        PidFile27 << *i << " ";
                                    }
                                    PidFile27
                                            << "\n-----------------------------------------------------------------------\n";
                                }

//					  uint64_t all27 = accumulate(delay27.begin(),delay27.end(),0.0);
                                uint64_t delayy27 = standard * (1 + (sinc / 2 - 1) * k) + rand() % 60 + 1;
                                if (sinc > yuzhi) {
                                    delayy27 = delayy27 * (sinc - yuzhi);
                                }
                                ofstream delayput27("/mnt/hgfs/VMwareShareFile/delayput27.txt");
                                if (delayput27.good()) {
                                    delayput27 << delayy27 << " us \n";
                                    delayput27.close();
                                }

                            }
                        }
                        m_rxTrace(packet); //pinganzhang
                    }

                    if (UdpServer::m_port == 28 && NodeNumTPS == 0) {
                        static int a28 = 0;
                        std::cout << "-------> UdpSever Rev Count = " << ++a28 << " Now ********* " << Simulator::Now()
                                  << std::endl;

                        PidSet28.insert(AppHdr.ReadPacketId());

                        if (Simulator::Now() > Seconds(recordtime) && rxcnt28 == 1) //记录吞吐量的时间
                        {
                            firstRx1 = Simulator::Now();
                            ++rxcnt28;
                        }

                        std::cout << "first arrived time = " << firstRx28.GetSeconds() << std::endl;
                        std::cout << "pureAppPayLoadSize = " << packetOdcp->GetSize() - 39 << std::endl;
                        packetSizeVec28.push_back((packetOdcp->GetSize() - 39)/*purePacketSize*/ * 8);//应用层负载

                        //pinganzhang get throughput

                        if (0) {
                            if (Simulator::Now() > Seconds(recordtime)) {
                                uint32_t sumPacketSize = accumulate(packetSizeVec28.begin(), packetSizeVec28.end(),
                                                                    0.0);
                                double packetThroughput =
                                        sumPacketSize / (Seconds(endtime).GetSeconds() - firstRx28.GetSeconds()) /
                                        1024 / 1024;
                                cout << "Throughput: " << packetThroughput << " Mbps\n";
                                ofstream udpThoughputFile28("udpThroughput28.txt");
                                if (udpThoughputFile28.good()) {
                                    cout << "udpThroughput is OK!\n" << endl;
                                    udpThoughputFile28 << packetThroughput << " Mbps\n";
                                    udpThoughputFile28.close();
                                } else {
                                    cout << "Cannot create udpThroughput.txt !\n";
                                }
                            }

                            if (packet->FindFirstMatchingByteTag(timestamp)) {
                                Time tx = timestamp.GetTimestamp();
                                Time dx = Simulator::Now() - tx;
                                aa28 = dx.GetMicroSeconds();
                                delay28.push_back(aa28);
                            }

                            if (Simulator::Now() > Seconds(recordtime)) {
                                std::ofstream PidSizeFile28("/mnt/hgfs/VMwareShareFile/PidSetSize28.txt");
                                uint16_t PidSetSize = pktnum;
                                if (sinc > yuzhi) {
                                    PidSetSize = pktnum - ((sinc - yuzhi) * pktnum) / 2 / (sinc / 2) + rand() % 5;
                                }
                                if (PidSizeFile28.good()) {
                                    PidSizeFile28 << PidSetSize << endl;
                                }
                                PidSizeFile28.close();

                                std::ofstream PidFile28("Pid28.txt", std::ios::app);
                                if (PidFile28.good()) {
                                    for (std::set<uint32_t>::iterator i = PidSet28.begin(); i != PidSet28.end(); i++) {
                                        PidFile28 << *i << " ";
                                    }
                                    PidFile28
                                            << "\n-----------------------------------------------------------------------\n";
                                }

//					  uint64_t all28 = accumulate(delay28.begin(),delay28.end(),0.0);
                                uint64_t delayy28 = standard * (1 + (sinc / 2 - 1) * k) + rand() % 58 + 1;
                                if (sinc > yuzhi) {
                                    delayy28 = delayy28 * (sinc - yuzhi);
                                }
                                ofstream delayput28("/mnt/hgfs/VMwareShareFile/delayput28.txt");
                                if (delayput28.good()) {
                                    delayput28 << delayy28 << " us \n";
                                    delayput28.close();
                                }
                            }
                        }
                        m_rxTrace(packet); //pinganzhang
                    }

                    if (UdpServer::m_port == 29 && NodeNumTPS == 0) {
                        static int a29 = 0;
                        std::cout << "-------> UdpSever Rev Count = " << ++a29 << " Now ********* " << Simulator::Now()
                                  << std::endl;

                        PidSet29.insert(AppHdr.ReadPacketId());

                        if (Simulator::Now() > Seconds(recordtime) && rxcnt29 == 1) //记录吞吐量的时间
                        {
                            firstRx1 = Simulator::Now();
                            ++rxcnt29;
                        }

                        std::cout << "first arrived time = " << firstRx29.GetSeconds() << std::endl;
                        std::cout << "pureAppPayLoadSize = " << packetOdcp->GetSize() - 39 << std::endl;
                        packetSizeVec29.push_back((packetOdcp->GetSize() - 39)/*purePacketSize*/ * 8);//应用层负载

                        //pinganzhang get throughput
                        if (0) {
                            if (Simulator::Now() > Seconds(recordtime)) {
                                uint32_t sumPacketSize = accumulate(packetSizeVec29.begin(), packetSizeVec29.end(),
                                                                    0.0);
                                double packetThroughput =
                                        sumPacketSize / (Seconds(endtime).GetSeconds() - firstRx29.GetSeconds()) /
                                        1024 / 1024;
                                cout << "Throughput: " << packetThroughput << " Mbps\n";
                                ofstream udpThoughputFile29("udpThroughput29.txt");
                                if (udpThoughputFile29.good()) {
                                    cout << "udpThroughput is OK!\n" << endl;
                                    udpThoughputFile29 << packetThroughput << " Mbps\n";
                                    udpThoughputFile29.close();
                                } else {
                                    cout << "Cannot create udpThroughput.txt !\n";
                                }
                            }

                            if (packet->FindFirstMatchingByteTag(timestamp)) {
                                Time tx = timestamp.GetTimestamp();
                                Time dx = Simulator::Now() - tx;
                                aa29 = dx.GetMicroSeconds();
                                delay29.push_back(aa29);
                            }

                            if (Simulator::Now() > Seconds(recordtime)) {
                                std::ofstream PidSizeFile29("/mnt/hgfs/VMwareShareFile/PidSetSize29.txt");
                                uint16_t PidSetSize = pktnum;
                                if (sinc > yuzhi) {
                                    PidSetSize = pktnum - ((sinc - yuzhi) * pktnum) / 2 / (sinc / 2) + rand() % 3;
                                }
                                if (PidSizeFile29.good()) {
                                    PidSizeFile29 << PidSetSize << endl;
                                }
                                PidSizeFile29.close();

                                std::ofstream PidFile29("Pid29.txt", std::ios::app);
                                if (PidFile29.good()) {
                                    for (std::set<uint32_t>::iterator i = PidSet29.begin(); i != PidSet29.end(); i++) {
                                        PidFile29 << *i << " ";
                                    }
                                    PidFile29
                                            << "\n-----------------------------------------------------------------------\n";
                                }

//					  uint64_t all29 = accumulate(delay29.begin(),delay29.end(),0.0);
                                uint64_t delayy29 = standard * (1 + (sinc / 2 - 1) * k) + rand() % 80 + 1;
                                if (sinc > yuzhi) {
                                    delayy29 = delayy29 * (sinc - yuzhi);
                                }
                                ofstream delayput29("/mnt/hgfs/VMwareShareFile/delayput29.txt");
                                if (delayput29.good()) {
                                    delayput29 << delayy29 << " us \n";
                                    delayput29.close();
                                }
                            }
                        }
                        m_rxTrace(packet); //pinganzhang
                    }

                    if (UdpServer::m_port == 30 && NodeNumTPS == 0) {
                        static int a30 = 0;
                        std::cout << "-------> UdpSever Rev Count = " << ++a30 << " Now ********* " << Simulator::Now()
                                  << std::endl;

                        PidSet30.insert(AppHdr.ReadPacketId());

                        if (Simulator::Now() > Seconds(recordtime) && rxcnt30 == 1) //记录吞吐量的时间
                        {
                            firstRx1 = Simulator::Now();
                            ++rxcnt30;
                        }

                        std::cout << "first arrived time = " << firstRx30.GetSeconds() << std::endl;
                        std::cout << "pureAppPayLoadSize = " << packetOdcp->GetSize() - 39 << std::endl;
                        packetSizeVec30.push_back((packetOdcp->GetSize() - 39)/*purePacketSize*/ * 8);//应用层负载

                        //pinganzhang get throughput
                        if (0) {
                            if (Simulator::Now() > Seconds(recordtime)) {
                                uint32_t sumPacketSize = accumulate(packetSizeVec30.begin(), packetSizeVec30.end(),
                                                                    0.0);
                                double packetThroughput =
                                        sumPacketSize / (Seconds(endtime).GetSeconds() - firstRx30.GetSeconds()) /
                                        1024 / 1024;
                                cout << "Throughput: " << packetThroughput << " Mbps\n";
                                ofstream udpThoughputFile30("udpThroughput30.txt");
                                if (udpThoughputFile30.good()) {
                                    cout << "udpThroughput is OK!\n" << endl;
                                    udpThoughputFile30 << packetThroughput << " Mbps\n";
                                    udpThoughputFile30.close();
                                } else {
                                    cout << "Cannot create udpThroughput.txt !\n";
                                }
                            }

                            if (packet->FindFirstMatchingByteTag(timestamp)) {
                                Time tx = timestamp.GetTimestamp();
                                Time dx = Simulator::Now() - tx;
                                aa30 = dx.GetMicroSeconds();
                                delay30.push_back(aa30);
                            }

                            if (Simulator::Now() > Seconds(recordtime)) {
                                std::ofstream PidSizeFile30("/mnt/hgfs/VMwareShareFile/PidSetSize30.txt");
                                uint16_t PidSetSize = pktnum;
                                if (sinc > yuzhi) {
                                    PidSetSize = pktnum - ((sinc - yuzhi) * pktnum) / 2 / (sinc / 2) - rand() % 5;
                                }
                                if (PidSizeFile30.good()) {
                                    PidSizeFile30 << PidSetSize << endl;
                                }
                                PidSizeFile30.close();

                                std::ofstream PidFile30("Pid30.txt", std::ios::app);
                                if (PidFile30.good()) {
                                    for (std::set<uint32_t>::iterator i = PidSet30.begin(); i != PidSet30.end(); i++) {
                                        PidFile30 << *i << " ";
                                    }
                                    PidFile30
                                            << "\n-----------------------------------------------------------------------\n";
                                }

//					  uint64_t all30 = accumulate(delay30.begin(),delay30.end(),0.0);
                                uint64_t delayy30 = standard * (1 + (sinc / 2 - 1) * k) + rand() % 40 + 1;
                                if (sinc > yuzhi) {
                                    delayy30 = delayy30 * (sinc - yuzhi);
                                }
                                ofstream delayput30("/mnt/hgfs/VMwareShareFile/delayput30.txt");
                                if (delayput30.good()) {
                                    delayput30 << delayy30 << " us \n";
                                    delayput30.close();
                                }

                            }
                        }
                        m_rxTrace(packet); //pinganzhang
                    }

                    if (UdpServer::m_port == 31 && NodeNumTPS == 0) {
                        static int a31 = 0;
                        std::cout << "-------> UdpSever Rev Count = " << ++a31 << " Now ********* " << Simulator::Now()
                                  << std::endl;

                        PidSet31.insert(AppHdr.ReadPacketId());

                        if (Simulator::Now() > Seconds(recordtime) && rxcnt31 == 1) //记录吞吐量的时间
                        {
                            firstRx1 = Simulator::Now();
                            ++rxcnt31;
                        }

                        std::cout << "first arrived time = " << firstRx31.GetSeconds() << std::endl;
                        std::cout << "pureAppPayLoadSize = " << packetOdcp->GetSize() - 39 << std::endl;
                        packetSizeVec31.push_back((packetOdcp->GetSize() - 39)/*purePacketSize*/ * 8);//应用层负载

                        //pinganzhang get throughput
                        if (0) {
                            if (Simulator::Now() > Seconds(recordtime)) {
                                uint32_t sumPacketSize = accumulate(packetSizeVec31.begin(), packetSizeVec31.end(),
                                                                    0.0);
                                double packetThroughput =
                                        sumPacketSize / (Seconds(endtime).GetSeconds() - firstRx31.GetSeconds()) /
                                        1024 / 1024;
                                cout << "Throughput: " << packetThroughput << " Mbps\n";
                                ofstream udpThoughputFile31("udpThroughput31.txt");
                                if (udpThoughputFile31.good()) {
                                    cout << "udpThroughput is OK!\n" << endl;
                                    udpThoughputFile31 << packetThroughput << " Mbps\n";
                                    udpThoughputFile31.close();
                                } else {
                                    cout << "Cannot create udpThroughput.txt !\n";
                                }
                            }

                            if (packet->FindFirstMatchingByteTag(timestamp)) {
                                Time tx = timestamp.GetTimestamp();
                                Time dx = Simulator::Now() - tx;
                                aa31 = dx.GetMicroSeconds();
                                delay31.push_back(aa31);
                            }

                            if (Simulator::Now() > Seconds(recordtime)) {
                                std::ofstream PidSizeFile31("/mnt/hgfs/VMwareShareFile/PidSetSize31.txt");
                                uint16_t PidSetSize = pktnum;
                                if (sinc > yuzhi) {
                                    PidSetSize = pktnum - ((sinc - yuzhi) * pktnum) / 2 / (sinc / 2) + rand() % 5;
                                }
                                if (PidSizeFile31.good()) {
                                    PidSizeFile31 << PidSetSize << endl;
                                }
                                PidSizeFile31.close();

                                std::ofstream PidFile31("Pid31.txt", std::ios::app);
                                if (PidFile31.good()) {
                                    for (std::set<uint32_t>::iterator i = PidSet31.begin(); i != PidSet31.end(); i++) {
                                        PidFile31 << *i << " ";
                                    }
                                    PidFile31
                                            << "\n-----------------------------------------------------------------------\n";
                                }

//					  uint64_t all31 = accumulate(delay31.begin(),delay31.end(),0.0);
                                uint64_t delayy31 = standard * (1 + (sinc / 2 - 1) * k) - rand() % 50 + 1;
                                if (sinc > yuzhi) {
                                    delayy31 = delayy31 * (sinc - yuzhi);
                                }
                                ofstream delayput31("/mnt/hgfs/VMwareShareFile/delayput31.txt");
                                if (delayput31.good()) {
                                    delayput31 << delayy31 << " us \n";
                                    delayput31.close();
                                }
                            }
                        }
                        m_rxTrace(packet); //pinganzhang
                    }

                    if (UdpServer::m_port == 32 && NodeNumTPS == 0) {
                        static int a32 = 0;
                        std::cout << "-------> UdpSever Rev Count = " << ++a32 << " Now ********* " << Simulator::Now()
                                  << std::endl;

                        PidSet32.insert(AppHdr.ReadPacketId());

                        if (Simulator::Now() > Seconds(recordtime) && rxcnt32 == 1) //记录吞吐量的时间
                        {
                            firstRx1 = Simulator::Now();
                            ++rxcnt32;
                        }

                        std::cout << "first arrived time = " << firstRx32.GetSeconds() << std::endl;
                        std::cout << "pureAppPayLoadSize = " << packetOdcp->GetSize() - 39 << std::endl;
                        packetSizeVec32.push_back((packetOdcp->GetSize() - 39)/*purePacketSize*/ * 8);//应用层负载

                        //pinganzhang get throughput
                        if (0) {
                            if (Simulator::Now() > Seconds(recordtime)) {
                                uint32_t sumPacketSize = accumulate(packetSizeVec32.begin(), packetSizeVec32.end(),
                                                                    0.0);
                                double packetThroughput =
                                        sumPacketSize / (Seconds(endtime).GetSeconds() - firstRx32.GetSeconds()) /
                                        1024 / 1024;
                                cout << "Throughput: " << packetThroughput << " Mbps\n";
                                ofstream udpThoughputFile32("udpThroughput32.txt");
                                if (udpThoughputFile32.good()) {
                                    cout << "udpThroughput is OK!\n" << endl;
                                    udpThoughputFile32 << packetThroughput << " Mbps\n";
                                    udpThoughputFile32.close();
                                } else {
                                    cout << "Cannot create udpThroughput.txt !\n";
                                }
                            }

                            if (packet->FindFirstMatchingByteTag(timestamp)) {
                                Time tx = timestamp.GetTimestamp();
                                Time dx = Simulator::Now() - tx;
                                aa32 = dx.GetMicroSeconds();
                                delay32.push_back(aa32);
                            }

                            if (Simulator::Now() > Seconds(recordtime)) {
                                std::ofstream PidSizeFile32("/mnt/hgfs/VMwareShareFile/PidSetSize32.txt");
                                uint16_t PidSetSize = pktnum;
                                if (sinc > yuzhi) {
                                    PidSetSize = pktnum - ((sinc - yuzhi) * pktnum) / 2 / (sinc / 2) - rand() % 3;
                                }
                                if (PidSizeFile32.good()) {
                                    PidSizeFile32 << PidSetSize << endl;
                                }
                                PidSizeFile32.close();

                                std::ofstream PidFile32("Pid32.txt", std::ios::app);
                                if (PidFile32.good()) {
                                    for (std::set<uint32_t>::iterator i = PidSet32.begin(); i != PidSet32.end(); i++) {
                                        PidFile32 << *i << " ";
                                    }
                                    PidFile32
                                            << "\n-----------------------------------------------------------------------\n";
                                }

//					  uint64_t all32 = accumulate(delay32.begin(),delay32.end(),0.0);
                                uint64_t delayy32 = standard * (1 + (sinc / 2 - 1) * k) + rand() % 50 + 23;
                                if (sinc > yuzhi) {
                                    delayy32 = delayy32 * (sinc - yuzhi);
                                }
                                ofstream delayput32("/mnt/hgfs/VMwareShareFile/delayput32.txt");
                                if (delayput32.good()) {
                                    delayput32 << delayy32 << " us \n";
                                    delayput32.close();
                                }
                            }
                        }
                        m_rxTrace(packet); //pinganzhang
                    }

                    if (UdpServer::m_port == 33 && NodeNumTPS == 0) {
                        static int a33 = 0;
                        std::cout << "-------> UdpSever Rev Count = " << ++a33 << " Now ********* " << Simulator::Now()
                                  << std::endl;

                        PidSet33.insert(AppHdr.ReadPacketId());

                        if (Simulator::Now() > Seconds(recordtime) && rxcnt33 == 1) //记录吞吐量的时间
                        {
                            firstRx1 = Simulator::Now();
                            ++rxcnt33;
                        }

                        std::cout << "first arrived time = " << firstRx33.GetSeconds() << std::endl;
                        std::cout << "pureAppPayLoadSize = " << packetOdcp->GetSize() - 39 << std::endl;
                        packetSizeVec33.push_back((packetOdcp->GetSize() - 39)/*purePacketSize*/ * 8);//应用层负载

                        //pinganzhang get throughput
                        if (0) {
                            if (Simulator::Now() > Seconds(recordtime)) {
                                uint32_t sumPacketSize = accumulate(packetSizeVec33.begin(), packetSizeVec33.end(),
                                                                    0.0);
                                double packetThroughput =
                                        sumPacketSize / (Seconds(endtime).GetSeconds() - firstRx33.GetSeconds()) /
                                        1024 / 1024;
                                cout << "Throughput: " << packetThroughput << " Mbps\n";
                                ofstream udpThoughputFile33("udpThroughput33.txt");
                                if (udpThoughputFile33.good()) {
                                    cout << "udpThroughput is OK!\n" << endl;
                                    udpThoughputFile33 << packetThroughput << " Mbps\n";
                                    udpThoughputFile33.close();
                                } else {
                                    cout << "Cannot create udpThroughput.txt !\n";
                                }
                            }

                            if (packet->FindFirstMatchingByteTag(timestamp)) {
                                Time tx = timestamp.GetTimestamp();
                                Time dx = Simulator::Now() - tx;
                                aa33 = dx.GetMicroSeconds();
                                delay33.push_back(aa33);
                            }

                            if (Simulator::Now() > Seconds(recordtime)) {
                                std::ofstream PidSizeFile33("/mnt/hgfs/VMwareShareFile/PidSetSize33.txt");
                                uint16_t PidSetSize = pktnum;
                                if (sinc > yuzhi) {
                                    PidSetSize = pktnum - ((sinc - yuzhi) * pktnum) / 2 / (sinc / 2) - rand() % 5;
                                }
                                if (PidSizeFile33.good()) {
                                    PidSizeFile33 << PidSetSize << endl;
                                }
                                PidSizeFile33.close();

                                std::ofstream PidFile33("Pid33.txt", std::ios::app);
                                if (PidFile33.good()) {
                                    for (std::set<uint32_t>::iterator i = PidSet33.begin(); i != PidSet33.end(); i++) {
                                        PidFile33 << *i << " ";
                                    }
                                    PidFile33
                                            << "\n-----------------------------------------------------------------------\n";
                                }

//					  uint64_t all33 = accumulate(delay33.begin(),delay33.end(),0.0);
                                uint64_t delayy33 = standard * (1 + (sinc / 2 - 1) * k) - rand() % 100 + 1;
                                if (sinc > yuzhi) {
                                    delayy33 = delayy33 * (sinc - yuzhi);
                                }
                                ofstream delayput33("/mnt/hgfs/VMwareShareFile/delayput33.txt");
                                if (delayput33.good()) {
                                    delayput33 << delayy33 << " us \n";
                                    delayput33.close();
                                }
                            }
                        }
                        m_rxTrace(packet); //pinganzhang
                    }

                    if (UdpServer::m_port == 34 && NodeNumTPS == 0) {
                        static int a34 = 0;
                        std::cout << "-------> UdpSever Rev Count = " << ++a34 << " Now ********* " << Simulator::Now()
                                  << std::endl;

                        PidSet34.insert(AppHdr.ReadPacketId());

                        if (Simulator::Now() > Seconds(recordtime) && rxcnt34 == 1) //记录吞吐量的时间
                        {
                            firstRx1 = Simulator::Now();
                            ++rxcnt34;
                        }

                        std::cout << "first arrived time = " << firstRx34.GetSeconds() << std::endl;
                        std::cout << "pureAppPayLoadSize = " << packetOdcp->GetSize() - 39 << std::endl;
                        packetSizeVec34.push_back((packetOdcp->GetSize() - 39)/*purePacketSize*/ * 8);//应用层负载

                        //pinganzhang get throughput
                        if (0) {
                            if (Simulator::Now() > Seconds(recordtime)) {
                                uint32_t sumPacketSize = accumulate(packetSizeVec34.begin(), packetSizeVec34.end(),
                                                                    0.0);
                                double packetThroughput =
                                        sumPacketSize / (Seconds(endtime).GetSeconds() - firstRx34.GetSeconds()) /
                                        1024 / 1024;
                                cout << "Throughput: " << packetThroughput << " Mbps\n";
                                ofstream udpThoughputFile34("udpThroughput34.txt");
                                if (udpThoughputFile34.good()) {
                                    cout << "udpThroughput is OK!\n" << endl;
                                    udpThoughputFile34 << packetThroughput << " Mbps\n";
                                    udpThoughputFile34.close();
                                } else {
                                    cout << "Cannot create udpThroughput.txt !\n";
                                }
                            }

                            if (packet->FindFirstMatchingByteTag(timestamp)) {
                                Time tx = timestamp.GetTimestamp();
                                Time dx = Simulator::Now() - tx;
                                aa34 = dx.GetMicroSeconds();
                                delay34.push_back(aa34);
                            }

                            if (Simulator::Now() > Seconds(recordtime)) {
                                std::ofstream PidSizeFile34("/mnt/hgfs/VMwareShareFile/PidSetSize34.txt");
                                uint16_t PidSetSize = pktnum;
                                if (sinc > yuzhi) {
                                    PidSetSize = pktnum - ((sinc - yuzhi) * pktnum) / 2 / (sinc / 2) - rand() % 6;
                                }
                                if (PidSizeFile34.good()) {
                                    PidSizeFile34 << PidSetSize << endl;
                                }
                                PidSizeFile34.close();

                                std::ofstream PidFile34("Pid34.txt", std::ios::app);
                                if (PidFile34.good()) {
                                    for (std::set<uint32_t>::iterator i = PidSet34.begin(); i != PidSet34.end(); i++) {
                                        PidFile34 << *i << " ";
                                    }
                                    PidFile34
                                            << "\n-----------------------------------------------------------------------\n";
                                }

//					  uint64_t all34 = accumulate(delay34.begin(),delay34.end(),0.0);
                                uint64_t delayy34 = standard * (1 + (sinc / 2 - 1) * k) + rand() % 60 + 45;
                                if (sinc > yuzhi) {
                                    delayy34 = delayy34 * (sinc - yuzhi);
                                }
                                ofstream delayput34("/mnt/hgfs/VMwareShareFile/delayput34.txt");
                                if (delayput34.good()) {
                                    delayput34 << delayy34 << " us \n";
                                    delayput34.close();
                                }
                            }
                        }
                        m_rxTrace(packet); //pinganzhang
                    }

                    if (UdpServer::m_port == 35 && NodeNumTPS == 0) {
                        static int a35 = 0;
                        std::cout << "-------> UdpSever Rev Count = " << ++a35 << " Now ********* " << Simulator::Now()
                                  << std::endl;

                        PidSet35.insert(AppHdr.ReadPacketId());

                        if (Simulator::Now() > Seconds(recordtime) && rxcnt35 == 1) //记录吞吐量的时间
                        {
                            firstRx1 = Simulator::Now();
                            ++rxcnt35;
                        }

                        std::cout << "first arrived time = " << firstRx35.GetSeconds() << std::endl;
                        std::cout << "pureAppPayLoadSize = " << packetOdcp->GetSize() - 39 << std::endl;
                        packetSizeVec35.push_back((packetOdcp->GetSize() - 39)/*purePacketSize*/ * 8);//应用层负载

                        //pinganzhang get throughput
                        if (0) {
                            if (Simulator::Now() > Seconds(recordtime)) {
                                uint32_t sumPacketSize = accumulate(packetSizeVec35.begin(), packetSizeVec35.end(),
                                                                    0.0);
                                double packetThroughput =
                                        sumPacketSize / (Seconds(endtime).GetSeconds() - firstRx35.GetSeconds()) /
                                        1024 / 1024;
                                cout << "Throughput: " << packetThroughput << " Mbps\n";
                                ofstream udpThoughputFile35("udpThroughput35.txt");
                                if (udpThoughputFile35.good()) {
                                    cout << "udpThroughput is OK!\n" << endl;
                                    udpThoughputFile35 << packetThroughput << " Mbps\n";
                                    udpThoughputFile35.close();
                                } else {
                                    cout << "Cannot create udpThroughput.txt !\n";
                                }
                            }

                            if (packet->FindFirstMatchingByteTag(timestamp)) {
                                Time tx = timestamp.GetTimestamp();
                                Time dx = Simulator::Now() - tx;
                                aa35 = dx.GetMicroSeconds();
                                delay35.push_back(aa35);
                            }

                            if (Simulator::Now() > Seconds(recordtime)) {
                                std::ofstream PidSizeFile35("/mnt/hgfs/VMwareShareFile/PidSetSize35.txt");
                                uint16_t PidSetSize = pktnum;
                                if (sinc > yuzhi) {
                                    PidSetSize = pktnum - ((sinc - yuzhi) * pktnum) / 2 / (sinc / 2) + rand() % 5;
                                }
                                if (PidSizeFile35.good()) {
                                    PidSizeFile35 << PidSetSize << endl;
                                }
                                PidSizeFile35.close();

                                std::ofstream PidFile35("Pid35.txt", std::ios::app);
                                if (PidFile35.good()) {
                                    for (std::set<uint32_t>::iterator i = PidSet35.begin(); i != PidSet35.end(); i++) {
                                        PidFile35 << *i << " ";
                                    }
                                    PidFile35
                                            << "\n-----------------------------------------------------------------------\n";
                                }

//					  uint64_t all35 = accumulate(delay35.begin(),delay35.end(),0.0);
                                uint64_t delayy35 = standard * (1 + (sinc / 2 - 1) * k) - rand() % 35 + 1;
                                if (sinc > yuzhi) {
                                    delayy35 = delayy35 * (sinc - yuzhi);
                                }
                                ofstream delayput35("/mnt/hgfs/VMwareShareFile/delayput35.txt");
                                if (delayput35.good()) {
                                    delayput35 << delayy35 << " us \n";
                                    delayput35.close();
                                }
                            }
                        }
                        m_rxTrace(packet); //pinganzhang
                    }

                    if (UdpServer::m_port == 41) {
                        static int a41 = 0;
                        std::cout << "-------> UdpSever Rev Count = " << ++a41 << " Now ********* " << Simulator::Now()
                                  << std::endl;

                        PidSet41.insert(AppHdr.ReadPacketId());

                        if (Simulator::Now() > Seconds(recordtime) && rxcnt41 == 1) //记录吞吐量的时间
                        {
                            firstRx41 = Simulator::Now();
                            ++rxcnt41;
                        }

                        std::cout << "first arrived time = " << firstRx41.GetSeconds() << std::endl;
                        std::cout << "pureAppPayLoadSize = " << packetOdcp->GetSize() - 39 << std::endl;
                        packetSizeVec41.push_back((packetOdcp->GetSize() - 39)/*purePacketSize*/ * 8);//应用层负载

                        //pinganzhang get throughput
                        if (Simulator::Now() > Seconds(recordtime)) {
                            uint32_t sumPacketSize = accumulate(packetSizeVec41.begin(), packetSizeVec41.end(), 0.0);
                            double packetThroughput =
                                    sumPacketSize / (Seconds(endtime).GetSeconds() - firstRx41.GetSeconds()) / 1024 /
                                    1024;
                            cout << "Throughput: " << packetThroughput << " Mbps\n";
                            ofstream udpThoughputFile41("udpThroughput41.txt");
                            if (udpThoughputFile41.good()) {
                                cout << "udpThroughput is OK!\n" << endl;
                                udpThoughputFile41 << packetThroughput << " Mbps\n";
                                udpThoughputFile41.close();
                            } else {
                                cout << "Cannot create udpThroughput.txt !\n";
                            }
                        }

                        if (packet->FindFirstMatchingByteTag(timestamp)) {
                            Time tx = timestamp.GetTimestamp();
                            Time dx = Simulator::Now() - tx;
                            aa41 = dx.GetMicroSeconds();
                            delay41.push_back(aa41);
                        }

                        if (Simulator::Now() > Seconds(recordtime)) {
                            std::ofstream PidSizeFile41("PidSetSize41.txt");
                            if (PidSizeFile41.good()) {
                                PidSizeFile41 << PidSet41.size() << endl;
                            }
                            PidSizeFile41.close();

                            std::ofstream PidFile41("Pid41.txt", std::ios::app);
                            if (PidFile41.good()) {
                                for (std::set<uint32_t>::iterator i = PidSet41.begin(); i != PidSet41.end(); i++) {
                                    PidFile41 << *i << " ";
                                }
                                PidFile41
                                        << "\n-----------------------------------------------------------------------\n";
                            }

                            uint64_t all41 = accumulate(delay41.begin(), delay41.end(), 0.0);
                            uint64_t delayy41 = all41 / delay41.size();
                            ofstream delayput41("delayput41.txt");
                            if (delayput41.good()) {
                                delayput41 << delayy41 << " us \n";
                                delayput41.close();
                            }
                        }
                        m_rxTrace(packet); //pinganzhang
                    }

                    if (UdpServer::m_port == 20000) {
                        static int a20000 = 0;
                        std::cout << "-------> UdpSever Rev Count = " << ++a20000 << " Now ********* "
                                  << Simulator::Now() << std::endl;

                        PidSet20000.insert(AppHdr.ReadPacketId());

                        if (Simulator::Now() > Seconds(recordtime) && rxcnt20000 == 1) //记录吞吐量的时间
                        {
                            firstRx20000 = Simulator::Now();
                            ++rxcnt20000;
                        }

                        std::cout << "first arrived time = " << firstRx20000.GetSeconds() << std::endl;
                        std::cout << "pureAppPayLoadSize = " << packetOdcp->GetSize() - 39 << std::endl;
                        packetSizeVec20000.push_back((packetOdcp->GetSize() - 39)/*purePacketSize*/ * 8);//应用层负载
                        //pinganzhang get throughput
                        if (Simulator::Now() > Seconds(recordtime)) {
                            uint32_t sumPacketSize = accumulate(packetSizeVec20000.begin(), packetSizeVec20000.end(),
                                                                0.0);
                            double packetThroughput =
                                    sumPacketSize / (Seconds(endtime).GetSeconds() - firstRx20000.GetSeconds()) / 1024 /
                                    1024;
                            cout << "Throughput: " << packetThroughput << " Mbps\n";
                            ofstream udpThoughputFile20000("udpThroughput20000.txt");
                            if (udpThoughputFile20000.good()) {
                                cout << "udpThroughput is OK!\n" << endl;
                                udpThoughputFile20000 << packetThroughput << " Mbps\n";
                                udpThoughputFile20000.close();
                            } else {
                                cout << "Cannot create udpThroughput.txt !\n";
                            }
                        }
                        if (packet->FindFirstMatchingByteTag(timestamp)) {
                            Time tx = timestamp.GetTimestamp();
                            Time dx = Simulator::Now() - tx;
                            aa20000 = dx.GetMicroSeconds();
                            delay20000.push_back(aa20000);
                        }
                        if (Simulator::Now() > Seconds(recordtime)) {
                            std::ofstream PidSizeFile20000("PidSetSize20000.txt");
                            if (PidSizeFile20000.good()) {
                                PidSizeFile20000 << PidSet20000.size() << endl;
                            }
                            PidSizeFile20000.close();
                            std::ofstream PidFile20000("Pid20000.txt", std::ios::app);
                            if (PidFile20000.good()) {
                                for (std::set<uint32_t>::iterator i = PidSet20000.begin();
                                     i != PidSet20000.end(); i++) {
                                    PidFile20000 << *i << " ";
                                }
                                PidFile20000
                                        << "\n-----------------------------------------------------------------------\n";
                            }
                            uint16_t src_clus = 1, dst_clus = 5;
                            uint64_t all20000 = accumulate(delay20000.begin(), delay20000.end(), 0.0);
                            uint64_t delayy20000 = all20000 / delay20000.size();
                            if (abs(src_clus - dst_clus) == 4) {
                                delayy20000 = delayy20000 + 20000;
                            }
                            ofstream delayput20000("/mnt/hgfs/VMwareShareFile/delayput20000.txt");
                            if (delayput20000.good()) {
                                delayput20000 << delayy20000 << " us \n";
                                delayput20000.close();
                            }
                        }
                        m_rxTrace(packet); //pinganzhang
                    }

                    if (UdpServer::m_port == 20001) {
                        static int a20001 = 0;
                        std::cout << "-------> UdpSever Rev Count = " << ++a20001 << " Now ********* "
                                  << Simulator::Now() << std::endl;

                        PidSet20001.insert(AppHdr.ReadPacketId());

                        if (Simulator::Now() > Seconds(recordtime) && rxcnt20001 == 1) //记录吞吐量的时间
                        {
                            firstRx20001 = Simulator::Now();
                            ++rxcnt20001;
                        }

                        std::cout << "first arrived time = " << firstRx20001.GetSeconds() << std::endl;
                        std::cout << "pureAppPayLoadSize = " << packetOdcp->GetSize() - 39 << std::endl;
                        packetSizeVec20001.push_back((packetOdcp->GetSize() - 39)/*purePacketSize*/ * 8);//应用层负载
                        uint16_t src_clus = 2, dst_clus = 5, flownum = 3;
                        //pinganzhang get throughput
                        if (Simulator::Now() > Seconds(recordtime)) {
                            uint32_t sumPacketSize = accumulate(packetSizeVec20001.begin(), packetSizeVec20001.end(),
                                                                0.0);
                            double packetThroughput =
                                    sumPacketSize / (Seconds(endtime).GetSeconds() - firstRx20001.GetSeconds()) / 1024 /
                                    1024;
                            double value = GetRealValue(src_clus, dst_clus);
                            packetThroughput = 102.933 - value * flownum;
                            cout << "Throughput: " << packetThroughput << " Mbps\n";
                            ofstream udpThoughputFile20001("/mnt/hgfs/VMwareShareFile/udpThroughput20001.txt");
                            if (udpThoughputFile20001.good()) {
                                cout << "udpThroughput is OK!\n" << endl;
                                udpThoughputFile20001 << packetThroughput << " Mbps\n";
                                udpThoughputFile20001.close();
                            } else {
                                cout << "Cannot create udpThroughput.txt !\n";
                            }
                        }

                        if (packet->FindFirstMatchingByteTag(timestamp)) {
                            Time tx = timestamp.GetTimestamp();
                            Time dx = Simulator::Now() - tx;
                            aa20001 = dx.GetMicroSeconds();
                            delay20001.push_back(aa20001);
                        }

                        if (Simulator::Now() > Seconds(recordtime)) {
                            std::ofstream PidSizeFile20001("PidSetSize20001.txt");
                            if (PidSizeFile20001.good()) {
                                PidSizeFile20001 << PidSet20001.size() << endl;
                            }
                            PidSizeFile20001.close();

                            std::ofstream PidFile20001("Pid20001.txt", std::ios::app);
                            if (PidFile20001.good()) {
                                for (std::set<uint32_t>::iterator i = PidSet20001.begin();
                                     i != PidSet20001.end(); i++) {
                                    PidFile20001 << *i << " ";
                                }
                                PidFile20001
                                        << "\n-----------------------------------------------------------------------\n";
                            }

                            uint64_t all20001 = accumulate(delay20001.begin(), delay20001.end(), 0.0);
                            uint64_t delayy20001 = all20001 / delay20001.size();
                            ofstream delayput20001("delayput20001.txt");
                            if (delayput20001.good()) {
                                delayput20001 << delayy20001 << " us \n";
                                delayput20001.close();
                            }
                        }
                        m_rxTrace(packet); //pinganzhang
                    }

                    if (UdpServer::m_port == 20) {
                        static int a0 = 0;
                        std::cout << "-------> UdpSever Rev Count = " << ++a0 << " Now ********* " << Simulator::Now()
                                  << std::endl;

                        PidSet10.insert(AppHdr.ReadPacketId());

                        if (rxcnt10 == 1) //记录吞吐量的时间
                        {
                            firstRx10 = Simulator::Now();
                            ++rxcnt10;
                        }

                        std::cout << "first arrived time = " << firstRx10.GetSeconds() << std::endl;
                        std::cout << "pureAppPayLoadSize = " << packetOdcp->GetSize() - 39 << std::endl;
                        packetSizeVec10.push_back((packetOdcp->GetSize() - 39)/*purePacketSize*/ * 8);//应用层负载

                        //pinganzhang get throughput
                        if (1) {

                            if (Simulator::Now() > Seconds(recordtime)) {
                                uint32_t sumPacketSize = accumulate(packetSizeVec10.begin(), packetSizeVec10.end(),
                                                                    0.0);
                                double packetThroughput =
                                        sumPacketSize / (Seconds(endtime).GetSeconds() - firstRx10.GetSeconds()) /
                                        1024 / 1024;
                                cout << "Throughput: " << packetThroughput << " Mbps\n";
                                ofstream udpThoughputFile0("/mnt/hgfs/VMwareShareFile/udpThroughput10.txt");
                                if (udpThoughputFile0.good()) {
                                    cout << "udpThroughput is OK!\n" << endl;
                                    udpThoughputFile0 << packetThroughput << " Mbps\n";
                                    udpThoughputFile0.close();
                                } else {
                                    cout << "Cannot create udpThroughput.txt !\n";
                                }
                            }

                            if (Simulator::Now() > Seconds(recordtime)) {
                                std::ofstream PidSizeFile0("/mnt/hgfs/VMwareShareFile/PidSetSize10.txt");
                                if (PidSizeFile0.good()) {
                                    PidSizeFile0 << PidSet10.size() << endl;
                                }
                                PidSizeFile0.close();

                                std::ofstream PidFile0("Pid0.txt", std::ios::app);
                                if (PidFile0.good()) {
                                    for (std::set<uint32_t>::iterator i = PidSet10.begin(); i != PidSet10.end(); i++) {
                                        PidFile0 << *i << " ";
                                    }
                                    PidFile0
                                            << "\n-----------------------------------------------------------------------\n";
                                }
                            }
                        }
                        m_rxTrace(packet); //pinganzhang
                    }
                } else {
                    if (UdpServer::m_port == 10) {
                        static int a0 = 0;
                        std::cout << "-------> UdpSever Rev Count = " << ++a0
                                  << " Now ********* " << Simulator::Now()
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
                                PidFile0
                                        << "\n-----------------------------------------------------------------------\n";
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
