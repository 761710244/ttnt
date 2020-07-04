/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014 Universita' degli Studi di Napoli "Federico II"
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
 * Author: Stefano Avallone <stefano.avallone@unina.it>
 */

#ifndef SINC_WIFI_TX_CURRENT_MODEL_H
#define SINC_WIFI_TX_CURRENT_MODEL_H

#include "ns3/object.h"

namespace ns3 {namespace sincgars {

/**
 * \ingroup energy
 * 
 * \brief Modelize the transmit current as a function of the transmit power and mode
 *
 */
class WifiTxCurrentModel : public Object
{
public:
  static TypeId GetTypeId (void);

  WifiTxCurrentModel ();
  virtual ~WifiTxCurrentModel ();

  /**
   * \param txPowerDbm the nominal tx power in dBm
   * \returns the transmit current (in Ampere)
   */
  virtual double CalcTxCurrent (double txPowerDbm) const = 0;

  /**
   * Convert from dBm to Watts.
   *
   * \param dbm the power in dBm
   * \return the equivalent Watts for the given dBm
   */
  static double DbmToW (double dbm);
};

/**
 * \ingroup energy
 *
 * \brief a linear model of the Wifi transmit current
 *
 * This model assumes that the transmit current is a linear function
 * of the nominal transmit power used to send the frame.
 * In particular, the power absorbed during the transmission of a frame \f$ W_{tx} \f$
 * is given by the power absorbed by the power amplifier \f$ W_{pa} \f$ plus the power
 * absorbed by the RF subsystem. The latter is assumed to be the same as the power
 * absorbed in the IDLE state \f$ W_{idle} \f$.
 * 
 * The efficiency \f$ \eta \f$ of the power amplifier is given by 
 * \f$ \eta = \frac{P_{tx}}{W_{pa}} \f$, where \f$ P_{tx} \f$ is the output power, i.e.,
 * the nominal transmit power. Hence, \f$ W_{pa} = \frac{P_{tx}}{\eta} \f$
 * 
 * It turns out that \f$ W_{tx} = \frac{P_{tx}}{\eta} + W_{idle} \f$. By dividing both
 * sides by the supply voltage \f$ V \f$: \f$ I_{tx} = \frac{P_{tx}}{V \cdot \eta} + I_{idle} \f$,
 * where \f$ I_{tx} \f$ and \f$ I_{idle} \f$ are, respectively, the transmit current and
 * the idle current.
 * 
 * For more information, refer to:
 * Francesco Ivan Di Piazza, Stefano Mangione, and Ilenia Tinnirello.
 * "On the Effects of Transmit Power Control on the Energy Consumption of WiFi Network Cards",
 * Proceedings of ICST QShine 2009, pp. 463--475
 * 
 * If the tx current corresponding to a given nominal transmit power is known, the efficiency
 * of the power amplifier is given by the above formula:
 * \f$ \eta = \frac{P_{tx}}{(I_{tx}-I_{idle})\cdot V} \f$
 * 
 */
class LinearWifiTxCurrentModel : public WifiTxCurrentModel
{
public:
  static TypeId GetTypeId (void);

  LinearWifiTxCurrentModel ();
  virtual ~LinearWifiTxCurrentModel ();
  
  /**
   * \param eta (dimension-less)
   *
   * Set the power amplifier efficiency.
   */
  void SetEta (double eta);

  /**
   * \param voltage (Volts)
   *
   * Set the supply voltage.
   */
  void SetVoltage (double voltage);

  /**
   * \param idleCurrent (Ampere)
   *
   * Set the current in the IDLE state.
   */
  void SetIdleCurrent (double idleCurrent);

  /**
   * \return the power amplifier efficiency.
   */
  double GetEta (void) const;

  /**
   * \return the supply voltage.
   */
  double GetVoltage (void) const;

  /**
   * \return the current in the IDLE state.
   */
  double GetIdleCurrent (void) const;

  double CalcTxCurrent (double txPowerDbm) const;

private:
  double m_eta;
  double m_voltage;
  double m_idleCurrent;
};

} // namespace ns3

}
#endif /* WIFI_TX_CURRENT_MODEL_H */
