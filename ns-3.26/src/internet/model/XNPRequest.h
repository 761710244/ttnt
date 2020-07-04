

#ifndef XNPREQUEST_H
#define XNPREQUEST_H

#include "ns3/header.h"
#include "ns3/mac48-address.h"
#include "ns3/nstime.h"
#include <stdint.h>

namespace ns3 {


class XNPRequest : public Header
{
public:
	uint8_t XNP_version=1;
	uint8_t XNP_type;

	XNPRequest ();
  ~XNPRequest ();

  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual void Print (std::ostream &os) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);


  void SetVersion(uint8_t ver);

  void SetType(uint8_t type);

  uint8_t GetVersion(void) const;

  uint8_t GetType(void) const;

  uint32_t
  GetSize (void) const;

  /**
   * Return a string corresponds to the header type.
   *
   * \returns a string corresponds to the header type.
   */

  typedef void (* TracedCallback)(const XNPRequest &header);


};

} //namespace ns3

#endif /* WIFI_MAC_HEADER_H */
