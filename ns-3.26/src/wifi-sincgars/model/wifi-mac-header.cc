/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2006, 2009 INRIA
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
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 * Author: Mirko Banchi <mk.banchi@gmail.com>
 */

#include "ns3/assert.h"
#include "ns3/address-utils.h"
#include "ns3/sinc-wifi-mac-header.h"
#include<stdio.h>
#include<string>
#include<vector>
#include<iostream>
#include<bitset>
#include<sstream>

namespace ns3 {namespace sincgars {
using namespace std;

NS_OBJECT_ENSURE_REGISTERED (WifiMacHeader);

enum
{
  TYPE_MGT = 0,
  TYPE_CTL  = 1,
  TYPE_DATA = 2
};

enum
{
  //Reserved: 0 - 6
  SUBTYPE_CTL_CTLWRAPPER = 7,
  SUBTYPE_CTL_BACKREQ = 8,
  SUBTYPE_CTL_BACKRESP = 9,
  SUBTYPE_CTL_RTS = 11,
  SUBTYPE_CTL_CTS = 12,
//*******************************OD
  SUBTYPE_CTL_ACK = 0x33
  //*********************************OD
};

WifiMacHeader::WifiMacHeader ()
  : m_ctrlMoreData (0),
    m_ctrlWep (0),
    m_ctrlOrder (0),
    m_amsduPresent (0)//QOS
{
}

WifiMacHeader::~WifiMacHeader ()
{
}

//==================================================================
const unsigned int crc32table[256] =
{
	0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA,   // 0x00
	0x076DC419, 0x706AF48F, 0xE963A535, 0x9E6495A3,   // 0x04
	0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988,   // 0x08
	0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91,   // 0x0C
	0x1DB71064, 0x6AB020F2, 0xF3B97148, 0x84BE41DE,   // 0x10
	0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7,   // 0x14
	0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC,   // 0x18
	0x14015C4F, 0x63066CD9, 0xFA0F3D63, 0x8D080DF5,   // 0x1C
	0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172,   // 0x20
	0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B,   // 0x24
	0x35B5A8FA, 0x42B2986C, 0xDBBBC9D6, 0xACBCF940,   // 0x28
	0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59,   // 0x2C
	0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116,   // 0x30
	0x21B4F4B5, 0x56B3C423, 0xCFBA9599, 0xB8BDA50F,   // 0x34
	0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924,   // 0x38
	0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D,   // 0x3C
	0x76DC4190, 0x01DB7106, 0x98D220BC, 0xEFD5102A,   // 0x40
	0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433,   // 0x44
	0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818,   // 0x48
	0x7F6A0DBB, 0x086D3D2D, 0x91646C97, 0xE6635C01,   // 0x4C
	0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E,   // 0x50
	0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457,   // 0x54
	0x65B0D9C6, 0x12B7E950, 0x8BBEB8EA, 0xFCB9887C,   // 0x58
	0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65,   // 0x5C
	0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2,   // 0x60
	0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB,   // 0x64
	0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0,   // 0x68
	0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9,   // 0x6C
	0x5005713C, 0x270241AA, 0xBE0B1010, 0xC90C2086,   // 0x70
	0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F,   // 0x74
	0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4,   // 0x78
	0x59B33D17, 0x2EB40D81, 0xB7BD5C3B, 0xC0BA6CAD,   // 0x7C
	0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A,   // 0x80
	0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683,   // 0x84
	0xE3630B12, 0x94643B84, 0x0D6D6A3E, 0x7A6A5AA8,   // 0x88
	0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,   // 0x8C
	0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE,   // 0x90
	0xF762575D, 0x806567CB, 0x196C3671, 0x6E6B06E7,   // 0x94
	0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC,   // 0x98
	0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5,   // 0x9C
	0xD6D6A3E8, 0xA1D1937E, 0x38D8C2C4, 0x4FDFF252,   // 0xA0
	0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B,   // 0xA4
	0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60,   // 0xA8
	0xDF60EFC3, 0xA867DF55, 0x316E8EEF, 0x4669BE79,   // 0xAC
	0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236,   // 0xB0
	0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F,   // 0xB4
	0xC5BA3BBE, 0xB2BD0B28, 0x2BB45A92, 0x5CB36A04,   // 0xB8
	0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D,   // 0xBC
	0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A,   // 0xC0
	0x9C0906A9, 0xEB0E363F, 0x72076785, 0x05005713,   // 0xC4
	0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38,   // 0xC8
	0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21,   // 0xCC
	0x86D3D2D4, 0xF1D4E242, 0x68DDB3F8, 0x1FDA836E,   // 0xD0
	0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777,   // 0xD4
	0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C,   // 0xD8
	0x8F659EFF, 0xF862AE69, 0x616BFFD3, 0x166CCF45,   // 0xDC
	0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2,   // 0xE0
	0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB,   // 0xE4
	0xAED16A4A, 0xD9D65ADC, 0x40DF0B66, 0x37D83BF0,   // 0xE8
	0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,   // 0xEC
	0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6,   // 0xF0
	0xBAD03605, 0xCDD70693, 0x54DE5729, 0x23D967BF,   // 0xF4
	0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94,   // 0xF8
	0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D,   // 0xFC
};

 uint32_t
WifiMacHeader::calculate_CRC32(uint8_t *pStart, uint32_t uSize)
{
	#define INIT  0xffffffff
	#define XOROT 0xffffffff
	uint32_t uCRCValue;
	uint8_t *pData;
	// init the start value
	uCRCValue = INIT;
	pData = pStart;
	// calculate CRC
	while (uSize--)
	{
		uCRCValue = crc32table[(uCRCValue ^ *pData++) & 0xFF] ^ (uCRCValue >> 8);
	}
	// XOR the output value
	return uCRCValue ^ XOROT;
}


void
WifiMacHeader::d_SetFEC (void)
{
	d_FEC=1;
}
void
WifiMacHeader::d_SetTDC (void)
{
	d_TDC=1;
}
void
WifiMacHeader::d_Setscramble (void)
{
	d_scramble=0;
}
void
WifiMacHeader::d_SetTversio (void)
{
	d_version=1;
}
void
WifiMacHeader::d_SetTranQue (void)
{
	d_TranQue=0x333;
}
void WifiMacHeader::d_SetFCS ()
{
	uint16_t d_mv=d_GetTransInfo();
	stringstream oss;
		        oss<<d_mv;
		        string s_temp=oss.str();
		        uint8_t *d_pointer=(uint8_t*)&(s_temp[0]);
		        uint32_t d_len1=s_temp.size();


	d_FCS=WifiMacHeader::calculate_CRC32(d_pointer,d_len1);

}


/*void WifiMacHeader::d_Setctl (void)
{
	d_ctl=0xee;
}

void
WifiMacHeader::d_Setinfo (void)
{
	//d_info=0xDD;
}*/

/*void
WifiMacHeader::d_SetAddr1 (Mac48Address address)
{
	d_addr1=address;
}
void
WifiMacHeader::d_SetAddr2 (Mac48Address address)
{
	d_addr2=address;
}
*/
uint16_t
WifiMacHeader::d_GetTransInfo (void) const
{
	uint16_t dval=0;
	dval|=(d_FEC) & (0x1);
	//std::cout<<"dval11: "<<dval<<std::endl;
	dval|=(d_TDC << 1) & (0x1 << 1);
	//std::cout<<"dval11: "<<dval<<std::endl;
	dval|=(d_scramble << 2) & (0x1 << 2);
	//std::cout<<"dval11: "<<dval<<std::endl;
	dval|=(d_version << 3) & (0x7 << 3);
	//std::cout<<"dval11: "<<dval<<std::endl;
	dval|=(m_duration<<6) & (0x3ff <<6);
	//std::cout<<"dval11: "<<dval<<std::endl;
	//std::cout<<"================================================== "<<std::endl;

	return dval;
}
void
WifiMacHeader::d_SetTransInfo (uint16_t ctrl)
{
	d_FEC=(ctrl)& (0x1);
	d_TDC=(ctrl>>1)&(0x1);
	d_scramble=(ctrl>>2)&(0x1);
	d_version=(ctrl>>3)&(0x7);
	m_duration=(ctrl>>6)&(0x3ff);
}

bool
WifiMacHeader::d_IsType1 (void) const
{
	return d_Type13==1;//3->type3 1->type1
}


void
WifiMacHeader::d_SetType (uint8_t typekind)
{
	d_Type13=typekind;
}


//==================================================================
void
WifiMacHeader::SetDsFrom (void)
{
  m_ctrlFromDs = 1;
}

void
WifiMacHeader::SetDsNotFrom (void)
{
  m_ctrlFromDs = 0;
}

void
WifiMacHeader::SetDsTo (void)
{
  m_ctrlToDs = 1;
}

void
WifiMacHeader::SetDsNotTo (void)
{
  m_ctrlToDs = 0;
}

void
WifiMacHeader::SetAddr1 (Mac48Address address)
{
  m_addr1 = address;
}

void
WifiMacHeader::SetAddr2 (Mac48Address address)
{
  m_addr2 = address;
}

void
WifiMacHeader::SetAddr3 (Mac48Address address)
{
  m_addr3 = address;
}

void
WifiMacHeader::SetAddr4 (Mac48Address address)
{
  m_addr4 = address;
}

void
WifiMacHeader::SetAssocReq (void)
{
  m_ctrlType = TYPE_MGT;
  m_ctrlSubtype = 0;
}

void
WifiMacHeader::SetAssocResp (void)
{
  m_ctrlType = TYPE_MGT;
  m_ctrlSubtype = 1;
}

void
WifiMacHeader::SetProbeReq (void)
{
  m_ctrlType = TYPE_MGT;
  m_ctrlSubtype = 4;
}

void
WifiMacHeader::SetProbeResp (void)
{
  m_ctrlType = TYPE_MGT;
  m_ctrlSubtype = 5;
}

void
WifiMacHeader::SetBeacon (void)
{
  m_ctrlType = TYPE_MGT;
  m_ctrlSubtype = 8;
}

void
WifiMacHeader::SetBlockAckReq (void)
{
  m_ctrlType = TYPE_CTL;
  m_ctrlSubtype = 8;
}

void
WifiMacHeader::SetBlockAck (void)
{
  m_ctrlType = TYPE_CTL;
  m_ctrlSubtype = 9;
}

void
WifiMacHeader::SetTypeData (void)
{

  m_ctrlType = TYPE_DATA;
  //******* Type1 UI COMMAND Control Field ******OD
  WifiMacHeader Od_Hdr;
  if(Od_Hdr.d_IsType1())
  {
	  m_ctrlSubtype = 0x3;
  }
  else
	  m_ctrlSubtype = 0x13;
  //*********************************************OD
}

void
WifiMacHeader::SetAction (void)
{
  m_ctrlType = TYPE_MGT;
  m_ctrlSubtype = 0x0D;
}

void
WifiMacHeader::SetMultihopAction (void)
{
  m_ctrlType = TYPE_MGT;
  m_ctrlSubtype = 0x0F;
}

void
WifiMacHeader::SetType (enum WifiMacType type)
{
  switch (type)
    {
    case WIFI_MAC_CTL_CTLWRAPPER:
      m_ctrlType = TYPE_CTL;
      m_ctrlSubtype = SUBTYPE_CTL_CTLWRAPPER;
      break;
    case WIFI_MAC_CTL_BACKREQ:
      m_ctrlType = TYPE_CTL;
      m_ctrlSubtype = SUBTYPE_CTL_BACKREQ;
      break;
    case WIFI_MAC_CTL_BACKRESP:
      m_ctrlType = TYPE_CTL;
      m_ctrlSubtype = SUBTYPE_CTL_BACKRESP;
      break;
    case WIFI_MAC_CTL_RTS:
      m_ctrlType = TYPE_CTL;
      m_ctrlSubtype = SUBTYPE_CTL_RTS;
      break;
    case WIFI_MAC_CTL_CTS:
      m_ctrlType = TYPE_CTL;
      m_ctrlSubtype = SUBTYPE_CTL_CTS;
      break;
    case WIFI_MAC_CTL_ACK:
      m_ctrlType = TYPE_CTL;
      m_ctrlSubtype = SUBTYPE_CTL_ACK;
      break;
    case WIFI_MAC_MGT_ASSOCIATION_REQUEST:
      m_ctrlType = TYPE_MGT;
      m_ctrlSubtype = 0;
      break;
    case WIFI_MAC_MGT_ASSOCIATION_RESPONSE:
      m_ctrlType = TYPE_MGT;
      m_ctrlSubtype = 1;
      break;
    case WIFI_MAC_MGT_REASSOCIATION_REQUEST:
      m_ctrlType = TYPE_MGT;
      m_ctrlSubtype = 2;
      break;
    case WIFI_MAC_MGT_REASSOCIATION_RESPONSE:
      m_ctrlType = TYPE_MGT;
      m_ctrlSubtype = 3;
      break;
    case WIFI_MAC_MGT_PROBE_REQUEST:
      m_ctrlType = TYPE_MGT;
      m_ctrlSubtype = 4;
      break;
    case WIFI_MAC_MGT_PROBE_RESPONSE:
      m_ctrlType = TYPE_MGT;
      m_ctrlSubtype = 5;
      break;
    case WIFI_MAC_MGT_BEACON:
      m_ctrlType = TYPE_MGT;
      m_ctrlSubtype = 8;
      break;
    case WIFI_MAC_MGT_DISASSOCIATION:
      m_ctrlType = TYPE_MGT;
      m_ctrlSubtype = 10;
      break;
    case WIFI_MAC_MGT_AUTHENTICATION:
      m_ctrlType = TYPE_MGT;
      m_ctrlSubtype = 11;
      break;
    case WIFI_MAC_MGT_DEAUTHENTICATION:
      m_ctrlType = TYPE_MGT;
      m_ctrlSubtype = 12;
      break;
    case WIFI_MAC_MGT_ACTION:
      m_ctrlType = TYPE_MGT;
      m_ctrlSubtype = 13;
      break;
    case WIFI_MAC_MGT_ACTION_NO_ACK:
      m_ctrlType = TYPE_MGT;
      m_ctrlSubtype = 14;
      break;
    case WIFI_MAC_MGT_MULTIHOP_ACTION:
      m_ctrlType = TYPE_MGT;
      m_ctrlSubtype = 15;
      break;
    case WIFI_MAC_DATA:
      m_ctrlType = TYPE_DATA;
      m_ctrlSubtype = 0x13;
      break;
    case WIFI_MAC_DATA_CFACK:
      m_ctrlType = TYPE_DATA;
      m_ctrlSubtype = 1;
      break;
    case WIFI_MAC_DATA_CFPOLL:
      m_ctrlType = TYPE_DATA;
      m_ctrlSubtype = 2;
      break;
    case WIFI_MAC_DATA_CFACK_CFPOLL:
      m_ctrlType = TYPE_DATA;
      m_ctrlSubtype = 3;
      break;
    case WIFI_MAC_DATA_NULL:
      m_ctrlType = TYPE_DATA;
      m_ctrlSubtype = 4;
      break;
    case WIFI_MAC_DATA_NULL_CFACK:
      m_ctrlType = TYPE_DATA;
      m_ctrlSubtype = 5;
      break;
    case WIFI_MAC_DATA_NULL_CFPOLL:
      m_ctrlType = TYPE_DATA;
      m_ctrlSubtype = 6;
      break;
    case WIFI_MAC_DATA_NULL_CFACK_CFPOLL:
      m_ctrlType = TYPE_DATA;
      m_ctrlSubtype = 7;
      break;
    case WIFI_MAC_QOSDATA:
      m_ctrlType = TYPE_DATA;
      m_ctrlSubtype = 8;
      break;
    case WIFI_MAC_QOSDATA_CFACK:
      m_ctrlType = TYPE_DATA;
      m_ctrlSubtype = 9;
      break;
    case WIFI_MAC_QOSDATA_CFPOLL:
      m_ctrlType = TYPE_DATA;
      m_ctrlSubtype = 10;
      break;
    case WIFI_MAC_QOSDATA_CFACK_CFPOLL:
      m_ctrlType = TYPE_DATA;
      m_ctrlSubtype = 11;
      break;
    case WIFI_MAC_QOSDATA_NULL:
      m_ctrlType = TYPE_DATA;
      m_ctrlSubtype = 12;
      break;
    case WIFI_MAC_QOSDATA_NULL_CFPOLL:
      m_ctrlType = TYPE_DATA;
      m_ctrlSubtype = 14;
      break;
    case WIFI_MAC_QOSDATA_NULL_CFACK_CFPOLL:
      m_ctrlType = TYPE_DATA;
      m_ctrlSubtype = 15;
      break;
    }
  m_ctrlToDs = 0;
  m_ctrlFromDs = 0;
}

void
WifiMacHeader::SetRawDuration (uint16_t duration)
{
  m_duration = duration;
}

void
WifiMacHeader::SetDuration (Time duration)
{
  int64_t duration_us = ceil ((double)duration.GetNanoSeconds () / 1000);
  NS_ASSERT (duration_us >= 0 && duration_us <= 0x7fff);
  m_duration = static_cast<uint16_t> (duration_us);
 // cout<<"m_duration: "<<m_duration<<endl;
}

void WifiMacHeader::SetId (uint16_t id)
{
  m_duration = id;
}

void WifiMacHeader::SetSequenceNumber (uint16_t seq)
{
  m_seqSeq = seq;
}

void WifiMacHeader::SetFragmentNumber (uint8_t frag)
{
  m_seqFrag = frag;
}

void WifiMacHeader::SetNoMoreFragments (void)
{
  m_ctrlMoreFrag = 0;
}

void WifiMacHeader::SetMoreFragments (void)
{
  m_ctrlMoreFrag = 1;
}

void WifiMacHeader::SetOrder (void)
{
  m_ctrlOrder = 1;
}

void WifiMacHeader::SetNoOrder (void)
{
  m_ctrlOrder = 0;
}

void WifiMacHeader::SetRetry (void)
{
  m_ctrlRetry = 1;
}

void WifiMacHeader::SetNoRetry (void)
{
  m_ctrlRetry = 0;
}

void WifiMacHeader::SetQosTid (uint8_t tid)
{
  m_qosTid = tid;
}

void WifiMacHeader::SetQosEosp ()
{
  m_qosEosp = 1;
}

void WifiMacHeader::SetQosNoEosp ()
{
  m_qosEosp = 0;
}

void WifiMacHeader::SetQosAckPolicy (enum QosAckPolicy policy)
{
  switch (policy)
    {
    case NORMAL_ACK:
      m_qosAckPolicy = 0;
      break;
    case NO_ACK:
      m_qosAckPolicy = 1;
      break;
    case NO_EXPLICIT_ACK:
      m_qosAckPolicy = 2;
      break;
    case BLOCK_ACK:
      m_qosAckPolicy = 3;
      break;
    }
}

void
WifiMacHeader::SetQosNormalAck ()
{
  m_qosAckPolicy = 0;
}

void
WifiMacHeader::SetQosBlockAck ()
{
  m_qosAckPolicy = 3;
}

void
WifiMacHeader::SetQosNoAck ()
{
  m_qosAckPolicy = 1;
}

void WifiMacHeader::SetQosAmsdu (void)
{
  m_amsduPresent = 1;
}

void WifiMacHeader::SetQosNoAmsdu (void)
{
  m_amsduPresent = 0;
}

void WifiMacHeader::SetQosTxopLimit (uint8_t txop)
{
  m_qosStuff = txop;
}

void WifiMacHeader::SetQosMeshControlPresent (void)
{
  //Mark bit 0 of this variable instead of bit 8, since m_qosStuff is
  //shifted by one byte when serialized
  m_qosStuff = m_qosStuff | 0x01; //bit 8 of QoS Control Field
}

void WifiMacHeader::SetQosNoMeshControlPresent ()
{
  //Clear bit 0 of this variable instead of bit 8, since m_qosStuff is
  //shifted by one byte when serialized
  m_qosStuff = m_qosStuff & 0xfe; //bit 8 of QoS Control Field
}


Mac48Address
WifiMacHeader::GetAddr1 (void) const
{
  return m_addr1;
}

Mac48Address
WifiMacHeader::GetAddr2 (void) const
{
  return m_addr2;
}

Mac48Address
WifiMacHeader::GetAddr3 (void) const
{
  return m_addr3;
}

Mac48Address
WifiMacHeader::GetAddr4 (void) const
{
  return m_addr4;
}

enum WifiMacType
WifiMacHeader::GetType (void) const
{
  switch (m_ctrlType)
    {
    case TYPE_MGT:
      switch (m_ctrlSubtype)
        {
        case 0:
          return WIFI_MAC_MGT_ASSOCIATION_REQUEST;
          break;
        case 1:
          return WIFI_MAC_MGT_ASSOCIATION_RESPONSE;
          break;
        case 2:
          return WIFI_MAC_MGT_REASSOCIATION_REQUEST;
          break;
        case 3:
          return WIFI_MAC_MGT_REASSOCIATION_RESPONSE;
          break;
        case 4:
          return WIFI_MAC_MGT_PROBE_REQUEST;
          break;
        case 5:
          return WIFI_MAC_MGT_PROBE_RESPONSE;
          break;
        case 8:
          return WIFI_MAC_MGT_BEACON;
          break;
        case 10:
          return WIFI_MAC_MGT_DISASSOCIATION;
          break;
        case 11:
          return WIFI_MAC_MGT_AUTHENTICATION;
          break;
        case 12:
          return WIFI_MAC_MGT_DEAUTHENTICATION;
          break;
        case 13:
          return WIFI_MAC_MGT_ACTION;
          break;
        case 14:
          return WIFI_MAC_MGT_ACTION_NO_ACK;
          break;
        case 15:
          return WIFI_MAC_MGT_MULTIHOP_ACTION;
          break;
        }
      break;
    case TYPE_CTL:
      switch (m_ctrlSubtype)
        {
        case SUBTYPE_CTL_BACKREQ:
          return WIFI_MAC_CTL_BACKREQ;
          break;
        case SUBTYPE_CTL_BACKRESP:
          return WIFI_MAC_CTL_BACKRESP;
          break;
        case SUBTYPE_CTL_RTS:
          return WIFI_MAC_CTL_RTS;
          break;
        case SUBTYPE_CTL_CTS:
          return WIFI_MAC_CTL_CTS;
          break;
        case SUBTYPE_CTL_ACK:
          return WIFI_MAC_CTL_ACK;
          break;
        }
      break;
    case TYPE_DATA:
      switch (m_ctrlSubtype)
        {
      //***********************************OD
        case 0x13:
      //***********************************OD
          return WIFI_MAC_DATA;
          break;
        case 1:
          return WIFI_MAC_DATA_CFACK;
          break;
        case 2:
          return WIFI_MAC_DATA_CFPOLL;
          break;
        case 3:
          return WIFI_MAC_DATA_CFACK_CFPOLL;
          break;
        case 4:
          return WIFI_MAC_DATA_NULL;
          break;
        case 5:
          return WIFI_MAC_DATA_NULL_CFACK;
          break;
        case 6:
          return WIFI_MAC_DATA_NULL_CFPOLL;
          break;
        case 7:
          return WIFI_MAC_DATA_NULL_CFACK_CFPOLL;
          break;
        case 8:
          return WIFI_MAC_QOSDATA;
          break;
        case 9:
          return WIFI_MAC_QOSDATA_CFACK;
          break;
        case 10:
          return WIFI_MAC_QOSDATA_CFPOLL;
          break;
        case 11:
          return WIFI_MAC_QOSDATA_CFACK_CFPOLL;
          break;
        case 12:
          return WIFI_MAC_QOSDATA_NULL;
          break;
        case 14:
          return WIFI_MAC_QOSDATA_NULL_CFPOLL;
          break;
        case 15:
          return WIFI_MAC_QOSDATA_NULL_CFACK_CFPOLL;
          break;
        }
      break;
    }
  // NOTREACHED
  NS_ASSERT (false);
  return (enum WifiMacType) -1;
}

bool
WifiMacHeader::IsFromDs (void) const
{
  return m_ctrlFromDs == 1;
}

bool
WifiMacHeader::IsToDs (void) const
{
  return m_ctrlToDs == 1;
}

bool
WifiMacHeader::IsData (void) const
{
  return (m_ctrlType == TYPE_DATA);

}

bool
WifiMacHeader::IsQosData (void) const
{
  return (m_ctrlType == TYPE_DATA && (m_ctrlSubtype & 0x08));
}

bool
WifiMacHeader::IsCtl (void) const
{
  return (m_ctrlType == TYPE_CTL);
}

bool
WifiMacHeader::IsMgt (void) const
{
  return (m_ctrlType == TYPE_MGT);
}

bool
WifiMacHeader::IsCfpoll (void) const
{
  switch (GetType ())
    {
    case WIFI_MAC_DATA_CFPOLL:
    case WIFI_MAC_DATA_CFACK_CFPOLL:
    case WIFI_MAC_DATA_NULL_CFPOLL:
    case WIFI_MAC_DATA_NULL_CFACK_CFPOLL:
    case WIFI_MAC_QOSDATA_CFPOLL:
    case WIFI_MAC_QOSDATA_CFACK_CFPOLL:
    case WIFI_MAC_QOSDATA_NULL_CFPOLL:
    case WIFI_MAC_QOSDATA_NULL_CFACK_CFPOLL:
      return true;
      break;
    default:
      return false;
      break;
    }
}

bool
WifiMacHeader::IsRts (void) const
{
  return (GetType () == WIFI_MAC_CTL_RTS);
}

bool
WifiMacHeader::IsCts (void) const
{
  return (GetType () == WIFI_MAC_CTL_CTS);
}

bool
WifiMacHeader::IsAck (void) const
{
  return (GetType () == WIFI_MAC_CTL_ACK);
}

bool
WifiMacHeader::IsAssocReq (void) const
{
  return (GetType () == WIFI_MAC_MGT_ASSOCIATION_REQUEST);
}

bool
WifiMacHeader::IsAssocResp (void) const
{
  return (GetType () == WIFI_MAC_MGT_ASSOCIATION_RESPONSE);
}

bool
WifiMacHeader::IsReassocReq (void) const
{
  return (GetType () == WIFI_MAC_MGT_REASSOCIATION_REQUEST);
}

bool
WifiMacHeader::IsReassocResp (void) const
{
  return (GetType () == WIFI_MAC_MGT_REASSOCIATION_RESPONSE);
}

bool
WifiMacHeader::IsProbeReq (void) const
{
  return (GetType () == WIFI_MAC_MGT_PROBE_REQUEST);
}

bool
WifiMacHeader::IsProbeResp (void) const
{
  return (GetType () == WIFI_MAC_MGT_PROBE_RESPONSE);
}

bool
WifiMacHeader::IsBeacon (void) const
{
  return (GetType () == WIFI_MAC_MGT_BEACON);
}

bool
WifiMacHeader::IsDisassociation (void) const
{
  return (GetType () == WIFI_MAC_MGT_DISASSOCIATION);
}

bool
WifiMacHeader::IsAuthentication (void) const
{
  return (GetType () == WIFI_MAC_MGT_AUTHENTICATION);
}

bool
WifiMacHeader::IsDeauthentication (void) const
{
  return (GetType () == WIFI_MAC_MGT_DEAUTHENTICATION);
}

bool
WifiMacHeader::IsAction (void) const
{
  return (GetType () == WIFI_MAC_MGT_ACTION);
}

bool
WifiMacHeader::IsMultihopAction (void) const
{
  return (GetType () == WIFI_MAC_MGT_MULTIHOP_ACTION);
}

bool
WifiMacHeader::IsBlockAckReq (void) const
{
  return (GetType () == WIFI_MAC_CTL_BACKREQ) ? true : false;
}

bool
WifiMacHeader::IsBlockAck (void) const
{
  return (GetType () == WIFI_MAC_CTL_BACKRESP) ? true : false;
}

uint16_t
WifiMacHeader::GetRawDuration (void) const
{
  return m_duration;
}

Time
WifiMacHeader::GetDuration (void) const
{
  return MicroSeconds (m_duration);
}

uint16_t
WifiMacHeader::GetSequenceControl (void) const
{
  return (m_seqSeq << 4) | m_seqFrag;
}

uint16_t
WifiMacHeader::GetSequenceNumber (void) const
{
  return m_seqSeq;
}

uint16_t
WifiMacHeader::GetFragmentNumber (void) const
{
  return m_seqFrag;
}

bool
WifiMacHeader::IsRetry (void) const
{
  return (m_ctrlRetry == 1);
}

bool
WifiMacHeader::IsMoreFragments (void) const
{
  return (m_ctrlMoreFrag == 1);
}

bool
WifiMacHeader::IsQosBlockAck (void) const
{
  NS_ASSERT (IsQosData ());
  return (m_qosAckPolicy == 3);
}

bool
WifiMacHeader::IsQosNoAck (void) const
{
  NS_ASSERT (IsQosData ());
  return (m_qosAckPolicy == 1);
}

bool
WifiMacHeader::IsQosAck (void) const
{
  NS_ASSERT (IsQosData ());
  return (m_qosAckPolicy == 0);
}

bool
WifiMacHeader::IsQosEosp (void) const
{
  NS_ASSERT (IsQosData ());
  return (m_qosEosp == 1);
}

bool
WifiMacHeader::IsQosAmsdu (void) const
{
  NS_ASSERT (IsQosData ());
  return (m_amsduPresent == 1);
}

uint8_t
WifiMacHeader::GetQosTid (void) const
{
  NS_ASSERT (IsQosData ());
  return m_qosTid;
}

enum WifiMacHeader::QosAckPolicy
WifiMacHeader::GetQosAckPolicy (void) const
{
  switch (m_qosAckPolicy)
    {
    case 0:
      return NORMAL_ACK;
      break;
    case 1:
      return NO_ACK;
      break;
    case 2:
      return NO_EXPLICIT_ACK;
      break;
    case 3:
      return BLOCK_ACK;
      break;
    }
  // NOTREACHED
  NS_ASSERT (false);
  return (enum QosAckPolicy) -1;
}

uint8_t
WifiMacHeader::GetQosTxopLimit (void) const
{
  NS_ASSERT (IsQosData ());
  return m_qosStuff;
}

uint8_t
WifiMacHeader::GetFrameControl (void) const
{
  uint8_t val = 0;
  //val |= (m_ctrlType << 2) & (0x3 << 2);
  val |= m_ctrlSubtype  &  0xff;
//  std::cout<<"FrameControl 0x"<<std::hex<<(uint16_t)val<<std::endl;
 /* val |= (m_ctrlToDs << 8) & (0x1 << 8);
  val |= (m_ctrlFromDs << 9) & (0x1 << 9);
  val |= (m_ctrlMoreFrag << 10) & (0x1 << 10);
  val |= (m_ctrlRetry << 11) & (0x1 << 11);
  val |= (m_ctrlMoreData << 13) & (0x1 << 13);
  val |= (m_ctrlWep << 14) & (0x1 << 14);
  val |= (m_ctrlOrder << 15) & (0x1 << 15);*/
  //std::cout<<"val = "<<std::hex<<(uint16_t)val<<std::endl;
  return val;
}

uint16_t
WifiMacHeader::GetQosControl (void) const
{
  uint16_t val = 0;
  val |= m_qosTid;
  val |= m_qosEosp << 4;
  val |= m_qosAckPolicy << 5;
  val |= m_amsduPresent << 7;
  val |= m_qosStuff << 8;
  return val;
}

void
WifiMacHeader::SetFrameControl (uint8_t ctrl)
{
	if(ctrl == 0x33)
	{
		m_ctrlType = 1;
	}
	else {
		m_ctrlType = 2;
	}
  m_ctrlSubtype = ctrl & 0xff;
/*  m_ctrlToDs = (ctrl >> 8) & 0x01;
  m_ctrlFromDs = (ctrl >> 9) & 0x01;
  m_ctrlMoreFrag = (ctrl >> 10) & 0x01;
  m_ctrlRetry = (ctrl >> 11) & 0x01;
  m_ctrlMoreData = (ctrl >> 13) & 0x01;
  m_ctrlWep = (ctrl >> 14) & 0x01;
  m_ctrlOrder = (ctrl >> 15) & 0x01;*/
}
void
WifiMacHeader::SetSequenceControl (uint16_t seq)
{
  m_seqFrag = seq & 0x0f;
  m_seqSeq = (seq >> 4) & 0x0fff;
}
void
WifiMacHeader::SetQosControl (uint16_t qos)
{
  m_qosTid = qos & 0x000f;
  m_qosEosp = (qos >> 4) & 0x0001;
  m_qosAckPolicy = (qos >> 5) & 0x0003;
  m_amsduPresent = (qos >> 7) & 0x0001;
  m_qosStuff = (qos >> 8) & 0x00ff;
}

uint32_t
WifiMacHeader::GetSize (void) const
{
  uint32_t size = 0;
  switch (m_ctrlType)
    {
    case TYPE_MGT:
      size = 2 + 2 + 6 + 6 + 6 + 2;
      break;
    case TYPE_CTL:
      switch (m_ctrlSubtype)
        {
        case SUBTYPE_CTL_RTS:
          size = 2 + 2 + 6 + 6;
          break;
        case SUBTYPE_CTL_CTS:
//        case SUBTYPE_CTL_ACK:
//          size = 2 + 2 + 6;
//          break;
//****************************************OD
        case SUBTYPE_CTL_ACK:
          size = 1+2+4+1+1+6+6+1;
          break;
//*****************************************OD
        case SUBTYPE_CTL_BACKREQ:
        case SUBTYPE_CTL_BACKRESP:
          size = 2 + 2 + 6 + 6;
          break;
        case SUBTYPE_CTL_CTLWRAPPER:
          size = 2 + 2 + 6 + 2 + 4;
          break;
        }
      break;
    case TYPE_DATA:
      size = 1+2+4+1+1+6+6+1;
      if (m_ctrlToDs && m_ctrlFromDs)
        {
          size += 6;
        }
      if (m_ctrlSubtype & 0x08)
        {
          size += 2;
        }
      break;
    }
  return size;
}

const char *
WifiMacHeader::GetTypeString (void) const
{
#define FOO(x) \
case WIFI_MAC_ ## x: \
  return # x; \
  break;

  switch (GetType ())
    {
      FOO (CTL_RTS);
      FOO (CTL_CTS);
      FOO (CTL_ACK);
      FOO (CTL_BACKREQ);
      FOO (CTL_BACKRESP);

      FOO (MGT_BEACON);
      FOO (MGT_ASSOCIATION_REQUEST);
      FOO (MGT_ASSOCIATION_RESPONSE);
      FOO (MGT_DISASSOCIATION);
      FOO (MGT_REASSOCIATION_REQUEST);
      FOO (MGT_REASSOCIATION_RESPONSE);
      FOO (MGT_PROBE_REQUEST);
      FOO (MGT_PROBE_RESPONSE);
      FOO (MGT_AUTHENTICATION);
      FOO (MGT_DEAUTHENTICATION);
      FOO (MGT_ACTION);
      FOO (MGT_ACTION_NO_ACK);
      FOO (MGT_MULTIHOP_ACTION);

      FOO (DATA);
      FOO (DATA_CFACK);
      FOO (DATA_CFPOLL);
      FOO (DATA_CFACK_CFPOLL);
      FOO (DATA_NULL);
      FOO (DATA_NULL_CFACK);
      FOO (DATA_NULL_CFPOLL);
      FOO (DATA_NULL_CFACK_CFPOLL);
      FOO (QOSDATA);
      FOO (QOSDATA_CFACK);
      FOO (QOSDATA_CFPOLL);
      FOO (QOSDATA_CFACK_CFPOLL);
      FOO (QOSDATA_NULL);
      FOO (QOSDATA_NULL_CFPOLL);
      FOO (QOSDATA_NULL_CFACK_CFPOLL);
    default:
      return "ERROR";
    }
#undef FOO
  // needed to make gcc 4.0.1 ppc darwin happy.
  return "BIG_ERROR";
}

TypeId
WifiMacHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::sinc-WifiMacHeader")
    .SetParent<Header> ()
    .SetGroupName ("Wifi")
    .AddConstructor<WifiMacHeader> ()
  ;
  return tid;
}

TypeId
WifiMacHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

void
WifiMacHeader::PrintFrameControl (std::ostream &os) const
{
  os << "ToDS=" << std::hex << (int) m_ctrlToDs << ", FromDS=" << std::hex << (int) m_ctrlFromDs
     << ", MoreFrag=" <<  std::hex << (int) m_ctrlMoreFrag << ", Retry=" << std::hex << (int) m_ctrlRetry
     << ", MoreData=" <<  std::hex << (int) m_ctrlMoreData << std::dec
  ;
}

void
WifiMacHeader::Print (std::ostream &os) const
{
  os << GetTypeString () << " ";
  switch (GetType ())
    {
    case WIFI_MAC_CTL_RTS:
      os << "Duration/ID=" << m_duration << "us"
         << ", RA=" << m_addr1 << ", TA=" << m_addr2;
      break;
    case WIFI_MAC_CTL_CTS:
    case WIFI_MAC_CTL_ACK:
      os << "Duration/ID=" << m_duration << "us"
         << ", RA=" << m_addr1;
      break;
    case WIFI_MAC_CTL_BACKREQ:
      break;
    case WIFI_MAC_CTL_BACKRESP:
      break;
    case WIFI_MAC_CTL_CTLWRAPPER:
      break;
    case WIFI_MAC_MGT_BEACON:
    case WIFI_MAC_MGT_ASSOCIATION_REQUEST:
    case WIFI_MAC_MGT_ASSOCIATION_RESPONSE:
    case WIFI_MAC_MGT_DISASSOCIATION:
    case WIFI_MAC_MGT_REASSOCIATION_REQUEST:
    case WIFI_MAC_MGT_REASSOCIATION_RESPONSE:
    case WIFI_MAC_MGT_PROBE_REQUEST:
    case WIFI_MAC_MGT_PROBE_RESPONSE:
    case WIFI_MAC_MGT_AUTHENTICATION:
    case WIFI_MAC_MGT_DEAUTHENTICATION:
      PrintFrameControl (os);
      os << " Duration/ID=" << m_duration << "us"
         << ", DA=" << m_addr1 << ", SA=" << m_addr2
         << ", BSSID=" << m_addr3 << ", FragNumber=" << std::hex << (int) m_seqFrag << std::dec
         << ", SeqNumber=" << m_seqSeq;
      break;
    case WIFI_MAC_MGT_ACTION:
    case WIFI_MAC_MGT_ACTION_NO_ACK:
      PrintFrameControl (os);
      os << " Duration/ID=" << m_duration << "us"
         << ", DA=" << m_addr1 << ", SA=" << m_addr2 << ", BSSID=" << m_addr3
         << ", FragNumber=" << std::hex << (int) m_seqFrag << std::dec << ", SeqNumber=" << m_seqSeq;
      break;
    case WIFI_MAC_MGT_MULTIHOP_ACTION:
      os << " Duration/ID=" << m_duration << "us"
         << ", RA=" << m_addr1 << ", TA=" << m_addr2 << ", DA=" << m_addr3
         << ", FragNumber=" << std::hex << (int) m_seqFrag << std::dec << ", SeqNumber=" << m_seqSeq;
      break;
    case WIFI_MAC_DATA:
      PrintFrameControl (os);
      os << " Duration/ID=" << m_duration << "us";
      if (!m_ctrlToDs && !m_ctrlFromDs)
        {
          os << ", DA=" << m_addr1 << ", SA=" << m_addr2 << ", BSSID=" << m_addr3;
        }
      else if (!m_ctrlToDs && m_ctrlFromDs)
        {
          os << ", DA=" << m_addr1 << ", SA=" << m_addr3 << ", BSSID=" << m_addr2;
        }
      else if (m_ctrlToDs && !m_ctrlFromDs)
        {
          os << ", DA=" << m_addr3 << ", SA=" << m_addr2 << ", BSSID=" << m_addr1;
        }
      else if (m_ctrlToDs && m_ctrlFromDs)
        {
          os << ", DA=" << m_addr3 << ", SA=" << m_addr4 << ", RA=" << m_addr1 << ", TA=" << m_addr2;
        }
      else
        {
          NS_FATAL_ERROR ("Impossible ToDs and FromDs flags combination");
        }
      os << ", FragNumber=" << std::hex << (int) m_seqFrag << std::dec
         << ", SeqNumber=" << m_seqSeq;
      break;
    case WIFI_MAC_DATA_CFACK:
    case WIFI_MAC_DATA_CFPOLL:
    case WIFI_MAC_DATA_CFACK_CFPOLL:
    case WIFI_MAC_DATA_NULL:
    case WIFI_MAC_DATA_NULL_CFACK:
    case WIFI_MAC_DATA_NULL_CFPOLL:
    case WIFI_MAC_DATA_NULL_CFACK_CFPOLL:
    case WIFI_MAC_QOSDATA:
    case WIFI_MAC_QOSDATA_CFACK:
    case WIFI_MAC_QOSDATA_CFPOLL:
    case WIFI_MAC_QOSDATA_CFACK_CFPOLL:
    case WIFI_MAC_QOSDATA_NULL:
    case WIFI_MAC_QOSDATA_NULL_CFPOLL:
    case WIFI_MAC_QOSDATA_NULL_CFACK_CFPOLL:
      break;
    }
}

uint32_t
WifiMacHeader::GetSerializedSize (void) const
{
  return GetSize ();
}

void
WifiMacHeader::Serialize (Buffer::Iterator i) const
{
//cout<<"serialize m_duration: "<<m_duration<<endl;

  /*if(m_ctrlType!=TYPE_DATA){///////////////////////////////////vvv
	  i.WriteU8 (GetFrameControl ());
	  i.WriteHtolsbU16 (m_duration);
	  WriteTo (i, m_addr1);
  }*/
  //**************************************************vvvv


	//******************************************OD
	if(m_ctrlType!=TYPE_DATA &&
			((m_ctrlSubtype!=SUBTYPE_CTL_ACK)))
	{
		i.WriteU8 (GetFrameControl ());
		i.WriteHtolsbU16 (m_duration);
		WriteTo (i, m_addr1);
	}
	//******************************************OD

  switch (m_ctrlType)
    {
    case TYPE_MGT:
      WriteTo (i, m_addr2);
      WriteTo (i, m_addr3);
      i.WriteHtolsbU16 (GetSequenceControl ());
      break;
    case TYPE_CTL:
      switch (m_ctrlSubtype)
        {
        case SUBTYPE_CTL_RTS:
          WriteTo (i, m_addr2);
          break;
        case SUBTYPE_CTL_CTS:
        case SUBTYPE_CTL_ACK:
//********* ACK Was in 220D ***************OD
        {
        	i.WriteU8(d_flag);
        	i.WriteHtolsbU16(d_GetTransInfo());
        	i.WriteHtolsbU32(d_FCS);
        	i.WriteU8(d_flag);
        	i.WriteU8(d_flag);
        	WriteTo (i, m_addr1);
        	WriteTo (i, m_addr2);
        	i.WriteU8 (GetFrameControl ());
        }
//******************************************OD
          break;
        case SUBTYPE_CTL_BACKREQ:
        case SUBTYPE_CTL_BACKRESP:
          WriteTo (i, m_addr2);
          break;
        default:
          //NOTREACHED
          NS_ASSERT (false);
          break;
        }
      break;
    case TYPE_DATA:
      {
         // cout<<"m_duration_de"<<m_duration<<endl;
        //i.WriteHtolsbU16 (GetSequenceControl ());
        //====================================================
        i.WriteU8(d_flag);
        i.WriteHtolsbU16(d_GetTransInfo());
        //cout<<"d_GetTransInfo: "<<d_GetTransInfo()<<endl;
        //*************
        //***************
        i.WriteHtolsbU32(d_FCS);

        i.WriteU8(d_flag);
        //====================================================
        i.WriteU8(d_flag);
        WriteTo (i, m_addr1);
        WriteTo (i, m_addr2);
        i.WriteU8 (GetFrameControl ());
        //cout<<"GetFrameControl: "<<bitset<8>((GetFrameControl()))<<endl;
        //====================================================
        //====================================================

        if (m_ctrlToDs && m_ctrlFromDs)
          {
            WriteTo (i, m_addr4);
          }
        if (m_ctrlSubtype & 0x08)
          {
            i.WriteHtolsbU16 (GetQosControl ());
          }
      } break;
    default:
      //NOTREACHED
      NS_ASSERT (false);
      break;
    }
//******************output total frame************************************************
	/*vector<uint8_t> v_d;
	while(!v_i.IsEnd())
	{
		v_d.push_back(v_i.ReadU8());

	}
	stringstream v_ss;
	for(uint32_t i=0;i<v_d.size();i++)
	{
		v_ss<<hex<<(uint16_t)v_d[i]<<"+";
	}
	string ss=v_ss.str();
	cout<<ss<<endl;*/
//***************************************************************
}

uint32_t
WifiMacHeader::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;
  Buffer::Iterator i_v = start;
  Buffer::Iterator i_v2 = start;
  //i_v.Next(2);
  uint8_t d_check =i_v.ReadU8();
 // cout<<"uint8_t d_check: "<<(uint16_t)d_check<<endl;

  if((uint16_t)d_check!=126)
  {
	  uint16_t frame_control = i.ReadU8 ();
	  SetFrameControl (frame_control);
	  m_duration = i.ReadLsbtohU16 ();
	  ReadFrom (i, m_addr1);
  }
  else
  {
	  //m_duration = i.ReadLsbtohU16 ();
	  i_v2.Next(21);
	  uint16_t frame_controld2 = i_v2.ReadU8 ();
	  SetFrameControl (frame_controld2);
  }

 // ReadFrom (i, m_addr1);

  switch (m_ctrlType)
    {
    case TYPE_MGT:
      ReadFrom (i, m_addr2);
      ReadFrom (i, m_addr3);
      SetSequenceControl (i.ReadLsbtohU16 ());
      break;
    case TYPE_CTL:
      switch (m_ctrlSubtype)
        {
        case SUBTYPE_CTL_RTS:
          ReadFrom (i, m_addr2);
          break;
        case SUBTYPE_CTL_CTS:
        case SUBTYPE_CTL_ACK:

//********* ACK Was in 220D ***************OD
        {
        d_flag = i.ReadU8();
        uint16_t traninfo_d=i.ReadLsbtohU16 ();;
        d_SetTransInfo(traninfo_d);
        d_FCS=i.ReadLsbtohU32();
        d_flag = i.ReadU8();
        d_flag = i.ReadU8();
        ReadFrom (i, m_addr1);
        ReadFrom (i, m_addr2);
        uint16_t frame_control_d = i.ReadU8 ();
        SetFrameControl (frame_control_d);
        }
//******************************************OD
          break;
        case SUBTYPE_CTL_BACKREQ:
        case SUBTYPE_CTL_BACKRESP:
          ReadFrom (i, m_addr2);
          break;
        }
      break;
    case TYPE_DATA:
     // SetSequenceControl (i.ReadLsbtohU16 ());
      //===================================================
      d_flag = i.ReadU8();
      uint16_t traninfo_d=i.ReadLsbtohU16 ();;
      d_SetTransInfo(traninfo_d);
      d_FCS=i.ReadLsbtohU32();
      d_flag = i.ReadU8();
      d_flag = i.ReadU8();
      ReadFrom (i, m_addr1);
      ReadFrom (i, m_addr2);


      //d_ctl=i.ReadU8();
      uint16_t frame_control_d = i.ReadU8 ();
      SetFrameControl (frame_control_d);



      if (m_ctrlToDs && m_ctrlFromDs)
        {
          ReadFrom (i, m_addr4);
        }
      if (m_ctrlSubtype & 0x08)
        {
          SetQosControl (i.ReadLsbtohU16 ());
        }
      break;
    }


  return i.GetDistanceFrom (start);
}

}
} //namespace ns3
