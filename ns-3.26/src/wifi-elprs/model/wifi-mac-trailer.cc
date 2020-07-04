/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2006 INRIA
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
#include "ns3/eps-wifi-mac-header.h"
#include<stdio.h>
#include<string>
#include<vector>
#include<iostream>
#include<bitset>
#include<sstream>

#include "eps-wifi-mac-trailer.h"
using namespace std;
namespace ns3 {namespace elprs {

NS_OBJECT_ENSURE_REGISTERED (WifiMacTrailer);

WifiMacTrailer::WifiMacTrailer ()
{
}

WifiMacTrailer::~WifiMacTrailer ()
{
}

TypeId
WifiMacTrailer::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::eps-WifiMacTrailer")
    .SetParent<Trailer> ()
    .SetGroupName ("Wifi")
    .AddConstructor<WifiMacTrailer> ()
  ;
  return tid;
}

TypeId
WifiMacTrailer::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

void
WifiMacTrailer::Print (std::ostream &os) const
{
}

uint32_t
WifiMacTrailer::GetSerializedSize (void) const
{
	return 5;//WIFI_MAC_FCS_LENGTH+flag   =4+1/////////////////////
  //return WIFI_MAC_FCS_LENGTH;
}

void
WifiMacTrailer::Serialize (Buffer::Iterator start) const
{
	  Buffer::Iterator v_iter=start;
	  start.Prev (5);//WIFI_MAC_FCS_LENGTH+flag   =4+1//////////////////
	  //uint3_t VV=start.ReadU64();
	  //std::cout<<std::hex<<VV<<"******"<<std::endl;
	  //start.Prev (WIFI_MAC_FCS_LENGTH);

	  //*****************************************************************
	  uint32_t d_buffer_size=start.GetSize();
	  stringstream oss;

	  if(d_buffer_size>40)
	  {
		  v_iter.Prev (d_buffer_size-33);
		  while(!v_iter.IsEnd())
		  {
			  uint32_t mm=v_iter.ReadU8();
			  oss<<mm;
		  }
		  string v_s=oss.str();
		  v_s=v_s.substr(0,v_s.size()-32);
	      uint8_t *d_pointer=(uint8_t*)&(v_s[0]);
	      uint32_t d_len1=v_s.size();

	      WifiMacHeader hdr;
	      uint32_t v_d_FCS= hdr.calculate_CRC32(d_pointer,d_len1);
	      start.WriteU32 (v_d_FCS);
		  //cout<<hex<<v_s<<"MMM";
	  }
	  else
		  start.WriteU32 (0xef);
	//*********************************************************************
	  /////////////////////////////////////////////////////////////////////////
	   // uint32_t size = start.GetSize();
	   // std::cout<<size<<"===";
	  // start.Prev (size);
	   // uint32_t mm= start.ReadLsbtohU32();
	    //std::cout<<std::hex<<mm<<"***"<<std::endl;
	   //uint8_t Tempbuffer[size];
	    //uint8_t *ptrTempbuffer;
	    //ptrTempbuffer = Tempbuffer;
	    //start.Read(ptrTempbuffer , size);
	    //std::cout<<size<<"******";
	  ////////////////////////////////////////////////////////////////////////////
	  start.WriteU8(0x7e);//add flag
}

uint32_t
WifiMacTrailer::Deserialize (Buffer::Iterator start)
{
	return 5;//WIFI_MAC_FCS_LENGTH+flag   =4+1///////////////////////
 // return WIFI_MAC_FCS_LENGTH;}
}
}
} //namespace ns3
