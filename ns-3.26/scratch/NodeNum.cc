//
// Created by yf518 on 7/19/20.
//

#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
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
#include<unistd.h>

using namespace ns3;
using namespace std;

NS_LOG_COMPONENT_DEFINE("SPMANodeNumTest");

void TxCallback(Ptr <CounterCalculator<uint32_t>> datac, std::string path,
                Ptr<const Packet> packet) {
    NS_LOG_INFO("Sent frame counted in " << datac->GetKey());
    datac->Update();
    // end TxCallback
}

int main(int argc, char *argv[]) {


    uint32_t ttnt = 2;
    bool verbose = true;

    // Set up command line parameters used to control the experiment.
    CommandLine cmd;
    cmd.AddValue("ttnt", "Number of \"extra\" CSMA nodes/devices", ttnt);
    cmd.AddValue("verbose", "Tell echo applications to log if true", verbose);
    cmd.Parse(argc, argv);

    Time::SetResolution(Time::NS);

    NodeContainer TTNTNode;
    TTNTNode.Create(ttnt);

    if (verbose) {
        LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_ALL);
        LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_ALL);
        LogComponentEnable("UdpClient", LOG_LEVEL_ALL);
        LogComponentEnable("UdpServer", LOG_LEVEL_ALL);
        LogComponentEnable("UdpTraceClient", LOG_LEVEL_ALL);
    }

//****************************   TTNT   *******************************************


//    YansWifiChannelHelper channel = YansWifiChannelHelper::Default();
//    YansWifiPhyHelper phy = YansWifiPhyHelper::Default();
//    phy.SetChannel(channel.Create());
//
//    WifiHelper wifi;
//    wifi.SetRemoteStationManager("ns3::AarfWifiManager");
//
//    WifiMacHelper mac;
//    Ssid ssid = Ssid("ns-3-ssid");
//    mac.SetType("ns3::StaWifiMac");
//
//    NetDeviceContainer wifiDevice;
//    wifiDevice = wifi.Install(phy, mac, TTNTNode);

    CsmaHelper spma;
    spma.SetChannelAttribute("DataRate", StringValue("2Kbps"));
    spma.SetChannelAttribute("Delay", TimeValue(MilliSeconds(2)));

    NetDeviceContainer ttntDevice;
    ttntDevice = spma.Install(TTNTNode);

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
    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer ttntInterface;
    ttntInterface = address.Assign(ttntDevice);
//    address.Assign(wifiDevice);

    uint16_t workflow[30] = {0};
    for (uint32_t i = 1; i <= (ttnt / 2); i++) {
        workflow[i] = 1;
    }
    double start_time = (ttnt / 2) * 7, end_time = start_time + 50.0;

    if (workflow[1]) {

        /**
         * route
         */

//        uint32_t packetSizev = 2;
//        uint32_t maxPacketCountv = 1;
//        uint32_t packetinterval = 1;
//        Time interPacketIntervalv = Seconds((1 / (double) packetinterval));
//
//        UdpServerHelper echoServerv(111);
//        ApplicationContainer serv1erAppsv = echoServerv.Install(TTNTNode.Get(1));
//        serv1erAppsv.Start(Seconds(0.0));
//        serv1erAppsv.Stop(Seconds(5.0));
//
//        UdpClientHelper echoClient(NetTTNTDevice.GetAddress(1), 111);
//        echoClient.SetAttribute("MaxPackets", UintegerValue(maxPacketCountv));
//        echoClient.SetAttribute("Interval", TimeValue(interPacketIntervalv));
//        echoClient.SetAttribute("PacketSize", UintegerValue(packetSizev));
//        echoClient.SetAttribute ("MaxPackets", UintegerValue (1));
//        echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
//        echoClient.SetAttribute ("PacketSize", UintegerValue (1024));
//
//        ApplicationContainer clientAppsv = echoClient.Install(
//                TTNTNode.Get(0));
//        clientAppsv.Start(Seconds(0.0));
//        clientAppsv.Stop(Seconds(5.0));

        /**
         * data
         */
        uint32_t packetSize = 500;
        uint32_t maxPacketCount = 1;
        uint32_t packetFrequency = 2;
        Time interPacketInterval = Seconds((1 / (double) packetFrequency)/*1/((double)20)*/);

        UdpServerHelper echoServerv(21);
        ApplicationContainer serverAppsv = echoServerv.Install(TTNTNode.Get(1));
        serverAppsv.Start(Seconds(start_time));
        serverAppsv.Stop(Seconds(end_time));

        UdpClientHelper echoClient(ttntInterface.GetAddress(1), 21);
        echoClient.SetAttribute("MaxPackets", UintegerValue(maxPacketCount));
        echoClient.SetAttribute("Interval", TimeValue(interPacketInterval));
        echoClient.SetAttribute("PacketSize", UintegerValue(packetSize));

        ApplicationContainer clientApps = echoClient.Install(TTNTNode.Get(0));
        clientApps.Start(Seconds(start_time));
        clientApps.Stop(Seconds(end_time));
    }

    if (1) {
        spma.EnablePcap("NodeNumTPS", ttntDevice.Get(1));
    }

    Simulator::Stop(Seconds(end_time + 1.0));

    AnimationInterface anim("amix.xml");

//    if (0) //得出每个节点的真实位置与IP地址
//    {
//        map <std::string, uint32_t> mapIpv4 = anim.getIpv4AddressNodeIdTable();
//        multimap <uint32_t, std::string> mapipv4 = anim.GetIpv4AddressNodeIdTable();
//        map <uint32_t, Vector> mapPositon = anim.getNodePosition();
//
//        map <std::string, Vector> NodeIPandPosition;
//        map <uint32_t, map<std::string, Vector>> NodeIDandIPandPosition;
//
//        if (mapPositon.size() == mapIpv4.size()) //判断两个map大小是否相等
//        {
//            for (auto i = mapIpv4.begin(); i != mapIpv4.end(); i++) {
//                auto temp = mapPositon.find(i->second);
//                if (temp != mapPositon.end()) {
//                    NodeIPandPosition[i->first] = temp->second;
//                }
//            }
//        }
//
//
//        for (auto i = NodeIPandPosition.begin(); i != NodeIPandPosition.end(); i++) {
//            auto tmp = mapIpv4.find(i->first);
//            if (tmp != mapIpv4.end()) {
//                NodeIDandIPandPosition[tmp->second][i->first] = i->second;
//            }
//        }
//
//        for (auto iterFinal = NodeIDandIPandPosition.begin(); iterFinal
//                                                              != NodeIDandIPandPosition.end(); iterFinal++) {
//            ofstream positionFILE("/mnt/hgfs/VMwareShareFile/NodeIDIPPositionFile.txt", ios::app);
//            auto ip = iterFinal->second.begin();
//
//            if (positionFILE.good()) {
//                positionFILE << iterFinal->first << " " << ip->first << " " << ip->second.x << " " << ip->second.y
//                             << " " << endl;
//                //							ip++;
//                positionFILE.close();
//            }
//        }
//
//    }

    Simulator::Run();
    Simulator::Destroy();


    return 0;
}