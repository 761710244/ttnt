/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009 MIRKO BANCHI
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
 * Author: Ghada Badawy <gbadawy@gmail.com>
 */

#ifndef SINC_HT_WIFI_MAC_HELPER_H
#define SINC_HT_WIFI_MAC_HELPER_H

#include "ns3/string.h"
#include "ns3/sinc-qos-utils.h"
#include <map>
#include "sinc-qos-wifi-mac-helper.h"
#include "sinc-wifi-helper.h"

/**
 * (Deprecated) ns3::HtWifiMacHelper declaration.
 */

namespace ns3 {namespace ttnt {

/**
 * \brief create HT-enabled MAC layers for a ns3::WifiNetDevice.
 *
 * This class can create MACs of type ns3::ApWifiMac, ns3::StaWifiMac,
 * and, ns3::AdhocWifiMac, with QosSupported and HTSupported attributes set to True.
 */
class HtWifiMacHelper : public QosWifiMacHelper
{
public:
  /**
   * Create a HtWifiMacHelper that is used to make life easier when working
   * with Wifi devices using a QOS MAC layer.
   */
  HtWifiMacHelper ();

  /**
   * Destroy a HtWifiMacHelper
   */
  virtual ~HtWifiMacHelper ();

  /**
   * Create a mac helper in a default working state.
   */
  static HtWifiMacHelper Default (void);

  /**
   * Converts a HT MCS value into a DataRate value
   */
  static StringValue DataRateForMcs (int mcs);
};

} //namespace ns3

}
#endif /* HT_WIFI_MAC_HELPER_H */
