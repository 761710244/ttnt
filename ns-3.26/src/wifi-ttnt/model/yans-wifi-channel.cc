/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2006,2007 INRIA
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
 * Author: Mathieu Lacage, <mathieu.lacage@sophia.inria.fr>
 */

#include "ns3/packet.h"
#include "ns3/simulator.h"
#include "ns3/mobility-model.h"
#include "ns3/net-device.h"
#include "ns3/node.h"
#include "ns3/log.h"
#include "ns3/pointer.h"
#include "ns3/object-factory.h"
#include "ns3/propagation-loss-model.h"
#include "ns3/propagation-delay-model.h"
#include "sinc-yans-wifi-channel.h"

namespace ns3 {namespace ttnt {

NS_LOG_COMPONENT_DEFINE ("sinYansWifiChannel");

NS_OBJECT_ENSURE_REGISTERED (YansWifiChannel);

TypeId
YansWifiChannel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::sinc-YansWifiChannel")
    .SetParent<WifiChannel> ()
    .SetGroupName ("Wifi")
    .AddConstructor<YansWifiChannel> ()
    .AddAttribute ("PropagationLossModel", "A pointer to the propagation loss model attached to this channel.",
                   PointerValue (),
                   MakePointerAccessor (&YansWifiChannel::m_loss),
                   MakePointerChecker<PropagationLossModel> ())
    .AddAttribute ("PropagationDelayModel", "A pointer to the propagation delay model attached to this channel.",
                   PointerValue (),
                   MakePointerAccessor (&YansWifiChannel::m_delay),
                   MakePointerChecker<PropagationDelayModel> ())
  ;
  return tid;
}

YansWifiChannel::YansWifiChannel ()
{
}

YansWifiChannel::~YansWifiChannel ()
{
  NS_LOG_FUNCTION_NOARGS ();
  m_phyList.clear ();
}

void
YansWifiChannel::SetPropagationLossModel (Ptr<PropagationLossModel> loss)
{
  m_loss = loss;
}

void
YansWifiChannel::SetPropagationDelayModel (Ptr<PropagationDelayModel> delay)
{
  m_delay = delay;
}

void
YansWifiChannel::Send (Ptr<YansWifiPhy> sender, Ptr<const Packet> packet, double txPowerDbm,
                       WifiTxVector txVector, WifiPreamble preamble, enum mpduType mpdutype, Time duration) const
{
//	std::cout<<"YansWifiChannel::Send &&&&&&&&&& Tag ---> "<<packet->GetTag()<<std::endl;
	if(packet->GetTag()==2)
	{
//		std::cout<<"YansWifiChannel::Send &&&&&&&&&& Tag ---> 2  Now = "<<Simulator::Now()<<std::endl;
	}
  Ptr<MobilityModel> senderMobility = sender->GetMobility ()->GetObject<MobilityModel> ();
  NS_ASSERT (senderMobility != 0);
  uint32_t j = 0;
  for (PhyList::const_iterator i = m_phyList.begin (); i != m_phyList.end (); i++, j++)
    {
      if (sender != (*i))
        {
          //For now don't account for inter channel interference
          if ((*i)->GetChannelNumber () != sender->GetChannelNumber ())
            {
              continue;
            }

          Ptr<MobilityModel> receiverMobility = (*i)->GetMobility ()->GetObject<MobilityModel> ();
          Time delay = m_delay->GetDelay (senderMobility, receiverMobility);
          double rxPowerDbm = m_loss->CalcRxPower (txPowerDbm, senderMobility, receiverMobility);
          NS_LOG_DEBUG ("propagation: txPower=" << txPowerDbm << "dbm, rxPower=" << rxPowerDbm << "dbm, " <<
                        "distance=" << senderMobility->GetDistanceFrom (receiverMobility) << "m, delay=" << delay);
          Ptr<Packet> copy = packet->Copy ();
          Ptr<Object> dstNetDevice = m_phyList[j]->GetDevice ();
          uint32_t dstNode;
          if (dstNetDevice == 0)
            {
              dstNode = 0xffffffff;
            }
          else
            {
              dstNode = dstNetDevice->GetObject<NetDevice> ()->GetNode ()->GetId ();
            }

          struct Parameters parameters;
          parameters.rxPowerDbm = rxPowerDbm;
          parameters.type = mpdutype;
          parameters.duration = duration;
          parameters.txVector = txVector;
          parameters.preamble = preamble;

          Simulator::ScheduleWithContext (dstNode,
                                          delay, &YansWifiChannel::Receive, this,
                                          j, copy, parameters);
        }
    }
}

void
YansWifiChannel::Receive (uint32_t i, Ptr<Packet> packet, struct Parameters parameters) const
{
  m_phyList[i]->StartReceivePreambleAndHeader (packet, parameters.rxPowerDbm, parameters.txVector, parameters.preamble, parameters.type, parameters.duration);
}

uint32_t
YansWifiChannel::GetNDevices (void) const
{
  return m_phyList.size ();
}

Ptr<NetDevice>
YansWifiChannel::GetDevice (uint32_t i) const
{
  return m_phyList[i]->GetDevice ()->GetObject<NetDevice> ();
}

void
YansWifiChannel::Add (Ptr<YansWifiPhy> phy)
{
  m_phyList.push_back (phy);
}

int64_t
YansWifiChannel::AssignStreams (int64_t stream)
{
  int64_t currentStream = stream;
  currentStream += m_loss->AssignStreams (stream);
  return (currentStream - stream);
}

}
} //namespace ns3
