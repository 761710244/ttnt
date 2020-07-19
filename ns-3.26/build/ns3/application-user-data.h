/*
 * application-user-data.h
 *
 *  Created on: May 24, 2018
 *      Author: root
 */
#include "ns3/header.h"
#include "ns3/nstime.h"
#include <fstream>
#include <stdint.h>
#include <bitset>

#ifndef SRC_APPLICATIONS_MODEL_APPLICATION_USER_DATA_H_
#define SRC_APPLICATIONS_MODEL_APPLICATION_USER_DATA_H_

namespace ns3 {

class ApplicationUserData: public Header {
public:
	ApplicationUserData();
	virtual ~ApplicationUserData();

	static TypeId GetTypeId (void);
	virtual TypeId GetInstanceTypeId (void) const;
	uint32_t GetSize (void) const;
	virtual void Print (std::ostream &os) const;
	uint32_t GetSerializedSize (void) const;
	void SetNodeID (uint32_t NodeID);
	uint32_t GetNodeID (void);
	void Serialize (Buffer::Iterator start) const;
	uint32_t Deserialize (Buffer::Iterator start);


	uint32_t m_NodeID = 0;

};

} /* namespace ns3 */

#endif /* SRC_APPLICATIONS_MODEL_APPLICATION_USER_DATA_H_ */
