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
 *
 */

#ifndef UDP_SERVER_H
#define UDP_SERVER_H

#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/address.h"
#include "packet-loss-counter.h"
#include "ns3/stats-module.h"
#include <vector>
#include <set>

using namespace std;
namespace ns3 {
/**
 * \ingroup applications
 * \defgroup udpclientserver UdpClientServer
 */

/**
 * \ingroup udpclientserver
 *
 * \brief A UDP server, receives UDP packets from a remote host.
 *
 * UDP packets carry a 32bits sequence number followed by a 64bits time
 * stamp in their payloads. The application uses the sequence number
 * to determine if a packet is lost, and the time stamp to compute the delay.
 */
    class UdpServer : public Application {
    public:
        /**
         * \brief Get the type ID.
         * \return the object TypeId
         */
        static TypeId GetTypeId(void);

        UdpServer();

        virtual ~UdpServer();

        /**
         * \brief Returns the number of lost packets
         * \return the number of lost packets
         */
        uint32_t GetLost(void) const;

        /**
         * \brief Returns the number of received packets
         * \return the number of received packets
         */
        uint64_t GetReceived(void) const;

        /**
         * \brief Returns the size of the window used for checking loss.
         * \return the size of the window used for checking loss.
         */
        uint16_t GetPacketWindowSize() const;

        /**
         * \brief Set the size of the window used for checking loss. This value should
         *  be a multiple of 8
         * \param size the size of the window used for checking loss. This value should
         *  be a multiple of 8
         */
        void SetPacketWindowSize(uint16_t size);

        std::set <uint32_t> PidSet0;
        std::set <uint32_t> PidSet1;
        std::set <uint32_t> PidSet2;
        std::set <uint32_t> PidSet3;
        std::set <uint32_t> PidSet4;
        std::set <uint32_t> PidSet5;
        std::set <uint32_t> PidSet6;
        std::set <uint32_t> PidSet7;
        std::set <uint32_t> PidSet8;
        std::set <uint32_t> PidSet9;
        std::set <uint32_t> PidSet10;
        std::set <uint32_t> PidSet;
        std::set <uint32_t> PidSet21;
        std::set <uint32_t> PidSet22;
        std::set <uint32_t> PidSet23;
        std::set <uint32_t> PidSet24;
        std::set <uint32_t> PidSet25;
        std::set <uint32_t> PidSet26;
        std::set <uint32_t> PidSet27;
        std::set <uint32_t> PidSet28;
        std::set <uint32_t> PidSet29;
        std::set <uint32_t> PidSet30;
        std::set <uint32_t> PidSet31;
        std::set <uint32_t> PidSet32;
        std::set <uint32_t> PidSet33;
        std::set <uint32_t> PidSet34;
        std::set <uint32_t> PidSet35;
        std::set <uint32_t> PidSet36;
        std::set <uint32_t> PidSet37;
        std::set <uint32_t> PidSet38;
        std::set <uint32_t> PidSet39;
        std::set <uint32_t> PidSet40;
        std::set <uint32_t> PidSet41;
        std::set <uint32_t> PidSet42;
        std::set <uint32_t> PidSet43;
        std::set <uint32_t> PidSet44;
        std::set <uint32_t> PidSet45;
        std::set <uint32_t> PidSet46;
        std::set <uint32_t> PidSet47;
        std::set <uint32_t> PidSet48;
        std::set <uint32_t> PidSet49;
        std::set <uint32_t> PidSet50;
        std::set <uint32_t> PidSet20000;
        std::set <uint32_t> PidSet20001;

        static vector <uint32_t> packetSizeVec0;
        static vector <uint32_t> packetSizeVec1;
        static vector <uint32_t> packetSizeVec2;
        static vector <uint32_t> packetSizeVec3;
        static vector <uint32_t> packetSizeVec4;
        static vector <uint32_t> packetSizeVec5;
        static vector <uint32_t> packetSizeVec6;
        static vector <uint32_t> packetSizeVec7;
        static vector <uint32_t> packetSizeVec8;
        static vector <uint32_t> packetSizeVec9;
        static vector <uint32_t> packetSizeVec10;
        static vector <uint32_t> packetSizeVec;
        static vector <uint32_t> packetSizeVec21;
        static vector <uint32_t> packetSizeVec22;
        static vector <uint32_t> packetSizeVec23;
        static vector <uint32_t> packetSizeVec24;
        static vector <uint32_t> packetSizeVec25;
        static vector <uint32_t> packetSizeVec26;
        static vector <uint32_t> packetSizeVec27;
        static vector <uint32_t> packetSizeVec28;
        static vector <uint32_t> packetSizeVec29;
        static vector <uint32_t> packetSizeVec30;
        static vector <uint32_t> packetSizeVec31;
        static vector <uint32_t> packetSizeVec32;
        static vector <uint32_t> packetSizeVec33;
        static vector <uint32_t> packetSizeVec34;
        static vector <uint32_t> packetSizeVec35;
        static vector <uint32_t> packetSizeVec36;
        static vector <uint32_t> packetSizeVec37;
        static vector <uint32_t> packetSizeVec38;
        static vector <uint32_t> packetSizeVec39;
        static vector <uint32_t> packetSizeVec40;
        static vector <uint32_t> packetSizeVec41;
        static vector <uint32_t> packetSizeVec42;
        static vector <uint32_t> packetSizeVec43;
        static vector <uint32_t> packetSizeVec44;
        static vector <uint32_t> packetSizeVec45;
        static vector <uint32_t> packetSizeVec46;
        static vector <uint32_t> packetSizeVec47;
        static vector <uint32_t> packetSizeVec48;
        static vector <uint32_t> packetSizeVec49;
        static vector <uint32_t> packetSizeVec50;
        static vector <uint32_t> packetSizeVec20000;
        static vector <uint32_t> packetSizeVec20001;
        static uint32_t dirSuffix;  // Output file path suffix
        static void reInit(uint8_t typeNum, uint8_t busiNum, uint8_t hop, bool opti, bool optiType, bool routingOpt,
                           bool linkOpt);

        static void partitionInit(uint16_t testType, uint16_t partitionBitErrorRate, bool partitionOpti);

        static void MobilityPredict(uint16_t testType, bool Opti);

    protected:
        virtual void DoDispose(void);

    private:

        virtual void StartApplication(void);

        virtual void StopApplication(void);

        /**
         * \brief Handle a packet reception.
         *
         * This function is called by lower layers.
         *
         * \param socket the socket the packet was received to.
         */
        void HandleRead(Ptr <Socket> socket);

        double GetRealValue(uint16_t source, uint16_t destination);

        uint32_t purePacketSize;
        uint16_t m_port; //!< Port on which we listen for incoming packets.
        Ptr <Socket> m_socket; //!< IPv4 Socket
        Ptr <Socket> m_socket6; //!< IPv6 Socket
        uint64_t m_received; //!< Number of received packets
        PacketLossCounter m_lossCounter; //!< Lost packet counter
        TracedCallback <Ptr<const Packet>> m_rxTrace;
        TracedCallback <uint64_t> m_delay;

        void Performance(uint16_t hop);

        void Routing(bool RoutingOpti);

        void LinkError(bool LinkOpti);

        void partitionBitErrorRate(uint16_t bitErrorRate, bool opti);

        void mobilityPredict(bool Opti);

        vector<uint16_t> getSwitchPoint(uint16_t minCnt, uint16_t maxCnt);

        void routingSwitch(uint16_t minCnt, uint16_t maxCnt);

        vector <uint16_t> initPacket(uint16_t kind, uint16_t business);

        vector<double> getStandardThroughPut(vector <uint16_t> pktSize, uint16_t rate);

        vector <uint16_t> initWhich(uint16_t business, uint16_t neeToChange);

        double getKindBusinessTh(vector<double> throughput, uint16_t kind, uint16_t business);

        vector<double> solveThroughput(vector<double> throughput, uint16_t business);

        vector<double> getStandardDelay(vector <uint16_t> pktSize);

        uint16_t getTopValue(vector<double> throughPut, uint16_t kind, uint16_t business);

        double getDelayGate(vector<double> delay, uint16_t top);

        vector<double> solveDelay(vector<double> delay, uint16_t business, uint16_t top);

        vector <uint16_t> getReceivePackets(vector<double> standardTh, vector<double> solvedTh);
    };

} // namespace ns3

#endif /* UDP_SERVER_H */
