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

#include "aqua-sim-routing-flooding.h"
#include "aqua-sim-address.h"
#include "aqua-sim-header-routing.h"
#include "aqua-sim-header.h"
#include "aqua-sim-datastructure.h"

#include "ns3/log.h"
#include "ns3/nstime.h"
#include "ns3/simulator.h"

//#include "underwatersensor/uw_common/uw_hash_table.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("AquaSimFloodingRouting");
NS_OBJECT_ENSURE_REGISTERED(AquaSimFloodingRouting);


AquaSimFloodingRouting::AquaSimFloodingRouting()
{
  // Initialize variables.
  //  printf("VB initialized\n");
  m_pkCount = 0;
}

TypeId
AquaSimFloodingRouting::GetTypeId()
{
  static TypeId tid = TypeId ("ns3::AquaSimFloodingRouting")
    .SetParent<AquaSimRouting> ()
    .AddConstructor<AquaSimFloodingRouting> ()
  ;
  return tid;
}

int64_t
AquaSimFloodingRouting::AssignStreams (int64_t stream)
{
  NS_LOG_FUNCTION (this << stream);
  return 0;
}

bool
AquaSimFloodingRouting::Recv(Ptr<Packet> packet, const Address &dest, uint16_t protocolNumber)
{
  NS_LOG_FUNCTION(this << packet << GetNetDevice()->GetAddress());

  VBHeader vbh;
  AquaSimHeader ash;
  if (!vbh.GetMessType())  //no headers
  {
    vbh.SetSenderAddr(AquaSimAddress::ConvertFrom(GetNetDevice()->GetAddress()));
    vbh.SetPkNum(packet->GetUid());
    vbh.SetMessType(AS_DATA);
    vbh.SetTargetAddr(AquaSimAddress::ConvertFrom(dest));

    ash.SetDirection(AquaSimHeader::DOWN);
    ash.SetNextHop(AquaSimAddress::GetBroadcast());
    ash.SetNumForwards(0);
    ash.SetSAddr(AquaSimAddress::ConvertFrom(GetNetDevice()->GetAddress()));
    ash.SetDAddr(AquaSimAddress::ConvertFrom(dest));
    ash.SetErrorFlag(false);
    ash.SetNumForwards(ash.GetNumForwards() + 1);
    ash.SetUId(packet->GetUid());

    packet->AddHeader(vbh);
    packet->AddHeader(ash);
  }
  else
  {
    packet->RemoveHeader(ash);
    packet->PeekHeader(vbh);
    ash.SetNumForwards(ash.GetNumForwards() + 1);
    packet->AddHeader(ash);
  }

  /* unused variables
	unsigned char msg_type =vbh.GetMessType();
	unsigned int dtype =0;// vbh.GetDataType();
  */

  /*std::cout << "\nFlooding @Recv check:\n";
  packet->Print(std::cout);
  std::cout << "\n";*/

	// Packet Hash Table is used to keep info about experienced pkts.
  vbf_neighborhood *hashPtr= PktTable.GetHash(vbh.GetSenderAddr(), packet->GetUid());
	// Received this packet before ?

	if (hashPtr != NULL) {
    packet=0;
    return false;
  }
	else {
		PktTable.PutInHash(vbh.GetSenderAddr(), packet->GetUid());
		// Take action for a new pkt.
		ConsiderNew(packet);
    return true;
	}
}

void
AquaSimFloodingRouting::ConsiderNew(Ptr<Packet> pkt)
{
  NS_LOG_FUNCTION(this << pkt);
  VBHeader vbh;
  AquaSimHeader ash;
  pkt->RemoveHeader(ash);
  pkt->PeekHeader(vbh);
  pkt->AddHeader(ash);
	//unsigned char msg_type =vbh.GetMessType();

  /* unused variables
    unsigned int dtype = 0;//vbh.GetDataType();
    double l,h;
  */

	//Pkt_Hash_Entry *hashPtr;
	//AquaSimAddress * hashPtr;  //not used...
	//  Agent_List *agentPtr;
	// PrvCurPtr  RetVal;
	AquaSimAddress nodeAddr; //, forward_nodeID, target_nodeID; //not used...

	//Ptr<Packet> gen_pkt;
	//VBHeader *gen_vbh; //not used...
	//   printf("uwflooding(%d,%d):it is data packet(%d)! it target id is %d  coordinate is %f,%f,%f and range is %f\n",here_.addr_,here_.port_,vbh->pk_num,vbh->target_id.addr_,vbh->info.tx, vbh->info.ty,vbh->info.tz,vbh->range);
	//  printf("Vectorbasedforward:oops!\n");
	switch (vbh.GetMessType()) {
	case AS_DATA:
		//    printf("uwflooding(%d,%d):it is data packet(%d)! it target id is %d  coordinate is %f,%f,%f and range is %f\n",here_.addr_,here_.port_,vbh->pk_num,vbh->target_id.addr_,vbh->info.tx, vbh->info.ty,vbh->info.tz,vbh->range);
		nodeAddr = vbh.GetSenderAddr();
		if (AquaSimAddress::ConvertFrom(GetNetDevice()->GetAddress()) == nodeAddr)
    {
			// come from the same node, briadcast it
			MACprepare(pkt);
			MACsend(pkt,Seconds(0));
			return;
		}


		if (AquaSimAddress::ConvertFrom(GetNetDevice()->GetAddress())==vbh.GetTargetAddr())
		{
			//	printf("uwflooding: %d is the target\n", here_.addr_);
			DataForSink(pkt); // process it
		}

		else{
			// printf("uwflooding: %d is the not  target\n", here_.addr_);
			MACprepare(pkt);
			MACsend(pkt, Seconds(0));
		}
		return;

	default:
    pkt=0;
    break;
	}
}


void
AquaSimFloodingRouting::Reset()
{

	PktTable.Reset();
	/*
	   for (int i=0; i<MAX_DATA_TYPE; i++) {
	   routing_table[i].reset();
	   }
	 */
}


void
AquaSimFloodingRouting::Terminate()
{
  NS_LOG_DEBUG("Terminate: Node=" << m_device->GetAddress() <<
        ": remaining energy=" << GetNetDevice()->EnergyModel()->GetEnergy() <<
        ", initial energy=" << GetNetDevice()->EnergyModel()->GetInitialEnergy());
}

void
AquaSimFloodingRouting::StopSource()
{
	/*
	   Agent_List *cur;

	   for (int i=0; i<MAX_DATA_TYPE; i++) {
	   for (cur=routing_table[i].source; cur!=NULL; cur=AGENT_NEXT(cur) ) {
	    SEND_MESSAGE(i, AGT_ADDR(cur), DATA_STOP);
	   }
	   }
	 */
}


Ptr<Packet>
AquaSimFloodingRouting::CreatePacket()
{
	Ptr<Packet> pkt = Create<Packet>();

	if (pkt==NULL) return NULL;

  AquaSimHeader ash;
  //ash.size(36);

  VBHeader vbh;
  vbh.SetTs(Simulator::Now().ToDouble(Time::S));

  pkt->AddHeader(vbh);
  pkt->AddHeader(ash);
	return pkt;
}

Ptr<Packet>
AquaSimFloodingRouting::PrepareMessage(unsigned int dtype, AquaSimAddress addr,  int msg_type)
{
	Ptr<Packet> pkt = Create<Packet>();
  VBHeader vbh;
  AquaSimHeader ash;
	//hdr_ip *iph;

	vbh.SetMessType(msg_type);
	vbh.SetPkNum(m_pkCount);
	m_pkCount++;
	vbh.SetSenderAddr(AquaSimAddress::ConvertFrom(m_device->GetAddress()));
	// vbh.SetDataType(dtype);
	vbh.SetForwardAddr(AquaSimAddress::ConvertFrom(m_device->GetAddress()));

	vbh.SetTs(Simulator::Now().ToDouble(Time::S));

  pkt->AddHeader(vbh);
  pkt->AddHeader(ash);
	return pkt;
}

void
AquaSimFloodingRouting::MACprepare(Ptr<Packet> pkt)
{
  NS_LOG_FUNCTION(this);
  VBHeader vbh;
  AquaSimHeader ash;
  pkt->RemoveHeader(ash);
  pkt->RemoveHeader(vbh);
	// hdr_ip*  iph = HDR_IP(pkt); // I am not sure if we need it

	vbh.SetForwardAddr(AquaSimAddress::ConvertFrom(m_device->GetAddress()));

  ash.SetErrorFlag(false);
	//cmh->xmit_failure_ = 0;
	// printf("vectorbased: the mac_Broadcast is:%d\n",MAC_BROADCAST);
	ash.SetNextHop(AquaSimAddress::GetBroadcast());
	//ash.addr_type() = NS_AF_ILINK;
	ash.SetDirection(AquaSimHeader::DOWN);

  pkt->AddHeader(vbh);
  pkt->AddHeader(ash);
}

void
AquaSimFloodingRouting::MACsend(Ptr<Packet> pkt, Time delay)
{
  NS_LOG_FUNCTION(this);

  VBHeader vbh;
  AquaSimHeader ash;
  pkt->RemoveHeader(ash);
  pkt->PeekHeader(vbh);

	if (vbh.GetMessType() == AS_DATA)
		ash.SetSize(64); //(God::instance()->data_pkt_size);
	else
		ash.SetSize(36);

  pkt->AddHeader(ash);
  Simulator::Schedule(delay, &AquaSimRouting::SendDown,this,
                        pkt,AquaSimAddress::GetBroadcast(),Seconds(0));
}

void
AquaSimFloodingRouting::DataForSink(Ptr<Packet> pkt)
{
	//  printf("DataforSink: the packet is send to demux\n");
	NS_LOG_FUNCTION(this << pkt << "Sending up to dmux.");
	if (!SendUp(pkt))
		NS_LOG_WARN("DataForSink: Something went wrong when passing packet up to dmux.");
}

void
AquaSimFloodingRouting::DoDispose()
{
  NS_LOG_FUNCTION(this);
  AquaSimRouting::DoDispose();
}

// Some methods for Flooding Entry
