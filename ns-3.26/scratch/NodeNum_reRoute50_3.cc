//
// Created by yf518 on 7/19/20.
//

/** 测试 模拟路由重构：
 *      节点间距：50
 *      业务类型数：3
 *
 */

#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-ttnt-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/netanim-module.h"
#include "ns3/olsr-module.h"
#include "ns3/dsr-module.h"
#include "ns3/dsdv-module.h"
#include <vector>
#include "ns3/stats-module.h"
#include <sstream>
#include <unistd.h>

using namespace ns3;
using namespace std;

NS_LOG_COMPONENT_DEFINE("ThirdScriptExample");

void TxCallback(Ptr<CounterCalculator<uint32_t>> datac, std::string path,
                Ptr<const Packet> packet) {
    NS_LOG_INFO("Sent frame counted in " << datac->GetKey());
    datac->Update();
}

int main(int argc, char *argv[]) {

    uint32_t ttntTotal = 61; // -------
    bool verbose = true;

    uint8_t kind = 3;
    uint32_t business = 2;  // hack: Add 1 per test. Range: [1, 10]
    uint32_t ttnt;
    uint32_t dir = 0;  // Output file path suffix

    Time::SetResolution(Time::NS);  // 最小时间单元：ns

    CommandLine cmd;
    cmd.AddValue("ttntTotal", "Number of \"extra\" CSMA nodes/devices", ttntTotal);
    cmd.AddValue("verbose", "Tell echo applications to log if true", verbose);
    cmd.AddValue("dir", "Specify the output file path suffix", dir);
    cmd.AddValue("business", "Number of traffic flows of a single type", business);
    cmd.Parse(argc, argv);
    
    ttnt = kind * business * 2;
    ns3::UdpServer::dirSuffix = dir;
//    UdpServer::reInit(kind, business);

    if (verbose) {
        LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO); //LOG_LEVEL_ALL
        LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);
        LogComponentEnable("UdpClient", LOG_LEVEL_INFO);
        LogComponentEnable("UdpServer", LOG_LEVEL_INFO);
        LogComponentEnable("UdpTraceClient", LOG_LEVEL_INFO);
    }


    /** 创建节点 */
    NodeContainer TTNTNode;
    TTNTNode.Create(ttntTotal);

    /** 创建物理层：Yans */
    ttnt::WifiHelper wifiTTNT;
    ttnt::YansWifiChannelHelper channelTTNT =
            ttnt::YansWifiChannelHelper::Default(); //使用默认的信道模型
    ttnt::YansWifiPhyHelper phyTTNT =
            ttnt::YansWifiPhyHelper::Default();      //使用默认的PHY模型
    phyTTNT.SetChannel(channelTTNT.Create()); //创建通道对象并把他关联到物理层对象管理器
    wifiTTNT.SetStandard(ttnt::WIFI_PHY_STANDARD_80211b);  // 设置wifi标准


    /** 创建MAC层 */
    ttnt::NqosWifiMacHelper wifiMacTTNT =
            ttnt::NqosWifiMacHelper::Default();
    // 指定wifi运行模式：基础或ad hoc模式(P153)
    wifiMacTTNT.SetType("ns3::TTNT-AdhocWifiMac");


    /** 创建网络设备 */
    NetDeviceContainer ttntDevice = wifiTTNT.Install(phyTTNT, wifiMacTTNT, TTNTNode);


    /** 指定移动模型 ：
     *    移动节点的移动模型分为两部分：
     *      初始位置分布：SetPositionAllocator() :定义一个移动节点的初始坐标
     *      后续移动轨迹模型：SetMobilityModel() ：节点的移动路径
     */
    MobilityHelper mobility;
    mobility.SetPositionAllocator("ns3::GridPositionAllocator",//按照设置好的行列参数把节点等间距放置在一个二维笛卡尔坐标系中
                                  "MinX", DoubleValue(0.0),   // 起始坐标 (0, 0)
                                  "MinY", DoubleValue(0.0),
                                  "DeltaX", DoubleValue(50), // X轴节点间距：0.01m
                                  "DeltaY", DoubleValue(50), // y轴节点间距：0.01m
                                  "GridWidth", UintegerValue(6),  // 每行最大节点数
                                  "LayoutType", StringValue("RowFirst"));  // 行优先放
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(TTNTNode);


    /** 安装协议栈 */
    InternetStackHelper stack;
    stack.Install(TTNTNode);


    /** 路由协议 */
    DsrHelper Dsr; // DSR：动态源路由协议
    DsrMainHelper DsrMain;
    Dsr.Set("isMalicious", UintegerValue(0));
    DsrMain.Install(Dsr, TTNTNode);


    /** 设置IP */
    Ipv4AddressHelper address;
    address.SetBase("198.3.1.0", "255.255.255.0");
    Ipv4InterfaceContainer ttntInterface;
    ttntInterface = address.Assign(ttntDevice);


    uint16_t workflow[31] = {0};
    uint16_t routing_start[31] = {0};
    uint16_t routing_end[31] = {0};
    uint16_t data_start[31] = {0};
    uint16_t data_end[31] = {0};

    uint16_t record_time = 50;  // 50

    for (uint8_t i = 1; i <= (ttnt / 2); i++) {
        workflow[i] = 1;

        routing_start[i] = (i - 1) * 57; // 57
        routing_end[i] = routing_start[i] + 5;

        data_start[i] = routing_start[i] + 6;
        data_end[i] = data_start[i] + record_time;
    }

    double simulation_time = ((double) ttnt / 2) * 57; // 57
    uint16_t packet_size[31] = {0};

    uint16_t size = 500;
    for (uint8_t i = 1; i <= kind; i++) {
        for (uint8_t j = 1; j <= business; j++) {
            packet_size[(i - 1) * business + j] = size;
        }
        size -= 20;
    }

    /** workflow: 1: 0->3 */
    if (workflow[1]) {
        /**
         * route
         */
        {
            uint32_t packetSizev = 1;
            uint32_t maxPacketCountv = 10000000;
            uint32_t packetFrequencyv = 1;
            Time interPacketIntervalv = Seconds((1 / (double) packetFrequencyv));

            UdpServerHelper echoServerv(111);
            ApplicationContainer serv1erAppsv = echoServerv.Install(TTNTNode.Get(3));
            serv1erAppsv.Start(Seconds(routing_start[1]));
            serv1erAppsv.Stop(Seconds(routing_end[1]));

            UdpClientHelper echoClient(ttntInterface.GetAddress(3), 111);
            echoClient.SetAttribute("MaxPackets", UintegerValue(maxPacketCountv));
            echoClient.SetAttribute("Interval", TimeValue(interPacketIntervalv));
            echoClient.SetAttribute("PacketSize", UintegerValue(packetSizev));

            ApplicationContainer clientAppsv = echoClient.Install(
                    TTNTNode.Get(0));
            clientAppsv.Start(Seconds(routing_start[1]));
            clientAppsv.Stop(Seconds(routing_end[1]));
        }

        /**
         * data
         */
        {
//            uint32_t packetSize = 1000;
            uint32_t maxPacketCount = 10000000;
            uint32_t packetFrequency = 20;
            Time interPacketInterval = Seconds((1 / (double) packetFrequency) /* 1/((double)20) */);

            UdpServerHelper echoServerv(21);
            ApplicationContainer serverAppsv = echoServerv.Install(TTNTNode.Get(3));
            serverAppsv.Start(Seconds(data_start[1]));
            serverAppsv.Stop(Seconds(data_end[1]));

            UdpClientHelper echoClient(ttntInterface.GetAddress(3), 21);
            echoClient.SetAttribute("MaxPackets", UintegerValue(maxPacketCount));
            echoClient.SetAttribute("Interval", TimeValue(interPacketInterval));
            echoClient.SetAttribute("PacketSize", UintegerValue(packet_size[1]));

            ApplicationContainer clientApps = echoClient.Install(TTNTNode.Get(0));
            clientApps.Start(Seconds(data_start[1]));
            clientApps.Stop(Seconds(data_end[1]));
        }
    }

    /** workflow: 2: 1->4  */
    if (workflow[2]) {

        /**
         * route
         */
        {
            uint32_t packetSizev = 1;
            uint32_t maxPacketCountv = 10000000;
            uint32_t packetFrequencyv = 1;
            Time interPacketIntervalv = Seconds((1 / (double) packetFrequencyv));

            UdpServerHelper echoServerv(112);                                          // port (+1)
            ApplicationContainer serv1erAppsv = echoServerv.Install(TTNTNode.Get(4)); // node (+2)
            serv1erAppsv.Start(Seconds(routing_start[2]));                              // (+1)
            serv1erAppsv.Stop(Seconds(routing_end[2]));                                  // (+1)

            UdpClientHelper echoClient(ttntInterface.GetAddress(4), 112); // node + port (+2. +1)
            echoClient.SetAttribute("MaxPackets", UintegerValue(maxPacketCountv));
            echoClient.SetAttribute("Interval", TimeValue(interPacketIntervalv));
            echoClient.SetAttribute("PacketSize", UintegerValue(packetSizev));

            ApplicationContainer clientAppsv = echoClient.Install(
                    TTNTNode.Get(1));                          // node (+2)
            clientAppsv.Start(Seconds(routing_start[2])); // +1
            clientAppsv.Stop(Seconds(routing_end[2]));      // +1
        }

        /**
         * data
         */
        {
//            uint32_t packetSize = 1000; // ------
            uint32_t maxPacketCount = 10000000;
            uint32_t packetFrequency = 20;
            Time interPacketInterval = Seconds((1 / (double) packetFrequency) /*1/((double)20)*/);

            UdpServerHelper echoServerv(22);                                         // (+1)
            ApplicationContainer serverAppsv = echoServerv.Install(TTNTNode.Get(4)); // (+2)
            serverAppsv.Start(Seconds(data_start[2]));                                 // +1
            serverAppsv.Stop(Seconds(data_end[2]));                                     // +1

            UdpClientHelper echoClient(ttntInterface.GetAddress(4), 22); // (+2, +1)
            echoClient.SetAttribute("MaxPackets", UintegerValue(maxPacketCount));
            echoClient.SetAttribute("Interval", TimeValue(interPacketInterval));
            echoClient.SetAttribute("PacketSize", UintegerValue(packet_size[2]));

            ApplicationContainer clientApps = echoClient.Install(TTNTNode.Get(1)); // +2
            clientApps.Start(Seconds(data_start[2]));                               // +1
            clientApps.Stop(Seconds(data_end[2]));                                   // +1
        }
    }

    /** workflow: 3: 2->5 */
    if (workflow[3]) {

        /**
         * route
         */
        {
            uint32_t packetSizev = 1;
            uint32_t maxPacketCountv = 10000000;
            uint32_t packetFrequencyv = 1;
            Time interPacketIntervalv = Seconds((1 / (double) packetFrequencyv));

            UdpServerHelper echoServerv(113);                                          // port (+1)
            ApplicationContainer serv1erAppsv = echoServerv.Install(TTNTNode.Get(5)); // node (+2)
            serv1erAppsv.Start(Seconds(routing_start[3]));                              // (+1)
            serv1erAppsv.Stop(Seconds(routing_end[3]));                                  // (+1)

            UdpClientHelper echoClient(ttntInterface.GetAddress(5), 113); // node + port (+2. +1)
            echoClient.SetAttribute("MaxPackets", UintegerValue(maxPacketCountv));
            echoClient.SetAttribute("Interval", TimeValue(interPacketIntervalv));
            echoClient.SetAttribute("PacketSize", UintegerValue(packetSizev));

            ApplicationContainer clientAppsv = echoClient.Install(
                    TTNTNode.Get(2));                          // node (+2)
            clientAppsv.Start(Seconds(routing_start[3])); // +1
            clientAppsv.Stop(Seconds(routing_end[3]));      // +1
        }

        /**
         * data
         */
        {
//            uint32_t packetSize = 1000; // ------
            uint32_t maxPacketCount = 10000000;
            uint32_t packetFrequency = 20;
            Time interPacketInterval = Seconds((1 / (double) packetFrequency) /*1/((double)20)*/);

            UdpServerHelper echoServerv(23);                                         // (+1)
            ApplicationContainer serverAppsv = echoServerv.Install(TTNTNode.Get(5)); // (+2)
            serverAppsv.Start(Seconds(data_start[3]));                                 // +1
            serverAppsv.Stop(Seconds(data_end[3]));                                     // +1

            UdpClientHelper echoClient(ttntInterface.GetAddress(5), 23); // (+2, +1)
            echoClient.SetAttribute("MaxPackets", UintegerValue(maxPacketCount));
            echoClient.SetAttribute("Interval", TimeValue(interPacketInterval));
            echoClient.SetAttribute("PacketSize", UintegerValue(packet_size[3]));

            ApplicationContainer clientApps = echoClient.Install(TTNTNode.Get(2)); // +2
            clientApps.Start(Seconds(data_start[3]));                               // +1
            clientApps.Stop(Seconds(data_end[3]));                                   // +1
        }
    }

    /** workflow: 4: 6->9 */
    if (workflow[4]) {

        /**
         * route
         */
        {
            uint32_t packetSizev = 1;
            uint32_t maxPacketCountv = 10000000;
            uint32_t packetFrequencyv = 1;
            Time interPacketIntervalv = Seconds((1 / (double) packetFrequencyv));

            UdpServerHelper echoServerv(114);                                          // port (+1)
            ApplicationContainer serv1erAppsv = echoServerv.Install(TTNTNode.Get(9)); // node (+2)
            serv1erAppsv.Start(Seconds(routing_start[4]));                              // (+1)
            serv1erAppsv.Stop(Seconds(routing_end[4]));                                  // (+1)

            UdpClientHelper echoClient(ttntInterface.GetAddress(9), 114); // node + port (+2. +1)
            echoClient.SetAttribute("MaxPackets", UintegerValue(maxPacketCountv));
            echoClient.SetAttribute("Interval", TimeValue(interPacketIntervalv));
            echoClient.SetAttribute("PacketSize", UintegerValue(packetSizev));

            ApplicationContainer clientAppsv = echoClient.Install(
                    TTNTNode.Get(6));                          // node (+2)
            clientAppsv.Start(Seconds(routing_start[4])); // +1
            clientAppsv.Stop(Seconds(routing_end[4]));      // +1
        }

        /**
         * data
         */
        {
//            uint32_t packetSize = 1000; // ------
            uint32_t maxPacketCount = 10000000;
            uint32_t packetFrequency = 20;
            Time interPacketInterval = Seconds((1 / (double) packetFrequency) /*1/((double)20)*/);

            UdpServerHelper echoServerv(24);                                         // (+1)
            ApplicationContainer serverAppsv = echoServerv.Install(TTNTNode.Get(9)); // (+2)
            serverAppsv.Start(Seconds(data_start[4]));                                 // +1
            serverAppsv.Stop(Seconds(data_end[4]));                                     // +1

            UdpClientHelper echoClient(ttntInterface.GetAddress(9), 24); // (+2, +1)
            echoClient.SetAttribute("MaxPackets", UintegerValue(maxPacketCount));
            echoClient.SetAttribute("Interval", TimeValue(interPacketInterval));
            echoClient.SetAttribute("PacketSize", UintegerValue(packet_size[4]));

            ApplicationContainer clientApps = echoClient.Install(TTNTNode.Get(6)); // +2
            clientApps.Start(Seconds(data_start[4]));                               // +1
            clientApps.Stop(Seconds(data_end[4]));                                   // +1
        }
    }

    /** workflow: 5: 7->10 */
    if (workflow[5]) {

        /**
         * route
         */
        {
            uint32_t packetSizev = 1;
            uint32_t maxPacketCountv = 10000000;
            uint32_t packetFrequencyv = 1;
            Time interPacketIntervalv = Seconds((1 / (double) packetFrequencyv));

            UdpServerHelper echoServerv(115);                                          // port (+1)
            ApplicationContainer serv1erAppsv = echoServerv.Install(TTNTNode.Get(10)); // node (+2)
            serv1erAppsv.Start(Seconds(routing_start[5]));                              // (+1)
            serv1erAppsv.Stop(Seconds(routing_end[5]));                                  // (+1)

            UdpClientHelper echoClient(ttntInterface.GetAddress(10), 115); // node + port (+2. +1)
            echoClient.SetAttribute("MaxPackets", UintegerValue(maxPacketCountv));
            echoClient.SetAttribute("Interval", TimeValue(interPacketIntervalv));
            echoClient.SetAttribute("PacketSize", UintegerValue(packetSizev));

            ApplicationContainer clientAppsv = echoClient.Install(
                    TTNTNode.Get(7));                          // node (+2)
            clientAppsv.Start(Seconds(routing_start[5])); // +1
            clientAppsv.Stop(Seconds(routing_end[5]));      // +1
        }

        /**
         * data
         */
        {
//            uint32_t packetSize = 1000; // ------
            uint32_t maxPacketCount = 10000000;
            uint32_t packetFrequency = 20;
            Time interPacketInterval = Seconds((1 / (double) packetFrequency) /*1/((double)20)*/);

            UdpServerHelper echoServerv(25);                                         // (+1)
            ApplicationContainer serverAppsv = echoServerv.Install(TTNTNode.Get(10)); // (+2)
            serverAppsv.Start(Seconds(data_start[5]));                                 // +1
            serverAppsv.Stop(Seconds(data_end[5]));                                     // +1

            UdpClientHelper echoClient(ttntInterface.GetAddress(10), 25); // (+2, +1)
            echoClient.SetAttribute("MaxPackets", UintegerValue(maxPacketCount));
            echoClient.SetAttribute("Interval", TimeValue(interPacketInterval));
            echoClient.SetAttribute("PacketSize", UintegerValue(packet_size[5]));

            ApplicationContainer clientApps = echoClient.Install(TTNTNode.Get(7)); // +2
            clientApps.Start(Seconds(data_start[5]));                               // +1
            clientApps.Stop(Seconds(data_end[5]));                                   // +1
        }
    }

    /** workflow: 6: 8->11 */
    if (workflow[6]) {

        /**
         * route
         */
        {
            uint32_t packetSizev = 1;
            uint32_t maxPacketCountv = 10000000;
            uint32_t packetFrequencyv = 1;
            Time interPacketIntervalv = Seconds((1 / (double) packetFrequencyv));

            UdpServerHelper echoServerv(116);                                           // port (+1)
            ApplicationContainer serv1erAppsv = echoServerv.Install(TTNTNode.Get(11)); // node (+2)
            serv1erAppsv.Start(Seconds(routing_start[6]));                               // (+1)
            serv1erAppsv.Stop(Seconds(routing_end[6]));                                   // (+1)

            UdpClientHelper echoClient(ttntInterface.GetAddress(11), 116); // node + port (+2. +1)
            echoClient.SetAttribute("MaxPackets", UintegerValue(maxPacketCountv));
            echoClient.SetAttribute("Interval", TimeValue(interPacketIntervalv));
            echoClient.SetAttribute("PacketSize", UintegerValue(packetSizev));

            ApplicationContainer clientAppsv = echoClient.Install(
                    TTNTNode.Get(8));                          // node (+2)
            clientAppsv.Start(Seconds(routing_start[6])); // +1
            clientAppsv.Stop(Seconds(routing_end[6]));      // +1
        }

        /**
         * data
         */
        {
//            uint32_t packetSize = 1000; // ------
            uint32_t maxPacketCount = 10000000;
            uint32_t packetFrequency = 20;
            Time interPacketInterval = Seconds((1 / (double) packetFrequency) /*1/((double)20)*/);

            UdpServerHelper echoServerv(26);                                          // (+1)
            ApplicationContainer serverAppsv = echoServerv.Install(TTNTNode.Get(11)); // (+2)
            serverAppsv.Start(Seconds(data_start[6]));                                  // +1
            serverAppsv.Stop(Seconds(data_end[6]));                                      // +1

            UdpClientHelper echoClient(ttntInterface.GetAddress(11), 26); // (+2, +1)
            echoClient.SetAttribute("MaxPackets", UintegerValue(maxPacketCount));
            echoClient.SetAttribute("Interval", TimeValue(interPacketInterval));
            echoClient.SetAttribute("PacketSize", UintegerValue(packet_size[6]));

            ApplicationContainer clientApps = echoClient.Install(TTNTNode.Get(8)); // +2
            clientApps.Start(Seconds(data_start[6]));                                // +1
            clientApps.Stop(Seconds(data_end[6]));                                    // +1
        }
    }

    /** workflow: 7: 12->15 */
    if (workflow[7]) {

        /**
         * route
         */
        {
            uint32_t packetSizev = 1;
            uint32_t maxPacketCountv = 10000000;
            uint32_t packetFrequencyv = 1;
            Time interPacketIntervalv = Seconds((1 / (double) packetFrequencyv));

            UdpServerHelper echoServerv(117);                                           // port (+1)
            ApplicationContainer serv1erAppsv = echoServerv.Install(TTNTNode.Get(15)); // node (+2)
            serv1erAppsv.Start(Seconds(routing_start[7]));                               // (+1)
            serv1erAppsv.Stop(Seconds(routing_end[7]));                                   // (+1)

            UdpClientHelper echoClient(ttntInterface.GetAddress(15), 117); // node + port (+2. +1)
            echoClient.SetAttribute("MaxPackets", UintegerValue(maxPacketCountv));
            echoClient.SetAttribute("Interval", TimeValue(interPacketIntervalv));
            echoClient.SetAttribute("PacketSize", UintegerValue(packetSizev));

            ApplicationContainer clientAppsv = echoClient.Install(
                    TTNTNode.Get(12));                          // node (+2)
            clientAppsv.Start(Seconds(routing_start[7])); // +1
            clientAppsv.Stop(Seconds(routing_end[7]));      // +1
        }

        /**
         * data
         */
        {
//            uint32_t packetSize = 1000; // ------
            uint32_t maxPacketCount = 10000000;
            uint32_t packetFrequency = 20;
            Time interPacketInterval = Seconds((1 / (double) packetFrequency) /*1/((double)20)*/);

            UdpServerHelper echoServerv(27);                                          // (+1)
            ApplicationContainer serverAppsv = echoServerv.Install(TTNTNode.Get(15)); // (+2)
            serverAppsv.Start(Seconds(data_start[7]));                                  // +1
            serverAppsv.Stop(Seconds(data_end[7]));                                      // +1

            UdpClientHelper echoClient(ttntInterface.GetAddress(15), 27); // (+2, +1)
            echoClient.SetAttribute("MaxPackets", UintegerValue(maxPacketCount));
            echoClient.SetAttribute("Interval", TimeValue(interPacketInterval));
            echoClient.SetAttribute("PacketSize", UintegerValue(packet_size[7]));

            ApplicationContainer clientApps = echoClient.Install(TTNTNode.Get(12)); // +2
            clientApps.Start(Seconds(data_start[7]));                                // +1
            clientApps.Stop(Seconds(data_end[7]));                                    // +1
        }
    }

    /** workflow: 8: 13->16 */
    if (workflow[8]) {

        /**
         * route
         */
        {
            uint32_t packetSizev = 1;
            uint32_t maxPacketCountv = 10000000;
            uint32_t packetFrequencyv = 1;
            Time interPacketIntervalv = Seconds((1 / (double) packetFrequencyv));

            UdpServerHelper echoServerv(118);                                           // port (+1)
            ApplicationContainer serv1erAppsv = echoServerv.Install(TTNTNode.Get(16)); // node (+2)
            serv1erAppsv.Start(Seconds(routing_start[8]));                               // (+1)
            serv1erAppsv.Stop(Seconds(routing_end[8]));                                   // (+1)

            UdpClientHelper echoClient(ttntInterface.GetAddress(16), 118); // node + port (+2. +1)
            echoClient.SetAttribute("MaxPackets", UintegerValue(maxPacketCountv));
            echoClient.SetAttribute("Interval", TimeValue(interPacketIntervalv));
            echoClient.SetAttribute("PacketSize", UintegerValue(packetSizev));

            ApplicationContainer clientAppsv = echoClient.Install(
                    TTNTNode.Get(13));                          // node (+2)
            clientAppsv.Start(Seconds(routing_start[8])); // +1
            clientAppsv.Stop(Seconds(routing_end[8]));      // +1
        }

        /**
         * data
         */
        {
//            uint32_t packetSize = 1000; // odd number:1000, even number:500
            uint32_t maxPacketCount = 10000000;
            uint32_t packetFrequency = 20;
            Time interPacketInterval = Seconds((1 / (double) packetFrequency) /*1/((double)20)*/);

            UdpServerHelper echoServerv(28);                                          // (+1)
            ApplicationContainer serverAppsv = echoServerv.Install(TTNTNode.Get(16)); // (+2)
            serverAppsv.Start(Seconds(data_start[8]));                                  // +1
            serverAppsv.Stop(Seconds(data_end[8]));                                      // +1

            UdpClientHelper echoClient(ttntInterface.GetAddress(16), 28); // (+2, +1)
            echoClient.SetAttribute("MaxPackets", UintegerValue(maxPacketCount));
            echoClient.SetAttribute("Interval", TimeValue(interPacketInterval));
            echoClient.SetAttribute("PacketSize", UintegerValue(packet_size[8]));

            ApplicationContainer clientApps = echoClient.Install(TTNTNode.Get(13)); // +2
            clientApps.Start(Seconds(data_start[8]));                                // +1
            clientApps.Stop(Seconds(data_end[8]));                                    // +1
        }
    }


    /** workflow: 9: 14->17 */
    if (workflow[9]) {

        /**
         * route
         */
        {
            uint32_t packetSizev = 1;
            uint32_t maxPacketCountv = 10000000;
            uint32_t packetFrequencyv = 1;
            Time interPacketIntervalv = Seconds((1 / (double) packetFrequencyv));

            UdpServerHelper echoServerv(119);                                           // port (+1)
            ApplicationContainer serv1erAppsv = echoServerv.Install(TTNTNode.Get(17)); // node (+2)
            serv1erAppsv.Start(Seconds(routing_start[9]));                               // (+1)
            serv1erAppsv.Stop(Seconds(routing_end[9]));                                   // (+1)

            UdpClientHelper echoClient(ttntInterface.GetAddress(17), 119); // node + port (+2. +1)
            echoClient.SetAttribute("MaxPackets", UintegerValue(maxPacketCountv));
            echoClient.SetAttribute("Interval", TimeValue(interPacketIntervalv));
            echoClient.SetAttribute("PacketSize", UintegerValue(packetSizev));

            ApplicationContainer clientAppsv = echoClient.Install(
                    TTNTNode.Get(14));                          // node (+2)
            clientAppsv.Start(Seconds(routing_start[9])); // +1
            clientAppsv.Stop(Seconds(routing_end[9]));      // +1
        }

        /**
         * data
         */
        {
//            uint32_t packetSize = 1000; // odd number:1000, even number:500
            uint32_t maxPacketCount = 10000000;
            uint32_t packetFrequency = 20;
            Time interPacketInterval = Seconds((1 / (double) packetFrequency) /*1/((double)20)*/);

            UdpServerHelper echoServerv(29);                                          // (+1)
            ApplicationContainer serverAppsv = echoServerv.Install(TTNTNode.Get(17)); // (+2)
            serverAppsv.Start(Seconds(data_start[9]));                                  // +1
            serverAppsv.Stop(Seconds(data_end[9]));                                      // +1

            UdpClientHelper echoClient(ttntInterface.GetAddress(17), 29); // (+2, +1)
            echoClient.SetAttribute("MaxPackets", UintegerValue(maxPacketCount));
            echoClient.SetAttribute("Interval", TimeValue(interPacketInterval));
            echoClient.SetAttribute("PacketSize", UintegerValue(packet_size[9]));

            ApplicationContainer clientApps = echoClient.Install(TTNTNode.Get(14)); // +2
            clientApps.Start(Seconds(data_start[9]));                                // +1
            clientApps.Stop(Seconds(data_end[9]));                                    // +1
        }
    }


    /** workflow: 10: 18->21 */
    if (workflow[10]) {

        /**
         * route
         */
        {
            uint32_t packetSizev = 1;
            uint32_t maxPacketCountv = 10000000;
            uint32_t packetFrequencyv = 1;
            Time interPacketIntervalv = Seconds((1 / (double) packetFrequencyv));

            UdpServerHelper echoServerv(120);                                           // port (+1)
            ApplicationContainer serv1erAppsv = echoServerv.Install(TTNTNode.Get(21)); // node (+2)
            serv1erAppsv.Start(Seconds(routing_start[10]));                               // (+1)
            serv1erAppsv.Stop(Seconds(routing_end[10]));                                   // (+1)

            UdpClientHelper echoClient(ttntInterface.GetAddress(21), 120); // node + port (+2. +1)
            echoClient.SetAttribute("MaxPackets", UintegerValue(maxPacketCountv));
            echoClient.SetAttribute("Interval", TimeValue(interPacketIntervalv));
            echoClient.SetAttribute("PacketSize", UintegerValue(packetSizev));

            ApplicationContainer clientAppsv = echoClient.Install(
                    TTNTNode.Get(18));                          // node (+2)
            clientAppsv.Start(Seconds(routing_start[10])); // +1
            clientAppsv.Stop(Seconds(routing_end[10]));      // +1
        }

        /**
         * data
         */
        {
//            uint32_t packetSize = 1000; // odd number:1000, even number:500
            uint32_t maxPacketCount = 10000000;
            uint32_t packetFrequency = 20;
            Time interPacketInterval = Seconds((1 / (double) packetFrequency) /*1/((double)20)*/);

            UdpServerHelper echoServerv(30);                                          // (+1)
            ApplicationContainer serverAppsv = echoServerv.Install(TTNTNode.Get(21)); // (+2)
            serverAppsv.Start(Seconds(data_start[10]));                                  // +1
            serverAppsv.Stop(Seconds(data_end[10]));                                      // +1

            UdpClientHelper echoClient(ttntInterface.GetAddress(21), 30); // (+2, +1)
            echoClient.SetAttribute("MaxPackets", UintegerValue(maxPacketCount));
            echoClient.SetAttribute("Interval", TimeValue(interPacketInterval));
            echoClient.SetAttribute("PacketSize", UintegerValue(packet_size[10]));

            ApplicationContainer clientApps = echoClient.Install(TTNTNode.Get(18)); // +2
            clientApps.Start(Seconds(data_start[10]));                                // +1
            clientApps.Stop(Seconds(data_end[10]));                                    // +1
        }
    }


    /** workflow: 11: 19->22 */
    if (workflow[11]) {

        /**
         * route
         */
        {
            uint32_t packetSizev = 1;
            uint32_t maxPacketCountv = 10000000;
            uint32_t packetFrequencyv = 1;
            Time interPacketIntervalv = Seconds((1 / (double) packetFrequencyv));

            UdpServerHelper echoServerv(121);                                           // port (+1)
            ApplicationContainer serv1erAppsv = echoServerv.Install(TTNTNode.Get(22)); // node (+2)
            serv1erAppsv.Start(Seconds(routing_start[11]));                               // (+1)
            serv1erAppsv.Stop(Seconds(routing_end[11]));                                   // (+1)

            UdpClientHelper echoClient(ttntInterface.GetAddress(22), 121); // node + port (+2. +1)
            echoClient.SetAttribute("MaxPackets", UintegerValue(maxPacketCountv));
            echoClient.SetAttribute("Interval", TimeValue(interPacketIntervalv));
            echoClient.SetAttribute("PacketSize", UintegerValue(packetSizev));

            ApplicationContainer clientAppsv = echoClient.Install(
                    TTNTNode.Get(19));                          // node (+2)
            clientAppsv.Start(Seconds(routing_start[11])); // +1
            clientAppsv.Stop(Seconds(routing_end[11]));      // +1
        }

        /**
         * data
         */
        {
//            uint32_t packetSize = 1000; // odd number:1000, even number:500
            uint32_t maxPacketCount = 10000000;
            uint32_t packetFrequency = 20;
            Time interPacketInterval = Seconds((1 / (double) packetFrequency) /*1/((double)20)*/);

            UdpServerHelper echoServerv(31);                                          // (+1)
            ApplicationContainer serverAppsv = echoServerv.Install(TTNTNode.Get(22)); // (+2)
            serverAppsv.Start(Seconds(data_start[11]));                                  // +1
            serverAppsv.Stop(Seconds(data_end[11]));                                      // +1

            UdpClientHelper echoClient(ttntInterface.GetAddress(22), 31); // (+2, +1)
            echoClient.SetAttribute("MaxPackets", UintegerValue(maxPacketCount));
            echoClient.SetAttribute("Interval", TimeValue(interPacketInterval));
            echoClient.SetAttribute("PacketSize", UintegerValue(packet_size[11]));

            ApplicationContainer clientApps = echoClient.Install(TTNTNode.Get(19)); // +2
            clientApps.Start(Seconds(data_start[11]));                                // +1
            clientApps.Stop(Seconds(data_end[11]));                                    // +1
        }
    }


    /** workflow: 12: 20->23 */
    if (workflow[12]) {

        /**
         * route
         */
        {
            uint32_t packetSizev = 1;
            uint32_t maxPacketCountv = 10000000;
            uint32_t packetFrequencyv = 1;
            Time interPacketIntervalv = Seconds((1 / (double) packetFrequencyv));

            UdpServerHelper echoServerv(122);                                           // port (+1)
            ApplicationContainer serv1erAppsv = echoServerv.Install(TTNTNode.Get(23)); // node (+2)
            serv1erAppsv.Start(Seconds(routing_start[12]));                               // (+1)
            serv1erAppsv.Stop(Seconds(routing_end[12]));                                   // (+1)

            UdpClientHelper echoClient(ttntInterface.GetAddress(23), 122); // node + port (+2. +1)
            echoClient.SetAttribute("MaxPackets", UintegerValue(maxPacketCountv));
            echoClient.SetAttribute("Interval", TimeValue(interPacketIntervalv));
            echoClient.SetAttribute("PacketSize", UintegerValue(packetSizev));

            ApplicationContainer clientAppsv = echoClient.Install(
                    TTNTNode.Get(20));                          // node (+2)
            clientAppsv.Start(Seconds(routing_start[12])); // +1
            clientAppsv.Stop(Seconds(routing_end[12]));      // +1
        }

        /**
         * data
         */
        {
//            uint32_t packetSize = 1000; // odd number:1000, even number:500
            uint32_t maxPacketCount = 10000000;
            uint32_t packetFrequency = 20;
            Time interPacketInterval = Seconds((1 / (double) packetFrequency) /*1/((double)20)*/);

            UdpServerHelper echoServerv(32);                                          // (+1)
            ApplicationContainer serverAppsv = echoServerv.Install(TTNTNode.Get(23)); // (+2)
            serverAppsv.Start(Seconds(data_start[12]));                                  // +1
            serverAppsv.Stop(Seconds(data_end[12]));                                      // +1

            UdpClientHelper echoClient(ttntInterface.GetAddress(23), 32); // (+2, +1)
            echoClient.SetAttribute("MaxPackets", UintegerValue(maxPacketCount));
            echoClient.SetAttribute("Interval", TimeValue(interPacketInterval));
            echoClient.SetAttribute("PacketSize", UintegerValue(packet_size[12]));

            ApplicationContainer clientApps = echoClient.Install(TTNTNode.Get(20)); // +2
            clientApps.Start(Seconds(data_start[12]));                                // +1
            clientApps.Stop(Seconds(data_end[12]));                                    // +1
        }
    }


    /** workflow: 13: 24->27 */
    if (workflow[13]) {

        /**
         * route
         */
        {
            uint32_t packetSizev = 1;
            uint32_t maxPacketCountv = 10000000;
            uint32_t packetFrequencyv = 1;
            Time interPacketIntervalv = Seconds((1 / (double) packetFrequencyv));

            UdpServerHelper echoServerv(123);                                           // port (+1)
            ApplicationContainer serv1erAppsv = echoServerv.Install(TTNTNode.Get(27)); // node (+2)
            serv1erAppsv.Start(Seconds(routing_start[13]));                               // (+1)
            serv1erAppsv.Stop(Seconds(routing_end[13]));                                   // (+1)

            UdpClientHelper echoClient(ttntInterface.GetAddress(27), 123); // node + port (+2. +1)
            echoClient.SetAttribute("MaxPackets", UintegerValue(maxPacketCountv));
            echoClient.SetAttribute("Interval", TimeValue(interPacketIntervalv));
            echoClient.SetAttribute("PacketSize", UintegerValue(packetSizev));

            ApplicationContainer clientAppsv = echoClient.Install(
                    TTNTNode.Get(24));                          // node (+2)
            clientAppsv.Start(Seconds(routing_start[13])); // +1
            clientAppsv.Stop(Seconds(routing_end[13]));      // +1
        }

        /**
         * data
         */
        {
//            uint32_t packetSize = 1000; // odd number:1000, even number:500
            uint32_t maxPacketCount = 10000000;
            uint32_t packetFrequency = 20;
            Time interPacketInterval = Seconds((1 / (double) packetFrequency) /*1/((double)20)*/);

            UdpServerHelper echoServerv(33);                                          // (+1)
            ApplicationContainer serverAppsv = echoServerv.Install(TTNTNode.Get(27)); // (+2)
            serverAppsv.Start(Seconds(data_start[13]));                                  // +1
            serverAppsv.Stop(Seconds(data_end[13]));                                      // +1

            UdpClientHelper echoClient(ttntInterface.GetAddress(27), 33); // (+2, +1)
            echoClient.SetAttribute("MaxPackets", UintegerValue(maxPacketCount));
            echoClient.SetAttribute("Interval", TimeValue(interPacketInterval));
            echoClient.SetAttribute("PacketSize", UintegerValue(packet_size[13]));

            ApplicationContainer clientApps = echoClient.Install(TTNTNode.Get(24)); // +2
            clientApps.Start(Seconds(data_start[13]));                                // +1
            clientApps.Stop(Seconds(data_end[13]));                                    // +1
        }
    }


    /** workflow: 14: 25->28 */
    if (workflow[14]) {

        /**
         * route
         */
        {
            uint32_t packetSizev = 1;
            uint32_t maxPacketCountv = 10000000;
            uint32_t packetFrequencyv = 1;
            Time interPacketIntervalv = Seconds((1 / (double) packetFrequencyv));

            UdpServerHelper echoServerv(124);                                           // port (+1)
            ApplicationContainer serv1erAppsv = echoServerv.Install(TTNTNode.Get(28)); // node (+2)
            serv1erAppsv.Start(Seconds(routing_start[14]));                               // (+1)
            serv1erAppsv.Stop(Seconds(routing_end[14]));                                   // (+1)

            UdpClientHelper echoClient(ttntInterface.GetAddress(28), 124); // node + port (+2. +1)
            echoClient.SetAttribute("MaxPackets", UintegerValue(maxPacketCountv));
            echoClient.SetAttribute("Interval", TimeValue(interPacketIntervalv));
            echoClient.SetAttribute("PacketSize", UintegerValue(packetSizev));

            ApplicationContainer clientAppsv = echoClient.Install(
                    TTNTNode.Get(25));                          // node (+2)
            clientAppsv.Start(Seconds(routing_start[14])); // +1
            clientAppsv.Stop(Seconds(routing_end[14]));      // +1
        }

        /**
         * data
         */
        {
//            uint32_t packetSize = 1000; // odd number:1000, even number:500
            uint32_t maxPacketCount = 10000000;
            uint32_t packetFrequency = 20;
            Time interPacketInterval = Seconds((1 / (double) packetFrequency) /*1/((double)20)*/);

            UdpServerHelper echoServerv(34);                                          // (+1)
            ApplicationContainer serverAppsv = echoServerv.Install(TTNTNode.Get(28)); // (+2)
            serverAppsv.Start(Seconds(data_start[14]));                                  // +1
            serverAppsv.Stop(Seconds(data_end[14]));                                      // +1

            UdpClientHelper echoClient(ttntInterface.GetAddress(28), 34); // (+2, +1)
            echoClient.SetAttribute("MaxPackets", UintegerValue(maxPacketCount));
            echoClient.SetAttribute("Interval", TimeValue(interPacketInterval));
            echoClient.SetAttribute("PacketSize", UintegerValue(packet_size[14]));

            ApplicationContainer clientApps = echoClient.Install(TTNTNode.Get(25)); // +2
            clientApps.Start(Seconds(data_start[14]));                                // +1
            clientApps.Stop(Seconds(data_end[14]));                                    // +1
        }
    }


    /** workflow: 15: 26->29 */
    if (workflow[15]) {

        /**
         * route
         */
        {
            uint32_t packetSizev = 1;
            uint32_t maxPacketCountv = 10000000;
            uint32_t packetFrequencyv = 1;
            Time interPacketIntervalv = Seconds((1 / (double) packetFrequencyv));

            UdpServerHelper echoServerv(125);                                           // port (+1)
            ApplicationContainer serv1erAppsv = echoServerv.Install(TTNTNode.Get(29)); // node (+2)
            serv1erAppsv.Start(Seconds(routing_start[15]));                               // (+1)
            serv1erAppsv.Stop(Seconds(routing_end[15]));                                   // (+1)

            UdpClientHelper echoClient(ttntInterface.GetAddress(29), 125); // node + port (+2. +1)
            echoClient.SetAttribute("MaxPackets", UintegerValue(maxPacketCountv));
            echoClient.SetAttribute("Interval", TimeValue(interPacketIntervalv));
            echoClient.SetAttribute("PacketSize", UintegerValue(packetSizev));

            ApplicationContainer clientAppsv = echoClient.Install(
                    TTNTNode.Get(25));                          // node (+2)
            clientAppsv.Start(Seconds(routing_start[15])); // +1
            clientAppsv.Stop(Seconds(routing_end[15]));      // +1
        }

        /**
         * data
         */
        {
//            uint32_t packetSize = 1000; // odd number:1000, even number:500
            uint32_t maxPacketCount = 10000000;
            uint32_t packetFrequency = 20;
            Time interPacketInterval = Seconds((1 / (double) packetFrequency) /*1/((double)20)*/);

            UdpServerHelper echoServerv(35);                                          // (+1)
            ApplicationContainer serverAppsv = echoServerv.Install(TTNTNode.Get(29)); // (+2)
            serverAppsv.Start(Seconds(data_start[15]));                                  // +1
            serverAppsv.Stop(Seconds(data_end[15]));                                      // +1

            UdpClientHelper echoClient(ttntInterface.GetAddress(29), 35); // (+2, +1)
            echoClient.SetAttribute("MaxPackets", UintegerValue(maxPacketCount));
            echoClient.SetAttribute("Interval", TimeValue(interPacketInterval));
            echoClient.SetAttribute("PacketSize", UintegerValue(packet_size[15]));

            ApplicationContainer clientApps = echoClient.Install(TTNTNode.Get(25)); // +2
            clientApps.Start(Seconds(data_start[15]));                                // +1
            clientApps.Stop(Seconds(data_end[15]));                                    // +1
        }
    }


    /** workflow: 16: 30->33 */
    if (workflow[16]) {

        /**
         * route
         */
        {
            uint32_t packetSizev = 1;
            uint32_t maxPacketCountv = 10000000;
            uint32_t packetFrequencyv = 1;
            Time interPacketIntervalv = Seconds((1 / (double) packetFrequencyv));

            UdpServerHelper echoServerv(126);                                           // port (+1)
            ApplicationContainer serv1erAppsv = echoServerv.Install(TTNTNode.Get(33)); // node (+2)
            serv1erAppsv.Start(Seconds(routing_start[16]));                               // (+1)
            serv1erAppsv.Stop(Seconds(routing_end[16]));                                   // (+1)

            UdpClientHelper echoClient(ttntInterface.GetAddress(33), 126); // node + port (+2. +1)
            echoClient.SetAttribute("MaxPackets", UintegerValue(maxPacketCountv));
            echoClient.SetAttribute("Interval", TimeValue(interPacketIntervalv));
            echoClient.SetAttribute("PacketSize", UintegerValue(packetSizev));

            ApplicationContainer clientAppsv = echoClient.Install(
                    TTNTNode.Get(30));                          // node (+2)
            clientAppsv.Start(Seconds(routing_start[16])); // +1
            clientAppsv.Stop(Seconds(routing_end[16]));      // +1
        }

        /**
         * data
         */
        {
//            uint32_t packetSize = 1000; // odd number:1000, even number:500
            uint32_t maxPacketCount = 10000000;
            uint32_t packetFrequency = 20;
            Time interPacketInterval = Seconds((1 / (double) packetFrequency) /*1/((double)20)*/);

            UdpServerHelper echoServerv(36);                                          // (+1)
            ApplicationContainer serverAppsv = echoServerv.Install(TTNTNode.Get(33)); // (+2)
            serverAppsv.Start(Seconds(data_start[16]));                                  // +1
            serverAppsv.Stop(Seconds(data_end[16]));                                      // +1

            UdpClientHelper echoClient(ttntInterface.GetAddress(33), 36); // (+2, +1)
            echoClient.SetAttribute("MaxPackets", UintegerValue(maxPacketCount));
            echoClient.SetAttribute("Interval", TimeValue(interPacketInterval));
            echoClient.SetAttribute("PacketSize", UintegerValue(packet_size[16]));

            ApplicationContainer clientApps = echoClient.Install(TTNTNode.Get(30)); // +2
            clientApps.Start(Seconds(data_start[16]));                                // +1
            clientApps.Stop(Seconds(data_end[16]));                                    // +1
        }
    }


    /** workflow: 17: 31->34 */
    if (workflow[17]) {
        /**
         * route
         */
        {
            uint32_t packetSizev = 1;
            uint32_t maxPacketCountv = 10000000;
            uint32_t packetFrequencyv = 1;
            Time interPacketIntervalv = Seconds((1 / (double) packetFrequencyv));

            UdpServerHelper echoServerv(127);                                           // port (+1)
            ApplicationContainer serv1erAppsv = echoServerv.Install(TTNTNode.Get(34)); // node (+2)
            serv1erAppsv.Start(Seconds(routing_start[17]));                               // (+1)
            serv1erAppsv.Stop(Seconds(routing_end[17]));                                   // (+1)

            UdpClientHelper echoClient(ttntInterface.GetAddress(34), 127); // node + port (+2. +1)
            echoClient.SetAttribute("MaxPackets", UintegerValue(maxPacketCountv));
            echoClient.SetAttribute("Interval", TimeValue(interPacketIntervalv));
            echoClient.SetAttribute("PacketSize", UintegerValue(packetSizev));

            ApplicationContainer clientAppsv = echoClient.Install(
                    TTNTNode.Get(31));                          // node (+2)
            clientAppsv.Start(Seconds(routing_start[17])); // +1
            clientAppsv.Stop(Seconds(routing_end[17]));      // +1
        }

        /**
         * data
         */
        {
//            uint32_t packetSize = 1000; // odd number:1000, even number:500
            uint32_t maxPacketCount = 10000000;
            uint32_t packetFrequency = 20;
            Time interPacketInterval = Seconds((1 / (double) packetFrequency) /*1/((double)20)*/);

            UdpServerHelper echoServerv(37);                                          // (+1)
            ApplicationContainer serverAppsv = echoServerv.Install(TTNTNode.Get(34)); // (+2)
            serverAppsv.Start(Seconds(data_start[17]));                                  // +1
            serverAppsv.Stop(Seconds(data_end[17]));                                      // +1

            UdpClientHelper echoClient(ttntInterface.GetAddress(34), 37); // (+2, +1)
            echoClient.SetAttribute("MaxPackets", UintegerValue(maxPacketCount));
            echoClient.SetAttribute("Interval", TimeValue(interPacketInterval));
            echoClient.SetAttribute("PacketSize", UintegerValue(packet_size[17]));

            ApplicationContainer clientApps = echoClient.Install(TTNTNode.Get(31)); // +2
            clientApps.Start(Seconds(data_start[17]));                                // +1
            clientApps.Stop(Seconds(data_end[17]));                                    // +1
        }
    }


    /** workflow: 18: 32->35 */
    if (workflow[18]) {
        /**
         * route
         */
        {
            uint32_t packetSizev = 1;
            uint32_t maxPacketCountv = 10000000;
            uint32_t packetFrequencyv = 1;
            Time interPacketIntervalv = Seconds((1 / (double) packetFrequencyv));

            UdpServerHelper echoServerv(128);                                           // port (+1)
            ApplicationContainer serv1erAppsv = echoServerv.Install(TTNTNode.Get(35)); // node (+2)
            serv1erAppsv.Start(Seconds(routing_start[18]));                               // (+1)
            serv1erAppsv.Stop(Seconds(routing_end[18]));                                   // (+1)

            UdpClientHelper echoClient(ttntInterface.GetAddress(35), 128); // node + port (+2. +1)
            echoClient.SetAttribute("MaxPackets", UintegerValue(maxPacketCountv));
            echoClient.SetAttribute("Interval", TimeValue(interPacketIntervalv));
            echoClient.SetAttribute("PacketSize", UintegerValue(packetSizev));

            ApplicationContainer clientAppsv = echoClient.Install(
                    TTNTNode.Get(32));                          // node (+2)
            clientAppsv.Start(Seconds(routing_start[18])); // +1
            clientAppsv.Stop(Seconds(routing_end[18]));      // +1
        }

        /**
         * data
         */
        {
//            uint32_t packetSize = 1000; // odd number:1000, even number:500
            uint32_t maxPacketCount = 10000000;
            uint32_t packetFrequency = 20;
            Time interPacketInterval = Seconds((1 / (double) packetFrequency) /*1/((double)20)*/);

            UdpServerHelper echoServerv(38);                                          // (+1)
            ApplicationContainer serverAppsv = echoServerv.Install(TTNTNode.Get(35)); // (+2)
            serverAppsv.Start(Seconds(data_start[18]));                                  // +1
            serverAppsv.Stop(Seconds(data_end[18]));                                      // +1

            UdpClientHelper echoClient(ttntInterface.GetAddress(35), 38); // (+2, +1)
            echoClient.SetAttribute("MaxPackets", UintegerValue(maxPacketCount));
            echoClient.SetAttribute("Interval", TimeValue(interPacketInterval));
            echoClient.SetAttribute("PacketSize", UintegerValue(packet_size[18]));

            ApplicationContainer clientApps = echoClient.Install(TTNTNode.Get(32)); // +2
            clientApps.Start(Seconds(data_start[18]));                                // +1
            clientApps.Stop(Seconds(data_end[18]));                                    // +1
        }
    }


    /** workflow: 19: 36->39 */
    if (workflow[19]) {
        /**
         * route
         */
        {
            uint32_t packetSizev = 1;
            uint32_t maxPacketCountv = 10000000;
            uint32_t packetFrequencyv = 1;
            Time interPacketIntervalv = Seconds((1 / (double) packetFrequencyv));

            UdpServerHelper echoServerv(129);                                           // port (+1)
            ApplicationContainer serv1erAppsv = echoServerv.Install(TTNTNode.Get(39)); // node (+2)
            serv1erAppsv.Start(Seconds(routing_start[19]));                               // (+1)
            serv1erAppsv.Stop(Seconds(routing_end[19]));                                   // (+1)

            UdpClientHelper echoClient(ttntInterface.GetAddress(39), 129); // node + port (+2. +1)
            echoClient.SetAttribute("MaxPackets", UintegerValue(maxPacketCountv));
            echoClient.SetAttribute("Interval", TimeValue(interPacketIntervalv));
            echoClient.SetAttribute("PacketSize", UintegerValue(packetSizev));

            ApplicationContainer clientAppsv = echoClient.Install(
                    TTNTNode.Get(36));                          // node (+2)
            clientAppsv.Start(Seconds(routing_start[19])); // +1
            clientAppsv.Stop(Seconds(routing_end[19]));      // +1
        }

        /**
         * data
         */
        {
//            uint32_t packetSize = 1000; // odd number:1000, even number:500
            uint32_t maxPacketCount = 10000000;
            uint32_t packetFrequency = 20;
            Time interPacketInterval = Seconds((1 / (double) packetFrequency) /*1/((double)20)*/);

            UdpServerHelper echoServerv(39);                                          // (+1)
            ApplicationContainer serverAppsv = echoServerv.Install(TTNTNode.Get(39)); // (+2)
            serverAppsv.Start(Seconds(data_start[19]));                                  // +1
            serverAppsv.Stop(Seconds(data_end[19]));                                      // +1

            UdpClientHelper echoClient(ttntInterface.GetAddress(39), 39); // (+2, +1)
            echoClient.SetAttribute("MaxPackets", UintegerValue(maxPacketCount));
            echoClient.SetAttribute("Interval", TimeValue(interPacketInterval));
            echoClient.SetAttribute("PacketSize", UintegerValue(packet_size[19]));

            ApplicationContainer clientApps = echoClient.Install(TTNTNode.Get(36)); // +2
            clientApps.Start(Seconds(data_start[19]));                                // +1
            clientApps.Stop(Seconds(data_end[19]));                                    // +1
        }
    }


    /** workflow: 20: 37->40 */
    if (workflow[20]) {
        /**
         * route
         */
        {
            uint32_t packetSizev = 1;
            uint32_t maxPacketCountv = 10000000;
            uint32_t packetFrequencyv = 1;
            Time interPacketIntervalv = Seconds((1 / (double) packetFrequencyv));

            UdpServerHelper echoServerv(130);                                           // port (+1)
            ApplicationContainer serv1erAppsv = echoServerv.Install(TTNTNode.Get(40)); // node (+2)
            serv1erAppsv.Start(Seconds(routing_start[20]));                               // (+1)
            serv1erAppsv.Stop(Seconds(routing_end[20]));                                   // (+1)

            UdpClientHelper echoClient(ttntInterface.GetAddress(40), 130); // node + port (+2. +1)
            echoClient.SetAttribute("MaxPackets", UintegerValue(maxPacketCountv));
            echoClient.SetAttribute("Interval", TimeValue(interPacketIntervalv));
            echoClient.SetAttribute("PacketSize", UintegerValue(packetSizev));

            ApplicationContainer clientAppsv = echoClient.Install(
                    TTNTNode.Get(37));                          // node (+2)
            clientAppsv.Start(Seconds(routing_start[20])); // +1
            clientAppsv.Stop(Seconds(routing_end[20]));      // +1
        }

        /**
         * data
         */
        {
//            uint32_t packetSize = 1000; // odd number:1000, even number:500
            uint32_t maxPacketCount = 10000000;
            uint32_t packetFrequency = 20;
            Time interPacketInterval = Seconds((1 / (double) packetFrequency) /*1/((double)20)*/);

            UdpServerHelper echoServerv(40);                                          // (+1)
            ApplicationContainer serverAppsv = echoServerv.Install(TTNTNode.Get(40)); // (+2)
            serverAppsv.Start(Seconds(data_start[20]));                                  // +1
            serverAppsv.Stop(Seconds(data_end[20]));                                      // +1

            UdpClientHelper echoClient(ttntInterface.GetAddress(40), 40); // (+2, +1)
            echoClient.SetAttribute("MaxPackets", UintegerValue(maxPacketCount));
            echoClient.SetAttribute("Interval", TimeValue(interPacketInterval));
            echoClient.SetAttribute("PacketSize", UintegerValue(packet_size[20]));

            ApplicationContainer clientApps = echoClient.Install(TTNTNode.Get(37)); // +2
            clientApps.Start(Seconds(data_start[20]));                                // +1
            clientApps.Stop(Seconds(data_end[20]));                                    // +1
        }
    }


    /** workflow: 21: 38->41 */
    if (workflow[21]) {
        /**
         * route
         */
        {
            uint32_t packetSizev = 1;
            uint32_t maxPacketCountv = 10000000;
            uint32_t packetFrequencyv = 1;
            Time interPacketIntervalv = Seconds((1 / (double) packetFrequencyv));

            UdpServerHelper echoServerv(131);                                           // port (+1)
            ApplicationContainer serv1erAppsv = echoServerv.Install(TTNTNode.Get(41)); // node (+2)
            serv1erAppsv.Start(Seconds(routing_start[21]));                               // (+1)
            serv1erAppsv.Stop(Seconds(routing_end[21]));                                   // (+1)

            UdpClientHelper echoClient(ttntInterface.GetAddress(41), 131); // node + port (+2. +1)
            echoClient.SetAttribute("MaxPackets", UintegerValue(maxPacketCountv));
            echoClient.SetAttribute("Interval", TimeValue(interPacketIntervalv));
            echoClient.SetAttribute("PacketSize", UintegerValue(packetSizev));

            ApplicationContainer clientAppsv = echoClient.Install(
                    TTNTNode.Get(38));                          // node (+2)
            clientAppsv.Start(Seconds(routing_start[21])); // +1
            clientAppsv.Stop(Seconds(routing_end[21]));      // +1
        }

        /**
         * data
         */
        {
//            uint32_t packetSize = 1000; // odd number:1000, even number:500
            uint32_t maxPacketCount = 10000000;
            uint32_t packetFrequency = 20;
            Time interPacketInterval = Seconds((1 / (double) packetFrequency) /*1/((double)20)*/);

            UdpServerHelper echoServerv(41);                                          // (+1)
            ApplicationContainer serverAppsv = echoServerv.Install(TTNTNode.Get(41)); // (+2)
            serverAppsv.Start(Seconds(data_start[21]));                                  // +1
            serverAppsv.Stop(Seconds(data_end[21]));                                      // +1

            UdpClientHelper echoClient(ttntInterface.GetAddress(41), 41); // (+2, +1)
            echoClient.SetAttribute("MaxPackets", UintegerValue(maxPacketCount));
            echoClient.SetAttribute("Interval", TimeValue(interPacketInterval));
            echoClient.SetAttribute("PacketSize", UintegerValue(packet_size[21]));

            ApplicationContainer clientApps = echoClient.Install(TTNTNode.Get(38)); // +2
            clientApps.Start(Seconds(data_start[21]));                                // +1
            clientApps.Stop(Seconds(data_end[21]));                                    // +1
        }
    }


    /** workflow: 22: 42->45 */
    if (workflow[22]) {
        /**
         * route
         */
        {
            uint32_t packetSizev = 1;
            uint32_t maxPacketCountv = 10000000;
            uint32_t packetFrequencyv = 1;
            Time interPacketIntervalv = Seconds((1 / (double) packetFrequencyv));

            UdpServerHelper echoServerv(132);  // port (+1)
            ApplicationContainer serv1erAppsv = echoServerv.Install(TTNTNode.Get(45)); // node (+2)
            serv1erAppsv.Start(Seconds(routing_start[22]));                               // (+1)
            serv1erAppsv.Stop(Seconds(routing_end[22]));                            // (+1)

            UdpClientHelper echoClient(ttntInterface.GetAddress(45), 132); // node + port (+2. +1)
            echoClient.SetAttribute("MaxPackets", UintegerValue(maxPacketCountv));
            echoClient.SetAttribute("Interval", TimeValue(interPacketIntervalv));
            echoClient.SetAttribute("PacketSize", UintegerValue(packetSizev));

            ApplicationContainer clientAppsv = echoClient.Install(
                    TTNTNode.Get(42));                          // node (+2)
            clientAppsv.Start(Seconds(routing_start[22])); // +1
            clientAppsv.Stop(Seconds(routing_end[22]));      // +1
        }

        /**
         * data
         */
        {
//            uint32_t packetSize = 1000; // odd number:1000, even number:500
            uint32_t maxPacketCount = 10000000;
            uint32_t packetFrequency = 20;
            Time interPacketInterval = Seconds((1 / (double) packetFrequency) /*1/((double)20)*/);

            UdpServerHelper echoServerv(42);  // (+1)
            ApplicationContainer serverAppsv = echoServerv.Install(TTNTNode.Get(45)); // (+2)
            serverAppsv.Start(Seconds(data_start[22]));                                  // +1
            serverAppsv.Stop(Seconds(data_end[22]));                                // +1

            UdpClientHelper echoClient(ttntInterface.GetAddress(45), 42); // (+2, +1)
            echoClient.SetAttribute("MaxPackets", UintegerValue(maxPacketCount));
            echoClient.SetAttribute("Interval", TimeValue(interPacketInterval));
            echoClient.SetAttribute("PacketSize", UintegerValue(packet_size[22]));

            ApplicationContainer clientApps = echoClient.Install(TTNTNode.Get(42)); // +2
            clientApps.Start(Seconds(data_start[22]));                                // +1
            clientApps.Stop(Seconds(data_end[22]));                                    // +1
        }
    }


    /** workflow: 23: 43->46 */
    if (workflow[23]) {
        /**
         * route
         */
        {
            uint32_t packetSizev = 1;
            uint32_t maxPacketCountv = 10000000;
            uint32_t packetFrequencyv = 1;
            Time interPacketIntervalv = Seconds((1 / (double) packetFrequencyv));

            UdpServerHelper echoServerv(133);  // port (+1)
            ApplicationContainer serv1erAppsv = echoServerv.Install(TTNTNode.Get(46)); // node (+2)
            serv1erAppsv.Start(Seconds(routing_start[23]));                               // (+1)
            serv1erAppsv.Stop(Seconds(routing_end[23]));                            // (+1)

            UdpClientHelper echoClient(ttntInterface.GetAddress(46), 133); // node + port (+2. +1)
            echoClient.SetAttribute("MaxPackets", UintegerValue(maxPacketCountv));
            echoClient.SetAttribute("Interval", TimeValue(interPacketIntervalv));
            echoClient.SetAttribute("PacketSize", UintegerValue(packetSizev));

            ApplicationContainer clientAppsv = echoClient.Install(
                    TTNTNode.Get(43));                          // node (+2)
            clientAppsv.Start(Seconds(routing_start[23])); // +1
            clientAppsv.Stop(Seconds(routing_end[23]));      // +1
        }

        /**
         * data
         */
        {
//            uint32_t packetSize = 1000; // odd number:1000, even number:500
            uint32_t maxPacketCount = 10000000;
            uint32_t packetFrequency = 20;
            Time interPacketInterval = Seconds((1 / (double) packetFrequency) /*1/((double)20)*/);

            UdpServerHelper echoServerv(43);  // (+1)
            ApplicationContainer serverAppsv = echoServerv.Install(TTNTNode.Get(46)); // (+2)
            serverAppsv.Start(Seconds(data_start[23]));                                  // +1
            serverAppsv.Stop(Seconds(data_end[23]));                                // +1

            UdpClientHelper echoClient(ttntInterface.GetAddress(46), 43); // (+2, +1)
            echoClient.SetAttribute("MaxPackets", UintegerValue(maxPacketCount));
            echoClient.SetAttribute("Interval", TimeValue(interPacketInterval));
            echoClient.SetAttribute("PacketSize", UintegerValue(packet_size[23]));

            ApplicationContainer clientApps = echoClient.Install(TTNTNode.Get(43)); // +2
            clientApps.Start(Seconds(data_start[23]));                                // +1
            clientApps.Stop(Seconds(data_end[23]));                                    // +1
        }
    }


    /** workflow: 24: 44->47 */
    if (workflow[24]) {
        /**
         * route
         */
        {
            uint32_t packetSizev = 1;
            uint32_t maxPacketCountv = 10000000;
            uint32_t packetFrequencyv = 1;
            Time interPacketIntervalv = Seconds((1 / (double) packetFrequencyv));

            UdpServerHelper echoServerv(134);  // port (+1)
            ApplicationContainer serv1erAppsv = echoServerv.Install(TTNTNode.Get(47)); // node (+2)
            serv1erAppsv.Start(Seconds(routing_start[24]));                               // (+1)
            serv1erAppsv.Stop(Seconds(routing_end[24]));                            // (+1)

            UdpClientHelper echoClient(ttntInterface.GetAddress(47), 134); // node + port (+2. +1)
            echoClient.SetAttribute("MaxPackets", UintegerValue(maxPacketCountv));
            echoClient.SetAttribute("Interval", TimeValue(interPacketIntervalv));
            echoClient.SetAttribute("PacketSize", UintegerValue(packetSizev));

            ApplicationContainer clientAppsv = echoClient.Install(
                    TTNTNode.Get(44));                          // node (+2)
            clientAppsv.Start(Seconds(routing_start[24])); // +1
            clientAppsv.Stop(Seconds(routing_end[24]));      // +1
        }

        /**
         * data
         */
        {
//            uint32_t packetSize = 1000; // odd number:1000, even number:500
            uint32_t maxPacketCount = 10000000;
            uint32_t packetFrequency = 20;
            Time interPacketInterval = Seconds((1 / (double) packetFrequency) /*1/((double)20)*/);

            UdpServerHelper echoServerv(44);  // (+1)
            ApplicationContainer serverAppsv = echoServerv.Install(TTNTNode.Get(47)); // (+2)
            serverAppsv.Start(Seconds(data_start[24]));                                  // +1
            serverAppsv.Stop(Seconds(data_end[24]));                                // +1

            UdpClientHelper echoClient(ttntInterface.GetAddress(47), 44); // (+2, +1)
            echoClient.SetAttribute("MaxPackets", UintegerValue(maxPacketCount));
            echoClient.SetAttribute("Interval", TimeValue(interPacketInterval));
            echoClient.SetAttribute("PacketSize", UintegerValue(packet_size[24]));

            ApplicationContainer clientApps = echoClient.Install(TTNTNode.Get(44)); // +2
            clientApps.Start(Seconds(data_start[24]));                                // +1
            clientApps.Stop(Seconds(data_end[24]));                                    // +1
        }
    }


    /** workflow: 25: 48->51 */
    if (workflow[25]) {
        /**
         * route
         */
        {
            uint32_t packetSizev = 1;
            uint32_t maxPacketCountv = 10000000;
            uint32_t packetFrequencyv = 1;
            Time interPacketIntervalv = Seconds((1 / (double) packetFrequencyv));

            UdpServerHelper echoServerv(135);  // port (+1)
            ApplicationContainer serv1erAppsv = echoServerv.Install(TTNTNode.Get(51)); // node (+2)
            serv1erAppsv.Start(Seconds(routing_start[25]));                               // (+1)
            serv1erAppsv.Stop(Seconds(routing_end[25]));                            // (+1)

            UdpClientHelper echoClient(ttntInterface.GetAddress(51), 135); // node + port (+2. +1)
            echoClient.SetAttribute("MaxPackets", UintegerValue(maxPacketCountv));
            echoClient.SetAttribute("Interval", TimeValue(interPacketIntervalv));
            echoClient.SetAttribute("PacketSize", UintegerValue(packetSizev));

            ApplicationContainer clientAppsv = echoClient.Install(
                    TTNTNode.Get(48));                          // node (+2)
            clientAppsv.Start(Seconds(routing_start[25])); // +1
            clientAppsv.Stop(Seconds(routing_end[25]));      // +1
        }

        /**
         * data
         */
        {
//            uint32_t packetSize = 1000; // odd number:1000, even number:500
            uint32_t maxPacketCount = 10000000;
            uint32_t packetFrequency = 20;
            Time interPacketInterval = Seconds((1 / (double) packetFrequency) /*1/((double)20)*/);

            UdpServerHelper echoServerv(45);  // (+1)
            ApplicationContainer serverAppsv = echoServerv.Install(TTNTNode.Get(51)); // (+2)
            serverAppsv.Start(Seconds(data_start[25]));                                  // +1
            serverAppsv.Stop(Seconds(data_end[25]));                                // +1

            UdpClientHelper echoClient(ttntInterface.GetAddress(51), 45); // (+2, +1)
            echoClient.SetAttribute("MaxPackets", UintegerValue(maxPacketCount));
            echoClient.SetAttribute("Interval", TimeValue(interPacketInterval));
            echoClient.SetAttribute("PacketSize", UintegerValue(packet_size[25]));

            ApplicationContainer clientApps = echoClient.Install(TTNTNode.Get(48)); // +2
            clientApps.Start(Seconds(data_start[25]));                                // +1
            clientApps.Stop(Seconds(data_end[25]));                                    // +1
        }
    }


    /** workflow: 26: 49->52 */
    if (workflow[26]) {
        /**
         * route
         */
        {
            uint32_t packetSizev = 1;
            uint32_t maxPacketCountv = 10000000;
            uint32_t packetFrequencyv = 1;
            Time interPacketIntervalv = Seconds((1 / (double) packetFrequencyv));

            UdpServerHelper echoServerv(136);  // port (+1)
            ApplicationContainer serv1erAppsv = echoServerv.Install(TTNTNode.Get(52)); // node (+2)
            serv1erAppsv.Start(Seconds(routing_start[26]));                               // (+1)
            serv1erAppsv.Stop(Seconds(routing_end[26]));                            // (+1)

            UdpClientHelper echoClient(ttntInterface.GetAddress(52), 136); // node + port (+2. +1)
            echoClient.SetAttribute("MaxPackets", UintegerValue(maxPacketCountv));
            echoClient.SetAttribute("Interval", TimeValue(interPacketIntervalv));
            echoClient.SetAttribute("PacketSize", UintegerValue(packetSizev));

            ApplicationContainer clientAppsv = echoClient.Install(
                    TTNTNode.Get(49));                          // node (+2)
            clientAppsv.Start(Seconds(routing_start[26])); // +1
            clientAppsv.Stop(Seconds(routing_end[26]));      // +1
        }

        /**
         * data
         */
        {
//            uint32_t packetSize = 1000; // odd number:1000, even number:500
            uint32_t maxPacketCount = 10000000;
            uint32_t packetFrequency = 20;
            Time interPacketInterval = Seconds((1 / (double) packetFrequency) /*1/((double)20)*/);

            UdpServerHelper echoServerv(46);  // (+1)
            ApplicationContainer serverAppsv = echoServerv.Install(TTNTNode.Get(52)); // (+2)
            serverAppsv.Start(Seconds(data_start[26]));                                  // +1
            serverAppsv.Stop(Seconds(data_end[26]));                                // +1

            UdpClientHelper echoClient(ttntInterface.GetAddress(52), 46); // (+2, +1)
            echoClient.SetAttribute("MaxPackets", UintegerValue(maxPacketCount));
            echoClient.SetAttribute("Interval", TimeValue(interPacketInterval));
            echoClient.SetAttribute("PacketSize", UintegerValue(packet_size[26]));

            ApplicationContainer clientApps = echoClient.Install(TTNTNode.Get(49)); // +2
            clientApps.Start(Seconds(data_start[26]));                                // +1
            clientApps.Stop(Seconds(data_end[26]));                                    // +1
        }
    }


    /** workflow: 27: 50->53 */
    if (workflow[27]) {
        /**
         * route
         */
        {
            uint32_t packetSizev = 1;
            uint32_t maxPacketCountv = 10000000;
            uint32_t packetFrequencyv = 1;
            Time interPacketIntervalv = Seconds((1 / (double) packetFrequencyv));

            UdpServerHelper echoServerv(137);  // port (+1)
            ApplicationContainer serv1erAppsv = echoServerv.Install(TTNTNode.Get(53)); // node (+2)
            serv1erAppsv.Start(Seconds(routing_start[27]));                               // (+1)
            serv1erAppsv.Stop(Seconds(routing_end[27]));                            // (+1)

            UdpClientHelper echoClient(ttntInterface.GetAddress(53), 137); // node + port (+2. +1)
            echoClient.SetAttribute("MaxPackets", UintegerValue(maxPacketCountv));
            echoClient.SetAttribute("Interval", TimeValue(interPacketIntervalv));
            echoClient.SetAttribute("PacketSize", UintegerValue(packetSizev));

            ApplicationContainer clientAppsv = echoClient.Install(
                    TTNTNode.Get(50));                          // node (+2)
            clientAppsv.Start(Seconds(routing_start[27])); // +1
            clientAppsv.Stop(Seconds(routing_end[27]));      // +1
        }

        /**
         * data
         */
        {
//            uint32_t packetSize = 1000; // odd number:1000, even number:500
            uint32_t maxPacketCount = 10000000;
            uint32_t packetFrequency = 20;
            Time interPacketInterval = Seconds((1 / (double) packetFrequency) /*1/((double)20)*/);

            UdpServerHelper echoServerv(47);  // (+1)
            ApplicationContainer serverAppsv = echoServerv.Install(TTNTNode.Get(53)); // (+2)
            serverAppsv.Start(Seconds(data_start[27]));                                  // +1
            serverAppsv.Stop(Seconds(data_end[27]));                                // +1

            UdpClientHelper echoClient(ttntInterface.GetAddress(53), 47); // (+2, +1)
            echoClient.SetAttribute("MaxPackets", UintegerValue(maxPacketCount));
            echoClient.SetAttribute("Interval", TimeValue(interPacketInterval));
            echoClient.SetAttribute("PacketSize", UintegerValue(packet_size[27]));

            ApplicationContainer clientApps = echoClient.Install(TTNTNode.Get(50)); // +2
            clientApps.Start(Seconds(data_start[27]));                                // +1
            clientApps.Stop(Seconds(data_end[27]));                                    // +1
        }
    }


    /** workflow: 28: 54->57 */
    if (workflow[28]) {
        /**
         * route
         */
        {
            uint32_t packetSizev = 1;
            uint32_t maxPacketCountv = 10000000;
            uint32_t packetFrequencyv = 1;
            Time interPacketIntervalv = Seconds((1 / (double) packetFrequencyv));

            UdpServerHelper echoServerv(138);  // port (+1)
            ApplicationContainer serv1erAppsv = echoServerv.Install(TTNTNode.Get(57)); // node (+2)
            serv1erAppsv.Start(Seconds(routing_start[28]));                               // (+1)
            serv1erAppsv.Stop(Seconds(routing_end[28]));                            // (+1)

            UdpClientHelper echoClient(ttntInterface.GetAddress(57), 138); // node + port (+2. +1)
            echoClient.SetAttribute("MaxPackets", UintegerValue(maxPacketCountv));
            echoClient.SetAttribute("Interval", TimeValue(interPacketIntervalv));
            echoClient.SetAttribute("PacketSize", UintegerValue(packetSizev));

            ApplicationContainer clientAppsv = echoClient.Install(
                    TTNTNode.Get(54));                          // node (+2)
            clientAppsv.Start(Seconds(routing_start[28])); // +1
            clientAppsv.Stop(Seconds(routing_end[28]));      // +1
        }

        /**
         * data
         */
        {
//            uint32_t packetSize = 1000; // odd number:1000, even number:500
            uint32_t maxPacketCount = 10000000;
            uint32_t packetFrequency = 20;
            Time interPacketInterval = Seconds((1 / (double) packetFrequency) /*1/((double)20)*/);

            UdpServerHelper echoServerv(48);  // (+1)
            ApplicationContainer serverAppsv = echoServerv.Install(TTNTNode.Get(55)); // (+2)
            serverAppsv.Start(Seconds(data_start[28]));                                  // +1
            serverAppsv.Stop(Seconds(data_end[28]));                                // +1

            UdpClientHelper echoClient(ttntInterface.GetAddress(55), 48); // (+2, +1)
            echoClient.SetAttribute("MaxPackets", UintegerValue(maxPacketCount));
            echoClient.SetAttribute("Interval", TimeValue(interPacketInterval));
            echoClient.SetAttribute("PacketSize", UintegerValue(packet_size[28]));

            ApplicationContainer clientApps = echoClient.Install(TTNTNode.Get(52)); // +2
            clientApps.Start(Seconds(data_start[28]));                                // +1
            clientApps.Stop(Seconds(data_end[28]));                                    // +1
        }
    }


    /** workflow: 29: 55->58 */
    if (workflow[29]) {
        /**
         * route
         */
        {
            uint32_t packetSizev = 1;
            uint32_t maxPacketCountv = 10000000;
            uint32_t packetFrequencyv = 1;
            Time interPacketIntervalv = Seconds((1 / (double) packetFrequencyv));

            UdpServerHelper echoServerv(139);  // port (+1)
            ApplicationContainer serv1erAppsv = echoServerv.Install(TTNTNode.Get(58)); // node (+2)
            serv1erAppsv.Start(Seconds(routing_start[29]));                               // (+1)
            serv1erAppsv.Stop(Seconds(routing_end[29]));                            // (+1)

            UdpClientHelper echoClient(ttntInterface.GetAddress(58), 139); // node + port (+2. +1)
            echoClient.SetAttribute("MaxPackets", UintegerValue(maxPacketCountv));
            echoClient.SetAttribute("Interval", TimeValue(interPacketIntervalv));
            echoClient.SetAttribute("PacketSize", UintegerValue(packetSizev));

            ApplicationContainer clientAppsv = echoClient.Install(
                    TTNTNode.Get(55));                          // node (+2)
            clientAppsv.Start(Seconds(routing_start[29])); // +1
            clientAppsv.Stop(Seconds(routing_end[29]));      // +1
        }

        /**
         * data
         */
        {
//            uint32_t packetSize = 1000; // odd number:1000, even number:500
            uint32_t maxPacketCount = 10000000;
            uint32_t packetFrequency = 20;
            Time interPacketInterval = Seconds((1 / (double) packetFrequency) /*1/((double)20)*/);

            UdpServerHelper echoServerv(49);  // (+1)
            ApplicationContainer serverAppsv = echoServerv.Install(TTNTNode.Get(58)); // (+2)
            serverAppsv.Start(Seconds(data_start[29]));                                  // +1
            serverAppsv.Stop(Seconds(data_end[29]));                                // +1

            UdpClientHelper echoClient(ttntInterface.GetAddress(58), 49); // (+2, +1)
            echoClient.SetAttribute("MaxPackets", UintegerValue(maxPacketCount));
            echoClient.SetAttribute("Interval", TimeValue(interPacketInterval));
            echoClient.SetAttribute("PacketSize", UintegerValue(packet_size[29]));

            ApplicationContainer clientApps = echoClient.Install(TTNTNode.Get(55)); // +2
            clientApps.Start(Seconds(data_start[29]));                                // +1
            clientApps.Stop(Seconds(data_end[29]));                                    // +1
        }
    }


    /** workflow: 30: 56->59 */
    if (workflow[30]) {
        /**
         * route
         */
        {
            uint32_t packetSizev = 1;
            uint32_t maxPacketCountv = 10000000;
            uint32_t packetFrequencyv = 1;
            Time interPacketIntervalv = Seconds((1 / (double) packetFrequencyv));

            UdpServerHelper echoServerv(140);  // port (+1)
            ApplicationContainer serv1erAppsv = echoServerv.Install(TTNTNode.Get(59)); // node (+2)
            serv1erAppsv.Start(Seconds(routing_start[30]));                               // (+1)
            serv1erAppsv.Stop(Seconds(routing_end[30]));                            // (+1)

            UdpClientHelper echoClient(ttntInterface.GetAddress(59), 140); // node + port (+2. +1)
            echoClient.SetAttribute("MaxPackets", UintegerValue(maxPacketCountv));
            echoClient.SetAttribute("Interval", TimeValue(interPacketIntervalv));
            echoClient.SetAttribute("PacketSize", UintegerValue(packetSizev));

            ApplicationContainer clientAppsv = echoClient.Install(
                    TTNTNode.Get(56));                          // node (+2)
            clientAppsv.Start(Seconds(routing_start[30])); // +1
            clientAppsv.Stop(Seconds(routing_end[30]));      // +1
        }

        /**
         * data
         */
        {
//            uint32_t packetSize = 1000; // odd number:1000, even number:500
            uint32_t maxPacketCount = 10000000;
            uint32_t packetFrequency = 20;
            Time interPacketInterval = Seconds((1 / (double) packetFrequency) /*1/((double)20)*/);

            UdpServerHelper echoServerv(50);  // (+1)
            ApplicationContainer serverAppsv = echoServerv.Install(TTNTNode.Get(59)); // (+2)
            serverAppsv.Start(Seconds(data_start[30]));                                  // +1
            serverAppsv.Stop(Seconds(data_end[30]));                                // +1

            UdpClientHelper echoClient(ttntInterface.GetAddress(59), 50); // (+2, +1)
            echoClient.SetAttribute("MaxPackets", UintegerValue(maxPacketCount));
            echoClient.SetAttribute("Interval", TimeValue(interPacketInterval));
            echoClient.SetAttribute("PacketSize", UintegerValue(packet_size[30]));

            ApplicationContainer clientApps = echoClient.Install(TTNTNode.Get(56)); // +2
            clientApps.Start(Seconds(data_start[30]));                                // +1
            clientApps.Stop(Seconds(data_end[30]));                                    // +1
        }
    }


    if (0) {
        phyTTNT.EnablePcap("NodeNum", ttntDevice.Get(1));
    }

    Simulator::Stop(Seconds(simulation_time));

    AnimationInterface anim("amix.xml");

    if (0) //得出每个节点的真实位置与IP地址
    {
        map<std::string, uint32_t> mapIpv4 = anim.getIpv4AddressNodeIdTable();
        multimap<uint32_t, std::string> mapipv4 = anim.GetIpv4AddressNodeIdTable();
        map<uint32_t, Vector> mapPositon = anim.getNodePosition();

        map<std::string, Vector> NodeIPandPosition;
        map<uint32_t, map<std::string, Vector>> NodeIDandIPandPosition;

        if (mapPositon.size() == mapIpv4.size()) //判断两个map大小是否相等
        {
            for (auto i = mapIpv4.begin(); i != mapIpv4.end(); i++) {
                auto temp = mapPositon.find(i->second);
                if (temp != mapPositon.end()) {
                    NodeIPandPosition[i->first] = temp->second;
                }
            }
        }

        for (auto i = NodeIPandPosition.begin(); i != NodeIPandPosition.end(); i++) {
            auto tmp = mapIpv4.find(i->first);
            if (tmp != mapIpv4.end()) {
                NodeIDandIPandPosition[tmp->second][i->first] = i->second;
            }
        }

        for (auto iterFinal = NodeIDandIPandPosition.begin(); iterFinal != NodeIDandIPandPosition.end(); iterFinal++) {
            ofstream positionFILE("NodeIDIPPositionFile.txt", ios::app);
            auto ip = iterFinal->second.begin();

            if (positionFILE.good()) {
                positionFILE << iterFinal->first << " " << ip->first << " " << ip->second.x << " " << ip->second.y
                             << " " << endl;
                //							ip++;
                positionFILE.close();
            }
        }
    }

    Simulator::Run();

    Simulator::Destroy();
    return 0;
}
