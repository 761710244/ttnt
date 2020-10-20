/*
 * xnp-rep-header.h
 *
 *  Created on: 2018年5月14日
 *      Author: Odie
 */

#ifndef SRC_INTERNET_MODEL_XNP_REP_HEADER_H_
#define SRC_INTERNET_MODEL_XNP_REP_HEADER_H_

#include "ns3/header.h"
#include "ns3/address.h"
#include "ns3/ipv4-address.h"
#include <string>

namespace ns3 {

class XnpRepHeader : public Header
{
public:
	void x_SetVersionNum(uint8_t num);
	void x_SetId(uint8_t id);

	uint8_t x_GetVersionNum(void);
	uint8_t x_GetId(void);

  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual void Print (std::ostream &os) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);

  uint8_t x_versionNum;
  uint8_t x_identifier;
};

} // namespace ns3

#endif /* SRC_INTERNET_MODEL_XNP_REP_HEADER_H_ */
