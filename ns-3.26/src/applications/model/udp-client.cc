/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2007,2008,2009 INRIA, UDCAST
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
 * Author: Amine Ismail <amine.ismail@sophia.inria.fr>
 *                      <amine.ismail@udcast.com>
 */
#include "ns3/log.h"
#include "ns3/ipv4-address.h"
#include "ns3/nstime.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "udp-client.h"
#include "seq-ts-header.h"
#include <cstdlib>
#include <cstdio>
#include "ns3/application-container.h"

#include "application-header.h"
#include "application-user-data.h"
#include "udp-trace-client.h"
#include <fstream>
#include <time.h>
#include <stdlib.h>
#include <iostream>

using namespace std;
namespace ns3 {

    NS_LOG_COMPONENT_DEFINE ("UdpClient");

    NS_OBJECT_ENSURE_REGISTERED (UdpClient);

    TypeId
    UdpClient::GetTypeId(void) {
        static TypeId tid = TypeId("ns3::UdpClient")
                .SetParent<Application>()
                .SetGroupName("Applications")
                .AddConstructor<UdpClient>()
                .AddAttribute("MaxPackets",
                              "The maximum number of packets the application will send",
                              UintegerValue(100),
                              MakeUintegerAccessor(&UdpClient::m_count),
                              MakeUintegerChecker<uint32_t>())
                .AddAttribute("Interval",
                              "The time to wait between packets", TimeValue(Seconds(1.0)),
                              MakeTimeAccessor(&UdpClient::m_interval),
                              MakeTimeChecker())
                .AddAttribute("RemoteAddress",
                              "The destination Address of the outbound packets",
                              AddressValue(),
                              MakeAddressAccessor(&UdpClient::m_peerAddress),
                              MakeAddressChecker())
                .AddAttribute("RemotePort", "The destination port of the outbound packets",
                              UintegerValue(100),
                              MakeUintegerAccessor(&UdpClient::m_peerPort),
                              MakeUintegerChecker<uint16_t>())
                .AddAttribute("PacketSize",
                              "Size of packets generated. The minimum packet size is 12 bytes which is the size of the header carrying the sequence number and the time stamp.",
                              UintegerValue(1024),
                              MakeUintegerAccessor(&UdpClient::m_size),
                              MakeUintegerChecker<uint32_t>(1, 50000))
                .AddTraceSource("Tx", "A new packet is created and is sent",
                                MakeTraceSourceAccessor(&UdpClient::m_txTrace),
                                "ns3::Packet::TracedCallback");
        return tid;
    }

    UdpClient::UdpClient() {
        NS_LOG_FUNCTION(this);
        m_sent = 0;
        m_socket = 0;
        m_sendEvent = EventId();
        randomFlag = 1;
        randomOnce = 1;
    }

    UdpClient::~UdpClient() {
        NS_LOG_FUNCTION(this);
    }

    void
    UdpClient::SetRemote(Address ip, uint16_t port) {
        NS_LOG_FUNCTION(this << ip << port);
        m_peerAddress = ip;
        m_peerPort = port;
    }

    void
    UdpClient::SetRemote(Address addr) {
        NS_LOG_FUNCTION(this << addr);
        m_peerAddress = addr;
    }

    void
    UdpClient::DoDispose(void) {
        NS_LOG_FUNCTION(this);
        Application::DoDispose();
    }

    void
    UdpClient::StartApplication(void) {
        NS_LOG_FUNCTION(this);

        if (m_socket == 0) {
            TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
            m_socket = Socket::CreateSocket(GetNode(), tid);
            if (Ipv4Address::IsMatchingType(m_peerAddress) == true) {
                m_socket->Bind();
                m_socket->Connect(InetSocketAddress(Ipv4Address::ConvertFrom(m_peerAddress), m_peerPort));
            } else if (Ipv6Address::IsMatchingType(m_peerAddress) == true) {
                m_socket->Bind6();
                m_socket->Connect(Inet6SocketAddress(Ipv6Address::ConvertFrom(m_peerAddress), m_peerPort));
            } else if (InetSocketAddress::IsMatchingType(m_peerAddress) == true) {
                m_socket->Bind();
                m_socket->Connect(m_peerAddress);
            } else if (Inet6SocketAddress::IsMatchingType(m_peerAddress) == true) {
                m_socket->Bind6();
                m_socket->Connect(m_peerAddress);
            } else {
                NS_ASSERT_MSG(false, "Incompatible address type: " << m_peerAddress);
            }
        }

        m_socket->SetRecvCallback(MakeNullCallback < void, Ptr < Socket > > ());
        m_socket->SetAllowBroadcast(true);

        //第一次启动后在多久后随机发业务
        if (0) {
            Ptr <UniformRandomVariable> uv = CreateObject<UniformRandomVariable>();
            double randomVal = uv->GetValue(0.0, 5.0);
            m_sendEvent = Simulator::Schedule(Seconds(randomVal), &UdpClient::Send, this);
        } else {
            m_sendEvent = Simulator::Schedule(Seconds(0.0), &UdpClient::Send, this);
        }
    }

    void
    UdpClient::StopApplication(void) {
        NS_LOG_FUNCTION(this);
        Simulator::Cancel(m_sendEvent);
    }

/*******************************************************************************************/
    void
    UdpClient::SetNodeID(uint32_t NodeID)   //添加一对Set和Get方法使得udp-client-server-helper.cc中的
    {                                        //NodeID可以传递给UdpClient对象中的成员变量m_NodeID
        m_NodeID = NodeID;                   //以便application-user-data.cc对定位字节读写时可以获得对应节点
    }

    uint32_t
    UdpClient::GetNodeID(void) {
        return m_NodeID;
    }

    void
    UdpClient::SetRandomFlag(uint16_t f) {
        randomFlag = f;
    }

    uint16_t
    UdpClient::GetRandomFlag(void) {
        return randomFlag;
    }

    void
    UdpClient::SetRandomOnce(uint16_t o) {
        randomOnce = o;
    }

    uint16_t
    UdpClient::GetRandomOnce(void) {
        return randomOnce;
    }

/*******************************************************************************************/

    void
    UdpClient::Send(void) {
        NS_LOG_FUNCTION(this);
        NS_ASSERT(m_sendEvent.IsExpired());
        SeqTsHeader seqTs;
        seqTs.SetSeq(m_sent);

        if (Simulator::Now() > Seconds(2.0)) {
            std::cout << std::endl;
        }
/*******************************************************************************************/
        uint32_t MessageNumber;
        uint32_t size_temp = m_size;

        if (m_size == 160)              // 512          //由于定位消息一般较短且需要定周期发送，因此对于小于512Byte的
        {                                       //用户数据在应用头中对应的bit位设置为0
            MessageNumber = 0;
            size_temp = 0;
        } else if (m_size == 300 || (m_size == 1024) || (m_size == 100) || (m_size == 800) ||
                   (m_size == 1499))       //    1024        //用于将来语音和图像应用的数据发送
        {
            MessageNumber = 1;
            size_temp = 0;
        } else if ((m_size == 10) || (m_size == 500) || (m_size == 50) || (m_size == 150))       // 1500
        {
            MessageNumber = 2;
            size_temp = 0;
        } else if (m_size == 2000) {
            MessageNumber = 1;
            size_temp = 0;
        } else if (m_size == 1) // This type is ready for building route
        {
            MessageNumber = 2;
            size_temp = 0;
        } else if (m_size == 80) {
            MessageNumber = 99;
            size_temp = 0;
        }

        std::ofstream write_type;
        write_type.open("AppType.txt");
        if (write_type.good()) {
            write_type << MessageNumber;
            write_type.close();
        } else {
            std::cout << "Open AppType.txt Failed ! " << std::endl;
        }

        Ptr <Packet> p;
        if (MessageNumber == 0) {
//		Ptr<UniformRandomVariable> uv = CreateObject<UniformRandomVariable>(); //定位信息加上随机数据
//		uint8_t randomArray[2000];
//		for(uint32_t i = 0; i < m_size; i++)
//		{
//			randomArray[i] = uv->GetInteger(0, 127);
//		}
//		p = Create<Packet> (randomArray, m_size); //m_size
            p = Create<Packet>(size_temp);
        } else {
            if (0) //随机指定
            {
                Ptr <UniformRandomVariable> uv = CreateObject<UniformRandomVariable>();
                uint8_t randomArray[2000];
                for (uint32_t i = 0; i < m_size; i++) {
                    randomArray[i] = uv->GetInteger(0, 127);
                }
                p = Create<Packet>(randomArray, m_size); //m_size
            } else {
                if (MessageNumber == 99) {
                    ifstream LoadFile;
                    LoadFile.open("maliciousMesseage.txt", std::ios::in);
                    uint8_t tmp[200];
                    memset(tmp, 0, sizeof(tmp));
                    for (uint32_t i = 0; i < m_size; i++) {
                        if (LoadFile) {
                            LoadFile >> tmp[i];
                        }
                    }
                    p = Create<Packet>(tmp, m_size); //m_size
                } else if (MessageNumber == 1) {
                    uint8_t m_Array[2000];
                    for (uint32_t i = 0; i < m_size; i++) {
                        m_Array[i] = (char) 0x1F;
                    }
                    p = Create<Packet>(m_Array, m_size); //m_size
                } else {
                    p = Create<Packet>(m_size); //m_size
                }
            }

        }

        time_t timeStamp = time(NULL);                                //将系统时间进行读取并写入应用头中
        tm *Time1 = localtime(&timeStamp);
        uint8_t time_year_val = 128 | Time1->tm_year;
        uint32_t time_month_val = 0;
        time_month_val += (Time1->tm_mon + 1) << 28;
        time_month_val += (Time1->tm_mday) << 23;
        time_month_val += (Time1->tm_hour) << 18;
        time_month_val += (Time1->tm_min) << 12;
        time_month_val += (Time1->tm_sec) << 6;
        ApplicationHeader AppHdr;
        AppHdr.SetVMFmessageIdentificationGroup_MessageNumber(MessageNumber);//4
        AppHdr.SetOriginatorDataTimeGroup_GPI_Year(time_year_val);//1
        AppHdr.SetOriginatorDataTimeGroup_Complementation(time_month_val);//4

        static uint32_t k = 0;
        if (UdpClient::m_peerPort == 39) {
            std::cout << "kkk = " << ++k << " Now ********* " << Simulator::Now() << std::endl;
        }
        AppHdr.SetPacketId(od_PID[UdpClient::m_peerPort]);
        od_PID[UdpClient::m_peerPort]++;
        p->SetTag(od_PID[UdpClient::m_peerPort] + 10000 * UdpClient::m_peerPort);//4

        ApplicationUserData AppUD;
        AppUD.SetNodeID(m_NodeID);    //将对应的NodeID传递给定位消息的发送端
        if (MessageNumber == 0) {
            p->AddHeader(AppUD);          //将包含有定位信息的负载加入packet
        }

        p->AddHeader(AppHdr);

/*************************************************************************************************/

        p->AddHeader(seqTs);

        std::stringstream peerAddressStringStream;
        //**********************************************ODD
        od_TimestampTag timestamp;
        timestamp.SetTimestamp(Simulator::Now());
        p->AddByteTag(timestamp);

        if (Ipv4Address::IsMatchingType(m_peerAddress)) {
            peerAddressStringStream << Ipv4Address::ConvertFrom(m_peerAddress);
        } else if (Ipv6Address::IsMatchingType(m_peerAddress)) {
            peerAddressStringStream << Ipv6Address::ConvertFrom(m_peerAddress);
        }
        static uint32_t a = 0;
        if ((m_socket->Send(p)) >= 0) {
            if (UdpClient::m_peerPort == 10)
                std::cout << "<------- UdpClient Send Count = " << ++a << std::endl;
            m_txTrace(p);
            ++m_sent;
            NS_LOG_INFO("TraceDelay TX " << m_size << " bytes to "
                                         << peerAddressStringStream.str() << " Uid: "
                                         << p->GetUid());

        } else {
            NS_LOG_INFO("Error while sending " << m_size << " bytes to "
                                               << peerAddressStringStream.str());
        }
        srand((unsigned) time(NULL));
        if (m_sent < m_count) {
//	  static int i = 0;
            Ptr <UniformRandomVariable> uv = CreateObject<UniformRandomVariable>();
            if (Simulator::Now() < Seconds(55.0)) // ##
            {
                m_sendEvent = Simulator::Schedule(m_interval, &UdpClient::Send, this);
            } else {
                // *****************************************发送随机时间，停止随机时间
                if (0) {
                    std::ofstream RCTxFile("RandomClientTx_Time.txt", std::ios::app);
                    if (randomFlag == 1) {
                        m_sendEvent = Simulator::Schedule(m_interval, &UdpClient::Send, this);
                        if (randomOnce == 1) {
                            double randomTime1 = uv->GetValue(5.0, 20.0); //发送时长随机范围 (5.0, 10.0)
                            if (RCTxFile.is_open()) {
                                std::cout << "randomTime1 = " << randomTime1 << std::endl;
                                RCTxFile << " randomTime_Tx = " << randomTime1
                                         << " UdpPort = " << UdpClient::m_peerPort;
                                RCTxFile.close();
                            }

                            Simulator::Schedule(Seconds(randomTime1), &UdpClient::SetRandomFlag, this, 0); //发多久 //运行一次
                            SetRandomOnce(0);
                        }
                    }
                    if (!randomFlag) {
                        double randomTime2 = uv->GetValue(2.0, 5.0); //停顿时长随机范围 (2.0, 5.0)
                        if (RCTxFile.is_open()) {
                            std::cout << "randomTime2 = " << randomTime2 << std::endl;
                            RCTxFile << " randomTime_Rx = " << randomTime2
                                     << " UdpPort = " << UdpClient::m_peerPort << std::endl;
                        }

                        m_sendEvent = Simulator::Schedule(Seconds(randomTime2), &UdpClient::Send, this); //停止发
                        SetRandomFlag(1);
                        SetRandomOnce(1);
                    }
                } else {
                    m_sendEvent = Simulator::Schedule(m_interval, &UdpClient::Send, this);
                }
                // ******************************************发送随机时间，停止随机时间
            }
        }
    }

} // Namespace ns3
