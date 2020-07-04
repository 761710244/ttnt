/*
 * application-user-data.cpp
 *
 *  Created on: May 24, 2018
 *      Author: root
 */

#include "application-user-data.h"
#include <bitset>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdio.h>
using namespace std;
namespace ns3 {

ApplicationUserData::ApplicationUserData()
{
}

ApplicationUserData::~ApplicationUserData()
{
}


TypeId
ApplicationUserData::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::ApplicationUserData")
    .SetParent<Header> ()
    .SetGroupName("Applications")
    .AddConstructor<ApplicationUserData> ();
  return tid;
}

TypeId
ApplicationUserData::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
ApplicationUserData::GetSize (void) const
{
	ifstream infile ("NodesPosBuff/1.txt", ios::in);
		      char ch;
		      int StringCounter = 0;
		      while(infile.get(ch))
		      {
		    	  StringCounter++;
		      }
		infile.close();
	  uint32_t size = StringCounter;
	  return size+10/* 20 */;
}


void
ApplicationUserData::Print (std::ostream &os) const
{
}

uint32_t
ApplicationUserData::GetSerializedSize (void) const
{
  return GetSize ();
}

void
ApplicationUserData::SetNodeID (uint32_t NodeID)
{
	m_NodeID = NodeID;
}
uint32_t
ApplicationUserData::GetNodeID (void)
{
	return m_NodeID;
}

void
ApplicationUserData::Serialize (Buffer::Iterator i) const
{
	  stringstream FileNameStream;
	  FileNameStream << "NodesPosBuff/" << m_NodeID << ".txt";
	  ifstream infile (FileNameStream.str(), ios::in);
	  if(!infile.good())
	  {
		  std::cout<<"no pos "<<FileNameStream.str()<<std::endl;
	  }
	  char ch;
	  while(infile.get(ch))
		  {
			  i.WriteU8(ch);
		  }
	  infile.close();
}

uint32_t
ApplicationUserData::Deserialize (Buffer::Iterator start)
{
	  Buffer::Iterator i = start;
	  char ch;
	  stringstream FileNameStream;
	  FileNameStream << "NodesPosBuff/" << m_NodeID << ".txt";
	  ofstream outfile (FileNameStream.str(), ios::out);
	  while (!i.IsEnd())
		  {
			  ch = i.ReadU8();
			  outfile << ch;
		  }
	  outfile.close();
	  return i.GetDistanceFrom (start);
}

}/* namespace ns3 */
