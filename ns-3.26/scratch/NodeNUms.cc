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
    double start_time = (ttnt / 2) * 7, end_time = start_time + 400.0;

    /**
     * route
     */
    if (workflow[1]) {

        uint32_t packetSizev = 1;
        uint32_t maxPacketCountv = 10000000;
        uint32_t packetinterval = 2;
        Time interPacketIntervalv = Seconds((1 / (double) packetinterval));

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

        /**
         * data
         */
        uint32_t packetSizev1 = 100;
        uint32_t maxPacketCountv1 = 10000000;
        uint32_t packetinterval1 = 2;
        Time interPacketIntervalv1 = Seconds((1 / (double) packetinterval1)/*1/((double)20)*/);

        UdpServerHelper echoServerv1(21);
        ApplicationContainer serverAppsv1 = echoServerv1.Install(TTNTNode.Get(1));
        serverAppsv1.Start(Seconds(start_time));
        serverAppsv1.Stop(Seconds(end_time));

        UdpClientHelper echoClientv1(NetTTNTDevice.GetAddress(1), 21);
        echoClientv1.SetAttribute("MaxPackets", UintegerValue(maxPacketCountv1));
        echoClientv1.SetAttribute("Interval", TimeValue(interPacketIntervalv1));
        echoClientv1.SetAttribute("PacketSize", UintegerValue(packetSizev1));

        ApplicationContainer clientAppsv1 = echoClientv1.Install(TTNTNode.Get(0));
        clientAppsv1.Start(Seconds(start_time));
        clientAppsv1.Stop(Seconds(end_time));
    }

    if (0) {
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
