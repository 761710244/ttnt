/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2005,2006 INRIA
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
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */

#include "ns3/assert.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include <cmath>

#include "eps-dcf-manager.h"
#include "eps-mac-low.h"
#include "eps-wifi-mac.h"
#include "eps-wifi-phy.h"

#include <vector>
#include <fstream>
#include <ctime> //生成随机数

using namespace std;
#define MY_DEBUG(x) \
  NS_LOG_DEBUG (Simulator::Now () << " " << this << " " << x)

namespace ns3 {namespace elprs {

NS_LOG_COMPONENT_DEFINE ("epsDcfManager");



//分配时隙 一共三类时隙 根据上层不同的数据类型选择不同的时隙分配
set<uint16_t> Lts5{/*1,9,17,25,33,41,49,57,65,73,81,89,97,105,113,121*/	 //  其实是Lts5 指挥控制 10
/*0,8,16,24,32,40,48,56,64,72,80,88,96,104,112,120*/
5,13,21,29,37,45,53,61,69,77,85,93,101,109,117,125};
set<uint16_t> Lts4{4,12,20,28,36,44,52,60,68,76,84,92,100,108,116,124 //  其实是Lts4 定位 150
/*,7,15,23,31,39,47,55,63,71,79,87,95,103,111,119,127*/};
set<uint16_t> Lts6{/*3,11,19,27,36,43,51,59,67,75,83,91,99,107,115,123*/ //lts3 300 //  其实是Lts6 图像视频 300
6,14,22,30,38,46,54,62,70,78,86,94,102,110,118,126};


/****************************************************************
 *      Implement the DCF state holder
 ****************************************************************/
DcfState::DcfState ()
  : m_backoffSlots (0),
    m_backoffStart (Seconds (0.0)),
    m_cwMin (0),
    m_cwMax (0),
    m_cw (0),
    m_accessRequested (false)
{
}

DcfState::~DcfState ()
{
}


void
DcfState::SetAifsn (uint32_t aifsn)
{
	//std::cout<<"aifsn = " << aifsn<<std::endl;
	m_aifsn = aifsn;
}

void
DcfState::SetTxopLimit (Time txopLimit)
{
  NS_ASSERT_MSG ((txopLimit.GetMicroSeconds () % 32 == 0), "The TXOP limit must be expressed in multiple of 32 microseconds!");
  m_txopLimit = txopLimit;
}

void
DcfState::SetCwMin (uint32_t minCw)
{
  bool changed = (m_cwMin != minCw);
  m_cwMin = minCw;
  if (changed == true)
    {
      ResetCw ();
    }
}

void
DcfState::SetCwMax (uint32_t maxCw)
{
  bool changed = (m_cwMax != maxCw);
  m_cwMax = maxCw;
  if (changed == true)
    {
      ResetCw ();
    }
}

uint32_t
DcfState::GetAifsn (void) const
{
	return m_aifsn;
}

Time
DcfState::GetTxopLimit (void) const
{
  return m_txopLimit;
}

uint32_t
DcfState::GetCwMin (void) const
{
  return m_cwMin;
}

uint32_t
DcfState::GetCwMax (void) const
{
  return m_cwMax;
}

void
DcfState::ResetCw (void)
{
  m_cw = m_cwMin;
}

void
DcfState::UpdateFailedCw (void)
{
  //see 802.11-2012, section 9.19.2.5
  m_cw = std::min ( 2 * (m_cw + 1) - 1, m_cwMax);
}

void
DcfState::UpdateBackoffSlotsNow (uint32_t nSlots, Time backoffUpdateBound)
{
  m_backoffSlots -= nSlots;
  m_backoffStart = backoffUpdateBound;
  MY_DEBUG ("update slots=" << nSlots << " slots, backoff=" << m_backoffSlots);
}

void
DcfState::StartBackoffNow (uint32_t nSlots)
{
  if (m_backoffSlots != 0)
    {
      MY_DEBUG ("reset backoff from " << m_backoffSlots << " to " << nSlots << " slots");
    }
  else
    {
      MY_DEBUG ("start backoff=" << nSlots << " slots");
    }
  m_backoffSlots = nSlots;
//  std::cout<<"NOw = "<<m_backoffSlots<<std::endl;
  m_backoffStart = Simulator::Now ();
  ////////////////////////////////////////
//  std::cout<<"m_backoffSlots = "<<m_backoffSlots<<std::endl;
//  std::cout<<"m_backoffStart = "<<m_backoffStart<<std::endl;

}

uint32_t
DcfState::GetCw (void) const
{
  return m_cw;
}

uint32_t
DcfState::GetBackoffSlots (void) const
{
  return m_backoffSlots;
}

Time
DcfState::GetBackoffStart (void) const
{
  return m_backoffStart;
}

bool
DcfState::IsAccessRequested (void) const
{
  return m_accessRequested;
}

void
DcfState::SetAccessRequested(bool b)
{
	m_accessRequested = b;
}
void
DcfState::NotifyAccessRequested (void)
{
  m_accessRequested = true;
}

void
DcfState::NotifyAccessGranted (void)
{
  NS_ASSERT (m_accessRequested);
  m_accessRequested = false;
  DoNotifyAccessGranted ();
}

void
DcfState::NotifyCollision (void)
{
  DoNotifyCollision ();
}

void
DcfState::NotifyInternalCollision (void)
{
  DoNotifyInternalCollision ();
}

void
DcfState::NotifyChannelSwitching (void)
{
  DoNotifyChannelSwitching ();
}

void
DcfState::NotifySleep (void)
{
  DoNotifySleep ();
}

void
DcfState::NotifyWakeUp (void)
{
  DoNotifyWakeUp ();
}


/**
 * Listener for NAV events. Forwards to DcfManager
 */
class LowDcfListener : public ns3::elprs::MacLowDcfListener
{
public:
  /**
   * Create a LowDcfListener for the given DcfManager.
   *
   * \param dcf
   */
  LowDcfListener (ns3::elprs::DcfManager *dcf)
    : m_dcf (dcf)
  {
  }
  virtual ~LowDcfListener ()
  {
  }
  virtual void NavStart (Time duration)
  {
    m_dcf->NotifyNavStartNow (duration);
  }
  virtual void NavReset (Time duration)
  {
    m_dcf->NotifyNavResetNow (duration);
  }
  virtual void AckTimeoutStart (Time duration)
  {
    m_dcf->NotifyAckTimeoutStartNow (duration);
  }
  virtual void AckTimeoutReset ()
  {
    m_dcf->NotifyAckTimeoutResetNow ();
  }
  virtual void CtsTimeoutStart (Time duration)
  {
    m_dcf->NotifyCtsTimeoutStartNow (duration);
  }
  virtual void CtsTimeoutReset ()
  {
    m_dcf->NotifyCtsTimeoutResetNow ();
  }

private:
  ns3::elprs::DcfManager *m_dcf;  //!< DcfManager to forward events to
};


/**
 * Listener for PHY events. Forwards to DcfManager
 */
class PhyListener : public ns3::elprs::WifiPhyListener
{
public:
  /**
   * Create a PhyListener for the given DcfManager.
   *
   * \param dcf
   */
  PhyListener (ns3::elprs::DcfManager *dcf)
    : m_dcf (dcf)
  {
  }
  virtual ~PhyListener ()
  {
  }
  virtual void NotifyRxStart (Time duration)
  {
    m_dcf->NotifyRxStartNow (duration);
  }
  virtual void NotifyRxEndOk (void)
  {
    m_dcf->NotifyRxEndOkNow ();
  }
  virtual void NotifyRxEndError (void)
  {
    m_dcf->NotifyRxEndErrorNow ();
  }
  virtual void NotifyTxStart (Time duration, double txPowerDbm)
  {
    m_dcf->NotifyTxStartNow (duration);
  }
  virtual void NotifyMaybeCcaBusyStart (Time duration)
  {
    m_dcf->NotifyMaybeCcaBusyStartNow (duration);
  }
  virtual void NotifySwitchingStart (Time duration)
  {
    m_dcf->NotifySwitchingStartNow (duration);
  }
  virtual void NotifySleep (void)
  {
    m_dcf->NotifySleepNow ();
  }
  virtual void NotifyWakeup (void)
  {
    m_dcf->NotifyWakeupNow ();
  }

private:
  ns3::elprs::DcfManager *m_dcf;  //!< DcfManager to forward events to
};


/****************************************************************
 *      Implement the DCF manager of all DCF state holders
 ****************************************************************/

DcfManager::DcfManager ()
  : m_lastAckTimeoutEnd (MicroSeconds (0)),
    m_lastCtsTimeoutEnd (MicroSeconds (0)),
    m_lastNavStart (MicroSeconds (0)),
    m_lastNavDuration (MicroSeconds (0)),
    m_lastRxStart (MicroSeconds (0)),
    m_lastRxDuration (MicroSeconds (0)),
    m_lastRxReceivedOk (true),
    m_lastRxEnd (MicroSeconds (0)),
    m_lastTxStart (MicroSeconds (0)),
    m_lastTxDuration (MicroSeconds (0)),
    m_lastBusyStart (MicroSeconds (0)),
    m_lastBusyDuration (MicroSeconds (0)),
    m_lastSwitchingStart (MicroSeconds (0)),
    m_lastSwitchingDuration (MicroSeconds (0)),
    m_rxing (false),
    m_sleeping (false),
    m_slotTimeUs (0),
    m_sifs (Seconds (0.0)),
    m_phyListener (0),
    m_lowListener (0)
{
  NS_LOG_FUNCTION (this);
}

DcfManager::~DcfManager ()
{
  delete m_phyListener;
  delete m_lowListener;
  m_phyListener = 0;
  m_lowListener = 0;
}

void
DcfManager::SetupPhyListener (Ptr<WifiPhy> phy)
{
  NS_LOG_FUNCTION (this << phy);
  if (m_phyListener != 0)
    {
      delete m_phyListener;
    }
  m_phyListener = new PhyListener (this);
  phy->RegisterListener (m_phyListener);
}


void
DcfManager::RemovePhyListener (Ptr<WifiPhy> phy)
{
  NS_LOG_FUNCTION (this << phy);
  if (m_phyListener != 0)
    {
      phy->UnregisterListener (m_phyListener);
      delete m_phyListener;
      m_phyListener = 0;
    }
}

void
DcfManager::SetupLowListener (Ptr<MacLow> low)
{
  NS_LOG_FUNCTION (this << low);
  if (m_lowListener != 0)
    {
      delete m_lowListener;
    }
  m_lowListener = new LowDcfListener (this);
  low->RegisterDcfListener (m_lowListener);
}

void
DcfManager::SetSlot (Time slotTime)
{
  NS_LOG_FUNCTION (this << slotTime);
  m_slotTimeUs = slotTime.GetMicroSeconds ();
}

void
DcfManager::SetSifs (Time sifs)
{
  NS_LOG_FUNCTION (this << sifs);
  m_sifs = sifs;
}

void
DcfManager::SetEifsNoDifs (Time eifsNoDifs)
{
  NS_LOG_FUNCTION (this << eifsNoDifs);
  m_eifsNoDifs = eifsNoDifs;
}

Time
DcfManager::GetEifsNoDifs () const
{
  NS_LOG_FUNCTION (this);
  return m_eifsNoDifs;
}

void
DcfManager::Add (DcfState *dcf)
{
  NS_LOG_FUNCTION (this << dcf);
  m_states.push_back (dcf);
  //std::cout<<"m_states.size()"<<m_states.size()<<std::endl;
}

Time
DcfManager::MostRecent (Time a, Time b) const
{
  NS_LOG_FUNCTION (this << a << b);
  return Max (a, b);
}

Time
DcfManager::MostRecent (Time a, Time b, Time c) const
{
  NS_LOG_FUNCTION (this << a << b << c);
  Time retval;
  retval = Max (a, b);
  retval = Max (retval, c);
  return retval;
}

Time
DcfManager::MostRecent (Time a, Time b, Time c, Time d) const
{
  NS_LOG_FUNCTION (this << a << b << c << d);
  Time e = Max (a, b);
  Time f = Max (c, d);
  Time retval = Max (e, f);
  return retval;
}

Time
DcfManager::MostRecent (Time a, Time b, Time c, Time d, Time e, Time f) const
{
  NS_LOG_FUNCTION (this << a << b << c << d << e << f);
  Time g = Max (a, b);
  Time h = Max (c, d);
  Time i = Max (e, f);
  Time k = Max (g, h);
  Time retval = Max (k, i);
  return retval;
}

Time
DcfManager::MostRecent (Time a, Time b, Time c, Time d, Time e, Time f, Time g) const
{
  NS_LOG_FUNCTION (this << a << b << c << d << e << f << g);
  Time h = Max (a, b);
  Time i = Max (c, d);
  Time j = Max (e, f);
  Time k = Max (h, i);
  Time l = Max (j, g);
  Time retval = Max (k, l);
  return retval;
}

bool
DcfManager::IsBusy (void) const
{
  NS_LOG_FUNCTION (this);
  // PHY busy
  if (m_rxing)
    {
      return true;
    }
  Time lastTxEnd = m_lastTxStart + m_lastTxDuration;
  if (lastTxEnd > Simulator::Now ())
    {
      return true;
    }
  // NAV busy
  ////////////////////////////////////////////////////////////////////
  Time lastNavEnd = m_lastNavStart + m_lastNavDuration;
  //std::cout<<"m_lastNavDuration = "<<m_lastNavDuration<<std::endl;
  if (lastNavEnd > Simulator::Now ())
    {
      return true;
    }
  // CCA busy
  Time lastCCABusyEnd = m_lastBusyStart + m_lastBusyDuration;
  if (lastCCABusyEnd > Simulator::Now ())
    {
      return true;
    }
  return false;
}

bool
DcfManager::IsWithinAifs (DcfState *state) const
{
  NS_LOG_FUNCTION (this << state);
  Time ifsEnd = GetAccessGrantStart () + MicroSeconds (state->GetAifsn () * m_slotTimeUs);
  //std::cout<<"m_slotTimeUs = "<<m_slotTimeUs<<std::endl;
  //std::cout<<"state->GetAifsn () = "<<state->GetAifsn ()<<std::endl;

  //std::cout<<"---------------------------------------"<<std::endl;
  //std::cout<<"ifsEnd  = "<<ifsEnd<<std::endl;
  //std::cout<<"Simulator::Now ()  = "<<Simulator::Now ()<<std::endl;

  if (ifsEnd > Simulator::Now ())
    {
      NS_LOG_DEBUG ("IsWithinAifs () true; ifsEnd is at " << ifsEnd.GetSeconds ());
      return true;
    }
  NS_LOG_DEBUG ("IsWithinAifs () false; ifsEnd was at " << ifsEnd.GetSeconds ());
  return false;
}

uint16_t
DcfManager:: GetAPPtype ()
{
//	char buffer[3];
//	 ifstream read_type;
//	 read_type.open("AppType.txt");
//	 read_type.getline(buffer,sizeof(buffer)-1);
//	 srand((unsigned)time(NULL));//暂时通过采用随机数的方法获取上层数据类型，实际上应该交给上层，上层通过不同数据包长度确定传输类型
//	 uint16_t randtype = rand()%3;
//	 return randtype;
	uint16_t apptype;
	ifstream readType("AppType.txt");
	if(readType.good())
	{
		readType >> apptype ;
	}
	else
	{
		cout << "Cannot read AppType.txt\n";
	}
	return apptype;
	 //return buffer[0]-'0';
}


//////////////////////////////////////////////////////////////////////////////
void
DcfManager::RequestAccess (DcfState *state)
{
//	cout <<"Lts1 = "<<Lts1.size()<<endl;
//	cout <<"Lts4 = "<<Lts4.size()<<endl;
//	cout <<"Lts3 = "<<Lts3.size()<<endl;
	NS_LOG_FUNCTION (this << state);
	  //Deny access if in sleep mode
	  if (m_sleeping)
	    {
	      return;
	    }
	  UpdateBackoff ();
	  NS_ASSERT (!state->IsAccessRequested ());
	  state->NotifyAccessRequested ();////////////////////////////让第一个m_state为true


	  // If currently transmitting; end of transmission (ACK or no ACK) will cause
	  // a later access request if needed from EndTxNoAck, GotAck, or MissedAck
	  Time lastTxEnd = m_lastTxStart + m_lastTxDuration;
	  if (lastTxEnd > Simulator::Now ())
	    {
	      NS_LOG_DEBUG ("Internal collision (currently transmitting)");
	      state->NotifyInternalCollision ();
	      DoRestartAccessTimeoutIfNeeded ();
	      return;
	    }
	  /**
	   * If there is a collision, generate a backoff
	   * by notifying the collision to the user.
	   */


		Time smx = Simulator::Now();
		uint64_t xnus = smx.GetMicroSeconds();
		uint32_t slotNo = xnus / 20 % (128);
//		std::cout<<"slotNo ==== "<<slotNo<<std::endl;

	    if (state->GetBackoffSlots () == 0)
	    {
	//**************************判断当前时隙是否为合法时隙，如果不是合法时隙应当collision****************************************************************
	    	uint16_t apptype=GetAPPtype();
			uint16_t permitaccess = 0;
			set<uint16_t>::iterator si;
			switch (apptype) {
			case 0: {
				si = Lts4.find(slotNo);
				if (si != Lts4.end())
					permitaccess = 1;	//地理位置信息
				break;
			}
			case 1: {
				si = Lts6.find(slotNo);
				if (si != Lts6.end())
					permitaccess = 1;	//语音和图像消息
				break;
			}
			case 2: {
				si = Lts5.find(slotNo);
				if (si != Lts5.end())	//指挥控制消息
					permitaccess = 1;			//当前时隙为合法时隙
				break;
			}
			default: {
				cout << "error in dcfmanager request slot";
			}
			}
			if (permitaccess != 1) {
				MY_DEBUG("not within own slots");
				NS_LOG_DEBUG(" collision (currently transmitting)");
				state->NotifyCollision();
				DoRestartAccessTimeoutIfNeeded();
				return;
			}

	      if (IsBusy ())
	        {
	          MY_DEBUG ("medium is busy: collision");
	          // someone else has accessed the medium; generate a backoff.
	          state->NotifyCollision ();
	          DoRestartAccessTimeoutIfNeeded ();
	          return;
	        }
	      else if (IsWithinAifs (state))
	        {
	    	  MY_DEBUG ("busy within AIFS");
	          state->NotifyCollision ();
	          DoRestartAccessTimeoutIfNeeded ();
	          return;
	        }
	    }
	   DoGrantAccess ();
	  DoRestartAccessTimeoutIfNeeded ();
}
//////////////////////////////////////////////////////////////////////////////

void
DcfManager::DoGrantAccess (void)
{
	NS_LOG_FUNCTION(this);
	uint32_t k = 0;

	for (States::const_iterator i = m_states.begin(); i != m_states.end();
			k++) {
		DcfState *state = *i;
		if (state->IsAccessRequested()
				&& GetBackoffEndFor(state) <= Simulator::Now()) {
			/**
			 * This is the first dcf we find with an expired backoff and which
			 * needs access to the medium. i.e., it has data to send.
			 */
			MY_DEBUG(
					"dcf " << k << " needs access. backoff expired. access granted. slots=" << state->GetBackoffSlots ());
			i++; //go to the next item in the list.
			k++;
			std::vector<DcfState *> internalCollisionStates;

			for (States::const_iterator j = i; j != m_states.end(); j++, k++) {
				DcfState *otherState = *j;

				if (otherState->IsAccessRequested()
						&& GetBackoffEndFor(otherState) <= Simulator::Now()) {
					MY_DEBUG(
							"dcf " << k << " needs access. backoff expired. internal collision. slots=" << otherState->GetBackoffSlots ());
					/**
					 * all other dcfs with a lower priority whose backoff
					 * has expired and which needed access to the medium
					 * must be notified that we did get an internal collision.
					 */
					internalCollisionStates.push_back(otherState);
				}
			}

			/**
			 * Now, we notify all of these changes in one go. It is necessary to
			 * perform first the calculations of which states are colliding and then
			 * only apply the changes because applying the changes through notification
			 * could change the global state of the manager, and, thus, could change
			 * the result of the calculations.
			 */
			state->NotifyAccessGranted();////////0
			for (std::vector<DcfState *>::const_iterator k =
					internalCollisionStates.begin();
					k != internalCollisionStates.end(); k++) {
				(*k)->NotifyInternalCollision();
			}
			break;
		}
		i++;
	}
	//std::cout<<"-------RHD-------"<<std::endl;
}

void
DcfManager::AccessTimeout (void)
{
  NS_LOG_FUNCTION (this);
  UpdateBackoff ();
  DoGrantAccess ();
  DoRestartAccessTimeoutIfNeeded ();
}

Time
DcfManager::GetAccessGrantStart (void) const
{
  NS_LOG_FUNCTION (this);
  Time rxAccessStart;
  if (!m_rxing)
    {
      rxAccessStart = m_lastRxEnd + m_sifs;
      if (!m_lastRxReceivedOk)
        {
          rxAccessStart += m_eifsNoDifs;
        }
    }
  else
    {
      rxAccessStart = m_lastRxStart + m_lastRxDuration + m_sifs;
    }
  Time busyAccessStart = m_lastBusyStart + m_lastBusyDuration + m_sifs;
  Time txAccessStart = m_lastTxStart + m_lastTxDuration + m_sifs;
  Time navAccessStart = m_lastNavStart + m_lastNavDuration + m_sifs;
  Time ackTimeoutAccessStart = m_lastAckTimeoutEnd + m_sifs;
  Time ctsTimeoutAccessStart = m_lastCtsTimeoutEnd + m_sifs;
  Time switchingAccessStart = m_lastSwitchingStart + m_lastSwitchingDuration + m_sifs;
  Time accessGrantedStart = MostRecent (rxAccessStart,
                                        busyAccessStart,
                                        txAccessStart,
                                        navAccessStart,
                                        ackTimeoutAccessStart,
                                        ctsTimeoutAccessStart,
                                        switchingAccessStart
                                        );
  NS_LOG_INFO ("access grant start=" << accessGrantedStart <<
               ", rx access start=" << rxAccessStart <<
               ", busy access start=" << busyAccessStart <<
               ", tx access start=" << txAccessStart <<
               ", nav access start=" << navAccessStart);
  return accessGrantedStart;
}

Time
DcfManager::GetBackoffStartFor (DcfState *state)
{
  NS_LOG_FUNCTION (this << state);
  Time mostRecentEvent = MostRecent (state->GetBackoffStart (),
                                     GetAccessGrantStart () + MicroSeconds (state->GetAifsn () * m_slotTimeUs));

  return mostRecentEvent;
}

Time
DcfManager::GetBackoffEndFor (DcfState *state)
{
  NS_LOG_FUNCTION (this << state);
  NS_LOG_DEBUG ("Backoff start: " << GetBackoffStartFor (state).As (Time::US) <<
    " end: " << (GetBackoffStartFor (state) +
    MicroSeconds (state->GetBackoffSlots () * m_slotTimeUs)).As (Time::US));
//  std::cout<<"DcfManager::GetBackoffEndFor 前面： "<<GetBackoffStartFor (state)<<" 后面: "
//		  <<MicroSeconds (state->GetBackoffSlots () * m_slotTimeUs)<<std::endl;
  return GetBackoffStartFor (state) + MicroSeconds (state->GetBackoffSlots () * m_slotTimeUs);
}

void
DcfManager::UpdateBackoff (void)
{
  NS_LOG_FUNCTION (this);
  uint32_t k = 0;
  for (States::const_iterator i = m_states.begin (); i != m_states.end (); i++, k++)
    {
      DcfState *state = *i;

      Time backoffStart = GetBackoffStartFor (state);
      if (backoffStart <= Simulator::Now ())
        {
          uint32_t nus = (Simulator::Now () - backoffStart).GetMicroSeconds ();
          uint32_t nIntSlots = nus / m_slotTimeUs;
          /*
           * EDCA behaves slightly different to DCA. For EDCA we
           * decrement once at the slot boundary at the end of AIFS as
           * well as once at the end of each clear slot
           * thereafter. For DCA we only decrement at the end of each
           * clear slot after DIFS. We account for the extra backoff
           * by incrementing the slot count here in the case of
           * EDCA. The if statement whose body we are in has confirmed
           * that a minimum of AIFS has elapsed since last busy
           * medium.
           */
          if (state->IsEdca ())
            {
              nIntSlots++;
            }

          uint32_t n = std::min (nIntSlots, state->GetBackoffSlots ());
          MY_DEBUG ("dcf " << k << " dec backoff slots=" << n);
          Time backoffUpdateBound = backoffStart + MicroSeconds (n * m_slotTimeUs);
          //fuction like a timer
          state->UpdateBackoffSlotsNow (n, backoffUpdateBound);
        }
    }
}

void
DcfManager::DoRestartAccessTimeoutIfNeeded (void)
{
  NS_LOG_FUNCTION (this);
  /**
   * Is there a DcfState which needs to access the medium, and,
   * if there is one, how many slots for AIFS+backoff does it require ?
   */
  bool accessTimeoutNeeded = false;
  Time expectedBackoffEnd = Simulator::GetMaximumSimulationTime ();
//  std::cout<<"expectedBackoffEnd = "<<expectedBackoffEnd<<std::endl;
  for (States::const_iterator i = m_states.begin (); i != m_states.end (); i++)
    {
      DcfState *state = *i;

	  if (state->IsAccessRequested ())
        {
          Time tmp = GetBackoffEndFor (state);
          if (tmp > Simulator::Now ())
            {
              accessTimeoutNeeded = true;
//              std::cout<<"***** &&& expectedBackoffEnd = "<<expectedBackoffEnd<<" tmp = "<<tmp<<std::endl;
              expectedBackoffEnd = std::min (expectedBackoffEnd, tmp);
            }

        }
    }

  NS_LOG_DEBUG ("Access timeout needed: " << accessTimeoutNeeded);
  if (accessTimeoutNeeded)
    {
      MY_DEBUG ("expected backoff end=" << expectedBackoffEnd);

      Time expectedBackoffDelay = expectedBackoffEnd - Simulator::Now ();///////移动到上面去
      if (m_accessTimeout.IsRunning ()
          && Simulator::GetDelayLeft (m_accessTimeout) > expectedBackoffDelay)
        {
          m_accessTimeout.Cancel ();
          static uint16_t y = 0;
          std::cout<<"isRunning"<<++y<<std::endl;
        }
      if (m_accessTimeout.IsExpired ())
        {
          m_accessTimeout = Simulator::Schedule (expectedBackoffDelay,
                                                 &DcfManager::AccessTimeout, this);
        }

    }
}

void
DcfManager::NotifyRxStartNow (Time duration)
{
  NS_LOG_FUNCTION (this << duration);
  MY_DEBUG ("rx start for=" << duration);
  UpdateBackoff ();
  m_lastRxStart = Simulator::Now ();
  m_lastRxDuration = duration;
  m_rxing = true;
}

void
DcfManager::NotifyRxEndOkNow (void)
{
  NS_LOG_FUNCTION (this);
  MY_DEBUG ("rx end ok");
  m_lastRxEnd = Simulator::Now ();
  m_lastRxReceivedOk = true;
  m_rxing = false;
}

void
DcfManager::NotifyRxEndErrorNow (void)
{
  NS_LOG_FUNCTION (this);
  MY_DEBUG ("rx end error");
  m_lastRxEnd = Simulator::Now ();
  m_lastRxReceivedOk = false;
  m_rxing = false;
}

void
DcfManager::NotifyTxStartNow (Time duration)
{
  NS_LOG_FUNCTION (this << duration);
  if (m_rxing)
    {
      if (Simulator::Now () - m_lastRxStart <= m_sifs)
        {
          //this may be caused if PHY has started to receive a packet
          //inside SIFS, so, we check that lastRxStart was within a SIFS ago
          m_lastRxEnd = Simulator::Now ();
          m_lastRxDuration = m_lastRxEnd - m_lastRxStart;
          m_lastRxReceivedOk = true;
          m_rxing = false;
        }
      else
        {
          // Bug 2477:  It is possible for the DCF to fall out of
          //            sync with the PHY state if there are problems
          //            receiving A-MPDUs.  PHY will cancel the reception
          //            and start to transmit
          NS_LOG_DEBUG ("Phy is transmitting despite DCF being in receive state");
          m_lastRxEnd = Simulator::Now ();
          m_lastRxDuration = m_lastRxEnd - m_lastRxStart;
          m_lastRxReceivedOk = false;
          m_rxing = false;
        }
    }

  MY_DEBUG ("tx start for " << duration);
  UpdateBackoff ();
  m_lastTxStart = Simulator::Now ();
  m_lastTxDuration = duration;
}

void
DcfManager::NotifyMaybeCcaBusyStartNow (Time duration)
{
  NS_LOG_FUNCTION (this << duration);
  MY_DEBUG ("busy start for " << duration);
  UpdateBackoff ();
  m_lastBusyStart = Simulator::Now ();
  m_lastBusyDuration = duration;
}

void
DcfManager::NotifySwitchingStartNow (Time duration)
{
  NS_LOG_FUNCTION (this << duration);
  Time now = Simulator::Now ();
  NS_ASSERT (m_lastTxStart + m_lastTxDuration <= now);
  NS_ASSERT (m_lastSwitchingStart + m_lastSwitchingDuration <= now);

  if (m_rxing)
    {
      //channel switching during packet reception
      m_lastRxEnd = Simulator::Now ();
      m_lastRxDuration = m_lastRxEnd - m_lastRxStart;
      m_lastRxReceivedOk = true;
      m_rxing = false;
    }
  if (m_lastNavStart + m_lastNavDuration > now)
    {
      m_lastNavDuration = now - m_lastNavStart;
    }
  if (m_lastBusyStart + m_lastBusyDuration > now)
    {
      m_lastBusyDuration = now - m_lastBusyStart;
    }
  if (m_lastAckTimeoutEnd > now)
    {
      m_lastAckTimeoutEnd = now;
    }
  if (m_lastCtsTimeoutEnd > now)
    {
      m_lastCtsTimeoutEnd = now;
    }

  //Cancel timeout
  if (m_accessTimeout.IsRunning ())
    {
      m_accessTimeout.Cancel ();
    }

  //Reset backoffs
  for (States::iterator i = m_states.begin (); i != m_states.end (); i++)
    {
      DcfState *state = *i;
      uint32_t remainingSlots = state->GetBackoffSlots ();
      if (remainingSlots > 0)
        {
          state->UpdateBackoffSlotsNow (remainingSlots, now);
          NS_ASSERT (state->GetBackoffSlots () == 0);
        }
      state->ResetCw ();
      state->m_accessRequested = false;
      state->NotifyChannelSwitching ();
    }

  MY_DEBUG ("switching start for " << duration);
  m_lastSwitchingStart = Simulator::Now ();
  m_lastSwitchingDuration = duration;

}

void
DcfManager::NotifySleepNow (void)
{
  NS_LOG_FUNCTION (this);
  m_sleeping = true;
  //Cancel timeout
  if (m_accessTimeout.IsRunning ())
    {
      m_accessTimeout.Cancel ();
    }

  //Reset backoffs
  for (States::iterator i = m_states.begin (); i != m_states.end (); i++)
    {
      DcfState *state = *i;
      state->NotifySleep ();
    }
}

void
DcfManager::NotifyWakeupNow (void)
{
  NS_LOG_FUNCTION (this);
  m_sleeping = false;
  for (States::iterator i = m_states.begin (); i != m_states.end (); i++)
    {
      DcfState *state = *i;
      uint32_t remainingSlots = state->GetBackoffSlots ();
      if (remainingSlots > 0)
        {
          state->UpdateBackoffSlotsNow (remainingSlots, Simulator::Now ());
          NS_ASSERT (state->GetBackoffSlots () == 0);
        }
      state->ResetCw ();
      state->m_accessRequested = false;
      state->NotifyWakeUp ();
    }
}

void
DcfManager::NotifyNavResetNow (Time duration)
{
  NS_LOG_FUNCTION (this << duration);
  MY_DEBUG ("nav reset for=" << duration);
  UpdateBackoff ();
  m_lastNavStart = Simulator::Now ();
  m_lastNavDuration = duration;
  /**
   * If the nav reset indicates an end-of-nav which is earlier
   * than the previous end-of-nav, the expected end of backoff
   * might be later than previously thought so, we might need
   * to restart a new access timeout.
   */
  DoRestartAccessTimeoutIfNeeded ();
}

void
DcfManager::NotifyNavStartNow (Time duration)
{
  NS_LOG_FUNCTION (this << duration);
  NS_ASSERT (m_lastNavStart <= Simulator::Now ());
  MY_DEBUG ("nav start for=" << duration);
  UpdateBackoff ();
  Time newNavEnd = Simulator::Now () + duration;
  Time lastNavEnd = m_lastNavStart + m_lastNavDuration;
  if (newNavEnd > lastNavEnd)
    {
      m_lastNavStart = Simulator::Now ();
      m_lastNavDuration = duration;
     // std::cout<<"888888888888888888888888888"<<std::endl;
    }
}

void
DcfManager::NotifyAckTimeoutStartNow (Time duration)
{
  NS_LOG_FUNCTION (this << duration);
  NS_ASSERT (m_lastAckTimeoutEnd < Simulator::Now ());
  m_lastAckTimeoutEnd = Simulator::Now () + duration;
}

void
DcfManager::NotifyAckTimeoutResetNow ()
{
  NS_LOG_FUNCTION (this);
  m_lastAckTimeoutEnd = Simulator::Now ();
  DoRestartAccessTimeoutIfNeeded ();
}

void
DcfManager::NotifyCtsTimeoutStartNow (Time duration)
{
  NS_LOG_FUNCTION (this << duration);
  m_lastCtsTimeoutEnd = Simulator::Now () + duration;
}

void
DcfManager::NotifyCtsTimeoutResetNow ()
{
  NS_LOG_FUNCTION (this);
  m_lastCtsTimeoutEnd = Simulator::Now ();
  DoRestartAccessTimeoutIfNeeded ();
}

}
} //namespace ns3
