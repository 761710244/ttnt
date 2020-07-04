/*
 * SPMA.cc
 *
 *  Created on: Jun 21, 2020
 *      Author: pinganzhang
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/netanim-module.h"
#include "ns3/mobility-module.h"
#include "map"
#include "string"
#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/internet-module.h"
#include "ns3/olsr-module.h"
#include "ns3/dsr-module.h"
#include "ns3/dsdv-module.h"
#include <vector>
#include "ns3/stats-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("SecondScriptExample");

int main(int argc, char *argv[]) {
	bool verbose = true;
	uint32_t nCsma = 4;

	CommandLine cmd;
	cmd.AddValue("nCsma", "Number of \"extra\" CSMA nodes/devices", nCsma);
	cmd.AddValue("verbose", "Tell echo applications to log if true", verbose);

	cmd.Parse(argc, argv);

	if (verbose) {
		LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
		LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);
	}

	nCsma = nCsma == 0 ? 1 : nCsma;

	NodeContainer csmaNodes;
	csmaNodes.Create(nCsma);

	CsmaHelper csma;
	csma.SetChannelAttribute("DataRate", StringValue("2Kbps"));
	csma.SetChannelAttribute("Delay", TimeValue(MilliSeconds(2)));

	NetDeviceContainer csmaDevices;
	csmaDevices = csma.Install(csmaNodes);

	MobilityHelper mobility;
	mobility.SetPositionAllocator("ns3::GridPositionAllocator", "MinX",
			DoubleValue(0.0), "MinY", DoubleValue(0.0), "DeltaX",
			DoubleValue(50.0), "DeltaY", DoubleValue(50.0), "GridWidth",
			UintegerValue(2), "LayoutType", StringValue("RowFirst"));
	mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
	mobility.Install(csmaNodes);  //TTNT Node

	InternetStackHelper stack;

	DsrHelper Dsr;
	DsrMainHelper DsrMain;
	stack.Install(csmaNodes);
	DsrMain.Install(Dsr, csmaNodes);

	Ipv4AddressHelper address;
	address.SetBase("10.1.1.0", "255.255.255.0");

	Ipv4InterfaceContainer csmaInterfaces;
	csmaInterfaces = address.Assign(csmaDevices);

	if (1) {

		uint32_t packetinterval1 = 500;

		Time interPacketIntervalv1 = Seconds((1 / (double) packetinterval1));

		UdpEchoServerHelper echoServer(2);

		ApplicationContainer serverApps = echoServer.Install(csmaNodes.Get(1));

		serverApps.Start(Seconds(1.0));
		serverApps.Stop(Seconds(10.0));

		UdpEchoClientHelper echoClient(csmaInterfaces.GetAddress(1), 10);
		echoClient.SetAttribute("MaxPackets", UintegerValue(1));
		echoClient.SetAttribute("Interval", TimeValue(interPacketIntervalv1));
		echoClient.SetAttribute("PacketSize", UintegerValue(200));

		ApplicationContainer clientApps = echoClient.Install(csmaNodes.Get(0));

		clientApps.Start(Seconds(2.0));
		clientApps.Stop(Seconds(10.0));
	}

	if (1) {

		uint32_t packetinterval1 = 500;

		Time interPacketIntervalv1 = Seconds((1 / (double) packetinterval1));

		UdpEchoServerHelper echoServer(2);

		ApplicationContainer serverApps = echoServer.Install(csmaNodes.Get(3));

		serverApps.Start(Seconds(1.0));
		serverApps.Stop(Seconds(10.0));

		UdpEchoClientHelper echoClient(csmaInterfaces.GetAddress(3), 11);
		echoClient.SetAttribute("MaxPackets", UintegerValue(1));
		echoClient.SetAttribute("Interval", TimeValue(interPacketIntervalv1));
		echoClient.SetAttribute("PacketSize", UintegerValue(200));

		ApplicationContainer clientApps = echoClient.Install(csmaNodes.Get(2));

		clientApps.Start(Seconds(2.0));
		clientApps.Stop(Seconds(10.0));
	}

	Ipv4GlobalRoutingHelper::PopulateRoutingTables();

	csma.EnablePcap("second", csmaDevices.Get(1), true);

	AnimationInterface anim("amix.xml");

	if (1) {

//	  map<string, string> mp = anim.
//	  map<string, uint32_t> mapIpv4 = anim.getIpv4AddressNodeIdTable();

	}
	Simulator::Run();
	Simulator::Destroy();
	return 0;
}

