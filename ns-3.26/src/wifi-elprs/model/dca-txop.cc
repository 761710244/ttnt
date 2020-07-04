/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2005 INRIA
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

#include "eps-dca-txop.h"
#include "eps-dcf-manager.h"
#include "eps-mac-low.h"
#include "eps-mac-tx-middle.h"
#include "eps-random-stream.h"
#include "eps-wifi-mac.h"
#include "eps-wifi-mac-queue.h"
#include "eps-wifi-mac-trailer.h"
#include "ns3/assert.h"
#include "ns3/packet.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/node.h"
#include "ns3/uinteger.h"
#include "ns3/pointer.h"
#include <fstream>
#include <set>
#include <fstream>


using namespace std;
#undef NS_LOG_APPEND_CONTEXT
#define NS_LOG_APPEND_CONTEXT if (m_low != 0) { std::clog << "[mac=" << m_low->GetAddress () << "] "; }

namespace ns3 {namespace elprs {

NS_LOG_COMPONENT_DEFINE ("epsDcaTxop");


//分配时隙 一共三类时隙 根据上层不同的数据类型选择不同的时隙分配
set<uint16_t> LTS5{/*1,9,17,25,33,41,49,57,65,73,81,89,97,105,113,121*/	 //  其实是LTS5 指挥控制 10
/*0,8,16,24,32,40,48,56,64,72,80,88,96,104,112,120*/
5,13,21,29,37,45,53,61,69,77,85,93,101,109,117,125};
set<uint16_t> LTS4{4,12,20,28,36,44,52,60,68,76,84,92,100,108,116,124 //  其实是LTS4 定位 150
/*,7,15,23,31,39,47,55,63,71,79,87,95,103,111,119,127*/};
set<uint16_t> LTS6{/*3,11,19,27,36,43,51,59,67,75,83,91,99,107,115,123*/ //lts3 300 //  其实是LTS6 图像视频 300
6,14,22,30,38,46,54,62,70,78,86,94,102,110,118,126};




class DcaTxop::Dcf : public DcfState
{
public:
  Dcf (DcaTxop * txop)
    : m_txop (txop)
  {
  }
  virtual bool IsEdca (void) const
  {
    return false;
  }
private:
  virtual void DoNotifyAccessGranted (void)
  {
    m_txop->NotifyAccessGranted ();
  }
  virtual void DoNotifyInternalCollision (void)
  {
    m_txop->NotifyInternalCollision ();
  }
  virtual void DoNotifyCollision (void)
  {
    m_txop->NotifyCollision ();
  }
  virtual void DoNotifyChannelSwitching (void)
  {
    m_txop->NotifyChannelSwitching ();
  }
  virtual void DoNotifySleep (void)
  {
    m_txop->NotifySleep ();
  }
  virtual void DoNotifyWakeUp (void)
  {
    m_txop->NotifyWakeUp ();
  }

  DcaTxop *m_txop;
};


/**
 * Listener for MacLow events. Forwards to DcaTxop.
 */
class DcaTxop::TransmissionListener : public MacLowTransmissionListener
{
public:
  /**
   * Create a TransmissionListener for the given DcaTxop.
   *
   * \param txop
   */
  TransmissionListener (DcaTxop * txop)
    : MacLowTransmissionListener (),
      m_txop (txop)
  {
  }

  virtual ~TransmissionListener ()
  {
  }

  virtual void GotCts (double snr, WifiMode txMode)
  {
    m_txop->GotCts (snr, txMode);
  }
  virtual void MissedCts (void)
  {
    m_txop->MissedCts ();
  }
  virtual void GotAck (double snr, WifiMode txMode)
  {
    m_txop->GotAck (snr, txMode);
  }
  virtual void MissedAck (void)
  {
    m_txop->MissedAck ();
  }
  virtual void StartNextFragment (void)
  {
    m_txop->StartNextFragment ();
  }
  virtual void StartNext (void)
  {
  }
  virtual void Cancel (void)
  {
    m_txop->Cancel ();
  }
  virtual void EndTxNoAck (void)
  {
    m_txop->EndTxNoAck ();
  }

private:
  DcaTxop *m_txop;
};

NS_OBJECT_ENSURE_REGISTERED (DcaTxop);

TypeId
DcaTxop::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::eps-DcaTxop")
    .SetParent<ns3::elprs::Dcf> ()
    .SetGroupName ("Wifi")
    .AddConstructor<DcaTxop> ()
    .AddAttribute ("Queue", "The WifiMacQueue object",
                   PointerValue (),
                   MakePointerAccessor (&DcaTxop::GetQueue),
                   MakePointerChecker<WifiMacQueue> ())
  ;
  return tid;
}

DcaTxop::DcaTxop ()
  : m_manager (0),
    m_currentPacket (0)
{
  NS_LOG_FUNCTION (this);
  m_transmissionListener = new DcaTxop::TransmissionListener (this);
  m_dcf = new DcaTxop::Dcf (this);
  m_queue = CreateObject<WifiMacQueue> ();
  m_rng = new RealRandomStream ();
}

DcaTxop::~DcaTxop ()
{
  NS_LOG_FUNCTION (this);
}

void
DcaTxop::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  m_queue = 0;
  m_low = 0;
  m_stationManager = 0;
  delete m_transmissionListener;
  delete m_dcf;
  delete m_rng;
  m_transmissionListener = 0;
  m_dcf = 0;
  m_rng = 0;
  m_txMiddle = 0;
}

void
DcaTxop::SetManager (DcfManager *manager)
{
  NS_LOG_FUNCTION (this << manager);
  m_manager = manager;
  m_manager->Add (m_dcf);
}

void DcaTxop::SetTxMiddle (MacTxMiddle *txMiddle)
{
  m_txMiddle = txMiddle;
}

void
DcaTxop::SetLow (Ptr<MacLow> low)
{
  NS_LOG_FUNCTION (this << low);
  m_low = low;
}

void
DcaTxop::SetWifiRemoteStationManager (Ptr<WifiRemoteStationManager> remoteManager)
{
  NS_LOG_FUNCTION (this << remoteManager);
  m_stationManager = remoteManager;
}

void
DcaTxop::SetTxOkCallback (TxOk callback)
{
  NS_LOG_FUNCTION (this << &callback);
  m_txOkCallback = callback;
}

void
DcaTxop::SetTxFailedCallback (TxFailed callback)
{
  NS_LOG_FUNCTION (this << &callback);
  m_txFailedCallback = callback;
}

Ptr<WifiMacQueue >
DcaTxop::GetQueue () const
{
  NS_LOG_FUNCTION (this);
  return m_queue;
}

void
DcaTxop::SetMinCw (uint32_t minCw)
{
  NS_LOG_FUNCTION (this << minCw);
  m_dcf->SetCwMin (minCw);
}

void
DcaTxop::SetMaxCw (uint32_t maxCw)
{
  NS_LOG_FUNCTION (this << maxCw);
  m_dcf->SetCwMax (maxCw);
}

void
DcaTxop::SetAifsn (uint32_t aifsn)
{
  NS_LOG_FUNCTION (this << aifsn);
  m_dcf->SetAifsn (aifsn);
}

void
DcaTxop::SetTxopLimit (Time txopLimit)
{
  NS_LOG_FUNCTION (this << txopLimit);
  m_dcf->SetTxopLimit (txopLimit);
  //std::cout<<"m_dcf->SetTxopLimit (txopLimit) = "<<txopLimit<<std::endl;
}

uint32_t
DcaTxop::GetMinCw (void) const
{
  NS_LOG_FUNCTION (this);
  return m_dcf->GetCwMin ();
}

uint32_t
DcaTxop::GetMaxCw (void) const
{
  NS_LOG_FUNCTION (this);
  return m_dcf->GetCwMax ();
}

uint32_t
DcaTxop::GetAifsn (void) const
{
  NS_LOG_FUNCTION (this);
  return m_dcf->GetAifsn ();
}

Time
DcaTxop::GetTxopLimit (void) const
{
  NS_LOG_FUNCTION (this);
  return m_dcf->GetTxopLimit ();
}

void
DcaTxop::Queue (Ptr<const Packet> packet, const WifiMacHeader &hdr)
{
  NS_LOG_FUNCTION (this << packet << &hdr);
  //static int cnt = 0;
  WifiMacTrailer fcs;
  m_stationManager->PrepareForQueue (hdr.GetAddr1 (), &hdr, packet);
//  std::cout<<"QUEUEUE TAg = "<<packet->GetTag()<<std::endl;
  m_queue->Enqueue (packet, hdr);
  //cnt++;
  StartAccessIfNeeded ();
  //std::cout<<"cnt = "<<cnt<<std::endl;

}

int64_t
DcaTxop::AssignStreams (int64_t stream)
{
  NS_LOG_FUNCTION (this << stream);
  m_rng->AssignStreams (stream);
  return 1;
}

void
DcaTxop::RestartAccessIfNeeded (void)
{
  NS_LOG_FUNCTION (this);
//  std::cout<<"@@@@@@@@@ m_dcf->IsAccessRequested () = "<<m_dcf->IsAccessRequested ()<<endl;
  if ((m_currentPacket != 0
       || !m_queue->IsEmpty ())
      && !m_dcf->IsAccessRequested ())
    {
      m_manager->RequestAccess (m_dcf);
    }
}

void
DcaTxop::StartAccessIfNeeded (void)
{
  NS_LOG_FUNCTION (this);
//static uint32_t aa = 0;
//std::cout<<"(m_currentPacket == 0)? = "<<(m_currentPacket == 0)<<" (!m_queue->IsEmpty ())? = "
//		<<(!m_queue->IsEmpty ())<<" (!m_dcf->IsAccessRequested ())? = "<<(!m_dcf->IsAccessRequested ())
//		<<" cccccoooppppptttTag : "<<std::endl;
  if (m_currentPacket == 0
      && !m_queue->IsEmpty ()
      && !m_dcf->IsAccessRequested ())
    {
//	  std::cout<<"DcaTxop::StartAccessIfNeeded = "<<++aa<<std::endl;
        m_manager->RequestAccess (m_dcf);

    }
}

Ptr<MacLow>
DcaTxop::Low (void)
{
  NS_LOG_FUNCTION (this);
  return m_low;
}

void
DcaTxop::DoInitialize ()
{
  NS_LOG_FUNCTION (this);
  m_dcf->ResetCw ();
  m_dcf->StartBackoffNow (m_rng->GetNext (0, m_dcf->GetCw ()));
  ns3::elprs::Dcf::DoInitialize ();
}

bool
DcaTxop::NeedRtsRetransmission (void)
{
  NS_LOG_FUNCTION (this);
  return m_stationManager->NeedRtsRetransmission (m_currentHdr.GetAddr1 (), &m_currentHdr,
                                                  m_currentPacket);
}

bool
DcaTxop::NeedDataRetransmission (void)
{
  NS_LOG_FUNCTION (this);
  return m_stationManager->NeedDataRetransmission (m_currentHdr.GetAddr1 (), &m_currentHdr,
                                                   m_currentPacket);
}

bool
DcaTxop::NeedFragmentation (void)
{
  NS_LOG_FUNCTION (this);
  return m_stationManager->NeedFragmentation (m_currentHdr.GetAddr1 (), &m_currentHdr,
                                              m_currentPacket);
}

void
DcaTxop::NextFragment (void)
{
  NS_LOG_FUNCTION (this);
  m_fragmentNumber++;
}

uint32_t
DcaTxop::GetFragmentSize (void)
{
  NS_LOG_FUNCTION (this);
  return m_stationManager->GetFragmentSize (m_currentHdr.GetAddr1 (), &m_currentHdr,
                                            m_currentPacket, m_fragmentNumber);
}

bool
DcaTxop::IsLastFragment (void)
{
  NS_LOG_FUNCTION (this);
  return m_stationManager->IsLastFragment (m_currentHdr.GetAddr1 (), &m_currentHdr,
                                           m_currentPacket, m_fragmentNumber);
}

uint32_t
DcaTxop::GetNextFragmentSize (void)
{
  NS_LOG_FUNCTION (this);
  return m_stationManager->GetFragmentSize (m_currentHdr.GetAddr1 (), &m_currentHdr,
                                            m_currentPacket, m_fragmentNumber + 1);
}

uint32_t
DcaTxop::GetFragmentOffset (void)
{
  NS_LOG_FUNCTION (this);
  return m_stationManager->GetFragmentOffset (m_currentHdr.GetAddr1 (), &m_currentHdr,
                                              m_currentPacket, m_fragmentNumber);
}

Ptr<Packet>
DcaTxop::GetFragmentPacket (WifiMacHeader *hdr)
{
  NS_LOG_FUNCTION (this << hdr);
  *hdr = m_currentHdr;
  hdr->SetFragmentNumber (m_fragmentNumber);
  uint32_t startOffset = GetFragmentOffset ();
  Ptr<Packet> fragment;
  if (IsLastFragment ())
    {
      hdr->SetNoMoreFragments ();
    }
  else
    {
      hdr->SetMoreFragments ();
    }
  fragment = m_currentPacket->CreateFragment (startOffset,
                                              GetFragmentSize ());
  return fragment;
}

bool
DcaTxop::NeedsAccess (void) const
{
  NS_LOG_FUNCTION (this);
  return !m_queue->IsEmpty () || m_currentPacket != 0;
}
void
DcaTxop::NotifyAccessGranted (void)
{
  NS_LOG_FUNCTION (this);
  if (m_currentPacket == 0)
    {
      if (m_queue->IsEmpty ())
        {
          NS_LOG_DEBUG ("queue empty");
          return;
        }
      m_currentPacket = m_queue->Dequeue (&m_currentHdr);
      NS_ASSERT (m_currentPacket != 0);
      uint16_t sequence = m_txMiddle->GetNextSequenceNumberfor (&m_currentHdr);
      m_currentHdr.SetSequenceNumber (sequence);
      m_stationManager->UpdateFragmentationThreshold ();
      m_currentHdr.SetFragmentNumber (0);
      m_currentHdr.SetNoMoreFragments ();
      m_currentHdr.SetNoRetry ();
      m_fragmentNumber = 0;
      NS_LOG_DEBUG ("dequeued size=" << m_currentPacket->GetSize () <<
                    ", to=" << m_currentHdr.GetAddr1 () <<
                    ", seq=" << m_currentHdr.GetSequenceControl ());
    }
  MacLowTransmissionParameters params;
  params.DisableOverrideDurationId ();
  if (m_currentHdr.GetAddr1 ().IsGroup ())
    {
      params.DisableRts ();
      params.DisableAck ();
      params.DisableNextData ();
      Low ()->StartTransmission (m_currentPacket,
                                 &m_currentHdr,
                                 params,
                                 m_transmissionListener);
      NS_LOG_DEBUG ("tx broadcast");
    }
  else
    {
	  //**************************************OD
	  if(m_currentHdr.d_IsType1())
	  {
		  params.DisableAck ();
	  }
	  else
      params.EnableAck ();
      //**************************************OD
      if (NeedFragmentation ())
        {
          WifiMacHeader hdr;
          Ptr<Packet> fragment = GetFragmentPacket (&hdr);
          if (IsLastFragment ())
            {
              NS_LOG_DEBUG ("fragmenting last fragment size=" << fragment->GetSize ());
              params.DisableNextData ();
            }
          else
            {
              NS_LOG_DEBUG ("fragmenting size=" << fragment->GetSize ());
              params.EnableNextData (GetNextFragmentSize ());
            }
          Low ()->StartTransmission (fragment, &hdr, params,
                                     m_transmissionListener);
        }
      else
        {
          params.DisableNextData ();
          Low ()->StartTransmission (m_currentPacket, &m_currentHdr,
                                     params, m_transmissionListener);
        }
    }
}

void
DcaTxop::NotifyInternalCollision (void)
{
  NS_LOG_FUNCTION (this);
  NotifyCollision ();
}

uint16_t
DcaTxop::GetAPPtype(void)
{
//	char buffer[3];
//	std::ifstream read_type;
//	read_type.open("AppType.txt");
//	read_type.getline(buffer, sizeof(buffer) - 1);
//	srand((unsigned) time(NULL)); //暂时通过采用随机数的方法获取上层数据类型，实际上应该交给上层，上层通过不同数据包长度确定传输类型
//	uint16_t randtype = rand() % 3;
//	return randtype;

	uint16_t app_type;
	ifstream read_type("AppType.txt");
	if(read_type.good())
	{
		read_type >> app_type;
	}
	else
	{
//		cout <<"Cannot Read AppType.txt\n";
	}
	return app_type;
	//return buffer[0]-'0';
}


void
DcaTxop::NotifyCollision (void)
{
	  NS_LOG_FUNCTION (this);
	  //////////////////////////////odie93?
	  uint16_t apptype = GetAPPtype();
//  	cout <<"DcaTxop::NotifyCollision apppppppppppp: "<<apptype<<endl;
	  Time smx = Simulator::Now();
	  uint64_t xnus = smx.GetMicroSeconds();
	  uint32_t slotNo = xnus / 20 % (128);
	  set<uint16_t>::iterator si;
	  uint16_t add = 0;
		switch (apptype) {
		case 0: {
//			std::cout<<"Apptype 0, slotNo "<<slotNo<<endl;
			uint16_t originSize1 =LTS4.size();
			LTS4.insert(slotNo);
			if(originSize1 != LTS4.size())
			{
				set<uint16_t>::iterator odItr1 = LTS4.find(slotNo);

					set<uint16_t>::iterator nextItr1;
					if(*odItr1 == *LTS4.rbegin())
					{
						nextItr1 = LTS4.begin();
						add=(128-*odItr1)+*nextItr1;
						LTS4.erase(slotNo);
					}
					else
						{
						nextItr1 = ++odItr1;
						add = *nextItr1 - slotNo;
						LTS4.erase(slotNo);
						}
					break;
			}
			else
			{
//				cout<<"Current Slot is Vaild!\n";
				set<uint16_t>::iterator kkit = LTS4.find(slotNo);
				set<uint16_t>::iterator kkitNext ;
				set<uint16_t>::iterator tmpItr = kkit;
				if(*kkit == *LTS4.rbegin())
				{
					kkitNext = LTS4.begin();
					add = 128-slotNo+*kkitNext;
				}
				else
				{
					kkitNext = ++kkit;
					add = *kkitNext - *tmpItr;
				}
				break;
			}
		}
		case 1: {

			//			std::cout<<"Apptype 0, slotNo "<<slotNo<<endl;
						uint16_t originSize1 =LTS6.size();
						LTS6.insert(slotNo);
						if(originSize1 != LTS6.size())
						{
							set<uint16_t>::iterator odItr1 = LTS6.find(slotNo);

								set<uint16_t>::iterator nextItr1;
								if(*odItr1 == *LTS6.rbegin())
								{
									nextItr1 = LTS6.begin();
									add=(128-*odItr1)+*nextItr1;
									LTS6.erase(slotNo);
								}
								else
									{
									nextItr1 = ++odItr1;
									add = *nextItr1 - slotNo;
									LTS6.erase(slotNo);
									}
								break;
						}
						else
						{
			//				cout<<"Current Slot is Vaild!\n";
							set<uint16_t>::iterator kkit = LTS6.find(slotNo);
							set<uint16_t>::iterator kkitNext ;
							set<uint16_t>::iterator tmpItr = kkit;
							if(*kkit == *LTS6.rbegin())
							{
								kkitNext = LTS6.begin();
								add = 128-slotNo+*kkitNext;
							}
							else
							{
								kkitNext = ++kkit;
								add = *kkitNext - *tmpItr;
							}
							break;
						}

		}
		case 2: {
			//			std::cout<<"Apptype 0, slotNo "<<slotNo<<endl;
						uint16_t originSize1 =LTS5.size();
						LTS5.insert(slotNo);
						if(originSize1 != LTS5.size())
						{
							set<uint16_t>::iterator odItr1 = LTS5.find(slotNo);

								set<uint16_t>::iterator nextItr1;
								if(*odItr1 == *LTS5.rbegin())
								{
									nextItr1 = LTS5.begin();
									add=(128-*odItr1)+*nextItr1;
									LTS5.erase(slotNo);
								}
								else
									{
									nextItr1 = ++odItr1;
									add = *nextItr1 - slotNo;
									LTS5.erase(slotNo);
									}
								break;
						}
						else
						{
			//				cout<<"Current Slot is Vaild!\n";
							set<uint16_t>::iterator kkit = LTS5.find(slotNo);
							set<uint16_t>::iterator kkitNext ;
							set<uint16_t>::iterator tmpItr = kkit;
							if(*kkit == *LTS5.rbegin())
							{
								kkitNext = LTS5.begin();
								add = 128-slotNo+*kkitNext;
							}
							else
							{
								kkitNext = ++kkit;
								add = *kkitNext - *tmpItr;
							}
							break;
						}
					}
		case 8:
		{
			//			std::cout<<"Apptype 0, slotNo "<<slotNo<<endl;
						uint16_t originSize1 =LTS6.size();
						LTS6.insert(slotNo);
						if(originSize1 != LTS6.size())
						{
							set<uint16_t>::iterator odItr1 = LTS6.find(slotNo);

								set<uint16_t>::iterator nextItr1;
								if(*odItr1 == *LTS6.rbegin())
								{
									nextItr1 = LTS6.begin();
									add=(128-*odItr1)+*nextItr1;
									LTS6.erase(slotNo);
								}
								else
									{
									nextItr1 = ++odItr1;
									add = *nextItr1 - slotNo;
									LTS6.erase(slotNo);
									}
								break;
						}
						else
						{
			//				cout<<"Current Slot is Vaild!\n";
							set<uint16_t>::iterator kkit = LTS6.find(slotNo);
							set<uint16_t>::iterator kkitNext ;
							set<uint16_t>::iterator tmpItr = kkit;
							if(*kkit == *LTS6.rbegin())
							{
								kkitNext = LTS6.begin();
								add = 128-slotNo+*kkitNext;
							}
							else
							{
								kkitNext = ++kkit;
								add = *kkitNext - *tmpItr;
							}
							break;
						}
		}
		default: {
//			cout << "error in dcfmanager request slot";
			break;
		}
		}
		static int addSum = 0;
		addSum += add;
		m_dcf->StartBackoffNow (100*add);
//	  m_dcf->StartBackoffNow (m_rng->GetNext (0, m_dcf->GetCw ()));
//	  std::cout<<"m_dcf->GetCw (): "<<m_dcf->GetCw ()<<std::endl;
	  RestartAccessIfNeeded ();
}


void
DcaTxop::NotifyChannelSwitching (void)
{
  NS_LOG_FUNCTION (this);
  m_queue->Flush ();
  m_currentPacket = 0;
}

void
DcaTxop::NotifySleep (void)
{
  NS_LOG_FUNCTION (this);
  if (m_currentPacket != 0)
    {
      m_queue->PushFront (m_currentPacket, m_currentHdr);
      m_currentPacket = 0;
    }
}

void
DcaTxop::NotifyWakeUp (void)
{
  NS_LOG_FUNCTION (this);
  RestartAccessIfNeeded ();
}

void
DcaTxop::GotCts (double snr, WifiMode txMode)
{
  NS_LOG_FUNCTION (this << snr << txMode);
  NS_LOG_DEBUG ("got cts");
}

void
DcaTxop::MissedCts (void)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_DEBUG ("missed cts");
  if (!NeedRtsRetransmission ())
    {
      NS_LOG_DEBUG ("Cts Fail");
      m_stationManager->ReportFinalRtsFailed (m_currentHdr.GetAddr1 (), &m_currentHdr);
      if (!m_txFailedCallback.IsNull ())
        {
          m_txFailedCallback (m_currentHdr);
        }
      //to reset the dcf.
      m_currentPacket = 0;
      m_dcf->ResetCw ();
    }
  else
    {
      m_dcf->UpdateFailedCw ();
    }
  m_dcf->StartBackoffNow (m_rng->GetNext (0, m_dcf->GetCw ()));
  RestartAccessIfNeeded ();
}

void
DcaTxop::GotAck (double snr, WifiMode txMode)
{
  NS_LOG_FUNCTION (this << snr << txMode);
  if (!NeedFragmentation ()
      || IsLastFragment ())
    {
      NS_LOG_DEBUG ("got ack. tx done.");
      if (!m_txOkCallback.IsNull ())
        {
          m_txOkCallback (m_currentHdr);
        }

      /* we are not fragmenting or we are done fragmenting
       * so we can get rid of that packet now.
       */
      m_currentPacket = 0;
      m_dcf->ResetCw ();
      m_dcf->StartBackoffNow (m_rng->GetNext (0, m_dcf->GetCw ()));
      RestartAccessIfNeeded ();
    }
  else
    {
      NS_LOG_DEBUG ("got ack. tx not done, size=" << m_currentPacket->GetSize ());
    }
}

void////////////////////////////////////////////*******************/////////////////////////////////****
DcaTxop::MissedAck (void)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_DEBUG ("missed ack");
//  std::cout<<"miss"<<std::endl;
  if (!NeedDataRetransmission ())
    {
      NS_LOG_DEBUG ("Ack Fail");
      m_stationManager->ReportFinalDataFailed (m_currentHdr.GetAddr1 (), &m_currentHdr);
      if (!m_txFailedCallback.IsNull ())
        {
          m_txFailedCallback (m_currentHdr);
        }
      //to reset the dcf.
      m_currentPacket = 0;
      m_dcf->ResetCw ();
    }
  else
    {
      NS_LOG_DEBUG ("Retransmit");
      m_currentHdr.SetRetry ();
      m_dcf->UpdateFailedCw ();
    }
  m_dcf->StartBackoffNow (m_rng->GetNext (0, m_dcf->GetCw ()));
//std::cout<<"ack cw = "<<m_dcf->GetCw()<<std::endl;
  RestartAccessIfNeeded ();
}

void
DcaTxop::StartNextFragment (void)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_DEBUG ("start next packet fragment");
  /* this callback is used only for fragments. */
  NextFragment ();
  WifiMacHeader hdr;
  Ptr<Packet> fragment = GetFragmentPacket (&hdr);
  MacLowTransmissionParameters params;
  params.EnableAck ();
  params.DisableRts ();
  params.DisableOverrideDurationId ();
  if (IsLastFragment ())
    {
      params.DisableNextData ();
    }
  else
    {
      params.EnableNextData (GetNextFragmentSize ());
    }
  Low ()->StartTransmission (fragment, &hdr, params, m_transmissionListener);
}

void
DcaTxop::Cancel (void)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_DEBUG ("transmission cancelled");
  /**
   * This happens in only one case: in an AP, you have two DcaTxop:
   *   - one is used exclusively for beacons and has a high priority.
   *   - the other is used for everything else and has a normal
   *     priority.
   *
   * If the normal queue tries to send a unicast data frame, but
   * if the tx fails (ack timeout), it starts a backoff. If the beacon
   * queue gets a tx oportunity during this backoff, it will trigger
   * a call to this Cancel function.
   *
   * Since we are already doing a backoff, we will get access to
   * the medium when we can, we have nothing to do here. We just
   * ignore the cancel event and wait until we are given again a
   * tx oportunity.
   *
   * Note that this is really non-trivial because each of these
   * frames is assigned a sequence number from the same sequence
   * counter (because this is a non-802.11e device) so, the scheme
   * described here fails to ensure in-order delivery of frames
   * at the receiving side. This, however, does not matter in
   * this case because we assume that the receiving side does not
   * update its <seq,ad> tupple for packets whose destination
   * address is a broadcast address.
   */
}

void
DcaTxop::EndTxNoAck (void)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_DEBUG ("a transmission that did not require an ACK just finished");
  m_currentPacket = 0;
  m_dcf->ResetCw ();
  //*********add TP for type1****************************************vv
  WifiMacHeader hdr;
  uint32_t d_duration=0;
  if(hdr.d_IsType1())
  {
	  d_duration=470/20;//duration/slot
  }
  //*************************************************vv
  m_dcf->StartBackoffNow (d_duration+m_rng->GetNext (0, m_dcf->GetCw ()));
  StartAccessIfNeeded ();
}

}
} //namespace ns3
