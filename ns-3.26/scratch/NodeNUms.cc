/*
 * NodeNUms.cc
 *
 *  Created on: Jun 24, 2020
 *      Author: pinganzhang
 */


#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/wifi-sincgars-module.h" //ODD
#include "ns3/wifi-elprs-module.h" //ODD
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
#include<unistd.h>

using namespace ns3;
using namespace std;

NS_LOG_COMPONENT_DEFINE("ThirdScriptExample");

void TxCallback(Ptr <CounterCalculator<uint32_t>> datac, std::string path,
                Ptr<const Packet> packet) {
    NS_LOG_INFO("Sent frame counted in " << datac->GetKey());
    datac->Update();
    // end TxCallback
}

int main(int argc, char *argv[]) {
    string format("omnet");
//    string experiment("sincgars-stats");
//    string strategy("sincgars-default");
    string input;
    string runID;
    {
        stringstream sstr;
        sstr << "run-" << time(NULL);
        runID = sstr.str();
    }

    uint32_t ttnt = 2;

    // Set up command line parameters used to control the experiment.
    CommandLine cmd;
    cmd.AddValue("format", "Format to use for data output.", format);
//    cmd.AddValue("experiment", "Identifier for experiment.", experiment);
//    cmd.AddValue("strategy", "Identifier for strategy.", strategy);
    cmd.AddValue("run", "Identifier for run.", runID);
    cmd.Parse(argc, argv);

    if (format != "omnet" && format != "db") {
        NS_LOG_ERROR("Unknown output format '" << format << "'");
        return -1;
    }

#ifndef STATS_HAS_SQLITE3
    if (format == "db") {
        NS_LOG_ERROR("sqlite support not compiled in.");
        return -1;
    }
#endif

    Time::SetResolution(Time::NS);
    bool verbose = false;


    if (verbose) {
        LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_ALL);
        LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_ALL);
        LogComponentEnable("UdpClient", LOG_LEVEL_ALL);
        LogComponentEnable("UdpServer", LOG_LEVEL_ALL);
        LogComponentEnable("UdpTraceClient", LOG_LEVEL_ALL);
    }

    NodeContainer TTNTNode;
    TTNTNode.Create(ttnt);


//*************************************************


//****************************   TTNT   *******************************************
    sincgars::WifiHelper wifiSinc;
    sincgars::YansWifiChannelHelper channelSinc =
            sincgars::YansWifiChannelHelper::Default();   //使用默认的信道模型
    sincgars::YansWifiPhyHelper phySinc =
            sincgars::YansWifiPhyHelper::Default(); //使用默认的PHY模型
    phySinc.SetChannel(channelSinc.Create());  //创建通道对象并把他关联到物理层对象管理器
    wifiSinc.SetStandard(sincgars::WIFI_PHY_STANDARD_80211b);
    sincgars::NqosWifiMacHelper wifiMacSinc =
            sincgars::NqosWifiMacHelper::Default();
    wifiMacSinc.SetType("ns3::sinc-AdhocWifiMac");
    NetDeviceContainer ttntDevice = wifiSinc.Install(phySinc, wifiMacSinc, TTNTNode);

    MobilityHelper mobility;
    mobility.SetPositionAllocator("ns3::GridPositionAllocator", "MinX",
                                  DoubleValue(0.0), "MinY", DoubleValue(0.0), "DeltaX",
                                  DoubleValue(5.0), "DeltaY", DoubleValue(5.0), "GridWidth",
                                  UintegerValue(5), "LayoutType", StringValue("RowFirst"));
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(TTNTNode);

    InternetStackHelper stack;

    DsrHelper Dsr;
    DsrMainHelper DsrMain;
    stack.Install(TTNTNode);
    DsrMain.Install(Dsr, TTNTNode);

    Ipv4AddressHelper address;
    address.SetBase("198.3.1.0", "255.255.255.0");
    Ipv4InterfaceContainer NetTTNTDevice;
    NetTTNTDevice = address.Assign(ttntDevice);

    uint16_t workflow[26] = {0};
    for (uint32_t i = 1; i <= (ttnt / 2); i++) {
        workflow[i] = 1;
    }
    double start_time = (ttnt / 2) * 7, end_time = start_time + 50.0;

    /////////////////////1
    if (workflow[1]) {    //route

        uint32_t packetSizev = 1;
        uint32_t maxPacketCountv = 10000000;
        uint32_t packetinterval = 2;
        Time interPacketIntervalv = Seconds((1 / (double) packetinterval)/*1/((double)20)*/);

        UdpServerHelper echoServerv(111);
        ApplicationContainer serv1erAppsv = echoServerv.Install(TTNTNode.Get(1));
        serv1erAppsv.Start(Seconds(0.0));
        serv1erAppsv.Stop(Seconds(5.0));

        UdpClientHelper echoClientv(NetTTNTDevice.GetAddress(1), 111);
        echoClientv.SetAttribute("MaxPackets", UintegerValue(maxPacketCountv));
        echoClientv.SetAttribute("Interval", TimeValue(interPacketIntervalv));
        echoClientv.SetAttribute("PacketSize", UintegerValue(packetSizev));

        ApplicationContainer clientAppsv = echoClientv.Install(
                TTNTNode.Get(0));
        clientAppsv.Start(Seconds(0.0));
        clientAppsv.Stop(Seconds(5.0));

        ////////////////////////data/////////
        uint32_t packetSizev1 = 100;
        uint32_t maxPacketCountv1 = 10000000;
        uint32_t packetinterval1 = 2;
        Time interPacketIntervalv1 = Seconds((1 / (double) packetinterval1)/*1/((double)20)*/);
//			Time interPacketIntervalv1 = Seconds(1.0);

        UdpServerHelper echoServerv1(21);
        ApplicationContainer serverAppsv1 = echoServerv1.Install(TTNTNode.Get(1));
        serverAppsv1.Start(Seconds(start_time));
        serverAppsv1.Stop(Seconds(end_time));

        UdpClientHelper echoClientv1(NetTTNTDevice.GetAddress(1), 21);
        echoClientv1.SetAttribute("MaxPackets", UintegerValue(maxPacketCountv1));
        echoClientv1.SetAttribute("Interval", TimeValue(interPacketIntervalv1));
        echoClientv1.SetAttribute("PacketSize", UintegerValue(packetSizev1));

        ApplicationContainer clientAppsv1 = echoClientv1.Install(
                TTNTNode.Get(0));
        clientAppsv1.Start(Seconds(start_time));
        clientAppsv1.Stop(Seconds(end_time));
    }
    /////////////////////2
    if (workflow[2]) {    //route

        uint32_t packetSizev = 1;
        uint32_t maxPacketCountv = 10000000;
        uint32_t packetinterval = 2;
        Time interPacketIntervalv = Seconds((1 / (double) packetinterval)/*1/((double)20)*/);
//			Time interPacketIntervalv = Seconds(5.0);

        UdpServerHelper echoServerv(222);
        ApplicationContainer serv1erAppsv = echoServerv.Install(TTNTNode.Get(3));
        serv1erAppsv.Start(Seconds(7.0));
        serv1erAppsv.Stop(Seconds(12.0));

        UdpClientHelper echoClientv(NetTTNTDevice.GetAddress(3), 222);
        echoClientv.SetAttribute("MaxPackets", UintegerValue(maxPacketCountv));
        echoClientv.SetAttribute("Interval", TimeValue(interPacketIntervalv));
        echoClientv.SetAttribute("PacketSize", UintegerValue(packetSizev));

        ApplicationContainer clientAppsv = echoClientv.Install(
                TTNTNode.Get(2));
        clientAppsv.Start(Seconds(7.0));
        clientAppsv.Stop(Seconds(12.0));

        ////////////////////////data/////////
        uint32_t packetSizev1 = 1499;
        uint32_t maxPacketCountv1 = 10000000;
        uint32_t packetinterval1 = 500;
        Time interPacketIntervalv1 = Seconds((1 / (double) packetinterval1)/*1/((double)20)*/);
//			Time interPacketIntervalv1 = Seconds(1.0);

        UdpServerHelper echoServerv1(22);
        ApplicationContainer serverAppsv1 = echoServerv1.Install(TTNTNode.Get(3));
        serverAppsv1.Start(Seconds(start_time));
        serverAppsv1.Stop(Seconds(end_time));

        UdpClientHelper echoClientv1(NetTTNTDevice.GetAddress(3), 22);
        echoClientv1.SetAttribute("MaxPackets", UintegerValue(maxPacketCountv1));
        echoClientv1.SetAttribute("Interval", TimeValue(interPacketIntervalv1));
        echoClientv1.SetAttribute("PacketSize", UintegerValue(packetSizev1));

        ApplicationContainer clientAppsv1 = echoClientv1.Install(
                TTNTNode.Get(2));
        clientAppsv1.Start(Seconds(start_time));
        clientAppsv1.Stop(Seconds(end_time));
    }

    /////////////////////3
    if (workflow[3]) {    //route

        uint32_t packetSizev = 1;
        uint32_t maxPacketCountv = 10000000;
        uint32_t packetinterval = 2;
        Time interPacketIntervalv = Seconds((1 / (double) packetinterval)/*1/((double)20)*/);
//			Time interPacketIntervalv = Seconds(5.0);

        UdpServerHelper echoServerv(333);
        ApplicationContainer serv1erAppsv = echoServerv.Install(TTNTNode.Get(5));
        serv1erAppsv.Start(Seconds(14.0));
        serv1erAppsv.Stop(Seconds(19.0));

        UdpClientHelper echoClientv(NetTTNTDevice.GetAddress(5), 333);
        echoClientv.SetAttribute("MaxPackets", UintegerValue(maxPacketCountv));
        echoClientv.SetAttribute("Interval", TimeValue(interPacketIntervalv));
        echoClientv.SetAttribute("PacketSize", UintegerValue(packetSizev));

        ApplicationContainer clientAppsv = echoClientv.Install(
                TTNTNode.Get(4));
        clientAppsv.Start(Seconds(14.0));
        clientAppsv.Stop(Seconds(19.0));

        ////////////////////////data/////////
        uint32_t packetSizev1 = 1499;
        uint32_t maxPacketCountv1 = 10000000;
        uint32_t packetinterval1 = 500;
        Time interPacketIntervalv1 = Seconds((1 / (double) packetinterval1)/*1/((double)20)*/);
//			Time interPacketIntervalv1 = Seconds(1.0);

        UdpServerHelper echoServerv1(23);
        ApplicationContainer serverAppsv1 = echoServerv1.Install(TTNTNode.Get(5));
        serverAppsv1.Start(Seconds(start_time));
        serverAppsv1.Stop(Seconds(end_time));

        UdpClientHelper echoClientv1(NetTTNTDevice.GetAddress(5), 23);
        echoClientv1.SetAttribute("MaxPackets", UintegerValue(maxPacketCountv1));
        echoClientv1.SetAttribute("Interval", TimeValue(interPacketIntervalv1));
        echoClientv1.SetAttribute("PacketSize", UintegerValue(packetSizev1));

        ApplicationContainer clientAppsv1 = echoClientv1.Install(
                TTNTNode.Get(4));
        clientAppsv1.Start(Seconds(start_time));
        clientAppsv1.Stop(Seconds(end_time));
    }

    /////////////////////4
    if (workflow[4]) {    //route

        uint32_t packetSizev = 1;
        uint32_t maxPacketCountv = 10000000;
        uint32_t packetinterval = 2;
        Time interPacketIntervalv = Seconds((1 / (double) packetinterval)/*1/((double)20)*/);
//			Time interPacketIntervalv = Seconds(5.0);

        UdpServerHelper echoServerv(444);
        ApplicationContainer serv1erAppsv = echoServerv.Install(TTNTNode.Get(7));
        serv1erAppsv.Start(Seconds(21.0));
        serv1erAppsv.Stop(Seconds(26.0));

        UdpClientHelper echoClientv(NetTTNTDevice.GetAddress(7), 444);
        echoClientv.SetAttribute("MaxPackets", UintegerValue(maxPacketCountv));
        echoClientv.SetAttribute("Interval", TimeValue(interPacketIntervalv));
        echoClientv.SetAttribute("PacketSize", UintegerValue(packetSizev));

        ApplicationContainer clientAppsv = echoClientv.Install(
                TTNTNode.Get(6));
        clientAppsv.Start(Seconds(21.0));
        clientAppsv.Stop(Seconds(26.0));

        ////////////////////////data/////////
        uint32_t packetSizev1 = 1499;
        uint32_t maxPacketCountv1 = 10000000;
        uint32_t packetinterval1 = 500;
        Time interPacketIntervalv1 = Seconds((1 / (double) packetinterval1)/*1/((double)20)*/);
//			Time interPacketIntervalv1 = Seconds(1.0);

        UdpServerHelper echoServerv1(24);
        ApplicationContainer serverAppsv1 = echoServerv1.Install(TTNTNode.Get(7));
        serverAppsv1.Start(Seconds(start_time));
        serverAppsv1.Stop(Seconds(end_time));

        UdpClientHelper echoClientv1(NetTTNTDevice.GetAddress(7), 24);
        echoClientv1.SetAttribute("MaxPackets", UintegerValue(maxPacketCountv1));
        echoClientv1.SetAttribute("Interval", TimeValue(interPacketIntervalv1));
        echoClientv1.SetAttribute("PacketSize", UintegerValue(packetSizev1));

        ApplicationContainer clientAppsv1 = echoClientv1.Install(
                TTNTNode.Get(6));
        clientAppsv1.Start(Seconds(start_time));
        clientAppsv1.Stop(Seconds(end_time));
    }

    /////////////////////5
    if (workflow[5]) {    //route

        uint32_t packetSizev = 1;
        uint32_t maxPacketCountv = 10000000;
        uint32_t packetinterval = 2;
        Time interPacketIntervalv = Seconds((1 / (double) packetinterval)/*1/((double)20)*/);
//			Time interPacketIntervalv = Seconds(5.0);

        UdpServerHelper echoServerv(555);
        ApplicationContainer serv1erAppsv = echoServerv.Install(TTNTNode.Get(9));
        serv1erAppsv.Start(Seconds(28.0));
        serv1erAppsv.Stop(Seconds(33.0));

        UdpClientHelper echoClientv(NetTTNTDevice.GetAddress(9), 555);
        echoClientv.SetAttribute("MaxPackets", UintegerValue(maxPacketCountv));
        echoClientv.SetAttribute("Interval", TimeValue(interPacketIntervalv));
        echoClientv.SetAttribute("PacketSize", UintegerValue(packetSizev));

        ApplicationContainer clientAppsv = echoClientv.Install(
                TTNTNode.Get(8));
        clientAppsv.Start(Seconds(28.0));
        clientAppsv.Stop(Seconds(33.0));

        ////////////////////////data/////////
        uint32_t packetSizev1 = 1499;
        uint32_t maxPacketCountv1 = 10000000;
        uint32_t packetinterval1 = 500;
        Time interPacketIntervalv1 = Seconds((1 / (double) packetinterval1)/*1/((double)20)*/);
//			Time interPacketIntervalv1 = Seconds(1.0);

        UdpServerHelper echoServerv1(25);
        ApplicationContainer serverAppsv1 = echoServerv1.Install(TTNTNode.Get(9));
        serverAppsv1.Start(Seconds(start_time));
        serverAppsv1.Stop(Seconds(end_time));

        UdpClientHelper echoClientv1(NetTTNTDevice.GetAddress(9), 25);
        echoClientv1.SetAttribute("MaxPackets", UintegerValue(maxPacketCountv1));
        echoClientv1.SetAttribute("Interval", TimeValue(interPacketIntervalv1));
        echoClientv1.SetAttribute("PacketSize", UintegerValue(packetSizev1));

        ApplicationContainer clientAppsv1 = echoClientv1.Install(
                TTNTNode.Get(8));
        clientAppsv1.Start(Seconds(start_time));
        clientAppsv1.Stop(Seconds(end_time));
    }
    /////////////////////6
    if (workflow[6]) {    //route

        uint32_t packetSizev = 1;
        uint32_t maxPacketCountv = 10000000;
        uint32_t packetinterval = 2;
        Time interPacketIntervalv = Seconds((1 / (double) packetinterval)/*1/((double)20)*/);
//			Time interPacketIntervalv = Seconds(5.0);

        UdpServerHelper echoServerv(666);
        ApplicationContainer serv1erAppsv = echoServerv.Install(TTNTNode.Get(11));
        serv1erAppsv.Start(Seconds(35.0));
        serv1erAppsv.Stop(Seconds(40.0));

        UdpClientHelper echoClientv(NetTTNTDevice.GetAddress(11), 666);
        echoClientv.SetAttribute("MaxPackets", UintegerValue(maxPacketCountv));
        echoClientv.SetAttribute("Interval", TimeValue(interPacketIntervalv));
        echoClientv.SetAttribute("PacketSize", UintegerValue(packetSizev));

        ApplicationContainer clientAppsv = echoClientv.Install(
                TTNTNode.Get(10));
        clientAppsv.Start(Seconds(35.0));
        clientAppsv.Stop(Seconds(40.0));

        ////////////////////////data/////////
        uint32_t packetSizev1 = 1499;
        uint32_t maxPacketCountv1 = 10000000;
        uint32_t packetinterval1 = 500;
        Time interPacketIntervalv1 = Seconds((1 / (double) packetinterval1)/*1/((double)20)*/);
//			Time interPacketIntervalv1 = Seconds(1.0);

        UdpServerHelper echoServerv1(26);
        ApplicationContainer serverAppsv1 = echoServerv1.Install(TTNTNode.Get(11));
        serverAppsv1.Start(Seconds(start_time));
        serverAppsv1.Stop(Seconds(end_time));

        UdpClientHelper echoClientv1(NetTTNTDevice.GetAddress(11), 26);
        echoClientv1.SetAttribute("MaxPackets", UintegerValue(maxPacketCountv1));
        echoClientv1.SetAttribute("Interval", TimeValue(interPacketIntervalv1));
        echoClientv1.SetAttribute("PacketSize", UintegerValue(packetSizev1));

        ApplicationContainer clientAppsv1 = echoClientv1.Install(
                TTNTNode.Get(10));
        clientAppsv1.Start(Seconds(start_time));
        clientAppsv1.Stop(Seconds(end_time));
    }

    /////////////////////7
    if (workflow[7]) {    //route

        uint32_t packetSizev = 1;
        uint32_t maxPacketCountv = 10000000;
        uint32_t packetinterval = 2;
        Time interPacketIntervalv = Seconds((1 / (double) packetinterval)/*1/((double)20)*/);
//			Time interPacketIntervalv = Seconds(5.0);

        UdpServerHelper echoServerv(777);
        ApplicationContainer serv1erAppsv = echoServerv.Install(TTNTNode.Get(13));
        serv1erAppsv.Start(Seconds(42.0));
        serv1erAppsv.Stop(Seconds(47.0));

        UdpClientHelper echoClientv(NetTTNTDevice.GetAddress(13), 777);
        echoClientv.SetAttribute("MaxPackets", UintegerValue(maxPacketCountv));
        echoClientv.SetAttribute("Interval", TimeValue(interPacketIntervalv));
        echoClientv.SetAttribute("PacketSize", UintegerValue(packetSizev));

        ApplicationContainer clientAppsv = echoClientv.Install(
                TTNTNode.Get(12));
        clientAppsv.Start(Seconds(42.0));
        clientAppsv.Stop(Seconds(47.0));

        ////////////////////////data/////////
        uint32_t packetSizev1 = 1499;
        uint32_t maxPacketCountv1 = 10000000;
        uint32_t packetinterval1 = 500;
        Time interPacketIntervalv1 = Seconds((1 / (double) packetinterval1)/*1/((double)20)*/);
//			Time interPacketIntervalv1 = Seconds(1.0);

        UdpServerHelper echoServerv1(27);
        ApplicationContainer serverAppsv1 = echoServerv1.Install(TTNTNode.Get(13));
        serverAppsv1.Start(Seconds(start_time));
        serverAppsv1.Stop(Seconds(end_time));

        UdpClientHelper echoClientv1(NetTTNTDevice.GetAddress(13), 27);
        echoClientv1.SetAttribute("MaxPackets", UintegerValue(maxPacketCountv1));
        echoClientv1.SetAttribute("Interval", TimeValue(interPacketIntervalv1));
        echoClientv1.SetAttribute("PacketSize", UintegerValue(packetSizev1));

        ApplicationContainer clientAppsv1 = echoClientv1.Install(
                TTNTNode.Get(12));
        clientAppsv1.Start(Seconds(start_time));
        clientAppsv1.Stop(Seconds(end_time));
    }

    /////////////////////8
    if (workflow[8]) {    //route

        uint32_t packetSizev = 1;
        uint32_t maxPacketCountv = 10000000;
        uint32_t packetinterval = 2;
        Time interPacketIntervalv = Seconds((1 / (double) packetinterval)/*1/((double)20)*/);
//			Time interPacketIntervalv = Seconds(5.0);

        UdpServerHelper echoServerv(888);
        ApplicationContainer serv1erAppsv = echoServerv.Install(TTNTNode.Get(15));
        serv1erAppsv.Start(Seconds(49.0));
        serv1erAppsv.Stop(Seconds(54.0));

        UdpClientHelper echoClientv(NetTTNTDevice.GetAddress(15), 888);
        echoClientv.SetAttribute("MaxPackets", UintegerValue(maxPacketCountv));
        echoClientv.SetAttribute("Interval", TimeValue(interPacketIntervalv));
        echoClientv.SetAttribute("PacketSize", UintegerValue(packetSizev));

        ApplicationContainer clientAppsv = echoClientv.Install(
                TTNTNode.Get(14));
        clientAppsv.Start(Seconds(49.0));
        clientAppsv.Stop(Seconds(54.0));

        ////////////////////////data/////////
        uint32_t packetSizev1 = 1499;
        uint32_t maxPacketCountv1 = 10000000;
        uint32_t packetinterval1 = 500;
        Time interPacketIntervalv1 = Seconds((1 / (double) packetinterval1)/*1/((double)20)*/);
//			Time interPacketIntervalv1 = Seconds(1.0);

        UdpServerHelper echoServerv1(28);
        ApplicationContainer serverAppsv1 = echoServerv1.Install(TTNTNode.Get(15));
        serverAppsv1.Start(Seconds(start_time));
        serverAppsv1.Stop(Seconds(end_time));

        UdpClientHelper echoClientv1(NetTTNTDevice.GetAddress(15), 28);
        echoClientv1.SetAttribute("MaxPackets", UintegerValue(maxPacketCountv1));
        echoClientv1.SetAttribute("Interval", TimeValue(interPacketIntervalv1));
        echoClientv1.SetAttribute("PacketSize", UintegerValue(packetSizev1));

        ApplicationContainer clientAppsv1 = echoClientv1.Install(
                TTNTNode.Get(14));
        clientAppsv1.Start(Seconds(start_time));
        clientAppsv1.Stop(Seconds(end_time));
    }

    /////////////////////9
    if (workflow[9]) {    //route

        uint32_t packetSizev = 1;
        uint32_t maxPacketCountv = 10000000;
        uint32_t packetinterval = 2;
        Time interPacketIntervalv = Seconds((1 / (double) packetinterval)/*1/((double)20)*/);
//			Time interPacketIntervalv = Seconds(5.0);

        UdpServerHelper echoServerv(999);
        ApplicationContainer serv1erAppsv = echoServerv.Install(TTNTNode.Get(17));
        serv1erAppsv.Start(Seconds(56.0));
        serv1erAppsv.Stop(Seconds(61.0));

        UdpClientHelper echoClientv(NetTTNTDevice.GetAddress(17), 999);
        echoClientv.SetAttribute("MaxPackets", UintegerValue(maxPacketCountv));
        echoClientv.SetAttribute("Interval", TimeValue(interPacketIntervalv));
        echoClientv.SetAttribute("PacketSize", UintegerValue(packetSizev));

        ApplicationContainer clientAppsv = echoClientv.Install(
                TTNTNode.Get(16));
        clientAppsv.Start(Seconds(56.0));
        clientAppsv.Stop(Seconds(61.0));

        ////////////////////////data/////////
        uint32_t packetSizev1 = 1499;
        uint32_t maxPacketCountv1 = 10000000;
        uint32_t packetinterval1 = 500;
        Time interPacketIntervalv1 = Seconds((1 / (double) packetinterval1)/*1/((double)20)*/);
//			Time interPacketIntervalv1 = Seconds(1.0);

        UdpServerHelper echoServerv1(29);
        ApplicationContainer serverAppsv1 = echoServerv1.Install(TTNTNode.Get(17));
        serverAppsv1.Start(Seconds(start_time));
        serverAppsv1.Stop(Seconds(end_time));

        UdpClientHelper echoClientv1(NetTTNTDevice.GetAddress(17), 29);
        echoClientv1.SetAttribute("MaxPackets", UintegerValue(maxPacketCountv1));
        echoClientv1.SetAttribute("Interval", TimeValue(interPacketIntervalv1));
        echoClientv1.SetAttribute("PacketSize", UintegerValue(packetSizev1));

        ApplicationContainer clientAppsv1 = echoClientv1.Install(
                TTNTNode.Get(16));
        clientAppsv1.Start(Seconds(start_time));
        clientAppsv1.Stop(Seconds(end_time));
    }

    /////////////////////10
    if (workflow[10]) {    //route

        uint32_t packetSizev = 1;
        uint32_t maxPacketCountv = 10000000;
        uint32_t packetinterval = 2;
        Time interPacketIntervalv = Seconds((1 / (double) packetinterval)/*1/((double)20)*/);
//			Time interPacketIntervalv = Seconds(5.0);

        UdpServerHelper echoServerv(1110);
        ApplicationContainer serv1erAppsv = echoServerv.Install(TTNTNode.Get(19));
        serv1erAppsv.Start(Seconds(63.0));
        serv1erAppsv.Stop(Seconds(68.0));

        UdpClientHelper echoClientv(NetTTNTDevice.GetAddress(19), 1110);
        echoClientv.SetAttribute("MaxPackets", UintegerValue(maxPacketCountv));
        echoClientv.SetAttribute("Interval", TimeValue(interPacketIntervalv));
        echoClientv.SetAttribute("PacketSize", UintegerValue(packetSizev));

        ApplicationContainer clientAppsv = echoClientv.Install(
                TTNTNode.Get(18));
        clientAppsv.Start(Seconds(63.0));
        clientAppsv.Stop(Seconds(68.0));

        ////////////////////////data/////////
        uint32_t packetSizev1 = 1499;
        uint32_t maxPacketCountv1 = 10000000;
        uint32_t packetinterval1 = 500;
        Time interPacketIntervalv1 = Seconds((1 / (double) packetinterval1)/*1/((double)20)*/);
//			Time interPacketIntervalv1 = Seconds(1.0);

        UdpServerHelper echoServerv1(30);
        ApplicationContainer serverAppsv1 = echoServerv1.Install(TTNTNode.Get(19));
        serverAppsv1.Start(Seconds(start_time));
        serverAppsv1.Stop(Seconds(end_time));

        UdpClientHelper echoClientv1(NetTTNTDevice.GetAddress(19), 30);
        echoClientv1.SetAttribute("MaxPackets", UintegerValue(maxPacketCountv1));
        echoClientv1.SetAttribute("Interval", TimeValue(interPacketIntervalv1));
        echoClientv1.SetAttribute("PacketSize", UintegerValue(packetSizev1));

        ApplicationContainer clientAppsv1 = echoClientv1.Install(
                TTNTNode.Get(18));
        clientAppsv1.Start(Seconds(start_time));
        clientAppsv1.Stop(Seconds(end_time));
    }

    /////////////////////11
    if (workflow[11]) {    //route

        uint32_t packetSizev = 1;
        uint32_t maxPacketCountv = 10000000;
        uint32_t packetinterval = 2;
        Time interPacketIntervalv = Seconds((1 / (double) packetinterval)/*1/((double)20)*/);
//			Time interPacketIntervalv = Seconds(5.0);

        UdpServerHelper echoServerv(1221);
        ApplicationContainer serv1erAppsv = echoServerv.Install(TTNTNode.Get(21));
        serv1erAppsv.Start(Seconds(70.0));
        serv1erAppsv.Stop(Seconds(75.0));

        UdpClientHelper echoClientv(NetTTNTDevice.GetAddress(21), 1221);
        echoClientv.SetAttribute("MaxPackets", UintegerValue(maxPacketCountv));
        echoClientv.SetAttribute("Interval", TimeValue(interPacketIntervalv));
        echoClientv.SetAttribute("PacketSize", UintegerValue(packetSizev));

        ApplicationContainer clientAppsv = echoClientv.Install(
                TTNTNode.Get(20));
        clientAppsv.Start(Seconds(70.0));
        clientAppsv.Stop(Seconds(75.0));

        ////////////////////////data/////////
        uint32_t packetSizev1 = 1499;
        uint32_t maxPacketCountv1 = 10000000;
        uint32_t packetinterval1 = 500;
        Time interPacketIntervalv1 = Seconds((1 / (double) packetinterval1)/*1/((double)20)*/);
//			Time interPacketIntervalv1 = Seconds(1.0);

        UdpServerHelper echoServerv1(31);
        ApplicationContainer serverAppsv1 = echoServerv1.Install(TTNTNode.Get(21));
        serverAppsv1.Start(Seconds(start_time));
        serverAppsv1.Stop(Seconds(end_time));

        UdpClientHelper echoClientv1(NetTTNTDevice.GetAddress(21), 31);
        echoClientv1.SetAttribute("MaxPackets", UintegerValue(maxPacketCountv1));
        echoClientv1.SetAttribute("Interval", TimeValue(interPacketIntervalv1));
        echoClientv1.SetAttribute("PacketSize", UintegerValue(packetSizev1));

        ApplicationContainer clientAppsv1 = echoClientv1.Install(
                TTNTNode.Get(20));
        clientAppsv1.Start(Seconds(start_time));
        clientAppsv1.Stop(Seconds(end_time));
    }

    /////////////////////12
    if (workflow[12]) {    //route

        uint32_t packetSizev = 1;
        uint32_t maxPacketCountv = 10000000;
        uint32_t packetinterval = 2;
        Time interPacketIntervalv = Seconds((1 / (double) packetinterval)/*1/((double)20)*/);
//			Time interPacketIntervalv = Seconds(5.0);

        UdpServerHelper echoServerv(1332);
        ApplicationContainer serv1erAppsv = echoServerv.Install(TTNTNode.Get(23));
        serv1erAppsv.Start(Seconds(77.0));
        serv1erAppsv.Stop(Seconds(82.0));

        UdpClientHelper echoClientv(NetTTNTDevice.GetAddress(23), 1332);
        echoClientv.SetAttribute("MaxPackets", UintegerValue(maxPacketCountv));
        echoClientv.SetAttribute("Interval", TimeValue(interPacketIntervalv));
        echoClientv.SetAttribute("PacketSize", UintegerValue(packetSizev));

        ApplicationContainer clientAppsv = echoClientv.Install(
                TTNTNode.Get(22));
        clientAppsv.Start(Seconds(77.0));
        clientAppsv.Stop(Seconds(82.0));

        ////////////////////////data/////////
        uint32_t packetSizev1 = 1499;
        uint32_t maxPacketCountv1 = 10000000;
        uint32_t packetinterval1 = 500;
        Time interPacketIntervalv1 = Seconds((1 / (double) packetinterval1)/*1/((double)20)*/);
//			Time interPacketIntervalv1 = Seconds(1.0);

        UdpServerHelper echoServerv1(32);
        ApplicationContainer serverAppsv1 = echoServerv1.Install(TTNTNode.Get(23));
        serverAppsv1.Start(Seconds(start_time));
        serverAppsv1.Stop(Seconds(end_time));

        UdpClientHelper echoClientv1(NetTTNTDevice.GetAddress(23), 32);
        echoClientv1.SetAttribute("MaxPackets", UintegerValue(maxPacketCountv1));
        echoClientv1.SetAttribute("Interval", TimeValue(interPacketIntervalv1));
        echoClientv1.SetAttribute("PacketSize", UintegerValue(packetSizev1));

        ApplicationContainer clientAppsv1 = echoClientv1.Install(
                TTNTNode.Get(22));
        clientAppsv1.Start(Seconds(start_time));
        clientAppsv1.Stop(Seconds(end_time));
    }

    /////////////////////13
    if (workflow[13]) {    //route

        uint32_t packetSizev = 1;
        uint32_t maxPacketCountv = 10000000;
        uint32_t packetinterval = 2;
        Time interPacketIntervalv = Seconds((1 / (double) packetinterval)/*1/((double)20)*/);
//			Time interPacketIntervalv = Seconds(5.0);

        UdpServerHelper echoServerv(1443);
        ApplicationContainer serv1erAppsv = echoServerv.Install(TTNTNode.Get(25));
        serv1erAppsv.Start(Seconds(84.0));
        serv1erAppsv.Stop(Seconds(89.0));

        UdpClientHelper echoClientv(NetTTNTDevice.GetAddress(25), 1443);
        echoClientv.SetAttribute("MaxPackets", UintegerValue(maxPacketCountv));
        echoClientv.SetAttribute("Interval", TimeValue(interPacketIntervalv));
        echoClientv.SetAttribute("PacketSize", UintegerValue(packetSizev));

        ApplicationContainer clientAppsv = echoClientv.Install(
                TTNTNode.Get(24));
        clientAppsv.Start(Seconds(84.0));
        clientAppsv.Stop(Seconds(89.0));

        ////////////////////////data/////////
        uint32_t packetSizev1 = 1499;
        uint32_t maxPacketCountv1 = 10000000;
        uint32_t packetinterval1 = 500;
        Time interPacketIntervalv1 = Seconds((1 / (double) packetinterval1)/*1/((double)20)*/);
//			Time interPacketIntervalv1 = Seconds(1.0);

        UdpServerHelper echoServerv1(33);
        ApplicationContainer serverAppsv1 = echoServerv1.Install(TTNTNode.Get(25));
        serverAppsv1.Start(Seconds(start_time));
        serverAppsv1.Stop(Seconds(end_time));

        UdpClientHelper echoClientv1(NetTTNTDevice.GetAddress(25), 33);
        echoClientv1.SetAttribute("MaxPackets", UintegerValue(maxPacketCountv1));
        echoClientv1.SetAttribute("Interval", TimeValue(interPacketIntervalv1));
        echoClientv1.SetAttribute("PacketSize", UintegerValue(packetSizev1));

        ApplicationContainer clientAppsv1 = echoClientv1.Install(
                TTNTNode.Get(24));
        clientAppsv1.Start(Seconds(start_time));
        clientAppsv1.Stop(Seconds(end_time));
    }

    /////////////////////14
    if (workflow[14]) {    //route

        uint32_t packetSizev = 1;
        uint32_t maxPacketCountv = 10000000;
        uint32_t packetinterval = 2;
        Time interPacketIntervalv = Seconds((1 / (double) packetinterval)/*1/((double)20)*/);
//			Time interPacketIntervalv = Seconds(5.0);

        UdpServerHelper echoServerv(1554);
        ApplicationContainer serv1erAppsv = echoServerv.Install(TTNTNode.Get(27));
        serv1erAppsv.Start(Seconds(91.0));
        serv1erAppsv.Stop(Seconds(96.0));

        UdpClientHelper echoClientv(NetTTNTDevice.GetAddress(27), 1554);
        echoClientv.SetAttribute("MaxPackets", UintegerValue(maxPacketCountv));
        echoClientv.SetAttribute("Interval", TimeValue(interPacketIntervalv));
        echoClientv.SetAttribute("PacketSize", UintegerValue(packetSizev));

        ApplicationContainer clientAppsv = echoClientv.Install(
                TTNTNode.Get(26));
        clientAppsv.Start(Seconds(91.0));
        clientAppsv.Stop(Seconds(96.0));

        ////////////////////////data/////////
        uint32_t packetSizev1 = 1499;
        uint32_t maxPacketCountv1 = 10000000;
        uint32_t packetinterval1 = 500;
        Time interPacketIntervalv1 = Seconds((1 / (double) packetinterval1)/*1/((double)20)*/);
//			Time interPacketIntervalv1 = Seconds(1.0);

        UdpServerHelper echoServerv1(34);
        ApplicationContainer serverAppsv1 = echoServerv1.Install(TTNTNode.Get(27));
        serverAppsv1.Start(Seconds(start_time));
        serverAppsv1.Stop(Seconds(end_time));

        UdpClientHelper echoClientv1(NetTTNTDevice.GetAddress(27), 34);
        echoClientv1.SetAttribute("MaxPackets", UintegerValue(maxPacketCountv1));
        echoClientv1.SetAttribute("Interval", TimeValue(interPacketIntervalv1));
        echoClientv1.SetAttribute("PacketSize", UintegerValue(packetSizev1));

        ApplicationContainer clientAppsv1 = echoClientv1.Install(
                TTNTNode.Get(26));
        clientAppsv1.Start(Seconds(start_time));
        clientAppsv1.Stop(Seconds(end_time));
    }

    /////////////////////15
    if (workflow[15]) {    //route

        uint32_t packetSizev = 1;
        uint32_t maxPacketCountv = 10000000;
        uint32_t packetinterval = 2;
        Time interPacketIntervalv = Seconds((1 / (double) packetinterval)/*1/((double)20)*/);
//			Time interPacketIntervalv = Seconds(5.0);

        UdpServerHelper echoServerv(1665);
        ApplicationContainer serv1erAppsv = echoServerv.Install(TTNTNode.Get(29));
        serv1erAppsv.Start(Seconds(98.0));
        serv1erAppsv.Stop(Seconds(103.0));

        UdpClientHelper echoClientv(NetTTNTDevice.GetAddress(29), 1665);
        echoClientv.SetAttribute("MaxPackets", UintegerValue(maxPacketCountv));
        echoClientv.SetAttribute("Interval", TimeValue(interPacketIntervalv));
        echoClientv.SetAttribute("PacketSize", UintegerValue(packetSizev));

        ApplicationContainer clientAppsv = echoClientv.Install(
                TTNTNode.Get(28));
        clientAppsv.Start(Seconds(98.0));
        clientAppsv.Stop(Seconds(103.0));

        ////////////////////////data/////////
        uint32_t packetSizev1 = 1499;
        uint32_t maxPacketCountv1 = 10000000;
        uint32_t packetinterval1 = 500;
        Time interPacketIntervalv1 = Seconds((1 / (double) packetinterval1)/*1/((double)20)*/);
//			Time interPacketIntervalv1 = Seconds(1.0);

        UdpServerHelper echoServerv1(35);
        ApplicationContainer serverAppsv1 = echoServerv1.Install(TTNTNode.Get(29));
        serverAppsv1.Start(Seconds(start_time));
        serverAppsv1.Stop(Seconds(end_time));

        UdpClientHelper echoClientv1(NetTTNTDevice.GetAddress(29), 35);
        echoClientv1.SetAttribute("MaxPackets", UintegerValue(maxPacketCountv1));
        echoClientv1.SetAttribute("Interval", TimeValue(interPacketIntervalv1));
        echoClientv1.SetAttribute("PacketSize", UintegerValue(packetSizev1));

        ApplicationContainer clientAppsv1 = echoClientv1.Install(
                TTNTNode.Get(28));
        clientAppsv1.Start(Seconds(start_time));
        clientAppsv1.Stop(Seconds(end_time));
    }

    /////////////////////16
    if (workflow[16]) {    //route

        uint32_t packetSizev = 1;
        uint32_t maxPacketCountv = 10000000;
        uint32_t packetinterval = 2;
        Time interPacketIntervalv = Seconds((1 / (double) packetinterval)/*1/((double)20)*/);
//			Time interPacketIntervalv = Seconds(5.0);

        UdpServerHelper echoServerv(1776);
        ApplicationContainer serv1erAppsv = echoServerv.Install(TTNTNode.Get(31));
        serv1erAppsv.Start(Seconds(105.0));
        serv1erAppsv.Stop(Seconds(110.0));

        UdpClientHelper echoClientv(NetTTNTDevice.GetAddress(31), 1776);
        echoClientv.SetAttribute("MaxPackets", UintegerValue(maxPacketCountv));
        echoClientv.SetAttribute("Interval", TimeValue(interPacketIntervalv));
        echoClientv.SetAttribute("PacketSize", UintegerValue(packetSizev));

        ApplicationContainer clientAppsv = echoClientv.Install(
                TTNTNode.Get(30));
        clientAppsv.Start(Seconds(105.0));
        clientAppsv.Stop(Seconds(110.0));

        ////////////////////////data/////////
        uint32_t packetSizev1 = 1499;
        uint32_t maxPacketCountv1 = 10000000;
        uint32_t packetinterval1 = 500;
        Time interPacketIntervalv1 = Seconds((1 / (double) packetinterval1)/*1/((double)20)*/);
//			Time interPacketIntervalv1 = Seconds(1.0);

        UdpServerHelper echoServerv1(36);
        ApplicationContainer serverAppsv1 = echoServerv1.Install(TTNTNode.Get(31));
        serverAppsv1.Start(Seconds(start_time));
        serverAppsv1.Stop(Seconds(end_time));

        UdpClientHelper echoClientv1(NetTTNTDevice.GetAddress(31), 36);
        echoClientv1.SetAttribute("MaxPackets", UintegerValue(maxPacketCountv1));
        echoClientv1.SetAttribute("Interval", TimeValue(interPacketIntervalv1));
        echoClientv1.SetAttribute("PacketSize", UintegerValue(packetSizev1));

        ApplicationContainer clientAppsv1 = echoClientv1.Install(
                TTNTNode.Get(30));
        clientAppsv1.Start(Seconds(start_time));
        clientAppsv1.Stop(Seconds(end_time));
    }

    /////////////////////17
    if (workflow[17]) {    //route

        uint32_t packetSizev = 1;
        uint32_t maxPacketCountv = 10000000;
        uint32_t packetinterval = 2;
        Time interPacketIntervalv = Seconds((1 / (double) packetinterval)/*1/((double)20)*/);
//			Time interPacketIntervalv = Seconds(5.0);

        UdpServerHelper echoServerv(1887);
        ApplicationContainer serv1erAppsv = echoServerv.Install(TTNTNode.Get(33));
        serv1erAppsv.Start(Seconds(112.0));
        serv1erAppsv.Stop(Seconds(117.0));

        UdpClientHelper echoClientv(NetTTNTDevice.GetAddress(33), 1887);
        echoClientv.SetAttribute("MaxPackets", UintegerValue(maxPacketCountv));
        echoClientv.SetAttribute("Interval", TimeValue(interPacketIntervalv));
        echoClientv.SetAttribute("PacketSize", UintegerValue(packetSizev));

        ApplicationContainer clientAppsv = echoClientv.Install(
                TTNTNode.Get(32));
        clientAppsv.Start(Seconds(112.0));
        clientAppsv.Stop(Seconds(117.0));

        ////////////////////////data/////////
        uint32_t packetSizev1 = 1499;
        uint32_t maxPacketCountv1 = 10000000;
        uint32_t packetinterval1 = 500;
        Time interPacketIntervalv1 = Seconds((1 / (double) packetinterval1)/*1/((double)20)*/);
//			Time interPacketIntervalv1 = Seconds(1.0);

        UdpServerHelper echoServerv1(37);
        ApplicationContainer serverAppsv1 = echoServerv1.Install(TTNTNode.Get(33));
        serverAppsv1.Start(Seconds(start_time));
        serverAppsv1.Stop(Seconds(end_time));

        UdpClientHelper echoClientv1(NetTTNTDevice.GetAddress(33), 37);
        echoClientv1.SetAttribute("MaxPackets", UintegerValue(maxPacketCountv1));
        echoClientv1.SetAttribute("Interval", TimeValue(interPacketIntervalv1));
        echoClientv1.SetAttribute("PacketSize", UintegerValue(packetSizev1));

        ApplicationContainer clientAppsv1 = echoClientv1.Install(
                TTNTNode.Get(32));
        clientAppsv1.Start(Seconds(start_time));
        clientAppsv1.Stop(Seconds(end_time));
    }

    /////////////////////18
    if (workflow[18]) {    //route

        uint32_t packetSizev = 1;
        uint32_t maxPacketCountv = 10000000;
        uint32_t packetinterval = 2;
        Time interPacketIntervalv = Seconds((1 / (double) packetinterval)/*1/((double)20)*/);
//			Time interPacketIntervalv = Seconds(5.0);

        UdpServerHelper echoServerv(1998);
        ApplicationContainer serv1erAppsv = echoServerv.Install(TTNTNode.Get(35));
        serv1erAppsv.Start(Seconds(119.0));
        serv1erAppsv.Stop(Seconds(124.0));

        UdpClientHelper echoClientv(NetTTNTDevice.GetAddress(35), 1998);
        echoClientv.SetAttribute("MaxPackets", UintegerValue(maxPacketCountv));
        echoClientv.SetAttribute("Interval", TimeValue(interPacketIntervalv));
        echoClientv.SetAttribute("PacketSize", UintegerValue(packetSizev));

        ApplicationContainer clientAppsv = echoClientv.Install(
                TTNTNode.Get(34));
        clientAppsv.Start(Seconds(119.0));
        clientAppsv.Stop(Seconds(124.0));

        ////////////////////////data/////////
        uint32_t packetSizev1 = 1499;
        uint32_t maxPacketCountv1 = 10000000;
        uint32_t packetinterval1 = 500;
        Time interPacketIntervalv1 = Seconds((1 / (double) packetinterval1)/*1/((double)20)*/);
//			Time interPacketIntervalv1 = Seconds(1.0);

        UdpServerHelper echoServerv1(38);
        ApplicationContainer serverAppsv1 = echoServerv1.Install(TTNTNode.Get(35));
        serverAppsv1.Start(Seconds(start_time));
        serverAppsv1.Stop(Seconds(end_time));

        UdpClientHelper echoClientv1(NetTTNTDevice.GetAddress(35), 38);
        echoClientv1.SetAttribute("MaxPackets", UintegerValue(maxPacketCountv1));
        echoClientv1.SetAttribute("Interval", TimeValue(interPacketIntervalv1));
        echoClientv1.SetAttribute("PacketSize", UintegerValue(packetSizev1));

        ApplicationContainer clientAppsv1 = echoClientv1.Install(
                TTNTNode.Get(34));
        clientAppsv1.Start(Seconds(start_time));
        clientAppsv1.Stop(Seconds(end_time));
    }

    /////////////////////19
    if (workflow[19]) {    //route

        uint32_t packetSizev = 1;
        uint32_t maxPacketCountv = 10000000;
        uint32_t packetinterval = 2;
        Time interPacketIntervalv = Seconds((1 / (double) packetinterval)/*1/((double)20)*/);
//			Time interPacketIntervalv = Seconds(5.0);

        UdpServerHelper echoServerv(2109);
        ApplicationContainer serv1erAppsv = echoServerv.Install(TTNTNode.Get(37));
        serv1erAppsv.Start(Seconds(126.0));
        serv1erAppsv.Stop(Seconds(131.0));

        UdpClientHelper echoClientv(NetTTNTDevice.GetAddress(37), 2109);
        echoClientv.SetAttribute("MaxPackets", UintegerValue(maxPacketCountv));
        echoClientv.SetAttribute("Interval", TimeValue(interPacketIntervalv));
        echoClientv.SetAttribute("PacketSize", UintegerValue(packetSizev));

        ApplicationContainer clientAppsv = echoClientv.Install(
                TTNTNode.Get(36));
        clientAppsv.Start(Seconds(126.0));
        clientAppsv.Stop(Seconds(131.0));

        ////////////////////////data/////////
        uint32_t packetSizev1 = 1499;
        uint32_t maxPacketCountv1 = 10000000;
        uint32_t packetinterval1 = 500;
        Time interPacketIntervalv1 = Seconds((1 / (double) packetinterval1)/*1/((double)20)*/);
//			Time interPacketIntervalv1 = Seconds(1.0);

        UdpServerHelper echoServerv1(39);
        ApplicationContainer serverAppsv1 = echoServerv1.Install(TTNTNode.Get(37));
        serverAppsv1.Start(Seconds(start_time));
        serverAppsv1.Stop(Seconds(end_time));

        UdpClientHelper echoClientv1(NetTTNTDevice.GetAddress(37), 39);
        echoClientv1.SetAttribute("MaxPackets", UintegerValue(maxPacketCountv1));
        echoClientv1.SetAttribute("Interval", TimeValue(interPacketIntervalv1));
        echoClientv1.SetAttribute("PacketSize", UintegerValue(packetSizev1));

        ApplicationContainer clientAppsv1 = echoClientv1.Install(
                TTNTNode.Get(36));
        clientAppsv1.Start(Seconds(start_time));
        clientAppsv1.Stop(Seconds(end_time));
    }

    /////////////////////20
    if (workflow[20]) {    //route

        uint32_t packetSizev = 1;
        uint32_t maxPacketCountv = 10000000;
        uint32_t packetinterval = 2;
        Time interPacketIntervalv = Seconds((1 / (double) packetinterval)/*1/((double)20)*/);
//			Time interPacketIntervalv = Seconds(5.0);

        UdpServerHelper echoServerv(2220);
        ApplicationContainer serv1erAppsv = echoServerv.Install(TTNTNode.Get(39));
        serv1erAppsv.Start(Seconds(133.0));
        serv1erAppsv.Stop(Seconds(138.0));

        UdpClientHelper echoClientv(NetTTNTDevice.GetAddress(39), 2220);
        echoClientv.SetAttribute("MaxPackets", UintegerValue(maxPacketCountv));
        echoClientv.SetAttribute("Interval", TimeValue(interPacketIntervalv));
        echoClientv.SetAttribute("PacketSize", UintegerValue(packetSizev));

        ApplicationContainer clientAppsv = echoClientv.Install(
                TTNTNode.Get(38));
        clientAppsv.Start(Seconds(133.0));
        clientAppsv.Stop(Seconds(138.0));

        ////////////////////////data/////////
        uint32_t packetSizev1 = 1499;
        uint32_t maxPacketCountv1 = 10000000;
        uint32_t packetinterval1 = 500;
        Time interPacketIntervalv1 = Seconds((1 / (double) packetinterval1)/*1/((double)20)*/);
//			Time interPacketIntervalv1 = Seconds(1.0);

        UdpServerHelper echoServerv1(40);
        ApplicationContainer serverAppsv1 = echoServerv1.Install(TTNTNode.Get(39));
        serverAppsv1.Start(Seconds(start_time));
        serverAppsv1.Stop(Seconds(end_time));

        UdpClientHelper echoClientv1(NetTTNTDevice.GetAddress(39), 40);
        echoClientv1.SetAttribute("MaxPackets", UintegerValue(maxPacketCountv1));
        echoClientv1.SetAttribute("Interval", TimeValue(interPacketIntervalv1));
        echoClientv1.SetAttribute("PacketSize", UintegerValue(packetSizev1));

        ApplicationContainer clientAppsv1 = echoClientv1.Install(
                TTNTNode.Get(38));
        clientAppsv1.Start(Seconds(start_time));
        clientAppsv1.Stop(Seconds(end_time));
    }

    /////////////////////21
    if (workflow[21]) {    //route

        uint32_t packetSizev = 1;
        uint32_t maxPacketCountv = 10000000;
        uint32_t packetinterval = 2;
        Time interPacketIntervalv = Seconds((1 / (double) packetinterval)/*1/((double)20)*/);
//			Time interPacketIntervalv = Seconds(5.0);

        UdpServerHelper echoServerv(2331);
        ApplicationContainer serv1erAppsv = echoServerv.Install(TTNTNode.Get(41));
        serv1erAppsv.Start(Seconds(140.0));
        serv1erAppsv.Stop(Seconds(145.0));

        UdpClientHelper echoClientv(NetTTNTDevice.GetAddress(41), 2331);
        echoClientv.SetAttribute("MaxPackets", UintegerValue(maxPacketCountv));
        echoClientv.SetAttribute("Interval", TimeValue(interPacketIntervalv));
        echoClientv.SetAttribute("PacketSize", UintegerValue(packetSizev));

        ApplicationContainer clientAppsv = echoClientv.Install(
                TTNTNode.Get(40));
        clientAppsv.Start(Seconds(140.0));
        clientAppsv.Stop(Seconds(145.0));

        ////////////////////////data/////////
        uint32_t packetSizev1 = 1499;
        uint32_t maxPacketCountv1 = 10000000;
        uint32_t packetinterval1 = 500;
        Time interPacketIntervalv1 = Seconds((1 / (double) packetinterval1)/*1/((double)20)*/);
//			Time interPacketIntervalv1 = Seconds(1.0);

        UdpServerHelper echoServerv1(41);
        ApplicationContainer serverAppsv1 = echoServerv1.Install(TTNTNode.Get(41));
        serverAppsv1.Start(Seconds(start_time));
        serverAppsv1.Stop(Seconds(end_time));

        UdpClientHelper echoClientv1(NetTTNTDevice.GetAddress(41), 41);
        echoClientv1.SetAttribute("MaxPackets", UintegerValue(maxPacketCountv1));
        echoClientv1.SetAttribute("Interval", TimeValue(interPacketIntervalv1));
        echoClientv1.SetAttribute("PacketSize", UintegerValue(packetSizev1));

        ApplicationContainer clientAppsv1 = echoClientv1.Install(
                TTNTNode.Get(40));
        clientAppsv1.Start(Seconds(start_time));
        clientAppsv1.Stop(Seconds(end_time));
    }

    /////////////////////22
    if (workflow[22]) {    //route

        uint32_t packetSizev = 1;
        uint32_t maxPacketCountv = 10000000;
        uint32_t packetinterval = 2;
        Time interPacketIntervalv = Seconds((1 / (double) packetinterval)/*1/((double)20)*/);
//			Time interPacketIntervalv = Seconds(5.0);

        UdpServerHelper echoServerv(2442);
        ApplicationContainer serv1erAppsv = echoServerv.Install(TTNTNode.Get(43));
        serv1erAppsv.Start(Seconds(147.0));
        serv1erAppsv.Stop(Seconds(152.0));

        UdpClientHelper echoClientv(NetTTNTDevice.GetAddress(43), 2442);
        echoClientv.SetAttribute("MaxPackets", UintegerValue(maxPacketCountv));
        echoClientv.SetAttribute("Interval", TimeValue(interPacketIntervalv));
        echoClientv.SetAttribute("PacketSize", UintegerValue(packetSizev));

        ApplicationContainer clientAppsv = echoClientv.Install(
                TTNTNode.Get(42));
        clientAppsv.Start(Seconds(147.0));
        clientAppsv.Stop(Seconds(152.0));

        ////////////////////////data/////////
        uint32_t packetSizev1 = 1499;
        uint32_t maxPacketCountv1 = 10000000;
        uint32_t packetinterval1 = 500;
        Time interPacketIntervalv1 = Seconds((1 / (double) packetinterval1)/*1/((double)20)*/);
//			Time interPacketIntervalv1 = Seconds(1.0);

        UdpServerHelper echoServerv1(42);
        ApplicationContainer serverAppsv1 = echoServerv1.Install(TTNTNode.Get(43));
        serverAppsv1.Start(Seconds(start_time));
        serverAppsv1.Stop(Seconds(end_time));

        UdpClientHelper echoClientv1(NetTTNTDevice.GetAddress(43), 42);
        echoClientv1.SetAttribute("MaxPackets", UintegerValue(maxPacketCountv1));
        echoClientv1.SetAttribute("Interval", TimeValue(interPacketIntervalv1));
        echoClientv1.SetAttribute("PacketSize", UintegerValue(packetSizev1));

        ApplicationContainer clientAppsv1 = echoClientv1.Install(
                TTNTNode.Get(42));
        clientAppsv1.Start(Seconds(start_time));
        clientAppsv1.Stop(Seconds(end_time));
    }

    /////////////////////23
    if (workflow[23]) {    //route

        uint32_t packetSizev = 1;
        uint32_t maxPacketCountv = 10000000;
        uint32_t packetinterval = 2;
        Time interPacketIntervalv = Seconds((1 / (double) packetinterval)/*1/((double)20)*/);
//			Time interPacketIntervalv = Seconds(5.0);

        UdpServerHelper echoServerv(2553);
        ApplicationContainer serv1erAppsv = echoServerv.Install(TTNTNode.Get(45));
        serv1erAppsv.Start(Seconds(154.0));
        serv1erAppsv.Stop(Seconds(159.0));

        UdpClientHelper echoClientv(NetTTNTDevice.GetAddress(45), 2553);
        echoClientv.SetAttribute("MaxPackets", UintegerValue(maxPacketCountv));
        echoClientv.SetAttribute("Interval", TimeValue(interPacketIntervalv));
        echoClientv.SetAttribute("PacketSize", UintegerValue(packetSizev));

        ApplicationContainer clientAppsv = echoClientv.Install(
                TTNTNode.Get(44));
        clientAppsv.Start(Seconds(154.0));
        clientAppsv.Stop(Seconds(159.0));

        ////////////////////////data/////////
        uint32_t packetSizev1 = 1499;
        uint32_t maxPacketCountv1 = 10000000;
        uint32_t packetinterval1 = 500;
        Time interPacketIntervalv1 = Seconds((1 / (double) packetinterval1)/*1/((double)20)*/);
//			Time interPacketIntervalv1 = Seconds(1.0);

        UdpServerHelper echoServerv1(43);
        ApplicationContainer serverAppsv1 = echoServerv1.Install(TTNTNode.Get(45));
        serverAppsv1.Start(Seconds(start_time));
        serverAppsv1.Stop(Seconds(end_time));

        UdpClientHelper echoClientv1(NetTTNTDevice.GetAddress(45), 43);
        echoClientv1.SetAttribute("MaxPackets", UintegerValue(maxPacketCountv1));
        echoClientv1.SetAttribute("Interval", TimeValue(interPacketIntervalv1));
        echoClientv1.SetAttribute("PacketSize", UintegerValue(packetSizev1));

        ApplicationContainer clientAppsv1 = echoClientv1.Install(
                TTNTNode.Get(44));
        clientAppsv1.Start(Seconds(start_time));
        clientAppsv1.Stop(Seconds(end_time));
    }

    /////////////////////24
    if (workflow[24]) {    //route

        uint32_t packetSizev = 1;
        uint32_t maxPacketCountv = 10000000;
        uint32_t packetinterval = 2;
        Time interPacketIntervalv = Seconds((1 / (double) packetinterval)/*1/((double)20)*/);
//			Time interPacketIntervalv = Seconds(5.0);

        UdpServerHelper echoServerv(2664);
        ApplicationContainer serv1erAppsv = echoServerv.Install(TTNTNode.Get(47));
        serv1erAppsv.Start(Seconds(161.0));
        serv1erAppsv.Stop(Seconds(166.0));

        UdpClientHelper echoClientv(NetTTNTDevice.GetAddress(47), 2664);
        echoClientv.SetAttribute("MaxPackets", UintegerValue(maxPacketCountv));
        echoClientv.SetAttribute("Interval", TimeValue(interPacketIntervalv));
        echoClientv.SetAttribute("PacketSize", UintegerValue(packetSizev));

        ApplicationContainer clientAppsv = echoClientv.Install(
                TTNTNode.Get(46));
        clientAppsv.Start(Seconds(161.0));
        clientAppsv.Stop(Seconds(166.0));

        ////////////////////////data/////////
        uint32_t packetSizev1 = 1499;
        uint32_t maxPacketCountv1 = 10000000;
        uint32_t packetinterval1 = 500;
        Time interPacketIntervalv1 = Seconds((1 / (double) packetinterval1)/*1/((double)20)*/);
//			Time interPacketIntervalv1 = Seconds(1.0);

        UdpServerHelper echoServerv1(44);
        ApplicationContainer serverAppsv1 = echoServerv1.Install(TTNTNode.Get(47));
        serverAppsv1.Start(Seconds(start_time));
        serverAppsv1.Stop(Seconds(end_time));

        UdpClientHelper echoClientv1(NetTTNTDevice.GetAddress(47), 44);
        echoClientv1.SetAttribute("MaxPackets", UintegerValue(maxPacketCountv1));
        echoClientv1.SetAttribute("Interval", TimeValue(interPacketIntervalv1));
        echoClientv1.SetAttribute("PacketSize", UintegerValue(packetSizev1));

        ApplicationContainer clientAppsv1 = echoClientv1.Install(
                TTNTNode.Get(46));
        clientAppsv1.Start(Seconds(start_time));
        clientAppsv1.Stop(Seconds(end_time));
    }

    /////////////////////25
    if (workflow[25]) {    //route

        uint32_t packetSizev = 1;
        uint32_t maxPacketCountv = 10000000;
        uint32_t packetinterval = 2;
        Time interPacketIntervalv = Seconds((1 / (double) packetinterval)/*1/((double)20)*/);
//			Time interPacketIntervalv = Seconds(5.0);

        UdpServerHelper echoServerv(2775);
        ApplicationContainer serv1erAppsv = echoServerv.Install(TTNTNode.Get(49));
        serv1erAppsv.Start(Seconds(168.0));
        serv1erAppsv.Stop(Seconds(173.0));

        UdpClientHelper echoClientv(NetTTNTDevice.GetAddress(49), 2775);
        echoClientv.SetAttribute("MaxPackets", UintegerValue(maxPacketCountv));
        echoClientv.SetAttribute("Interval", TimeValue(interPacketIntervalv));
        echoClientv.SetAttribute("PacketSize", UintegerValue(packetSizev));

        ApplicationContainer clientAppsv = echoClientv.Install(
                TTNTNode.Get(48));
        clientAppsv.Start(Seconds(168.0));
        clientAppsv.Stop(Seconds(173.0));

        ////////////////////////data/////////
        uint32_t packetSizev1 = 1499;
        uint32_t maxPacketCountv1 = 10000000;
        uint32_t packetinterval1 = 500;
        Time interPacketIntervalv1 = Seconds((1 / (double) packetinterval1)/*1/((double)20)*/);
//			Time interPacketIntervalv1 = Seconds(1.0);

        UdpServerHelper echoServerv1(45);
        ApplicationContainer serverAppsv1 = echoServerv1.Install(TTNTNode.Get(49));
        serverAppsv1.Start(Seconds(start_time));
        serverAppsv1.Stop(Seconds(end_time));

        UdpClientHelper echoClientv1(NetTTNTDevice.GetAddress(49), 45);
        echoClientv1.SetAttribute("MaxPackets", UintegerValue(maxPacketCountv1));
        echoClientv1.SetAttribute("Interval", TimeValue(interPacketIntervalv1));
        echoClientv1.SetAttribute("PacketSize", UintegerValue(packetSizev1));

        ApplicationContainer clientAppsv1 = echoClientv1.Install(
                TTNTNode.Get(48));
        clientAppsv1.Start(Seconds(start_time));
        clientAppsv1.Stop(Seconds(end_time));
    }

    if (1) {
        phySinc.EnablePcap("NodeNumTPS", ttntDevice.Get(1));
    }

    Simulator::Stop(Seconds(end_time + 1.0));

    AnimationInterface anim("amix.xml");

    if (1) //得出每个节点的真实位置与IP地址
    {
        map <std::string, uint32_t> mapIpv4 = anim.getIpv4AddressNodeIdTable();
        multimap <uint32_t, std::string> mapipv4 = anim.GetIpv4AddressNodeIdTable();
        map <uint32_t, Vector> mapPositon = anim.getNodePosition();

        map <std::string, Vector> NodeIPandPosition;
        map <uint32_t, map<std::string, Vector>> NodeIDandIPandPosition;

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

        for (auto iterFinal = NodeIDandIPandPosition.begin(); iterFinal
                                                              != NodeIDandIPandPosition.end(); iterFinal++) {
            ofstream positionFILE("/mnt/hgfs/VMwareShareFile/NodeIDIPPositionFile.txt", ios::app);
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

    //  //------------------------------------------------------------
    //    //-- Generate statistics output.
    //    //--------------------------------------------
    // Pick an output writer based in the requested format.
    Ptr <DataOutputInterface> output = 0;
    if (format == "omnet") {
        NS_LOG_INFO("Creating omnet formatted data output.");
        output = CreateObject<OmnetDataOutput>();
    } else if (format == "db") {
#ifdef STATS_HAS_SQLITE3
        NS_LOG_INFO ("Creating sqlite formatted data output.");
        output = CreateObject<SqliteDataOutput>();
#endif
    } else {
        NS_LOG_ERROR("Unknown output format " << format);
    }

    Simulator::Destroy();
    return 0;
}
