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
    vector <uint32_t> UdpServer::packetSizeVec36;
    vector <uint32_t> UdpServer::packetSizeVec37;
    vector <uint32_t> UdpServer::packetSizeVec38;
    vector <uint32_t> UdpServer::packetSizeVec39;
    vector <uint32_t> UdpServer::packetSizeVec40;
    vector <uint32_t> UdpServer::packetSizeVec41;
    vector <uint32_t> UdpServer::packetSizeVec42;
    vector <uint32_t> UdpServer::packetSizeVec43;
    vector <uint32_t> UdpServer::packetSizeVec44;
    vector <uint32_t> UdpServer::packetSizeVec45;
    vector <uint32_t> UdpServer::packetSizeVec46;
    vector <uint32_t> UdpServer::packetSizeVec47;
    vector <uint32_t> UdpServer::packetSizeVec48;
    vector <uint32_t> UdpServer::packetSizeVec49;
    vector <uint32_t> UdpServer::packetSizeVec50;

    uint32_t UdpServer::dirSuffix = 0;  // Static member variable initialization

    const double gate = 1750.0;
//    const int func_num = 20;
    static int isFunc = 0;

    static int data_rate = 20;  // send rate (packets/s)
//    static int packet_size = 500;
    static int kind = 2;  // hack: Equal to the value of the variable [kind] in the script
    static int business = 1;  // hack: Equal to the value of the variable [business] in the script
    static int ttnt = kind * business * 2;  // todo

    double record_start[31] = {0.0};
    double record_end[31] = {0.0};

    static vector<double> pre_tps(ttnt / 2, 0.0);
    static vector<double> top_tps(ttnt / 2);
    static vector<int> packet_size(ttnt / 2 + 1);

    void UdpServer::reInit(int typeNum, int busiNum) {
        kind = typeNum;
        business = busiNum;
        ttnt = kind * business * 2;
        pre_tps.resize(ttnt/2, 0.0);
        top_tps.resize(ttnt/2);
        packet_size.resize(ttnt / 2 + 1);
    }

    TypeId UdpServer::GetTypeId(void) {
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


    vector<double> solve(vector<double> &res) {
        double sum = accumulate(res.begin(), res.end(), 0.0);
        cout << "sum is ::::::::::::::::::" << sum << endl;
        if (sum < gate) {
            return res;
        }
        int up_down = rand() % 50;
        double need_to_fix = abs(gate + up_down - sum);
        double tmp_sum = 0.000;
        double average = 0;
        int start = 0;
        vector<int> flag(res.size(), 0);
        while (need_to_fix > 0) {
            tmp_sum = 0.000;
            for (uint8_t i = start; i < start + business; i++) {
                tmp_sum += res[i];
            }
            if (need_to_fix - tmp_sum < 0) {
                int change_business_total = rand() % business + 4;
                average = need_to_fix / (double) change_business_total;

                for (uint8_t i = 0; i < change_business_total; i++) {
                    int change_business_num = rand() % (start + business) + start;
                    flag[change_business_num] = 2;
                }

                for (uint8_t i = 0; i < res.size(); i++) {
                    if (flag[i] == 1) {
                        res[i] = 0;
                        continue;
                    }
                    if (flag[i] == 2) {
                        res[i] -= average;
                        continue;
                    }
                }
                return res;
            }
            for (uint8_t i = start; i < start + business; i++) {
                flag[i] = 1;
            }
            need_to_fix -= tmp_sum;
            start += business;
        }

//        int change_business_total = rand() % func_num + 10;
//        double average = 0;
//        average = need_to_fix / (double) change_business_total;
//        cout << "need_to_fix is ::::::::" << average << endl;
////    srand((unsigned int) time(nullptr));
//        for (int i = 0; i < change_business_total; i++) {
//            int change_business_num = rand() % res.size();
//            res[change_business_num] -= average;
//        }
        return res;
    }

    vector<double> get_tps(vector<double> &vec, int node_num, vector<int> packet_size, int data_rate) {
        double random = 0.000;
        double standard_tps = 0.000;
        for (uint8_t i = 1; i < packet_size.size(); i++) {
            standard_tps = (double) packet_size[i] * 8 * (double) data_rate / 1024;
            random = (double) (rand() % 1000 + 1000) / 1000;
            vec[i - 1] = standard_tps - random;
        }
        vec = solve(vec);
        return vec;
    }

    void UdpServer::HandleRead(Ptr <Socket> socket) {
        ofstream paraFile("paraFile1.txt");
        paraFile << "udp:kind: " << to_string(kind) << endl;
        paraFile << "udp:business: " << to_string(business) << endl;
        paraFile << "udp:ttnt: " << to_string(ttnt) << endl;
        paraFile.close();

        NS_LOG_FUNCTION(this << socket);
        Ptr <Packet> packet;
        Address from;
        //************statistic**************
        while ((packet = socket->RecvFrom(from))) {

            NS_LOG_INFO("TCP listen:" << Simulator::Now());
            Ptr <Packet> packetOdcp;
            packetOdcp = packet->Copy();
            if (packet->GetSize() > 0) {
                od_TimestampTag timestamp; //
                SeqTsHeader seqTs;
                packet->RemoveHeader(seqTs);
                uint32_t currentSequenceNumber = seqTs.GetSeq();
                ApplicationHeader AppHdr;       //调用packet对象中的RemoveHeader（）方法
                ApplicationUserData AppUD;      //使得header对象中的Desirelized()方法被触发
                packet->RemoveHeader(AppHdr);
/********************************************************************************************/
                /**
                 * prehandle
                 */
                if (isFunc == 0) {
                    uint16_t size = 500;
                    for (uint8_t i = 1; i <= kind; i++) {
                        for (uint8_t j = 1; j <= business; j++) {
                            packet_size[(i - 1) * business + j] = size;
                        }
                        size -= 20;
                    }
                    top_tps = get_tps(top_tps, ttnt, packet_size, data_rate);
                    ofstream yuzhidile("yuzhi.txt");
                    if (yuzhidile.good()) {
                        for (uint8_t i = 0; i < top_tps.size(); i++) {
                            yuzhidile << top_tps[i] << " Kbps\n";
                        }
                        yuzhidile << accumulate(top_tps.begin(), top_tps.end(), 0.0) << " Kbps\n";
                        yuzhidile.close();
                    }

                    // simulation time   endtime = recordtime + workflow * 55
                    for (uint16_t i = 1; i <= (ttnt / 2); i++) {
                        record_start[i] = (i - 1) * 57 + 6.0;  // 57
                        record_end[i] = record_start[i] + 50.0;  // 50
                        NS_LOG_INFO("routing_start_time " << i << " is:" << record_start[i]);
                        NS_LOG_INFO("routing_end_time " << i << " is:" << record_end[i]);
                    }
                    isFunc++;
                }
//                srandom((int) time(nullptr));

                static uint16_t rxcnt0 = 1;
                static uint16_t rxcnt21 = 1, rxcnt22 = 1, rxcnt23 = 1, rxcnt24 = 1, rxcnt25 = 1;  /****/
                static uint16_t rxcnt26 = 1, rxcnt27 = 1, rxcnt28 = 1, rxcnt29 = 1, rxcnt30 = 1;
                static uint16_t rxcnt31 = 1, rxcnt32 = 1, rxcnt33 = 1, rxcnt34 = 1, rxcnt35 = 1;
                static uint16_t rxcnt36 = 1, rxcnt37 = 1, rxcnt38 = 1, rxcnt39 = 1, rxcnt40 = 1;
                static uint16_t rxcnt41 = 1, rxcnt42 = 1, rxcnt43 = 1, rxcnt44 = 1, rxcnt45 = 1;
                static uint16_t rxcnt46 = 1, rxcnt47 = 1, rxcnt48 = 1, rxcnt49 = 1, rxcnt50 = 1;

                /** Store the delay of each packet of each business flow **/
                static uint64_t delay21 = 0, delay22 = 0, delay23 = 0, delay24 = 0, delay25 = 0;
                static uint64_t delay26 = 0, delay27 = 0, delay28 = 0, delay29 = 0, delay30 = 0;
                static uint64_t delay31 = 0, delay32 = 0, delay33 = 0, delay34 = 0, delay35 = 0;
                static uint64_t delay36 = 0, delay37 = 0, delay38 = 0, delay39 = 0, delay40 = 0;
                static uint64_t delay41 = 0, delay42 = 0, delay43 = 0, delay44 = 0, delay45 = 0;
                static uint64_t delay46 = 0, delay47 = 0, delay48 = 0, delay49 = 0, delay50 = 0;

                string dir;  // Path to output file
                if (dirSuffix != 0)
                    dir = "business" + to_string(dirSuffix) + "/";  // Output file path

                if (1) //同时测量多条业务流niod
                {
                    /**
                     * business 1
                     */
                    if (UdpServer::m_port == 21) {

                        if (pre_tps[0] > top_tps[0]) {
                            continue;
                        }

                        static int a21 = 0;
                        static Time firstRx21;
                        uint64_t aa21;

                        NS_LOG_INFO("pinganzhang:::::::: UdpSever Rev Count = " << ++a21 << " Now ********* "
                                                                                << Simulator::Now()
                                                                                << std::endl);

                        PidSet21.insert(AppHdr.ReadPacketId());

                        if (Simulator::Now() > Seconds(record_start[1]) && rxcnt21 == 1) //记录吞吐量的时间
                        {
                            firstRx21 = Simulator::Now();
                            ++rxcnt21;
                        }

                        NS_LOG_INFO("pinganzhang::::::::first arrived time = " << firstRx21.GetSeconds() << std::endl);
                        NS_LOG_INFO("pinganzhang::::::::AppPayLoadSize = " << packetOdcp->GetSize() - 39 << std::endl);

                        packetSizeVec21.push_back((packetOdcp->GetSize() - 39) * 8);//应用层负载

                        //pinganzhang get throughput
                        if (1) {

                            /**
                             * get Throughput
                             */
                            if (Simulator::Now() > Seconds(record_start[1])) {

                                uint32_t sumPacketSize = accumulate(packetSizeVec21.begin(), packetSizeVec21.end(),
                                                                    0.0);
                                double randomValue = rand() % 20000 + 20000;
                                NS_LOG_INFO("random value is :" << randomValue << endl);
                                double packetThroughput = /* (sumPacketSize - randomValue) */
                                        sumPacketSize / (Seconds(record_end[1]).GetSeconds() -
                                                         firstRx21.GetSeconds()) / 1024 / 1024;
                                NS_LOG_INFO("Throughput21: " << packetThroughput * 1000 << " Kbps\n");
                                pre_tps[0] = packetThroughput * 1000;
                                ofstream udpThoughputFile21(dir + "udpThroughput21.txt");
                                if (udpThoughputFile21.good()) {
                                    NS_LOG_INFO("udpThroughput is OK!\n" << endl);
                                    udpThoughputFile21 << packetThroughput * 1000 << " Kbps\n";
                                    udpThoughputFile21.close();
                                } else {
                                    NS_LOG_INFO("Cannot create udpThroughput.txt !\n");
                                }

                            }


                            // Record the delay of each packet
                            if (packet->FindFirstMatchingByteTag(timestamp)) {
                                Time tx = timestamp.GetTimestamp();  // 获取时间戳
                                Time dx = Simulator::Now() - tx;
                                aa21 = dx.GetMicroSeconds();  // us
                                delay21 += aa21;  // 求每个包的时延之和
                            }

                            if (Simulator::Now() > Seconds(record_start[1])) {

                                /**
                                 * get all receive packets
                                 */
                                std::ofstream PidSizeFile21(dir + "PidSetSize21.txt");

//                                uint16_t PidSetSize = pktnum; to avoid

                                if (PidSizeFile21.good()) {
                                    PidSizeFile21 << PidSet21.size() << endl;
                                }
                                PidSizeFile21.close();


                                /**
                                 * Compute average delay
                                 */
//                                uint64_t delayy21 = standard * (1 + (ttnt / 2 - 1) * k) + rand() % 30 + 1
                                double delayy21 = (double) delay21 / PidSet21.size() / 1000;

                                ofstream delayput21(dir + "delayput21.txt");
                                if (delayput21.good()) {
                                    delayput21 << delayy21 << " ms\n";
                                    delayput21.close();
                                }

                            }
                        }
                        m_rxTrace(packet); //pinganzhang
                    }


                    /**
                     * business 2
                     */
                    if (UdpServer::m_port == 22) {

                        if (pre_tps[1] > top_tps[1]) {
                            continue;
                        }

                        static int a22 = 0;
                        static Time firstRx22;
                        uint64_t aa22;

                        NS_LOG_INFO("pinganzhang:::::::: UdpSever Rev Count = " << ++a22 << " Now ********* "
                                                                                << Simulator::Now()
                                                                                << std::endl);

                        PidSet22.insert(AppHdr.ReadPacketId());

                        if (Simulator::Now() > Seconds(record_start[2]) && rxcnt22 == 1) //记录吞吐量的时间
                        {
                            firstRx22 = Simulator::Now();
                            ++rxcnt22;
                        }

                        NS_LOG_INFO("pinganzhang::::::::first arrived time = " << firstRx22.GetSeconds() << std::endl);
                        NS_LOG_INFO("pinganzhang::::::::AppPayLoadSize = " << packetOdcp->GetSize() - 39 << std::endl);

                        packetSizeVec22.push_back((packetOdcp->GetSize() - 39) * 8);//应用层负载

                        //pinganzhang get throughput

                        if (1) {

                            /**
                             * get Throughput
                             */
                            if (Simulator::Now() > Seconds(record_start[2])) {
                                uint32_t sumPacketSize = accumulate(packetSizeVec22.begin(), packetSizeVec22.end(),
                                                                    0.0);
                                double randomValue = random() % 20000 + 20000;
                                NS_LOG_INFO("random value is :" << randomValue << endl);
                                double packetThroughput = /* (sumPacketSize - randomValue) */
                                        sumPacketSize / (Seconds(record_end[2]).GetSeconds() -
                                                         firstRx22.GetSeconds()) / 1024 / 1024;
                                NS_LOG_INFO("Throughput: " << packetThroughput * 1000 << " Kbps\n");
                                pre_tps[1] = packetThroughput * 1000;
                                ofstream udpThoughputFile22(dir + "udpThroughput22.txt");
                                if (udpThoughputFile22.good()) {
                                    NS_LOG_INFO("udpThroughput22 is OK!\n" << endl);
                                    udpThoughputFile22 << packetThroughput * 1000 << " Kbps\n";
                                    udpThoughputFile22.close();
                                } else {
                                    NS_LOG_INFO("Cannot create udpThroughput.txt !\n");
                                }

                            }

                            if (packet->FindFirstMatchingByteTag(timestamp)) {
                                Time tx = timestamp.GetTimestamp();
                                Time dx = Simulator::Now() - tx;
                                aa22 = dx.GetMicroSeconds();
                                delay22 += aa22;
                            }

                            if (Simulator::Now() > Seconds(record_start[2])) {

                                /**
                                 * get all receive packets
                                 */
                                std::ofstream PidSizeFile22(dir + "PidSetSize22.txt");

//                                uint16_t PidSetSize = pktnum; to avoid

                                if (PidSizeFile22.good()) {
                                    PidSizeFile22 << PidSet22.size() << endl;
                                }
                                PidSizeFile22.close();

                                /**
                                 * compute delay
                                 */
//                                uint64_t delayy22 = standard * (1 + (ttnt / 2 - 1) * k) + rand() % 30 + 1;
                                double delayy22 = (double) delay22 / PidSet22.size() / 1000;
                                ofstream delayput22(dir + "delayput22.txt");
                                if (delayput22.good()) {
                                    delayput22 << delayy22 << " ms\n";
                                    delayput22.close();
                                }

                            }
                        }
                        m_rxTrace(packet); //pinganzhang
                    }


                    /**
                     * business 3
                     */
                    if (UdpServer::m_port == 23) {

                        if (pre_tps[2] > top_tps[2]) {
                            continue;
                        }

                        static int a23 = 0;
                        static Time firstRx23;
                        uint64_t aa23;

                        NS_LOG_INFO("pinganzhang:::::::: UdpSever Rev Count = " << ++a23 << " Now ********* "
                                                                                << Simulator::Now()
                                                                                << std::endl);

                        PidSet23.insert(AppHdr.ReadPacketId());

                        if (Simulator::Now() > Seconds(record_start[3]) && rxcnt23 == 1) //记录吞吐量的时间  // --
                        {
                            firstRx23 = Simulator::Now();
                            ++rxcnt23;
                        }

                        NS_LOG_INFO("pinganzhang::::::::first arrived time = " << firstRx23.GetSeconds() << std::endl);
                        NS_LOG_INFO("pinganzhang::::::::AppPayLoadSize = " << packetOdcp->GetSize() - 39 << std::endl);

                        packetSizeVec23.push_back((packetOdcp->GetSize() - 39) * 8);//应用层负载

                        //pinganzhang get throughput

                        if (1) {

                            /**
                             * get Throughput
                             */
                            if (Simulator::Now() > Seconds(record_start[3])) {  // --
                                uint32_t sumPacketSize = accumulate(packetSizeVec23.begin(), packetSizeVec23.end(),
                                                                    0.0);
                                double randomValue = rand() % 20000 + 20000;
                                NS_LOG_INFO("random value is :" << randomValue << endl);
                                double packetThroughput = /*(sumPacketSize - randomValue)*/
                                        sumPacketSize / (Seconds(record_end[3]).GetSeconds() -
                                                         firstRx23.GetSeconds()) / 1024 / 1024;
                                NS_LOG_INFO("Throughput: " << packetThroughput * 1000 << " Kbps\n");
                                pre_tps[2] = packetThroughput * 1000;
                                ofstream udpThoughputFile23(dir + "udpThroughput23.txt");
                                if (udpThoughputFile23.good()) {
                                    NS_LOG_INFO("udpThroughput is OK!\n" << endl);
                                    udpThoughputFile23 << packetThroughput * 1000 << " Kbps\n";
                                    udpThoughputFile23.close();
                                } else {
                                    NS_LOG_INFO("Cannot create udpThroughput.txt !\n");
                                }

                            }

                            if (packet->FindFirstMatchingByteTag(timestamp)) {
                                Time tx = timestamp.GetTimestamp();
                                Time dx = Simulator::Now() - tx;
                                aa23 = dx.GetMicroSeconds();
                                delay23 += aa23;
                            }

                            if (Simulator::Now() > Seconds(record_start[3])) {  // --

                                /**
                                 * get all receive packets
                                 */
                                std::ofstream PidSizeFile23(dir + "PidSetSize23.txt");

//                                uint16_t PidSetSize = pktnum; to avoid

                                if (PidSizeFile23.good()) {
                                    PidSizeFile23 << PidSet23.size() << endl;
                                }
                                PidSizeFile23.close();

                                /**
                                 * compute delay
                                 */
//                                uint64_t delayy23 = standard * (1 + (ttnt / 2 - 1) * k) + rand() % 30 + 1;
                                double delayy23 = (double) delay23 / PidSet23.size() / 1000;
                                ofstream delayput23(dir + "delayput23.txt");
                                if (delayput23.good()) {
                                    delayput23 << delayy23 << " ms\n";
                                    delayput23.close();
                                }

                            }
                        }
                        m_rxTrace(packet); //pinganzhang
                    }


                    /**
                     * business 4
                     */
                    if (UdpServer::m_port == 24) {

                        if (pre_tps[3] > top_tps[3]) {
                            continue;
                        }
                        static int a24 = 0;
                        static Time firstRx24;
                        uint64_t aa24;

                        NS_LOG_INFO("pinganzhang:::::::: UdpSever Rev Count = " << ++a24 << " Now ********* "
                                                                                << Simulator::Now()
                                                                                << std::endl);

                        PidSet24.insert(AppHdr.ReadPacketId());

                        if (Simulator::Now() > Seconds(record_start[4]) && rxcnt24 == 1) //记录吞吐量的时间  // --
                        {
                            firstRx24 = Simulator::Now();
                            ++rxcnt24;
                        }

                        NS_LOG_INFO("pinganzhang::::::::first arrived time = " << firstRx24.GetSeconds() << std::endl);
                        NS_LOG_INFO("pinganzhang::::::::AppPayLoadSize = " << packetOdcp->GetSize() - 39 << std::endl);

                        packetSizeVec24.push_back((packetOdcp->GetSize() - 39) * 8);//应用层负载

                        //pinganzhang get throughput

                        if (1) {

                            /**
                             * get Throughput
                             */
                            if (Simulator::Now() > Seconds(record_start[4])) {  // --
                                uint32_t sumPacketSize = accumulate(packetSizeVec24.begin(), packetSizeVec24.end(),
                                                                    0.0);
                                double randomValue = rand() % 20000 + 20000;
                                NS_LOG_INFO("random value is :" << randomValue << endl);
                                double packetThroughput = /*(sumPacketSize - randomValue)*/
                                        sumPacketSize / (Seconds(record_end[4]).GetSeconds() -
                                                         firstRx24.GetSeconds()) / 1024 / 1024;
                                NS_LOG_INFO("Throughput: " << packetThroughput * 1000 << " Kbps\n");
                                pre_tps[3] = packetThroughput * 1000;
                                ofstream udpThoughputFile24(dir + "udpThroughput24.txt");
                                if (udpThoughputFile24.good()) {
                                    NS_LOG_INFO("udpThroughput is OK!\n" << endl);
                                    udpThoughputFile24 << packetThroughput * 1000 << " Kbps\n";
                                    udpThoughputFile24.close();
                                } else {
                                    NS_LOG_INFO("Cannot create udpThroughput.txt !\n");
                                }

                            }

                            if (packet->FindFirstMatchingByteTag(timestamp)) {
                                Time tx = timestamp.GetTimestamp();
                                Time dx = Simulator::Now() - tx;
                                aa24 = dx.GetMicroSeconds();
                                delay24 += aa24;
                            }

                            if (Simulator::Now() > Seconds(record_start[4])) {  // --

                                /**
                                 * get all receive packets
                                 */
                                std::ofstream PidSizeFile24(dir + "PidSetSize24.txt");

//                                uint16_t PidSetSize = pktnum; to avoid

                                if (PidSizeFile24.good()) {
                                    PidSizeFile24 << PidSet24.size() << endl;
                                }
                                PidSizeFile24.close();

                                /**
                                 * compute delay
                                 */
//                                uint64_t delayy24 = standard * (1 + (ttnt / 2 - 1) * k) + rand() % 30 + 1;
                                double delayy24 = (double) delay24 / PidSet24.size() / 1000;
                                ofstream delayput24(dir + "delayput24.txt");
                                if (delayput24.good()) {
                                    delayput24 << delayy24 << " ms\n";
                                    delayput24.close();
                                }

                            }
                        }
                        m_rxTrace(packet); //pinganzhang
                    }


                    /**
                     * business 5
                     */
                    if (UdpServer::m_port == 25) {

                        if (pre_tps[4] > top_tps[4]) {
                            continue;
                        }
                        static int a25 = 0;
                        static Time firstRx25;
                        uint64_t aa25;

                        NS_LOG_INFO("pinganzhang:::::::: UdpSever Rev Count = " << ++a25 << " Now ********* "
                                                                                << Simulator::Now()
                                                                                << std::endl);

                        PidSet25.insert(AppHdr.ReadPacketId());

                        if (Simulator::Now() > Seconds(record_start[5]) && rxcnt25 == 1) //记录吞吐量的时间  // --
                        {
                            firstRx25 = Simulator::Now();
                            ++rxcnt25;
                        }

                        NS_LOG_INFO("pinganzhang::::::::first arrived time = " << firstRx25.GetSeconds() << std::endl);
                        NS_LOG_INFO("pinganzhang::::::::AppPayLoadSize = " << packetOdcp->GetSize() - 39 << std::endl);

                        packetSizeVec25.push_back((packetOdcp->GetSize() - 39) * 8);//应用层负载

                        //pinganzhang get throughput

                        if (1) {

                            /**
                             * get Throughput
                             */
                            if (Simulator::Now() > Seconds(record_start[5])) {  // --
                                uint32_t sumPacketSize = accumulate(packetSizeVec25.begin(), packetSizeVec25.end(),
                                                                    0.0);
                                double randomValue = rand() % 20000 + 20000;
                                NS_LOG_INFO("random value is :" << randomValue << endl);
                                double packetThroughput = /*(sumPacketSize - randomValue)*/
                                        sumPacketSize / (Seconds(record_end[5]).GetSeconds() -
                                                         firstRx25.GetSeconds()) / 1024 / 1024;
                                NS_LOG_INFO("Throughput: " << packetThroughput * 1000 << " Kbps\n");
                                pre_tps[4] = packetThroughput * 1000;
                                ofstream udpThoughputFile25(dir + "udpThroughput25.txt");
                                if (udpThoughputFile25.good()) {
                                    NS_LOG_INFO("udpThroughput is OK!\n" << endl);
                                    udpThoughputFile25 << packetThroughput * 1000 << " Kbps\n";
                                    udpThoughputFile25.close();
                                } else {
                                    NS_LOG_INFO("Cannot create udpThroughput.txt !\n");
                                }

                            }

                            if (packet->FindFirstMatchingByteTag(timestamp)) {
                                Time tx = timestamp.GetTimestamp();
                                Time dx = Simulator::Now() - tx;
                                aa25 = dx.GetMicroSeconds();
                                delay25 += aa25;
                            }

                            if (Simulator::Now() > Seconds(record_start[5])) {  // --

                                /**
                                 * get all receive packets
                                 */
                                std::ofstream PidSizeFile25(dir + "PidSetSize25.txt");

//                                uint16_t PidSetSize = pktnum; to avoid

                                if (PidSizeFile25.good()) {
                                    PidSizeFile25 << PidSet25.size() << endl;
                                }
                                PidSizeFile25.close();

                                /**
                                 * compute delay
                                 */
//                                uint64_t delayy25 = standard * (1 + (ttnt / 2 - 1) * k) + rand() % 30 + 1;
                                double delayy25 = (double) delay25 / PidSet25.size() / 1000;
                                ofstream delayput25(dir + "delayput25.txt");
                                if (delayput25.good()) {
                                    delayput25 << delayy25 << " ms\n";
                                    delayput25.close();
                                }

                            }
                        }
                        m_rxTrace(packet); //pinganzhang
                    }


                    /**
                     * business 6
                     */
                    if (UdpServer::m_port == 26) {

                        if (pre_tps[5] > top_tps[5]) {
                            continue;
                        }

                        static int a26 = 0;
                        static Time firstRx26;
                        uint64_t aa26;

                        NS_LOG_INFO("pinganzhang:::::::: UdpSever Rev Count = " << ++a26 << " Now ********* "
                                                                                << Simulator::Now()
                                                                                << std::endl);

                        PidSet26.insert(AppHdr.ReadPacketId());

                        if (Simulator::Now() > Seconds(record_start[6]) && rxcnt26 == 1) //记录吞吐量的时间  // --
                        {
                            firstRx26 = Simulator::Now();
                            ++rxcnt26;
                        }

                        NS_LOG_INFO("pinganzhang::::::::first arrived time = " << firstRx26.GetSeconds() << std::endl);
                        NS_LOG_INFO("pinganzhang::::::::AppPayLoadSize = " << packetOdcp->GetSize() - 39 << std::endl);

                        packetSizeVec26.push_back((packetOdcp->GetSize() - 39) * 8);//应用层负载

                        //pinganzhang get throughput

                        if (1) {

                            /**
                             * get Throughput
                             */
                            if (Simulator::Now() > Seconds(record_start[6])) {  // --
                                uint32_t sumPacketSize = accumulate(packetSizeVec26.begin(), packetSizeVec26.end(),
                                                                    0.0);
                                double randomValue = rand() % 20000 + 20000;
                                NS_LOG_INFO("random value is :" << randomValue << endl);
                                double packetThroughput = /*(sumPacketSize - randomValue)*/
                                        sumPacketSize / (Seconds(record_end[6]).GetSeconds() -
                                                         firstRx26.GetSeconds()) / 1024 / 1024;
                                NS_LOG_INFO("Throughput: " << packetThroughput * 1000 << " Kbps\n");
                                pre_tps[5] = packetThroughput * 1000;
                                ofstream udpThoughputFile26(dir + "udpThroughput26.txt");
                                if (udpThoughputFile26.good()) {
                                    NS_LOG_INFO("udpThroughput is OK!\n" << endl);
                                    udpThoughputFile26 << packetThroughput * 1000 << " Kbps\n";
                                    udpThoughputFile26.close();
                                } else {
                                    NS_LOG_INFO("Cannot create udpThroughput.txt !\n");
                                }

                            }

                            if (packet->FindFirstMatchingByteTag(timestamp)) {
                                Time tx = timestamp.GetTimestamp();
                                Time dx = Simulator::Now() - tx;
                                aa26 = dx.GetMicroSeconds();
                                delay26 += aa26;
                            }

                            if (Simulator::Now() > Seconds(record_start[6])) {  // --

                                /**
                                 * get all receive packets
                                 */
                                std::ofstream PidSizeFile26(dir + "PidSetSize26.txt");

//                                uint16_t PidSetSize = pktnum; to avoid

                                if (PidSizeFile26.good()) {
                                    PidSizeFile26 << PidSet26.size() << endl;
                                }
                                PidSizeFile26.close();

                                /**
                                 * compute delay
                                 */
//                                uint64_t delayy26 = standard * (1 + (ttnt / 2 - 1) * k) + rand() % 30 + 1;
                                double delayy26 = (double) delay26 / PidSet26.size() / 1000;
                                ofstream delayput26(dir + "delayput26.txt");
                                if (delayput26.good()) {
                                    delayput26 << delayy26 << " ms\n";
                                    delayput26.close();
                                }

                            }
                        }
                        m_rxTrace(packet); //pinganzhang
                    }


                    /**
                     * business 7
                     */
                    if (UdpServer::m_port == 27) {

                        if (pre_tps[6] > top_tps[6]) {
                            continue;
                        }

                        static int a27 = 0;
                        static Time firstRx27;
                        uint64_t aa27;

                        NS_LOG_INFO("pinganzhang:::::::: UdpSever Rev Count = " << ++a27 << " Now ********* "
                                                                                << Simulator::Now()
                                                                                << std::endl);

                        PidSet27.insert(AppHdr.ReadPacketId());

                        if (Simulator::Now() > Seconds(record_start[7]) && rxcnt27 == 1) //记录吞吐量的时间  // --
                        {
                            firstRx27 = Simulator::Now();
                            ++rxcnt27;
                        }

                        NS_LOG_INFO("pinganzhang::::::::first arrived time = " << firstRx27.GetSeconds() << std::endl);
                        NS_LOG_INFO("pinganzhang::::::::AppPayLoadSize = " << packetOdcp->GetSize() - 39 << std::endl);

                        packetSizeVec27.push_back((packetOdcp->GetSize() - 39) * 8);//应用层负载

                        //pinganzhang get throughput

                        if (1) {

                            /**
                             * get Throughput
                             */
                            if (Simulator::Now() > Seconds(record_start[7])) {  // --
                                uint32_t sumPacketSize = accumulate(packetSizeVec27.begin(), packetSizeVec27.end(),
                                                                    0.0);
                                double randomValue = rand() % 20000 + 20000;
                                NS_LOG_INFO("random value is :" << randomValue << endl);
                                double packetThroughput = /*(sumPacketSize - randomValue)*/
                                        sumPacketSize / (Seconds(record_end[7]).GetSeconds() -
                                                         firstRx27.GetSeconds()) / 1024 / 1024;
                                NS_LOG_INFO("Throughput: " << packetThroughput * 1000 << " Kbps\n");
                                pre_tps[6] = packetThroughput * 1000;
                                ofstream udpThoughputFile27(dir + "udpThroughput27.txt");
                                if (udpThoughputFile27.good()) {
                                    NS_LOG_INFO("udpThroughput is OK!\n" << endl);
                                    udpThoughputFile27 << packetThroughput * 1000 << " Kbps\n";
                                    udpThoughputFile27.close();
                                } else {
                                    NS_LOG_INFO("Cannot create udpThroughput.txt !\n");
                                }

                            }

                            if (packet->FindFirstMatchingByteTag(timestamp)) {
                                Time tx = timestamp.GetTimestamp();
                                Time dx = Simulator::Now() - tx;
                                aa27 = dx.GetMicroSeconds();
                                delay27 += aa27;
                            }

                            if (Simulator::Now() > Seconds(record_start[7])) {  // --

                                /**
                                 * get all receive packets
                                 */
                                std::ofstream PidSizeFile27(dir + "PidSetSize27.txt");

//                                uint16_t PidSetSize = pktnum; to avoid

                                if (PidSizeFile27.good()) {
                                    PidSizeFile27 << PidSet27.size() << endl;
                                }
                                PidSizeFile27.close();

                                /**
                                 * compute delay
                                 */
//                                uint64_t delayy27 = standard * (1 + (ttnt / 2 - 1) * k) + rand() % 30 + 1;
                                double delayy27 = (double) delay27 / PidSet27.size() / 1000;
                                ofstream delayput27(dir + "delayput27.txt");
                                if (delayput27.good()) {
                                    delayput27 << delayy27 << " ms\n";
                                    delayput27.close();
                                }

                            }
                        }
                        m_rxTrace(packet); //pinganzhang
                    }


                    /**
                    * business 8
                    */
                    if (UdpServer::m_port == 28) {

                        if (pre_tps[7] > top_tps[7]) {
                            continue;
                        }

                        static int a28 = 0;
                        static Time firstRx28;
                        uint64_t aa28;

                        NS_LOG_INFO("pinganzhang:::::::: UdpSever Rev Count = " << ++a28 << " Now ********* "
                                                                                << Simulator::Now()
                                                                                << std::endl);

                        PidSet28.insert(AppHdr.ReadPacketId());

                        if (Simulator::Now() > Seconds(record_start[8]) && rxcnt28 == 1) //记录吞吐量的时间  // --
                        {
                            firstRx28 = Simulator::Now();
                            ++rxcnt28;
                        }

                        NS_LOG_INFO("pinganzhang::::::::first arrived time = " << firstRx28.GetSeconds() << std::endl);
                        NS_LOG_INFO("pinganzhang::::::::AppPayLoadSize = " << packetOdcp->GetSize() - 39 << std::endl);

                        packetSizeVec28.push_back((packetOdcp->GetSize() - 39) * 8);//应用层负载

                        //pinganzhang get throughput

                        if (1) {

                            /**
                             * get Throughput
                             */
                            if (Simulator::Now() > Seconds(record_start[8])) {  // --
                                uint32_t sumPacketSize = accumulate(packetSizeVec28.begin(), packetSizeVec28.end(),
                                                                    0.0);
                                double randomValue = rand() % 20000 + 20000;
                                NS_LOG_INFO("random value is :" << randomValue << endl);
                                double packetThroughput = /*(sumPacketSize - randomValue)*/
                                        sumPacketSize / (Seconds(record_end[8]).GetSeconds() -
                                                         firstRx28.GetSeconds()) / 1024 / 1024;
                                NS_LOG_INFO("Throughput: " << packetThroughput * 1000 << " Kbps\n");
                                pre_tps[7] = packetThroughput * 1000;
                                ofstream udpThoughputFile28(dir + "udpThroughput28.txt");
                                if (udpThoughputFile28.good()) {
                                    NS_LOG_INFO("udpThroughput is OK!\n" << endl);
                                    udpThoughputFile28 << packetThroughput * 1000 << " Kbps\n";
                                    udpThoughputFile28.close();
                                } else {
                                    NS_LOG_INFO("Cannot create udpThroughput.txt !\n");
                                }

                            }

                            if (packet->FindFirstMatchingByteTag(timestamp)) {
                                Time tx = timestamp.GetTimestamp();
                                Time dx = Simulator::Now() - tx;
                                aa28 = dx.GetMicroSeconds();
                                delay28 += aa28;
                            }

                            if (Simulator::Now() > Seconds(record_start[8])) {  // --

                                /**
                                 * get all receive packets
                                 */
                                std::ofstream PidSizeFile28(dir + "PidSetSize28.txt");

//                                uint16_t PidSetSize = pktnum; to avoid

                                if (PidSizeFile28.good()) {
                                    PidSizeFile28 << PidSet28.size() << endl;
                                }
                                PidSizeFile28.close();

                                /**
                                 * compute delay
                                 */
//                                uint64_t delayy28 = standard * (1 + (ttnt / 2 - 1) * k) + rand() % 30 + 1;
                                double delayy28 = (double) delay28 / PidSet28.size() / 1000;
                                ofstream delayput28(dir + "delayput28.txt");
                                if (delayput28.good()) {
                                    delayput28 << delayy28 << " ms\n";
                                    delayput28.close();
                                }

                            }
                        }
                        m_rxTrace(packet); //pinganzhang
                    }


                    /**
                    * business 9
                    */
                    if (UdpServer::m_port == 29) {

                        if (pre_tps[8] > top_tps[8]) {
                            continue;
                        }

                        static int a29 = 0;
                        static Time firstRx29;
                        uint64_t aa29;

                        NS_LOG_INFO("pinganzhang:::::::: UdpSever Rev Count = " << ++a29 << " Now ********* "
                                                                                << Simulator::Now()
                                                                                << std::endl);

                        PidSet29.insert(AppHdr.ReadPacketId());

                        if (Simulator::Now() > Seconds(record_start[9]) && rxcnt29 == 1) //记录吞吐量的时间  // --
                        {
                            firstRx29 = Simulator::Now();
                            ++rxcnt29;
                        }

                        NS_LOG_INFO("pinganzhang::::::::first arrived time = " << firstRx29.GetSeconds() << std::endl);
                        NS_LOG_INFO("pinganzhang::::::::AppPayLoadSize = " << packetOdcp->GetSize() - 39 << std::endl);

                        packetSizeVec29.push_back((packetOdcp->GetSize() - 39) * 8);//应用层负载

                        //pinganzhang get throughput

                        if (1) {

                            /**
                             * get Throughput
                             */
                            if (Simulator::Now() > Seconds(record_start[9])) {  // --
                                uint32_t sumPacketSize = accumulate(packetSizeVec29.begin(), packetSizeVec29.end(),
                                                                    0.0);
                                double randomValue = rand() % 20000 + 20000;
                                NS_LOG_INFO("random value is :" << randomValue << endl);
                                double packetThroughput = /*(sumPacketSize - randomValue)*/
                                        sumPacketSize / (Seconds(record_end[9]).GetSeconds() -
                                                         firstRx29.GetSeconds()) / 1024 / 1024;
                                NS_LOG_INFO("Throughput: " << packetThroughput * 1000 << " Kbps\n");
                                pre_tps[8] = packetThroughput * 1000;
                                ofstream udpThoughputFile29(dir + "udpThroughput29.txt");
                                if (udpThoughputFile29.good()) {
                                    NS_LOG_INFO("udpThroughput is OK!\n" << endl);
                                    udpThoughputFile29 << packetThroughput * 1000 << " Kbps\n";
                                    udpThoughputFile29.close();
                                } else {
                                    NS_LOG_INFO("Cannot create udpThroughput.txt !\n");
                                }

                            }

                            if (packet->FindFirstMatchingByteTag(timestamp)) {
                                Time tx = timestamp.GetTimestamp();
                                Time dx = Simulator::Now() - tx;
                                aa29 = dx.GetMicroSeconds();
                                delay29 += aa29;
                            }

                            if (Simulator::Now() > Seconds(record_start[9])) {  // --

                                /**
                                 * get all receive packets
                                 */
                                std::ofstream PidSizeFile29(dir + "PidSetSize29.txt");

//                                uint16_t PidSetSize = pktnum; to avoid

                                if (PidSizeFile29.good()) {
                                    PidSizeFile29 << PidSet29.size() << endl;
                                }
                                PidSizeFile29.close();

                                /**
                                 * compute delay
                                 */
//                                uint64_t delayy29 = standard * (1 + (ttnt / 2 - 1) * k) + rand() % 30 + 1;
                                double delayy29 = (double) delay29 / PidSet29.size() / 1000;
                                ofstream delayput29(dir + "delayput29.txt");
                                if (delayput29.good()) {
                                    delayput29 << delayy29 << " ms\n";
                                    delayput29.close();
                                }

                            }
                        }
                        m_rxTrace(packet); //pinganzhang
                    }


                    /**
                    * business 10
                    */
                    if (UdpServer::m_port == 30) {

                        if (pre_tps[9] > top_tps[9]) {
                            continue;
                        }

                        static int a30 = 0;
                        static Time firstRx30;
                        uint64_t aa30;

                        NS_LOG_INFO("pinganzhang:::::::: UdpSever Rev Count = " << ++a30 << " Now ********* "
                                                                                << Simulator::Now()
                                                                                << std::endl);

                        PidSet30.insert(AppHdr.ReadPacketId());

                        if (Simulator::Now() > Seconds(record_start[10]) && rxcnt30 == 1) //记录吞吐量的时间  // --
                        {
                            firstRx30 = Simulator::Now();
                            ++rxcnt30;
                        }

                        NS_LOG_INFO("pinganzhang::::::::first arrived time = " << firstRx30.GetSeconds() << std::endl);
                        NS_LOG_INFO("pinganzhang::::::::AppPayLoadSize = " << packetOdcp->GetSize() - 39 << std::endl);

                        packetSizeVec30.push_back((packetOdcp->GetSize() - 39) * 8);//应用层负载

                        //pinganzhang get throughput

                        if (1) {

                            /**
                             * get Throughput
                             */
                            if (Simulator::Now() > Seconds(record_start[10])) {  // --
                                uint32_t sumPacketSize = accumulate(packetSizeVec30.begin(), packetSizeVec30.end(),
                                                                    0.0);
                                double randomValue = rand() % 20000 + 20000;
                                NS_LOG_INFO("random value is :" << randomValue << endl);
                                double packetThroughput = /*(sumPacketSize - randomValue)*/
                                        sumPacketSize / (Seconds(record_end[10]).GetSeconds() -
                                                         firstRx30.GetSeconds()) / 1024 / 1024;
                                NS_LOG_INFO("Throughput: " << packetThroughput * 1000 << " Kbps\n");
                                pre_tps[9] = packetThroughput * 1000;
                                ofstream udpThoughputFile30(dir + "udpThroughput30.txt");
                                if (udpThoughputFile30.good()) {
                                    NS_LOG_INFO("udpThroughput is OK!\n" << endl);
                                    udpThoughputFile30 << packetThroughput * 1000 << " Kbps\n";
                                    udpThoughputFile30.close();
                                } else {
                                    NS_LOG_INFO("Cannot create udpThroughput.txt !\n");
                                }

                            }

                            if (packet->FindFirstMatchingByteTag(timestamp)) {
                                Time tx = timestamp.GetTimestamp();
                                Time dx = Simulator::Now() - tx;
                                aa30 = dx.GetMicroSeconds();
                                delay30 += aa30;
                            }

                            if (Simulator::Now() > Seconds(record_start[10])) {  // --

                                /**
                                 * get all receive packets
                                 */
                                std::ofstream PidSizeFile30(dir + "PidSetSize30.txt");

//                                uint16_t PidSetSize = pktnum; to avoid

                                if (PidSizeFile30.good()) {
                                    PidSizeFile30 << PidSet30.size() << endl;
                                }
                                PidSizeFile30.close();

                                /**
                                 * compute delay
                                 */
//                                uint64_t delayy30 = standard * (1 + (ttnt / 2 - 1) * k) + rand() % 30 + 1;
                                double delayy30 = (double) delay30 / PidSet30.size() / 1000;
                                ofstream delayput30(dir + "delayput30.txt");
                                if (delayput30.good()) {
                                    delayput30 << delayy30 << " ms\n";
                                    delayput30.close();
                                }

                            }
                        }
                        m_rxTrace(packet); //pinganzhang
                    }


                    /**
                    * business 11
                    */
                    if (UdpServer::m_port == 31) {

                        if (pre_tps[10] > top_tps[10]) {
                            continue;
                        }

                        static int a31 = 0;
                        static Time firstRx31;
                        uint64_t aa31;

                        NS_LOG_INFO("pinganzhang:::::::: UdpSever Rev Count = " << ++a31 << " Now ********* "
                                                                                << Simulator::Now()
                                                                                << std::endl);

                        PidSet31.insert(AppHdr.ReadPacketId());

                        if (Simulator::Now() > Seconds(record_start[11]) && rxcnt31 == 1) //记录吞吐量的时间  // --
                        {
                            firstRx31 = Simulator::Now();
                            ++rxcnt31;
                        }

                        NS_LOG_INFO("pinganzhang::::::::first arrived time = " << firstRx31.GetSeconds() << std::endl);
                        NS_LOG_INFO("pinganzhang::::::::AppPayLoadSize = " << packetOdcp->GetSize() - 39 << std::endl);

                        packetSizeVec31.push_back((packetOdcp->GetSize() - 39) * 8);//应用层负载

                        //pinganzhang get throughput

                        if (1) {

                            /**
                             * get Throughput
                             */
                            if (Simulator::Now() > Seconds(record_start[11])) {  // --
                                uint32_t sumPacketSize = accumulate(packetSizeVec31.begin(), packetSizeVec31.end(),
                                                                    0.0);
                                double randomValue = rand() % 20000 + 20000;
                                NS_LOG_INFO("random value is :" << randomValue << endl);
                                double packetThroughput = /*(sumPacketSize - randomValue)*/
                                        sumPacketSize / (Seconds(record_end[11]).GetSeconds() -
                                                         firstRx31.GetSeconds()) / 1024 / 1024;
                                NS_LOG_INFO("Throughput: " << packetThroughput * 1000 << " Kbps\n");
                                pre_tps[10] = packetThroughput * 1000;
                                ofstream udpThoughputFile31(dir + "udpThroughput31.txt");
                                if (udpThoughputFile31.good()) {
                                    NS_LOG_INFO("udpThroughput is OK!\n" << endl);
                                    udpThoughputFile31 << packetThroughput * 1000 << " Kbps\n";
                                    udpThoughputFile31.close();
                                } else {
                                    NS_LOG_INFO("Cannot create udpThroughput.txt !\n");
                                }

                            }

                            if (packet->FindFirstMatchingByteTag(timestamp)) {
                                Time tx = timestamp.GetTimestamp();
                                Time dx = Simulator::Now() - tx;
                                aa31 = dx.GetMicroSeconds();
                                delay31 += aa31;
                            }

                            if (Simulator::Now() > Seconds(record_start[11])) {  // --

                                /**
                                 * get all receive packets
                                 */
                                std::ofstream PidSizeFile31(dir + "PidSetSize31.txt");

//                                uint16_t PidSetSize = pktnum; to avoid

                                if (PidSizeFile31.good()) {
                                    PidSizeFile31 << PidSet31.size() << endl;
                                }
                                PidSizeFile31.close();

                                /**
                                 * compute delay
                                 */
//                                uint64_t delayy31 = standard * (1 + (ttnt / 2 - 1) * k) + rand() % 30 + 1;
                                double delayy31 = (double) delay31 / PidSet31.size() / 1000;
                                ofstream delayput31(dir + "delayput31.txt");
                                if (delayput31.good()) {
                                    delayput31 << delayy31 << " ms\n";
                                    delayput31.close();
                                }

                            }
                        }
                        m_rxTrace(packet); //pinganzhang
                    }


                    /**
                    * business 12
                    */
                    if (UdpServer::m_port == 32) {

                        if (pre_tps[11] > top_tps[11]) {
                            continue;
                        }
                        static int a32 = 0;
                        static Time firstRx32;
                        uint64_t aa32;

                        NS_LOG_INFO("pinganzhang:::::::: UdpSever Rev Count = " << ++a32 << " Now ********* "
                                                                                << Simulator::Now()
                                                                                << std::endl);

                        PidSet32.insert(AppHdr.ReadPacketId());

                        if (Simulator::Now() > Seconds(record_start[12]) && rxcnt32 == 1) //记录吞吐量的时间  // --
                        {
                            firstRx32 = Simulator::Now();
                            ++rxcnt32;
                        }

                        NS_LOG_INFO("pinganzhang::::::::first arrived time = " << firstRx32.GetSeconds() << std::endl);
                        NS_LOG_INFO("pinganzhang::::::::AppPayLoadSize = " << packetOdcp->GetSize() - 39 << std::endl);

                        packetSizeVec32.push_back((packetOdcp->GetSize() - 39) * 8);//应用层负载

                        //pinganzhang get throughput

                        if (1) {

                            /**
                             * get Throughput
                             */
                            if (Simulator::Now() > Seconds(record_start[12])) {  // --
                                uint32_t sumPacketSize = accumulate(packetSizeVec32.begin(), packetSizeVec32.end(),
                                                                    0.0);
                                double randomValue = rand() % 20000 + 20000;
                                NS_LOG_INFO("random value is :" << randomValue << endl);
                                double packetThroughput = /*(sumPacketSize - randomValue)*/
                                        sumPacketSize / (Seconds(record_end[12]).GetSeconds() -
                                                         firstRx32.GetSeconds()) / 1024 / 1024;
                                NS_LOG_INFO("Throughput: " << packetThroughput * 1000 << " Kbps\n");
                                pre_tps[11] = packetThroughput * 1000;
                                ofstream udpThoughputFile32(dir + "udpThroughput32.txt");
                                if (udpThoughputFile32.good()) {
                                    NS_LOG_INFO("udpThroughput is OK!\n" << endl);
                                    udpThoughputFile32 << packetThroughput * 1000 << " Kbps\n";
                                    udpThoughputFile32.close();
                                } else {
                                    NS_LOG_INFO("Cannot create udpThroughput.txt !\n");
                                }

                            }

                            if (packet->FindFirstMatchingByteTag(timestamp)) {
                                Time tx = timestamp.GetTimestamp();
                                Time dx = Simulator::Now() - tx;
                                aa32 = dx.GetMicroSeconds();
                                delay32 += aa32;
                            }

                            if (Simulator::Now() > Seconds(record_start[12])) {  // --

                                /**
                                 * get all receive packets
                                 */
                                std::ofstream PidSizeFile32(dir + "PidSetSize32.txt");

//                                uint16_t PidSetSize = pktnum; to avoid

                                if (PidSizeFile32.good()) {
                                    PidSizeFile32 << PidSet32.size() << endl;
                                }
                                PidSizeFile32.close();

                                /**
                                 * compute delay
                                 */
//                                uint64_t delayy32 = standard * (1 + (ttnt / 2 - 1) * k) + rand() % 30 + 1;
                                double delayy32 = (double) delay32 / PidSet32.size() / 1000;
                                ofstream delayput32(dir + "delayput32.txt");
                                if (delayput32.good()) {
                                    delayput32 << delayy32 << " ms\n";
                                    delayput32.close();
                                }

                            }
                        }
                        m_rxTrace(packet); //pinganzhang
                    }


                    /**
                    * business 13
                    */
                    if (UdpServer::m_port == 33) {
                        if (pre_tps[12] > top_tps[12]) {
                            continue;
                        }
                        static int a33 = 0;
                        static Time firstRx33;
                        uint64_t aa33;

                        NS_LOG_INFO("pinganzhang:::::::: UdpSever Rev Count = " << ++a33 << " Now ********* "
                                                                                << Simulator::Now()
                                                                                << std::endl);

                        PidSet33.insert(AppHdr.ReadPacketId());

                        if (Simulator::Now() > Seconds(record_start[13]) && rxcnt33 == 1) //记录吞吐量的时间  // --
                        {
                            firstRx33 = Simulator::Now();
                            ++rxcnt33;
                        }

                        NS_LOG_INFO("pinganzhang::::::::first arrived time = " << firstRx33.GetSeconds() << std::endl);
                        NS_LOG_INFO("pinganzhang::::::::AppPayLoadSize = " << packetOdcp->GetSize() - 39 << std::endl);

                        packetSizeVec33.push_back((packetOdcp->GetSize() - 39) * 8);//应用层负载

                        //pinganzhang get throughput

                        if (1) {

                            /**
                             * get Throughput
                             */
                            if (Simulator::Now() > Seconds(record_start[13])) {  // --
                                uint32_t sumPacketSize = accumulate(packetSizeVec33.begin(), packetSizeVec33.end(),
                                                                    0.0);
                                double randomValue = rand() % 20000 + 20000;
                                NS_LOG_INFO("random value is :" << randomValue << endl);
                                double packetThroughput = /*(sumPacketSize - randomValue)*/
                                        sumPacketSize / (Seconds(record_end[13]).GetSeconds() -
                                                         firstRx33.GetSeconds()) / 1024 / 1024;
                                NS_LOG_INFO("Throughput: " << packetThroughput * 1000 << " Kbps\n");
                                pre_tps[12] = packetThroughput * 1000;
                                ofstream udpThoughputFile33(dir + "udpThroughput33.txt");
                                if (udpThoughputFile33.good()) {
                                    NS_LOG_INFO("udpThroughput is OK!\n" << endl);
                                    udpThoughputFile33 << packetThroughput * 1000 << " Kbps\n";
                                    udpThoughputFile33.close();
                                } else {
                                    NS_LOG_INFO("Cannot create udpThroughput.txt !\n");
                                }

                            }

                            if (packet->FindFirstMatchingByteTag(timestamp)) {
                                Time tx = timestamp.GetTimestamp();
                                Time dx = Simulator::Now() - tx;
                                aa33 = dx.GetMicroSeconds();
                                delay33 += aa33;
                            }

                            if (Simulator::Now() > Seconds(record_start[13])) {  // --

                                /**
                                 * get all receive packets
                                 */
                                std::ofstream PidSizeFile33(dir + "PidSetSize33.txt");

//                                uint16_t PidSetSize = pktnum; to avoid

                                if (PidSizeFile33.good()) {
                                    PidSizeFile33 << PidSet33.size() << endl;
                                }
                                PidSizeFile33.close();

                                /**
                                 * compute delay
                                 */
//                                uint64_t delayy33 = standard * (1 + (ttnt / 2 - 1) * k) + rand() % 30 + 1;
                                double delayy33 = (double) delay33 / PidSet33.size() / 1000;
                                ofstream delayput33(dir + "delayput33.txt");
                                if (delayput33.good()) {
                                    delayput33 << delayy33 << " ms\n";
                                    delayput33.close();
                                }

                            }
                        }
                        m_rxTrace(packet); //pinganzhang
                    }


                    /**
                    * business 14
                    */
                    if (UdpServer::m_port == 34) {

                        if (pre_tps[13] > top_tps[13]) {
                            continue;
                        }

                        static int a34 = 0;
                        static Time firstRx34;
                        uint64_t aa34;

                        NS_LOG_INFO("pinganzhang:::::::: UdpSever Rev Count = " << ++a34 << " Now ********* "
                                                                                << Simulator::Now()
                                                                                << std::endl);

                        PidSet34.insert(AppHdr.ReadPacketId());

                        if (Simulator::Now() > Seconds(record_start[14]) && rxcnt34 == 1) //记录吞吐量的时间  // --
                        {
                            firstRx34 = Simulator::Now();
                            ++rxcnt34;
                        }

                        NS_LOG_INFO("pinganzhang::::::::first arrived time = " << firstRx34.GetSeconds() << std::endl);
                        NS_LOG_INFO("pinganzhang::::::::AppPayLoadSize = " << packetOdcp->GetSize() - 39 << std::endl);

                        packetSizeVec34.push_back((packetOdcp->GetSize() - 39) * 8);//应用层负载

                        //pinganzhang get throughput

                        if (1) {

                            /**
                             * get Throughput
                             */
                            if (Simulator::Now() > Seconds(record_start[14])) {  // --
                                uint32_t sumPacketSize = accumulate(packetSizeVec34.begin(), packetSizeVec34.end(),
                                                                    0.0);
                                double randomValue = rand() % 20000 + 20000;
                                NS_LOG_INFO("random value is :" << randomValue << endl);
                                double packetThroughput = /*(sumPacketSize - randomValue)*/
                                        sumPacketSize /
                                        (Seconds(record_end[14]).GetSeconds() -
                                         firstRx34.GetSeconds()) / 1024 / 1024;
                                NS_LOG_INFO("Throughput: " << packetThroughput * 1000 << " Kbps\n");
                                pre_tps[13] = packetThroughput * 1000;
                                ofstream udpThoughputFile34(dir + "udpThroughput34.txt");
                                if (udpThoughputFile34.good()) {
                                    NS_LOG_INFO("udpThroughput is OK!\n" << endl);
                                    udpThoughputFile34 << packetThroughput * 1000 << " Kbps\n";
                                    udpThoughputFile34.close();
                                } else {
                                    NS_LOG_INFO("Cannot create udpThroughput.txt !\n");
                                }

                            }

                            if (packet->FindFirstMatchingByteTag(timestamp)) {
                                Time tx = timestamp.GetTimestamp();
                                Time dx = Simulator::Now() - tx;
                                aa34 = dx.GetMicroSeconds();
                                delay34 += aa34;
                            }

                            if (Simulator::Now() > Seconds(record_start[14])) {  // --

                                /**
                                 * get all receive packets
                                 */
                                std::ofstream PidSizeFile34(dir + "PidSetSize34.txt");

//                                uint16_t PidSetSize = pktnum; to avoid

                                if (PidSizeFile34.good()) {
                                    PidSizeFile34 << PidSet34.size() << endl;
                                }
                                PidSizeFile34.close();

                                /**
                                 * compute delay
                                 */
//                                uint64_t delayy34 = standard * (1 + (ttnt / 2 - 1) * k) + rand() % 30 + 1;
                                double delayy34 = (double) delay34 / PidSet34.size() / 1000;
                                ofstream delayput34(dir + "delayput34.txt");
                                if (delayput34.good()) {
                                    delayput34 << delayy34 << " ms\n";
                                    delayput34.close();
                                }

                            }
                        }
                        m_rxTrace(packet); //pinganzhang
                    }


                    /**
                    * business 15
                    */
                    if (UdpServer::m_port == 35) {

                        if (pre_tps[14] > top_tps[14]) {
                            continue;
                        }

                        static int a35 = 0;
                        static Time firstRx35;
                        uint64_t aa35;

                        NS_LOG_INFO("pinganzhang:::::::: UdpSever Rev Count = " << ++a35 << " Now ********* "
                                                                                << Simulator::Now()
                                                                                << std::endl);

                        PidSet35.insert(AppHdr.ReadPacketId());

                        if (Simulator::Now() > Seconds(record_start[15]) && rxcnt35 == 1) //记录吞吐量的时间  // --
                        {
                            firstRx35 = Simulator::Now();
                            ++rxcnt35;
                        }

                        NS_LOG_INFO("pinganzhang::::::::first arrived time = " << firstRx35.GetSeconds() << std::endl);
                        NS_LOG_INFO("pinganzhang::::::::AppPayLoadSize = " << packetOdcp->GetSize() - 39 << std::endl);

                        packetSizeVec35.push_back((packetOdcp->GetSize() - 39) * 8);//应用层负载

                        //pinganzhang get throughput

                        if (1) {

                            /**
                             * get Throughput
                             */
                            if (Simulator::Now() > Seconds(record_start[15])) {  // --
                                uint32_t sumPacketSize = accumulate(packetSizeVec35.begin(), packetSizeVec35.end(),
                                                                    0.0);
                                double randomValue = rand() % 20000 + 20000;
                                NS_LOG_INFO("random value is :" << randomValue << endl);
                                double packetThroughput = /*(sumPacketSize - randomValue)*/
                                        sumPacketSize /
                                        (Seconds(record_end[15]).GetSeconds() -
                                         firstRx35.GetSeconds()) / 1024 / 1024;
                                NS_LOG_INFO("Throughput: " << packetThroughput * 1000 << " Kbps\n");
                                pre_tps[14] = packetThroughput * 1000;
                                ofstream udpThoughputFile35(dir + "udpThroughput35.txt");
                                if (udpThoughputFile35.good()) {
                                    NS_LOG_INFO("udpThroughput is OK!\n" << endl);
                                    udpThoughputFile35 << packetThroughput * 1000 << " Kbps\n";
                                    udpThoughputFile35.close();
                                } else {
                                    NS_LOG_INFO("Cannot create udpThroughput.txt !\n");
                                }

                            }

                            if (packet->FindFirstMatchingByteTag(timestamp)) {
                                Time tx = timestamp.GetTimestamp();
                                Time dx = Simulator::Now() - tx;
                                aa35 = dx.GetMicroSeconds();
                                delay35 += aa35;
                            }

                            if (Simulator::Now() > Seconds(record_start[15])) {  // --

                                /**
                                 * get all receive packets
                                 */
                                std::ofstream PidSizeFile35(dir + "PidSetSize35.txt");

//                                uint16_t PidSetSize = pktnum; to avoid

                                if (PidSizeFile35.good()) {
                                    PidSizeFile35 << PidSet35.size() << endl;
                                }
                                PidSizeFile35.close();

                                /**
                                 * compute delay
                                 */
//                                uint64_t delayy35 = standard * (1 + (ttnt / 2 - 1) * k) + rand() % 30 + 1;
                                double delayy35 = (double) delay35 / PidSet35.size() / 1000;
                                ofstream delayput35(dir + "delayput35.txt");
                                if (delayput35.good()) {
                                    delayput35 << delayy35 << " ms\n";
                                    delayput35.close();
                                }

                            }
                        }
                        m_rxTrace(packet); //pinganzhang
                    }


                    /**
                    * business 16
                    */
                    if (UdpServer::m_port == 36) {

                        if (pre_tps[15] > top_tps[15]) {
                            continue;
                        }

                        static int a36 = 0;
                        static Time firstRx36;
                        uint64_t aa36;

                        NS_LOG_INFO("pinganzhang:::::::: UdpSever Rev Count = " << ++a36 << " Now ********* "
                                                                                << Simulator::Now()
                                                                                << std::endl);

                        PidSet36.insert(AppHdr.ReadPacketId());

                        if (Simulator::Now() > Seconds(record_start[16]) && rxcnt36 == 1) //记录吞吐量的时间  // --
                        {
                            firstRx36 = Simulator::Now();
                            ++rxcnt36;
                        }

                        NS_LOG_INFO("pinganzhang::::::::first arrived time = " << firstRx36.GetSeconds() << std::endl);
                        NS_LOG_INFO("pinganzhang::::::::AppPayLoadSize = " << packetOdcp->GetSize() - 39 << std::endl);

                        packetSizeVec36.push_back((packetOdcp->GetSize() - 39) * 8);//应用层负载

                        //pinganzhang get throughput

                        if (1) {

                            /**
                             * get Throughput
                             */
                            if (Simulator::Now() > Seconds(record_start[16])) {  // --
                                uint32_t sumPacketSize = accumulate(packetSizeVec36.begin(), packetSizeVec36.end(),
                                                                    0.0);
                                double randomValue = rand() % 20000 + 20000;
                                NS_LOG_INFO("random value is :" << randomValue << endl);
                                double packetThroughput = /*(sumPacketSize - randomValue)*/
                                        sumPacketSize /
                                        (Seconds(record_end[16]).GetSeconds() -
                                         firstRx36.GetSeconds()) / 1024 / 1024;
                                NS_LOG_INFO("Throughput: " << packetThroughput * 1000 << " Kbps\n");
                                pre_tps[15] = packetThroughput * 1000;
                                ofstream udpThoughputFile36(dir + "udpThroughput36.txt");
                                if (udpThoughputFile36.good()) {
                                    NS_LOG_INFO("udpThroughput is OK!\n" << endl);
                                    udpThoughputFile36 << packetThroughput * 1000 << " Kbps\n";
                                    udpThoughputFile36.close();
                                } else {
                                    NS_LOG_INFO("Cannot create udpThroughput.txt !\n");
                                }

                            }

                            if (packet->FindFirstMatchingByteTag(timestamp)) {
                                Time tx = timestamp.GetTimestamp();
                                Time dx = Simulator::Now() - tx;
                                aa36 = dx.GetMicroSeconds();
                                delay36 += aa36;
                            }

                            if (Simulator::Now() > Seconds(record_start[16])) {  // --

                                /**
                                 * get all receive packets
                                 */
                                std::ofstream PidSizeFile36(dir + "PidSetSize36.txt");

//                                uint16_t PidSetSize = pktnum; to avoid

                                if (PidSizeFile36.good()) {
                                    PidSizeFile36 << PidSet36.size() << endl;
                                }
                                PidSizeFile36.close();

                                /**
                                 * compute delay
                                 */
//                                uint64_t delayy36 = standard * (1 + (ttnt / 2 - 1) * k) + rand() % 30 + 1;
                                double delayy36 = (double) delay36 / PidSet36.size() / 1000;
                                ofstream delayput36(dir + "delayput36.txt");
                                if (delayput36.good()) {
                                    delayput36 << delayy36 << " ms\n";
                                    delayput36.close();
                                }

                            }
                        }
                        m_rxTrace(packet); //pinganzhang
                    }


                    /**
                    * business 17
                    */
                    if (UdpServer::m_port == 37) {

                        if (pre_tps[16] > top_tps[16]) {
                            continue;
                        }

                        static int a37 = 0;
                        static Time firstRx37;
                        uint64_t aa37;

                        NS_LOG_INFO("pinganzhang:::::::: UdpSever Rev Count = " << ++a37 << " Now ********* "
                                                                                << Simulator::Now()
                                                                                << std::endl);

                        PidSet37.insert(AppHdr.ReadPacketId());

                        if (Simulator::Now() > Seconds(record_start[17]) && rxcnt37 == 1) //记录吞吐量的时间  // --
                        {
                            firstRx37 = Simulator::Now();
                            ++rxcnt37;
                        }

                        NS_LOG_INFO("pinganzhang::::::::first arrived time = " << firstRx37.GetSeconds() << std::endl);
                        NS_LOG_INFO("pinganzhang::::::::AppPayLoadSize = " << packetOdcp->GetSize() - 39 << std::endl);

                        packetSizeVec37.push_back((packetOdcp->GetSize() - 39) * 8);//应用层负载

                        //pinganzhang get throughput

                        if (1) {

                            /**
                             * get Throughput
                             */
                            if (Simulator::Now() > Seconds(record_start[17])) {  // --
                                uint32_t sumPacketSize = accumulate(packetSizeVec37.begin(), packetSizeVec37.end(),
                                                                    0.0);
                                double randomValue = rand() % 20000 + 20000;
                                NS_LOG_INFO("random value is :" << randomValue << endl);
                                double packetThroughput = /*(sumPacketSize - randomValue)*/
                                        sumPacketSize /
                                        (Seconds(record_end[17]).GetSeconds() -
                                         firstRx37.GetSeconds()) / 1024 / 1024;
                                NS_LOG_INFO("Throughput: " << packetThroughput * 1000 << " Kbps\n");
                                pre_tps[16] = packetThroughput * 1000;
                                ofstream udpThoughputFile37(dir + "udpThroughput37.txt");
                                if (udpThoughputFile37.good()) {
                                    NS_LOG_INFO("udpThroughput is OK!\n" << endl);
                                    udpThoughputFile37 << packetThroughput * 1000 << " Kbps\n";
                                    udpThoughputFile37.close();
                                } else {
                                    NS_LOG_INFO("Cannot create udpThroughput.txt !\n");
                                }

                            }

                            if (packet->FindFirstMatchingByteTag(timestamp)) {
                                Time tx = timestamp.GetTimestamp();
                                Time dx = Simulator::Now() - tx;
                                aa37 = dx.GetMicroSeconds();
                                delay37 += aa37;
                            }

                            if (Simulator::Now() > Seconds(record_start[17])) {  // --

                                /**
                                 * get all receive packets
                                 */
                                std::ofstream PidSizeFile37(dir + "PidSetSize37.txt");

//                                uint16_t PidSetSize = pktnum; to avoid

                                if (PidSizeFile37.good()) {
                                    PidSizeFile37 << PidSet37.size() << endl;
                                }
                                PidSizeFile37.close();

                                /**
                                 * compute delay
                                 */
//                                uint64_t delayy37 = standard * (1 + (ttnt / 2 - 1) * k) + rand() % 30 + 1;
                                double delayy37 = (double) delay37 / PidSet37.size() / 1000;
                                ofstream delayput37(dir + "delayput37.txt");
                                if (delayput37.good()) {
                                    delayput37 << delayy37 << " ms\n";
                                    delayput37.close();
                                }

                            }
                        }
                        m_rxTrace(packet); //pinganzhang
                    }


                    /**
                    * business 18
                    */
                    if (UdpServer::m_port == 38) {

                        if (pre_tps[17] > top_tps[17]) {
                            continue;
                        }

                        static int a38 = 0;
                        static Time firstRx38;
                        uint64_t aa38;

                        NS_LOG_INFO("pinganzhang:::::::: UdpSever Rev Count = " << ++a38 << " Now ********* "
                                                                                << Simulator::Now()
                                                                                << std::endl);

                        PidSet38.insert(AppHdr.ReadPacketId());

                        if (Simulator::Now() > Seconds(record_start[18]) && rxcnt38 == 1) //记录吞吐量的时间  // --
                        {
                            firstRx38 = Simulator::Now();
                            ++rxcnt38;
                        }

                        NS_LOG_INFO("pinganzhang::::::::first arrived time = " << firstRx38.GetSeconds() << std::endl);
                        NS_LOG_INFO("pinganzhang::::::::AppPayLoadSize = " << packetOdcp->GetSize() - 39 << std::endl);

                        packetSizeVec38.push_back((packetOdcp->GetSize() - 39) * 8);//应用层负载

                        //pinganzhang get throughput

                        if (1) {

                            /**
                             * get Throughput
                             */
                            if (Simulator::Now() > Seconds(record_start[18])) {  // --
                                uint32_t sumPacketSize = accumulate(packetSizeVec38.begin(), packetSizeVec38.end(),
                                                                    0.0);
                                double randomValue = rand() % 20000 + 20000;
                                NS_LOG_INFO("random value is :" << randomValue << endl);
                                double packetThroughput = /*(sumPacketSize - randomValue)*/
                                        sumPacketSize /
                                        (Seconds(record_end[18]).GetSeconds() -
                                         firstRx38.GetSeconds()) / 1024 / 1024;
                                NS_LOG_INFO("Throughput: " << packetThroughput * 1000 << " Kbps\n");
                                pre_tps[17] = packetThroughput * 1000;
                                ofstream udpThoughputFile38(dir + "udpThroughput38.txt");
                                if (udpThoughputFile38.good()) {
                                    NS_LOG_INFO("udpThroughput is OK!\n" << endl);
                                    udpThoughputFile38 << packetThroughput * 1000 << " Kbps\n";
                                    udpThoughputFile38.close();
                                } else {
                                    NS_LOG_INFO("Cannot create udpThroughput.txt !\n");
                                }

                            }

                            if (packet->FindFirstMatchingByteTag(timestamp)) {
                                Time tx = timestamp.GetTimestamp();
                                Time dx = Simulator::Now() - tx;
                                aa38 = dx.GetMicroSeconds();
                                delay38 += aa38;
                            }

                            if (Simulator::Now() > Seconds(record_start[18])) {  // --

                                /**
                                 * get all receive packets
                                 */
                                std::ofstream PidSizeFile38(dir + "PidSetSize38.txt");

//                                uint16_t PidSetSize = pktnum; to avoid

                                if (PidSizeFile38.good()) {
                                    PidSizeFile38 << PidSet38.size() << endl;
                                }
                                PidSizeFile38.close();

                                /**
                                 * compute delay
                                 */
//                                uint64_t delayy38 = standard * (1 + (ttnt / 2 - 1) * k) + rand() % 30 + 1;
                                double delayy38 = (double) delay38 / PidSet38.size() / 1000;
                                ofstream delayput38(dir + "delayput38.txt");
                                if (delayput38.good()) {
                                    delayput38 << delayy38 << " ms\n";
                                    delayput38.close();
                                }

                            }
                        }
                        m_rxTrace(packet); //pinganzhang
                    }


                    /**
                    * business 19
                    */
                    if (UdpServer::m_port == 39) {

                        if (pre_tps[18] > top_tps[18]) {
                            continue;
                        }

                        static int a39 = 0;
                        static Time firstRx39;
                        uint64_t aa39;

                        NS_LOG_INFO("pinganzhang:::::::: UdpSever Rev Count = " << ++a39 << " Now ********* "
                                                                                << Simulator::Now()
                                                                                << std::endl);

                        PidSet39.insert(AppHdr.ReadPacketId());

                        if (Simulator::Now() > Seconds(record_start[19]) && rxcnt39 == 1) //记录吞吐量的时间  // --
                        {
                            firstRx39 = Simulator::Now();
                            ++rxcnt39;
                        }

                        NS_LOG_INFO("pinganzhang::::::::first arrived time = " << firstRx39.GetSeconds() << std::endl);
                        NS_LOG_INFO("pinganzhang::::::::AppPayLoadSize = " << packetOdcp->GetSize() - 39 << std::endl);

                        packetSizeVec39.push_back((packetOdcp->GetSize() - 39) * 8);//应用层负载

                        //pinganzhang get throughput

                        if (1) {

                            /**
                             * get Throughput
                             */
                            if (Simulator::Now() > Seconds(record_start[19])) {  // --
                                uint32_t sumPacketSize = accumulate(packetSizeVec39.begin(), packetSizeVec39.end(),
                                                                    0.0);
                                double randomValue = rand() % 20000 + 20000;
                                NS_LOG_INFO("random value is :" << randomValue << endl);
                                double packetThroughput = /*(sumPacketSize - randomValue)*/
                                        sumPacketSize /
                                        (Seconds(record_end[19]).GetSeconds() -
                                         firstRx39.GetSeconds()) / 1024 / 1024;
                                NS_LOG_INFO("Throughput: " << packetThroughput * 1000 << " Kbps\n");
                                pre_tps[18] = packetThroughput * 1000;
                                ofstream udpThoughputFile39(dir + "udpThroughput39.txt");
                                if (udpThoughputFile39.good()) {
                                    NS_LOG_INFO("udpThroughput is OK!\n" << endl);
                                    udpThoughputFile39 << packetThroughput * 1000 << " Kbps\n";
                                    udpThoughputFile39.close();
                                } else {
                                    NS_LOG_INFO("Cannot create udpThroughput.txt !\n");
                                }

                            }

                            if (packet->FindFirstMatchingByteTag(timestamp)) {
                                Time tx = timestamp.GetTimestamp();
                                Time dx = Simulator::Now() - tx;
                                aa39 = dx.GetMicroSeconds();
                                delay39 += aa39;
                            }

                            if (Simulator::Now() > Seconds(record_start[19])) {  // --

                                /**
                                 * get all receive packets
                                 */
                                std::ofstream PidSizeFile39(dir + "PidSetSize39.txt");

//                                uint16_t PidSetSize = pktnum; to avoid

                                if (PidSizeFile39.good()) {
                                    PidSizeFile39 << PidSet39.size() << endl;
                                }
                                PidSizeFile39.close();

                                /**
                                 * compute delay
                                 */
//                                uint64_t delayy39 = standard * (1 + (ttnt / 2 - 1) * k) + rand() % 30 + 1;
                                double delayy39 = (double) delay39 / PidSet39.size() / 1000;
                                ofstream delayput39(dir + "delayput39.txt");
                                if (delayput39.good()) {
                                    delayput39 << delayy39 << " ms\n";
                                    delayput39.close();
                                }

                            }
                        }
                        m_rxTrace(packet); //pinganzhang
                    }


                    /**
                    * business 20
                    */
                    if (UdpServer::m_port == 40) {

                        if (pre_tps[19] > top_tps[19]) {
                            continue;
                        }

                        static int a40 = 0;
                        static Time firstRx40;
                        uint64_t aa40;

                        NS_LOG_INFO("pinganzhang:::::::: UdpSever Rev Count = " << ++a40 << " Now ********* "
                                                                                << Simulator::Now()
                                                                                << std::endl);

                        PidSet40.insert(AppHdr.ReadPacketId());

                        if (Simulator::Now() > Seconds(record_start[20]) && rxcnt40 == 1) //记录吞吐量的时间  // --
                        {
                            firstRx40 = Simulator::Now();
                            ++rxcnt40;
                        }

                        NS_LOG_INFO("pinganzhang::::::::first arrived time = " << firstRx40.GetSeconds() << std::endl);
                        NS_LOG_INFO("pinganzhang::::::::AppPayLoadSize = " << packetOdcp->GetSize() - 39 << std::endl);

                        packetSizeVec40.push_back((packetOdcp->GetSize() - 39) * 8);//应用层负载

                        //pinganzhang get throughput

                        if (1) {

                            /**
                             * get Throughput
                             */
                            if (Simulator::Now() > Seconds(record_start[20])) {  // --
                                uint32_t sumPacketSize = accumulate(packetSizeVec40.begin(), packetSizeVec40.end(),
                                                                    0.0);
                                double randomValue = rand() % 20000 + 20000;
                                NS_LOG_INFO("random value is :" << randomValue << endl);
                                double packetThroughput = /*(sumPacketSize - randomValue)*/
                                        sumPacketSize /
                                        (Seconds(record_end[20]).GetSeconds() -
                                         firstRx40.GetSeconds()) / 1024 / 1024;
                                NS_LOG_INFO("Throughput: " << packetThroughput * 1000 << " Kbps\n");
                                pre_tps[19] = packetThroughput * 1000;
                                ofstream udpThoughputFile40(dir + "udpThroughput40.txt");
                                if (udpThoughputFile40.good()) {
                                    NS_LOG_INFO("udpThroughput is OK!\n" << endl);
                                    udpThoughputFile40 << packetThroughput * 1000 << " Kbps\n";
                                    udpThoughputFile40.close();
                                } else {
                                    NS_LOG_INFO("Cannot create udpThroughput.txt !\n");
                                }

                            }

                            if (packet->FindFirstMatchingByteTag(timestamp)) {
                                Time tx = timestamp.GetTimestamp();
                                Time dx = Simulator::Now() - tx;
                                aa40 = dx.GetMicroSeconds();
                                delay40 += aa40;
                            }

                            if (Simulator::Now() > Seconds(record_start[20])) {  // --

                                /**
                                 * get all receive packets
                                 */
                                std::ofstream PidSizeFile40(dir + "PidSetSize40.txt");

//                                uint16_t PidSetSize = pktnum; to avoid

                                if (PidSizeFile40.good()) {
                                    PidSizeFile40 << PidSet40.size() << endl;
                                }
                                PidSizeFile40.close();

                                /**
                                 * compute delay
                                 */
//                                uint64_t delayy40 = standard * (1 + (ttnt / 2 - 1) * k) + rand() % 30 + 1;
                                double delayy40 = (double) delay40 / PidSet40.size() / 1000;
                                ofstream delayput40(dir + "delayput40.txt");
                                if (delayput40.good()) {
                                    delayput40 << delayy40 << " ms\n";
                                    delayput40.close();
                                }

                            }
                        }
                        m_rxTrace(packet); //pinganzhang
                    }


                    /**
                     * business 21
                     */
                    if (UdpServer::m_port == 41) {

                        if (pre_tps[20] > top_tps[20]) {
                            continue;
                        }

                        static int a41 = 0;
                        static Time firstRx41;
                        uint64_t aa41;

                        NS_LOG_INFO("pinganzhang:::::::: UdpSever Rev Count = " << ++a41 << " Now ********* "
                                                                                << Simulator::Now()
                                                                                << std::endl);

                        PidSet41.insert(AppHdr.ReadPacketId());

                        if (Simulator::Now() > Seconds(record_start[21]) && rxcnt41 == 1) //记录吞吐量的时间  // --
                        {
                            firstRx41 = Simulator::Now();
                            ++rxcnt41;
                        }

                        NS_LOG_INFO("pinganzhang::::::::first arrived time = " << firstRx41.GetSeconds() << std::endl);
                        NS_LOG_INFO("pinganzhang::::::::AppPayLoadSize = " << packetOdcp->GetSize() - 39 << std::endl);

                        packetSizeVec41.push_back((packetOdcp->GetSize() - 39) * 8);//应用层负载

                        //pinganzhang get throughput

                        if (1) {

                            /**
                             * get Throughput
                             */
                            if (Simulator::Now() > Seconds(record_start[21])) {  // --
                                uint32_t sumPacketSize = accumulate(packetSizeVec41.begin(), packetSizeVec41.end(),
                                                                    0.0);
                                double randomValue = rand() % 20000 + 20000;
                                NS_LOG_INFO("random value is :" << randomValue << endl);
                                double packetThroughput = /*(sumPacketSize - randomValue)*/
                                        sumPacketSize /
                                        (Seconds(record_end[21]).GetSeconds() -
                                         firstRx41.GetSeconds()) / 1024 / 1024;
                                NS_LOG_INFO("Throughput: " << packetThroughput * 1000 << " Kbps\n");
                                pre_tps[20] = packetThroughput * 1000;
                                ofstream udpThoughputFile41(dir + "udpThroughput41.txt");
                                if (udpThoughputFile41.good()) {
                                    NS_LOG_INFO("udpThroughput is OK!\n" << endl);
                                    udpThoughputFile41 << packetThroughput * 1000 << " Kbps\n";
                                    udpThoughputFile41.close();
                                } else {
                                    NS_LOG_INFO("Cannot create udpThroughput.txt !\n");
                                }

                            }

                            if (packet->FindFirstMatchingByteTag(timestamp)) {
                                Time tx = timestamp.GetTimestamp();
                                Time dx = Simulator::Now() - tx;
                                aa41 = dx.GetMicroSeconds();
                                delay41 += aa41;
                            }

                            if (Simulator::Now() > Seconds(record_start[21])) {  // --

                                /**
                                 * get all receive packets
                                 */
                                std::ofstream PidSizeFile41(dir + "PidSetSize41.txt");

//                                uint16_t PidSetSize = pktnum; to avoid

                                if (PidSizeFile41.good()) {
                                    PidSizeFile41 << PidSet41.size() << endl;
                                }
                                PidSizeFile41.close();

                                /**
                                 * compute delay
                                 */
//                                uint64_t delayy41 = standard * (1 + (ttnt / 2 - 1) * k) + rand() % 30 + 1;
                                double delayy41 = (double) delay41 / PidSet41.size() / 1000;
                                ofstream delayput41(dir + "delayput41.txt");
                                if (delayput41.good()) {
                                    delayput41 << delayy41 << " ms\n";
                                    delayput41.close();
                                }

                            }
                        }
                        m_rxTrace(packet); //pinganzhang
                    }


                    /**
                     * business 22
                     */
                    if (UdpServer::m_port == 42) {

                        if (pre_tps[21] > top_tps[21]) {
                            continue;
                        }

                        static int a42 = 0;
                        static Time firstRx42;
                        uint64_t aa42;

                        NS_LOG_INFO("pinganzhang:::::::: UdpSever Rev Count = " << ++a42 << " Now ********* "
                                                                                << Simulator::Now()
                                                                                << std::endl);

                        PidSet42.insert(AppHdr.ReadPacketId());

                        if (Simulator::Now() > Seconds(record_start[22]) && rxcnt42 == 1) //记录吞吐量的时间  // --
                        {
                            firstRx42 = Simulator::Now();
                            ++rxcnt42;
                        }

                        NS_LOG_INFO("pinganzhang::::::::first arrived time = " << firstRx42.GetSeconds() << std::endl);
                        NS_LOG_INFO("pinganzhang::::::::AppPayLoadSize = " << packetOdcp->GetSize() - 39 << std::endl);

                        packetSizeVec42.push_back((packetOdcp->GetSize() - 39) * 8);//应用层负载

                        //pinganzhang get throughput

                        if (1) {

                            /**
                             * get Throughput
                             */
                            if (Simulator::Now() > Seconds(record_start[22])) {  // --
                                uint32_t sumPacketSize = accumulate(packetSizeVec42.begin(), packetSizeVec42.end(),
                                                                    0.0);
                                double randomValue = rand() % 20000 + 20000;
                                NS_LOG_INFO("random value is :" << randomValue << endl);
                                double packetThroughput = /*(sumPacketSize - randomValue)*/
                                        sumPacketSize /
                                        (Seconds(record_end[22]).GetSeconds() -
                                         firstRx42.GetSeconds()) / 1024 / 1024;
                                NS_LOG_INFO("Throughput: " << packetThroughput * 1000 << " Kbps\n");
                                pre_tps[21] = packetThroughput * 1000;
                                ofstream udpThoughputFile42(dir + "udpThroughput42.txt");
                                if (udpThoughputFile42.good()) {
                                    NS_LOG_INFO("udpThroughput is OK!\n" << endl);
                                    udpThoughputFile42 << packetThroughput * 1000 << " Kbps\n";
                                    udpThoughputFile42.close();
                                } else {
                                    NS_LOG_INFO("Cannot create udpThroughput.txt !\n");
                                }

                            }

                            if (packet->FindFirstMatchingByteTag(timestamp)) {
                                Time tx = timestamp.GetTimestamp();
                                Time dx = Simulator::Now() - tx;
                                aa42 = dx.GetMicroSeconds();
                                delay42 += aa42;
                            }

                            if (Simulator::Now() > Seconds(record_start[22])) {  // --

                                /**
                                 * get all receive packets
                                 */
                                std::ofstream PidSizeFile42(dir + "PidSetSize42.txt");

//                                uint16_t PidSetSize = pktnum; to avoid

                                if (PidSizeFile42.good()) {
                                    PidSizeFile42 << PidSet42.size() << endl;
                                }
                                PidSizeFile42.close();

                                /**
                                 * compute delay
                                 */
//                                uint64_t delayy42 = standard * (1 + (ttnt / 2 - 1) * k) + rand() % 30 + 1;
                                double delayy42 = (double) delay42 / PidSet42.size() / 1000;
                                ofstream delayput42(dir + "delayput42.txt");
                                if (delayput42.good()) {
                                    delayput42 << delayy42 << " ms\n";
                                    delayput42.close();
                                }

                            }
                        }
                        m_rxTrace(packet); //pinganzhang
                    }


                    /**
                     * business 23
                     */
                    if (UdpServer::m_port == 43) {

                        if (pre_tps[22] > top_tps[22]) {
                            continue;
                        }

                        static int a43 = 0;
                        static Time firstRx43;
                        uint64_t aa43;

                        NS_LOG_INFO("pinganzhang:::::::: UdpSever Rev Count = " << ++a43 << " Now ********* "
                                                                                << Simulator::Now()
                                                                                << std::endl);

                        PidSet43.insert(AppHdr.ReadPacketId());

                        if (Simulator::Now() > Seconds(record_start[23]) && rxcnt43 == 1) //记录吞吐量的时间  // --
                        {
                            firstRx43 = Simulator::Now();
                            ++rxcnt43;
                        }

                        NS_LOG_INFO("pinganzhang::::::::first arrived time = " << firstRx43.GetSeconds() << std::endl);
                        NS_LOG_INFO("pinganzhang::::::::AppPayLoadSize = " << packetOdcp->GetSize() - 39 << std::endl);

                        packetSizeVec43.push_back((packetOdcp->GetSize() - 39) * 8);//应用层负载

                        //pinganzhang get throughput

                        if (1) {

                            /**
                             * get Throughput
                             */
                            if (Simulator::Now() > Seconds(record_start[23])) {  // --
                                uint32_t sumPacketSize = accumulate(packetSizeVec43.begin(), packetSizeVec43.end(),
                                                                    0.0);
                                double randomValue = rand() % 20000 + 20000;
                                NS_LOG_INFO("random value is :" << randomValue << endl);
                                double packetThroughput = /*(sumPacketSize - randomValue)*/
                                        sumPacketSize /
                                        (Seconds(record_end[23]).GetSeconds() -
                                         firstRx43.GetSeconds()) / 1024 / 1024;
                                NS_LOG_INFO("Throughput: " << packetThroughput * 1000 << " Kbps\n");
                                pre_tps[22] = packetThroughput * 1000;
                                ofstream udpThoughputFile43(dir + "udpThroughput43.txt");
                                if (udpThoughputFile43.good()) {
                                    NS_LOG_INFO("udpThroughput is OK!\n" << endl);
                                    udpThoughputFile43 << packetThroughput * 1000 << " Kbps\n";
                                    udpThoughputFile43.close();
                                } else {
                                    NS_LOG_INFO("Cannot create udpThroughput.txt !\n");
                                }

                            }

                            if (packet->FindFirstMatchingByteTag(timestamp)) {
                                Time tx = timestamp.GetTimestamp();
                                Time dx = Simulator::Now() - tx;
                                aa43 = dx.GetMicroSeconds();
                                delay43 += aa43;
                            }

                            if (Simulator::Now() > Seconds(record_start[23])) {  // --

                                /**
                                 * get all receive packets
                                 */
                                std::ofstream PidSizeFile43(dir + "PidSetSize43.txt");

//                                uint16_t PidSetSize = pktnum; to avoid

                                if (PidSizeFile43.good()) {
                                    PidSizeFile43 << PidSet43.size() << endl;
                                }
                                PidSizeFile43.close();

                                /**
                                 * compute delay
                                 */
//                                uint64_t delayy43 = standard * (1 + (ttnt / 2 - 1) * k) + rand() % 30 + 1;
                                double delayy43 = (double) delay43 / PidSet43.size() / 1000;
                                ofstream delayput43(dir + "delayput43.txt");
                                if (delayput43.good()) {
                                    delayput43 << delayy43 << " ms\n";
                                    delayput43.close();
                                }

                            }
                        }
                        m_rxTrace(packet); //pinganzhang
                    }


                    /**
                     * business 24
                     */
                    if (UdpServer::m_port == 44) {

                        if (pre_tps[23] > top_tps[23]) {
                            continue;
                        }

                        static int a44 = 0;
                        static Time firstRx44;
                        uint64_t aa44;

                        NS_LOG_INFO("pinganzhang:::::::: UdpSever Rev Count = " << ++a44 << " Now ********* "
                                                                                << Simulator::Now()
                                                                                << std::endl);

                        PidSet44.insert(AppHdr.ReadPacketId());

                        if (Simulator::Now() > Seconds(record_start[24]) && rxcnt44 == 1) //记录吞吐量的时间  // --
                        {
                            firstRx44 = Simulator::Now();
                            ++rxcnt44;
                        }

                        NS_LOG_INFO("pinganzhang::::::::first arrived time = " << firstRx44.GetSeconds() << std::endl);
                        NS_LOG_INFO("pinganzhang::::::::AppPayLoadSize = " << packetOdcp->GetSize() - 39 << std::endl);

                        packetSizeVec44.push_back((packetOdcp->GetSize() - 39) * 8);//应用层负载

                        //pinganzhang get throughput

                        if (1) {

                            /**
                             * get Throughput
                             */
                            if (Simulator::Now() > Seconds(record_start[24])) {  // --
                                uint32_t sumPacketSize = accumulate(packetSizeVec44.begin(), packetSizeVec44.end(),
                                                                    0.0);
                                double randomValue = rand() % 20000 + 20000;
                                NS_LOG_INFO("random value is :" << randomValue << endl);
                                double packetThroughput = /*(sumPacketSize - randomValue)*/
                                        sumPacketSize /
                                        (Seconds(record_end[24]).GetSeconds() -
                                         firstRx44.GetSeconds()) / 1024 / 1024;
                                NS_LOG_INFO("Throughput: " << packetThroughput * 1000 << " Kbps\n");
                                pre_tps[23] = packetThroughput * 1000;
                                ofstream udpThoughputFile44(dir + "udpThroughput44.txt");
                                if (udpThoughputFile44.good()) {
                                    NS_LOG_INFO("udpThroughput is OK!\n" << endl);
                                    udpThoughputFile44 << packetThroughput * 1000 << " Kbps\n";
                                    udpThoughputFile44.close();
                                } else {
                                    NS_LOG_INFO("Cannot create udpThroughput.txt !\n");
                                }

                            }

                            if (packet->FindFirstMatchingByteTag(timestamp)) {
                                Time tx = timestamp.GetTimestamp();
                                Time dx = Simulator::Now() - tx;
                                aa44 = dx.GetMicroSeconds();
                                delay44 += aa44;
                            }

                            if (Simulator::Now() > Seconds(record_start[24])) {  // --

                                /**
                                 * get all receive packets
                                 */
                                std::ofstream PidSizeFile44(dir + "PidSetSize44.txt");

//                                uint16_t PidSetSize = pktnum; to avoid

                                if (PidSizeFile44.good()) {
                                    PidSizeFile44 << PidSet44.size() << endl;
                                }
                                PidSizeFile44.close();

                                /**
                                 * compute delay
                                 */
//                                uint64_t delayy44 = standard * (1 + (ttnt / 2 - 1) * k) + rand() % 30 + 1;
                                double delayy44 = (double) delay44 / PidSet44.size() / 1000;
                                ofstream delayput44(dir + "delayput44.txt");
                                if (delayput44.good()) {
                                    delayput44 << delayy44 << " ms\n";
                                    delayput44.close();
                                }

                            }
                        }
                        m_rxTrace(packet); //pinganzhang
                    }


                    /**
                     * business 25
                     */
                    if (UdpServer::m_port == 45) {

                        if (pre_tps[24] > top_tps[24]) {
                            continue;
                        }

                        static int a45 = 0;
                        static Time firstRx45;
                        uint64_t aa45;

                        NS_LOG_INFO("pinganzhang:::::::: UdpSever Rev Count = " << ++a45 << " Now ********* "
                                                                                << Simulator::Now()
                                                                                << std::endl);

                        PidSet45.insert(AppHdr.ReadPacketId());

                        if (Simulator::Now() > Seconds(record_start[25]) && rxcnt45 == 1) //记录吞吐量的时间  // --
                        {
                            firstRx45 = Simulator::Now();
                            ++rxcnt45;
                        }

                        NS_LOG_INFO("pinganzhang::::::::first arrived time = " << firstRx45.GetSeconds() << std::endl);
                        NS_LOG_INFO("pinganzhang::::::::AppPayLoadSize = " << packetOdcp->GetSize() - 39 << std::endl);

                        packetSizeVec45.push_back((packetOdcp->GetSize() - 39) * 8);//应用层负载

                        //pinganzhang get throughput

                        if (1) {

                            /**
                             * get Throughput
                             */
                            if (Simulator::Now() > Seconds(record_start[25])) {  // --
                                uint32_t sumPacketSize = accumulate(packetSizeVec45.begin(), packetSizeVec45.end(),
                                                                    0.0);
                                double randomValue = rand() % 20000 + 20000;
                                NS_LOG_INFO("random value is :" << randomValue << endl);
                                double packetThroughput = /*(sumPacketSize - randomValue)*/
                                        sumPacketSize /
                                        (Seconds(record_end[25]).GetSeconds() -
                                         firstRx45.GetSeconds()) / 1024 / 1024;
                                NS_LOG_INFO("Throughput: " << packetThroughput * 1000 << " Kbps\n");
                                pre_tps[24] = packetThroughput * 1000;
                                ofstream udpThoughputFile45(dir + "udpThroughput45.txt");
                                if (udpThoughputFile45.good()) {
                                    NS_LOG_INFO("udpThroughput is OK!\n" << endl);
                                    udpThoughputFile45 << packetThroughput * 1000 << " Kbps\n";
                                    udpThoughputFile45.close();
                                } else {
                                    NS_LOG_INFO("Cannot create udpThroughput.txt !\n");
                                }

                            }

                            if (packet->FindFirstMatchingByteTag(timestamp)) {
                                Time tx = timestamp.GetTimestamp();
                                Time dx = Simulator::Now() - tx;
                                aa45 = dx.GetMicroSeconds();
                                delay45 += aa45;
                            }

                            if (Simulator::Now() > Seconds(record_start[25])) {  // --

                                /**
                                 * get all receive packets
                                 */
                                std::ofstream PidSizeFile45(dir + "PidSetSize45.txt");

//                                uint16_t PidSetSize = pktnum; to avoid

                                if (PidSizeFile45.good()) {
                                    PidSizeFile45 << PidSet45.size() << endl;
                                }
                                PidSizeFile45.close();

                                /**
                                 * compute delay
                                 */
//                                uint64_t delayy45 = standard * (1 + (ttnt / 2 - 1) * k) + rand() % 30 + 1;
                                double delayy45 = (double) delay45 / PidSet45.size() / 1000;
                                ofstream delayput45(dir + "delayput45.txt");
                                if (delayput45.good()) {
                                    delayput45 << delayy45 << " ms\n";
                                    delayput45.close();
                                }

                            }
                        }
                        m_rxTrace(packet); //pinganzhang
                    }


                    /**
                     * business 26
                     */
                    if (UdpServer::m_port == 46) {

                        if (pre_tps[25] > top_tps[25]) {
                            continue;
                        }

                        static int a46 = 0;
                        static Time firstRx46;
                        uint64_t aa46;

                        NS_LOG_INFO("pinganzhang:::::::: UdpSever Rev Count = " << ++a46 << " Now ********* "
                                                                                << Simulator::Now()
                                                                                << std::endl);

                        PidSet46.insert(AppHdr.ReadPacketId());

                        if (Simulator::Now() > Seconds(record_start[26]) && rxcnt46 == 1) //记录吞吐量的时间  // --
                        {
                            firstRx46 = Simulator::Now();
                            ++rxcnt46;
                        }

                        NS_LOG_INFO("pinganzhang::::::::first arrived time = " << firstRx46.GetSeconds() << std::endl);
                        NS_LOG_INFO("pinganzhang::::::::AppPayLoadSize = " << packetOdcp->GetSize() - 39 << std::endl);

                        packetSizeVec46.push_back((packetOdcp->GetSize() - 39) * 8);//应用层负载

                        //pinganzhang get throughput

                        if (1) {

                            /**
                             * get Throughput
                             */
                            if (Simulator::Now() > Seconds(record_start[26])) {  // --
                                uint32_t sumPacketSize = accumulate(packetSizeVec46.begin(), packetSizeVec46.end(),
                                                                    0.0);
                                double randomValue = rand() % 20000 + 20000;
                                NS_LOG_INFO("random value is :" << randomValue << endl);
                                double packetThroughput = /*(sumPacketSize - randomValue)*/
                                        sumPacketSize /
                                        (Seconds(record_end[26]).GetSeconds() -
                                         firstRx46.GetSeconds()) / 1024 / 1024;
                                NS_LOG_INFO("Throughput: " << packetThroughput * 1000 << " Kbps\n");
                                pre_tps[25] = packetThroughput * 1000;
                                ofstream udpThoughputFile46(dir + "udpThroughput46.txt");
                                if (udpThoughputFile46.good()) {
                                    NS_LOG_INFO("udpThroughput is OK!\n" << endl);
                                    udpThoughputFile46 << packetThroughput * 1000 << " Kbps\n";
                                    udpThoughputFile46.close();
                                } else {
                                    NS_LOG_INFO("Cannot create udpThroughput.txt !\n");
                                }

                            }

                            if (packet->FindFirstMatchingByteTag(timestamp)) {
                                Time tx = timestamp.GetTimestamp();
                                Time dx = Simulator::Now() - tx;
                                aa46 = dx.GetMicroSeconds();
                                delay46 += aa46;
                            }

                            if (Simulator::Now() > Seconds(record_start[26])) {  // --

                                /**
                                 * get all receive packets
                                 */
                                std::ofstream PidSizeFile46(dir + "PidSetSize46.txt");

//                                uint16_t PidSetSize = pktnum; to avoid

                                if (PidSizeFile46.good()) {
                                    PidSizeFile46 << PidSet46.size() << endl;
                                }
                                PidSizeFile46.close();

                                /**
                                 * compute delay
                                 */
//                                uint64_t delayy46 = standard * (1 + (ttnt / 2 - 1) * k) + rand() % 30 + 1;
                                double delayy46 = (double) delay46 / PidSet46.size() / 1000;
                                ofstream delayput46(dir + "delayput46.txt");
                                if (delayput46.good()) {
                                    delayput46 << delayy46 << " ms\n";
                                    delayput46.close();
                                }

                            }
                        }
                        m_rxTrace(packet); //pinganzhang
                    }


                    /**
                     * business 27
                     */
                    if (UdpServer::m_port == 47) {

                        if (pre_tps[26] > top_tps[26]) {
                            continue;
                        }

                        static int a47 = 0;
                        static Time firstRx47;
                        uint64_t aa47;

                        NS_LOG_INFO("pinganzhang:::::::: UdpSever Rev Count = " << ++a47 << " Now ********* "
                                                                                << Simulator::Now()
                                                                                << std::endl);

                        PidSet47.insert(AppHdr.ReadPacketId());

                        if (Simulator::Now() > Seconds(record_start[27]) && rxcnt47 == 1) //记录吞吐量的时间  // --
                        {
                            firstRx47 = Simulator::Now();
                            ++rxcnt47;
                        }

                        NS_LOG_INFO("pinganzhang::::::::first arrived time = " << firstRx47.GetSeconds() << std::endl);
                        NS_LOG_INFO("pinganzhang::::::::AppPayLoadSize = " << packetOdcp->GetSize() - 39 << std::endl);

                        packetSizeVec47.push_back((packetOdcp->GetSize() - 39) * 8);//应用层负载

                        //pinganzhang get throughput

                        if (1) {

                            /**
                             * get Throughput
                             */
                            if (Simulator::Now() > Seconds(record_start[27])) {  // --
                                uint32_t sumPacketSize = accumulate(packetSizeVec47.begin(), packetSizeVec47.end(),
                                                                    0.0);
                                double randomValue = rand() % 20000 + 20000;
                                NS_LOG_INFO("random value is :" << randomValue << endl);
                                double packetThroughput = /*(sumPacketSize - randomValue)*/
                                        sumPacketSize /
                                        (Seconds(record_end[27]).GetSeconds() -
                                         firstRx47.GetSeconds()) / 1024 / 1024;
                                NS_LOG_INFO("Throughput: " << packetThroughput * 1000 << " Kbps\n");
                                pre_tps[26] = packetThroughput * 1000;
                                ofstream udpThoughputFile47(dir + "udpThroughput47.txt");
                                if (udpThoughputFile47.good()) {
                                    NS_LOG_INFO("udpThroughput is OK!\n" << endl);
                                    udpThoughputFile47 << packetThroughput * 1000 << " Kbps\n";
                                    udpThoughputFile47.close();
                                } else {
                                    NS_LOG_INFO("Cannot create udpThroughput.txt !\n");
                                }

                            }

                            if (packet->FindFirstMatchingByteTag(timestamp)) {
                                Time tx = timestamp.GetTimestamp();
                                Time dx = Simulator::Now() - tx;
                                aa47 = dx.GetMicroSeconds();
                                delay47 += aa47;
                            }

                            if (Simulator::Now() > Seconds(record_start[27])) {  // --

                                /**
                                 * get all receive packets
                                 */
                                std::ofstream PidSizeFile47(dir + "PidSetSize47.txt");

//                                uint16_t PidSetSize = pktnum; to avoid

                                if (PidSizeFile47.good()) {
                                    PidSizeFile47 << PidSet47.size() << endl;
                                }
                                PidSizeFile47.close();

                                /**
                                 * compute delay
                                 */
//                                uint64_t delayy47 = standard * (1 + (ttnt / 2 - 1) * k) + rand() % 30 + 1;
                                double delayy47 = (double) delay47 / PidSet47.size() / 1000;
                                ofstream delayput47(dir + "delayput47.txt");
                                if (delayput47.good()) {
                                    delayput47 << delayy47 << " ms\n";
                                    delayput47.close();
                                }

                            }
                        }
                        m_rxTrace(packet); //pinganzhang
                    }


                    /**
                     * business 28
                     */
                    if (UdpServer::m_port == 48) {

                        if (pre_tps[27] > top_tps[27]) {
                            continue;
                        }

                        static int a48 = 0;
                        static Time firstRx48;
                        uint64_t aa48;

                        NS_LOG_INFO("pinganzhang:::::::: UdpSever Rev Count = " << ++a48 << " Now ********* "
                                                                                << Simulator::Now()
                                                                                << std::endl);

                        PidSet48.insert(AppHdr.ReadPacketId());

                        if (Simulator::Now() > Seconds(record_start[28]) && rxcnt48 == 1) //记录吞吐量的时间  // --
                        {
                            firstRx48 = Simulator::Now();
                            ++rxcnt48;
                        }

                        NS_LOG_INFO("pinganzhang::::::::first arrived time = " << firstRx48.GetSeconds() << std::endl);
                        NS_LOG_INFO("pinganzhang::::::::AppPayLoadSize = " << packetOdcp->GetSize() - 39 << std::endl);

                        packetSizeVec48.push_back((packetOdcp->GetSize() - 39) * 8);//应用层负载

                        //pinganzhang get throughput

                        if (1) {

                            /**
                             * get Throughput
                             */
                            if (Simulator::Now() > Seconds(record_start[28])) {  // --
                                uint32_t sumPacketSize = accumulate(packetSizeVec48.begin(), packetSizeVec48.end(),
                                                                    0.0);
                                double randomValue = rand() % 20000 + 20000;
                                NS_LOG_INFO("random value is :" << randomValue << endl);
                                double packetThroughput = /*(sumPacketSize - randomValue)*/
                                        sumPacketSize /
                                        (Seconds(record_end[28]).GetSeconds() -
                                         firstRx48.GetSeconds()) / 1024 / 1024;
                                NS_LOG_INFO("Throughput: " << packetThroughput * 1000 << " Kbps\n");
                                pre_tps[27] = packetThroughput * 1000;
                                ofstream udpThoughputFile48(dir + "udpThroughput48.txt");
                                if (udpThoughputFile48.good()) {
                                    NS_LOG_INFO("udpThroughput is OK!\n" << endl);
                                    udpThoughputFile48 << packetThroughput * 1000 << " Kbps\n";
                                    udpThoughputFile48.close();
                                } else {
                                    NS_LOG_INFO("Cannot create udpThroughput.txt !\n");
                                }

                            }

                            if (packet->FindFirstMatchingByteTag(timestamp)) {
                                Time tx = timestamp.GetTimestamp();
                                Time dx = Simulator::Now() - tx;
                                aa48 = dx.GetMicroSeconds();
                                delay48 += aa48;
                            }

                            if (Simulator::Now() > Seconds(record_start[28])) {  // --

                                /**
                                 * get all receive packets
                                 */
                                std::ofstream PidSizeFile48(dir + "PidSetSize48.txt");

//                                uint16_t PidSetSize = pktnum; to avoid

                                if (PidSizeFile48.good()) {
                                    PidSizeFile48 << PidSet48.size() << endl;
                                }
                                PidSizeFile48.close();

                                /**
                                 * compute delay
                                 */
//                                uint64_t delayy48 = standard * (1 + (ttnt / 2 - 1) * k) + rand() % 30 + 1;
                                double delayy48 = (double) delay48 / PidSet48.size() / 1000;
                                ofstream delayput48(dir + "delayput48.txt");
                                if (delayput48.good()) {
                                    delayput48 << delayy48 << " ms\n";
                                    delayput48.close();
                                }

                            }
                        }
                        m_rxTrace(packet); //pinganzhang
                    }


                    /**
                     * business 29
                     */
                    if (UdpServer::m_port == 49) {

                        if (pre_tps[28] > top_tps[28]) {
                            continue;
                        }

                        static int a49 = 0;
                        static Time firstRx49;
                        uint64_t aa49;

                        NS_LOG_INFO("pinganzhang:::::::: UdpSever Rev Count = " << ++a49 << " Now ********* "
                                                                                << Simulator::Now()
                                                                                << std::endl);

                        PidSet49.insert(AppHdr.ReadPacketId());

                        if (Simulator::Now() > Seconds(record_start[29]) && rxcnt49 == 1) //记录吞吐量的时间  // --
                        {
                            firstRx49 = Simulator::Now();
                            ++rxcnt49;
                        }

                        NS_LOG_INFO("pinganzhang::::::::first arrived time = " << firstRx49.GetSeconds() << std::endl);
                        NS_LOG_INFO("pinganzhang::::::::AppPayLoadSize = " << packetOdcp->GetSize() - 39 << std::endl);

                        packetSizeVec49.push_back((packetOdcp->GetSize() - 39) * 8);//应用层负载

                        //pinganzhang get throughput

                        if (1) {

                            /**
                             * get Throughput
                             */
                            if (Simulator::Now() > Seconds(record_start[29])) {  // --
                                uint32_t sumPacketSize = accumulate(packetSizeVec49.begin(), packetSizeVec49.end(),
                                                                    0.0);
                                double randomValue = rand() % 20000 + 20000;
                                NS_LOG_INFO("random value is :" << randomValue << endl);
                                double packetThroughput = /*(sumPacketSize - randomValue)*/
                                        sumPacketSize /
                                        (Seconds(record_end[29]).GetSeconds() -
                                         firstRx49.GetSeconds()) / 1024 / 1024;
                                NS_LOG_INFO("Throughput: " << packetThroughput * 1000 << " Kbps\n");
                                pre_tps[28] = packetThroughput * 1000;
                                ofstream udpThoughputFile49(dir + "udpThroughput49.txt");
                                if (udpThoughputFile49.good()) {
                                    NS_LOG_INFO("udpThroughput is OK!\n" << endl);
                                    udpThoughputFile49 << packetThroughput * 1000 << " Kbps\n";
                                    udpThoughputFile49.close();
                                } else {
                                    NS_LOG_INFO("Cannot create udpThroughput.txt !\n");
                                }

                            }

                            if (packet->FindFirstMatchingByteTag(timestamp)) {
                                Time tx = timestamp.GetTimestamp();
                                Time dx = Simulator::Now() - tx;
                                aa49 = dx.GetMicroSeconds();
                                delay49 += aa49;
                            }

                            if (Simulator::Now() > Seconds(record_start[29])) {  // --

                                /**
                                 * get all receive packets
                                 */
                                std::ofstream PidSizeFile49(dir + "PidSetSize49.txt");

//                                uint16_t PidSetSize = pktnum; to avoid

                                if (PidSizeFile49.good()) {
                                    PidSizeFile49 << PidSet49.size() << endl;
                                }
                                PidSizeFile49.close();

                                /**
                                 * compute delay
                                 */
//                                uint64_t delayy49 = standard * (1 + (ttnt / 2 - 1) * k) + rand() % 30 + 1;
                                double delayy49 = (double) delay49 / PidSet49.size() / 1000;
                                ofstream delayput49(dir + "delayput49.txt");
                                if (delayput49.good()) {
                                    delayput49 << delayy49 << " ms\n";
                                    delayput49.close();
                                }

                            }
                        }
                        m_rxTrace(packet); //pinganzhang
                    }


                    /**
                     * business 30
                     */
                    if (UdpServer::m_port == 50) {

                        if (pre_tps[29] > top_tps[29]) {
                            continue;
                        }

                        static int a50 = 0;
                        static Time firstRx50;
                        uint64_t aa50;

                        NS_LOG_INFO("pinganzhang:::::::: UdpSever Rev Count = " << ++a50 << " Now ********* "
                                                                                << Simulator::Now()
                                                                                << std::endl);

                        PidSet50.insert(AppHdr.ReadPacketId());

                        if (Simulator::Now() > Seconds(record_start[30]) && rxcnt50 == 1) //记录吞吐量的时间  // --
                        {
                            firstRx50 = Simulator::Now();
                            ++rxcnt50;
                        }

                        NS_LOG_INFO("pinganzhang::::::::first arrived time = " << firstRx50.GetSeconds() << std::endl);
                        NS_LOG_INFO("pinganzhang::::::::AppPayLoadSize = " << packetOdcp->GetSize() - 39 << std::endl);

                        packetSizeVec50.push_back((packetOdcp->GetSize() - 39) * 8);//应用层负载

                        //pinganzhang get throughput

                        if (1) {

                            /**
                             * get Throughput
                             */
                            if (Simulator::Now() > Seconds(record_start[30])) {  // --
                                uint32_t sumPacketSize = accumulate(packetSizeVec50.begin(), packetSizeVec50.end(),
                                                                    0.0);
                                double randomValue = rand() % 20000 + 20000;
                                NS_LOG_INFO("random value is :" << randomValue << endl);
                                double packetThroughput = /*(sumPacketSize - randomValue)*/
                                        sumPacketSize /
                                        (Seconds(record_end[30]).GetSeconds() -
                                         firstRx50.GetSeconds()) / 1024 / 1024;
                                NS_LOG_INFO("Throughput: " << packetThroughput * 1000 << " Kbps\n");
                                pre_tps[29] = packetThroughput * 1000;
                                ofstream udpThoughputFile50(dir + "udpThroughput50.txt");
                                if (udpThoughputFile50.good()) {
                                    NS_LOG_INFO("udpThroughput is OK!\n" << endl);
                                    udpThoughputFile50 << packetThroughput * 1000 << " Kbps\n";
                                    udpThoughputFile50.close();
                                } else {
                                    NS_LOG_INFO("Cannot create udpThroughput.txt !\n");
                                }

                            }

                            if (packet->FindFirstMatchingByteTag(timestamp)) {
                                Time tx = timestamp.GetTimestamp();
                                Time dx = Simulator::Now() - tx;
                                aa50 = dx.GetMicroSeconds();
                                delay50 += aa50;
                            }

                            if (Simulator::Now() > Seconds(record_start[30])) {  // --

                                /**
                                 * get all receive packets
                                 */
                                std::ofstream PidSizeFile50(dir + "PidSetSize50.txt");

//                                uint16_t PidSetSize = pktnum; to avoid

                                if (PidSizeFile50.good()) {
                                    PidSizeFile50 << PidSet50.size() << endl;
                                }
                                PidSizeFile50.close();

                                /**
                                 * compute delay
                                 */
//                                uint64_t delayy50 = standard * (1 + (ttnt / 2 - 1) * k) + rand() % 30 + 1;
                                double delayy50 = (double) delay50 / PidSet50.size() / 1000;
                                ofstream delayput50(dir + "delayput50.txt");
                                if (delayput50.good()) {
                                    delayput50 << delayy50 << " ms\n";
                                    delayput50.close();
                                }

                            }
                        }
                        m_rxTrace(packet); //pinganzhang
                    }


/** --------------------------------------------------------------------------------------------------------------- **/
                } else {
                    if (UdpServer::m_port == 21) {
                        static int a0 = 0;
                        static Time firstRx0;
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
                                    sumPacketSize / (Seconds(40.0).GetSeconds() - firstRx0.GetSeconds()) / 1024 /
                                    1024;
                            cout << "Throughput: " << packetThroughput << " Mbps\n";
                            ofstream udpThoughputFile0(dir + "udpThroughput0.txt");
                            if (udpThoughputFile0.good()) {
                                NS_LOG_INFO("udpThroughput is OK!\n" << endl);
                                udpThoughputFile0 << packetThroughput << " Mbps\n";
                                udpThoughputFile0.close();
                            } else {
                                NS_LOG_INFO("Cannot create udpThroughput.txt !\n");
                            }
                        }

                        if (Simulator::Now() > Seconds(40.0)) {
                            std::ofstream PidSizeFile0(dir + "PidSetSize0.txt");
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

                if (InetSocketAddress::IsMatchingType(from)) {
                    NS_LOG_INFO("TraceDelay: RX " << packet->GetSize() <<  // packet->GetSize()
                                                  " bytes from " << InetSocketAddress::ConvertFrom(from).GetIpv4()
                                                  <<
                                                  " Sequence Number: " << currentSequenceNumber <<
                                                  " Uid: " << packet->GetUid() <<
                                                  " TXtime: " << seqTs.GetTs() <<
                                                  " RXtime: " << Simulator::Now() <<
                                                  " Delay: " << Simulator::Now() - seqTs.GetTs());
                } else if (Inet6SocketAddress::IsMatchingType(from)) {
                    NS_LOG_INFO("TraceDelay: RX " << packet->GetSize() <<
                                                  " bytes from " << Inet6SocketAddress::ConvertFrom(from).GetIpv6()
                                                  <<
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