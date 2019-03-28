/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2016 University of Connecticut
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
 * Author: Robert Martin <robert.martin@engr.uconn.edu>
 */

#include "ns3/log.h"
#include "ns3/ptr.h"
#include "ns3/pointer.h"
#include "ns3/simulator.h"
#include "ns3/double.h"

#include "aqua-sim-channel.h"
#include "aqua-sim-header.h"
#include "aqua-sim-header-routing.h"

#include <cstdio>
#include <fstream>

#define FLOODING_TEST 0

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("AquaSimChannel");
NS_OBJECT_ENSURE_REGISTERED (AquaSimChannel);

AquaSimChannel::AquaSimChannel ()
{
  NS_LOG_FUNCTION(this);
  m_deviceList.clear();
  allPktCounter=0;
  sentPktCounter=0;
  allRecvPktCounter=0;
}

AquaSimChannel::~AquaSimChannel ()
{
}

TypeId
AquaSimChannel::GetTypeId ()
{
  static TypeId tid = TypeId("ns3::AquaSimChannel")
    .SetParent<Channel> ()
    .AddConstructor<AquaSimChannel> ()
    .AddAttribute ("SetProp", "A pointer to set the propagation model.",
       PointerValue (0),
       MakePointerAccessor (&AquaSimChannel::m_prop),
       MakePointerChecker<AquaSimPropagation> ())
    .AddAttribute ("SetNoise", "A pointer to set the noise generator.",
       PointerValue (0),
       MakePointerAccessor (&AquaSimChannel::m_noiseGen),
       MakePointerChecker<AquaSimNoiseGen> ())
    ;
  return tid;
}

void
AquaSimChannel::SetNoiseGenerator (Ptr<AquaSimNoiseGen> noiseGen)
{
  NS_LOG_FUNCTION(this);
  NS_ASSERT (noiseGen);
  m_noiseGen = noiseGen;
}

void
AquaSimChannel::SetPropagation (Ptr<AquaSimPropagation> prop)
{
  NS_LOG_FUNCTION(this);
  NS_ASSERT (prop);
  m_prop = prop;
}

Ptr<NetDevice>
AquaSimChannel::GetDevice (uint32_t i) const
{
  return m_deviceList[i];
}

uint32_t
AquaSimChannel::GetId (void) const
{
  NS_LOG_INFO("AquaSimChannel::GetId not implemented");
  return 0;
}

uint32_t
AquaSimChannel::GetNDevices (void) const
{
  return m_deviceList.size();
}

void
AquaSimChannel::AddDevice (Ptr<AquaSimNetDevice> device)
{
  NS_LOG_FUNCTION(this);
  m_deviceList.push_back(device);
}

void
AquaSimChannel::RemoveDevice(Ptr<AquaSimNetDevice> device)
{
  NS_LOG_FUNCTION(this);
  if (m_deviceList.empty())
    NS_LOG_DEBUG("AquaSimChannel::RemoveDevice: deviceList is empty");
  else
  {
    std::vector<Ptr<AquaSimNetDevice> >::iterator it = m_deviceList.begin();
    for(; it != m_deviceList.end(); ++it)
      {
        if(*it == device)
          {
            m_deviceList.erase(it);
          }
      }
  }
}

bool
AquaSimChannel::Recv(Ptr<Packet> p, Ptr<AquaSimPhy> phy)
{
  /*std::cout << "\nChannel: @Recv check:\n";
  p->Print(std::cout);
  std::cout << "\n";*/

  NS_LOG_FUNCTION(this << p << phy);
  NS_ASSERT(p != NULL || phy != NULL);
  return SendUp(p,phy);
}

bool
AquaSimChannel::SendUp (Ptr<Packet> p, Ptr<AquaSimPhy> tifp)
{
  NS_LOG_FUNCTION(this);
  NS_LOG_DEBUG("Packet:" << p << " Phy:" << tifp << " Channel:" << this);

  Ptr<AquaSimNetDevice> sender = Ptr<AquaSimNetDevice>(tifp->GetNetDevice());
  Ptr<AquaSimNetDevice> recver;
  //std::vector<Ptr<AquaSimPhy> > rifp;	//must support multiple recv phy in future
  Ptr<AquaSimPhy> rifp;
  //Ptr<Packet> pCopy;
  Time pDelay;
  /*
  if(!m_sorted){
    SortLists();
  }
  */

  std::vector<PktRecvUnit> * recvUnits = m_prop->ReceivedCopies(sender, p, m_deviceList);

  allPktCounter++;  //Debug... remove
  for (std::vector<PktRecvUnit>::size_type i = 0; i < recvUnits->size(); i++) {
    allRecvPktCounter++;  //Debug .. remove
    if (sender == (*recvUnits)[i].recver)
    {
      continue;
    }

    //TODO remove ... this is a local addition for flooding_test.
    if (FLOODING_TEST && (Distance(sender, (*recvUnits)[i].recver) > Distance((*recvUnits)[0].recver,(*recvUnits)[1].recver)*1.25/*arbitrary*/))
      {
        NS_LOG_DEBUG("Channel:SendUp: FloodTest(OutOfRange): sender(" << sender->GetAddress() << ") recver:(" <<  (*recvUnits)[i].recver->GetAddress() << ") dist(" << Distance(sender, (*recvUnits)[i].recver) << ")");
        continue;
      }

    sentPktCounter++; //Debug... remove

    recver = (*recvUnits)[i].recver;
    pDelay = GetPropDelay(sender, (*recvUnits)[i].recver);
    //pDelay = (*recvUnits)[i].pDelay;
    rifp = recver->GetPhy();
    //rifp = recver->ifhead().lh_first;

    AquaSimPacketStamp pstamp;
    AquaSimHeader asHeader;
    p->RemoveHeader(pstamp);
    p->RemoveHeader(asHeader);

    pstamp.SetPr((*recvUnits)[i].pR);
    pstamp.SetNoise(m_noiseGen->Noise((Simulator::Now() + pDelay), (GetMobilityModel(recver)->GetPosition())));
    asHeader.SetDirection(AquaSimHeader::UP);
    asHeader.SetTxTime(pDelay);

    p->AddHeader(asHeader);
    p->AddHeader(pstamp);

    /**
     * Send to each interface a copy, and we will filter the packet
     * in physical layer according to freq and modulation
     */
    NS_LOG_DEBUG ("Channel. NodeS:" << sender->GetAddress() << " NodeR:" << recver->GetAddress() << " S.Phy:" << sender->GetPhy() << " R.Phy:" << recver->GetPhy() << " packet:" << p
		  << " TxTime:" << asHeader.GetTxTime() << pDelay);

    Simulator::Schedule(pDelay, &AquaSimPhy::Recv, rifp, p->Copy());

    /* TODO in future support multiple phy with below code.
     *
     * for (std::vector<Ptr<AquaSimPhy> >::iterator it = rifp.begin(); it!=rifp.end(); it++) {
     *	 pCopy = p->Copy();
     *   Simulator::Schedule(pDelay, it, &pCopy);
     * }
     *
     */
  }

  p = 0; //smart pointer will unref automatically once out of scope
  delete recvUnits;
  return true;
}

void
AquaSimChannel::PrintCounters()
{
  std::cout << "Channel Counters= SendUpFromChannel(" << allPktCounter << ") AllRecvers(should be =n*sendup)("
            << allRecvPktCounter << ") SchedPhyRecv(" << sentPktCounter << ")\n";


  //****gather total amount of messages sent
  int totalSentPkts =0;
  std::cout << "SentPkts(Source_NetDevice->Stack):\n";
  for (std::vector<Ptr<AquaSimNetDevice> >::iterator it = m_deviceList.begin(); it != m_deviceList.end(); ++it)
  {
    totalSentPkts += (*it)->TotalSentPkts();
    //std::cout << " (" << (*it)->GetAddress() << ") " << (*it)->TotalSentPkts() << "\n";
  }
  std::cout << " (NetworkTotal) " << totalSentPkts << "\n";

  //*****gather specific net device receival amount
  int totalSendUpPkts =0;
  std::set<int> totalSendupPktsRecord;
  std::cout << "SendUp Pkts(Sink_RoutingLayer):\n";
  for (std::vector<Ptr<AquaSimNetDevice> >::iterator it = m_deviceList.begin(); it != m_deviceList.end(); ++it)
  {
    totalSendUpPkts += (*it)->GetRouting()->SendUpPktCount();
    std::set<int> tempRecord=(*it)->GetRouting()->SendUpPktsetMerg();
    totalSendupPktsRecord.insert(tempRecord.begin(),tempRecord.end());
    //std::cout << " (" << (*it)->GetAddress() << ") " << (*it)->GetRouting()->SendUpPktCount() << "\n";
  }
  std::cout << " (NetworkTotal) " << totalSendUpPkts << "\n";
  std::cout << " (NetworkTotal) " << totalSendupPktsRecord.size() << "\n";
  int totalRecvPkts = 0;
  std::cout << "Recv Pkts(@PhyLayer):\n";
  for (std::vector<Ptr<AquaSimNetDevice> >::iterator it = m_deviceList.begin(); it != m_deviceList.end(); ++it)
  {
    totalRecvPkts += (*it)->GetPhy()->PktRecvCount();
    //std::cout << " (" << (*it)->GetAddress() << ") " << (*it)->GetPhy()->PktRecvCount() << "\n";
  }
  std::cout << " (NetworkTotal) " << totalRecvPkts << "\n";

  std::cout << "Packet Deliver Ratio = " << (double(totalSendupPktsRecord.size())/totalSentPkts)* 100 << " %\n";

  //****gather number of forwards for each pkt if possible.
  //possible look at phy-cmn layer namely AquaSimPhyCmn::PktTransmit()
}

/*
  While outdated due to the use of tracers within Routing layer, this is left
  due to potential legacy issues.
*/
void
AquaSimChannel::FilePrintCounters(double simTime, int attSlot)
{
  NS_LOG_FUNCTION(this << "Does not implement anything.");

  //output layerout:
  // SimTime | N0_Pkts | N0_Bytes | N0_DiffFromLastTrace | N1_Pkts | ... | Sink_Sent | Sink_Recv | Att0_Sent

   //out of date file creation & data collection.
  /*
  std::ofstream csvFile;
  csvFile.open("static_grid_0a.csv", std::ios_base::app);
  csvFile << simTime;
  for (std::vector<Ptr<AquaSimNetDevice> >::iterator ite = m_deviceList.begin(); ite != m_deviceList.end(); ++ite)
  {
    csvFile << "," << (*ite)->GetRouting()->TrafficInPkts();
    //csvFile << "," << (*ite)->GetRouting()->TrafficInBytes(); //ignoring the diff and just getting the raw bytes.
    //csvFile << "," << (*ite)->GetRouting()->TrafficInBytes(true);
  }
  csvFile << "\n";
  csvFile.close();
  */
}

Time
AquaSimChannel::GetPropDelay (Ptr<AquaSimNetDevice> tdevice, Ptr<AquaSimNetDevice> rdevice)
{
  NS_LOG_DEBUG("Channel Propagation Delay:" << m_prop->PDelay(GetMobilityModel(tdevice), GetMobilityModel(rdevice)).GetSeconds());

  return m_prop->PDelay(GetMobilityModel(tdevice), GetMobilityModel(rdevice));
}

double
AquaSimChannel::Distance(Ptr<AquaSimNetDevice> tdevice, Ptr<AquaSimNetDevice> rdevice)
{
  return GetMobilityModel(tdevice)->GetDistanceFrom(GetMobilityModel(rdevice));
}

Ptr<MobilityModel>
AquaSimChannel::GetMobilityModel(Ptr<AquaSimNetDevice> device)
{
  Ptr<MobilityModel> model = device->GetNode()->GetObject<MobilityModel>();
  if (model == 0)
    {
      NS_LOG_DEBUG("MobilityModel does not exist for device " << device);
    }
  return model;
}

Ptr<AquaSimNoiseGen>
AquaSimChannel::GetNoiseGen()
{
  return m_noiseGen;
}

void
AquaSimChannel::DoDispose()
{
  NS_LOG_FUNCTION (this);
  for (std::vector<Ptr<AquaSimNetDevice> >::iterator iter = m_deviceList.begin (); iter != m_deviceList.end (); iter++)
    {
      *iter = 0;
    }
  m_deviceList.clear();
  m_noiseGen=0;
  m_prop=0;
}


} // namespace ns3
