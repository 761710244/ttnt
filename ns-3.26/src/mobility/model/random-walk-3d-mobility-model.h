/*
 * random-walk-3d-mobility-model.h
 *
 *  Created on: May 28, 2018
 *      Author: root
 */

#ifndef SRC_MOBILITY_MODEL_RANDOM_WALK_3D_MOBILITY_MODEL_H_
#define SRC_MOBILITY_MODEL_RANDOM_WALK_3D_MOBILITY_MODEL_H_

#include "ns3/object.h"
#include "ns3/nstime.h"
#include "ns3/event-id.h"
#include "ns3/box.h"
#include "ns3/random-variable-stream.h"
#include "mobility-model.h"
#include "constant-velocity-helper.h"

namespace ns3 {

class RandomWalk3dMobilityModel: public MobilityModel
{
public:
	RandomWalk3dMobilityModel();
	virtual ~RandomWalk3dMobilityModel();
	static TypeId GetTypeId (void);
	  /** An enum representing the different working modes of this module. */
	  enum Mode  {
	    MODE_DISTANCE,
	    MODE_TIME
	  };

	private:
	  /**
	   * \brief Performs the rebound of the node if it reaches a boundary
	   * \param timeLeft The remaining time of the walk
	   */
	  void Rebound (Time timeLeft);
	  /**
	   * Walk according to position and velocity, until distance is reached,
	   * time is reached, or intersection with the bounding box
	   */
	  void DoWalk (Time timeLeft);
	  /**
	   * Perform initialization of the object before MobilityModel::DoInitialize ()
	   */
	  void DoInitializePrivate (void);
	  virtual void DoDispose (void);
	  virtual void DoInitialize (void);
	  virtual Vector DoGetPosition (void) const;
	  virtual void DoSetPosition (const Vector &position);
	  virtual Vector DoGetVelocity (void) const;
	  virtual int64_t DoAssignStreams (int64_t);

	  ConstantVelocityHelper m_helper;       //!< helper for this object
	  EventId m_event;                       //!< stored event ID
	  enum Mode m_mode;                      //!< whether in time or distance mode
	  double m_modeDistance;                 //!< Change direction and speed after this distance
	  Time m_modeTime;                       //!< Change current direction and speed after this delay
	  Ptr<RandomVariableStream> m_speed;     //!< rv for picking speed
	  Ptr<RandomVariableStream> m_direction; //!< rv for picking direction
	  Box m_bounds;                    //!< Bounds of the area to cruise
};

} /* namespace ns3 */

#endif /* SRC_MOBILITY_MODEL_RANDOM_WALK_3D_MOBILITY_MODEL_H_ */
