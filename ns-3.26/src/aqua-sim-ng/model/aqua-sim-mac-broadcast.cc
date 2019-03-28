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

#include "aqua-sim-mac-broadcast.h"
#include "aqua-sim-header.h"
#include "aqua-sim-header-mac.h"
#include "aqua-sim-address.h"

#include "ns3/log.h"
#include "ns3/integer.h"
#include "ns3/simulator.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("AquaSimBroadcastMac");
NS_OBJECT_ENSURE_REGISTERED(AquaSimBroadcastMac);


/* ======================================================================
Broadcast MAC for  underwater sensor
====================================================================== */

AquaSimBroadcastMac::AquaSimBroadcastMac()
{
  m_backoffCounter=0;
  m_rand = CreateObject<UniformRandomVariable> ();
}

TypeId
AquaSimBroadcastMac::GetTypeId()
{
  static TypeId tid = TypeId("ns3::AquaSimBroadcastMac")
      .SetParent<AquaSimMac>()
      .AddConstructor<AquaSimBroadcastMac>()
      .AddAttribute("PacketHeaderSize", "Size of packet header",
        IntegerValue(0),
        MakeIntegerAccessor (&AquaSimBroadcastMac::m_packetHeaderSize),
        MakeIntegerChecker<int> ())
      .AddAttribute("PacketSize", "Size of packet",
	IntegerValue(0),
	MakeIntegerAccessor (&AquaSimBroadcastMac::m_packetSize),
	MakeIntegerChecker<int> ())
    ;
  return tid;
}

int64_t
AquaSimBroadcastMac::AssignStreams (int64_t stream)
{
  NS_LOG_FUNCTION (this << stream);
  m_rand->SetStream(stream);
  return 1;
}

/*
this program is used to handle the received packet,
it should be virtual function, different class may have
different versions.
*/
bool
AquaSimBroadcastMac::RecvProcess (Ptr<Packet> pkt)
{
  NS_LOG_FUNCTION(this);
  /*std::cout << "\nBMac @RecvProcess check:\n";
  pkt->Print(std::cout);
  std::cout << "\n";*/

	AquaSimHeader ash;
  MacHeader mach;
  pkt->RemoveHeader(ash);
  pkt->RemoveHeader(mach);
	AquaSimAddress dst = mach.GetDA();

	//get a packet from modem, remove the sync hdr from txtime first
	//cmh->txtime() -= getSyncHdrLen();

	if (ash.GetErrorFlag())
	{
		NS_LOG_DEBUG("BroadcastMac:RecvProcess: received corrupt packet.");
		pkt=0;
		return false;
	}

	if (dst == AquaSimAddress::GetBroadcast() || dst == AquaSimAddress::ConvertFrom(m_device->GetAddress()))
	{
		if (m_packetSize == 0)
		{
			ash.SetSize(ash.GetSize() - m_packetHeaderSize);
		}
    pkt->AddHeader(ash);  //leave MacHeader off since sending to upper layers
		return SendUp(pkt);
	}

//	printf("underwaterAquaSimBroadcastMac: this is neither broadcast nor my packet, just drop it\n");
	pkt=0;
	return false;
}


void
AquaSimBroadcastMac::DropPacket(Ptr<Packet> pkt)
{
  //this is not necessary... only kept for current legacy issues
  pkt=0;
  return;
}


/*
this program is used to handle the transmitted packet,
it should be virtual function, different class may have
different versions.
*/
bool
AquaSimBroadcastMac::TxProcess(Ptr<Packet> pkt)
{

    NS_LOG_FUNCTION(this << pkt);
  AquaSimHeader ash;
  MacHeader mach;
  pkt->RemoveHeader(ash);

  mach.SetDA(AquaSimAddress::GetBroadcast());
  mach.SetSA(AquaSimAddress::ConvertFrom(m_device->GetAddress()));
  if( m_packetSize != 0 )
  {

      ash.SetSize(m_packetSize);
  }

  else
    ash.SetSize(m_packetHeaderSize + ash.GetSize());

  ash.SetTxTime(GetTxTime(pkt));

  switch( m_device->GetTransmissionStatus() )
  {
  case SLEEP:
      PowerOn();
      break;
  case NIDLE:
      ash.SetDirection(AquaSimHeader::DOWN);
      //ash->addr_type()=NS_AF_ILINK;
      //add the sync hdr
      pkt->AddHeader(mach);
      pkt->AddHeader(ash);
      //Phy()->SetPhyStatus(PHY_SEND);
      SendDown(pkt);
      m_backoffCounter=0;
      return true;
  case RECV:
    {
      double backoff=m_rand->GetValue()*BC_BACKOFF;
      NS_LOG_DEBUG("BACKOFF time:" << backoff << " on node:" << m_device->GetAddress() << "\n");
      //pkt->AddHeader(mach);
      pkt->AddHeader(ash);
      Simulator::Schedule(Seconds(backoff),&AquaSimBroadcastMac::BackoffHandler,this,pkt);
    }
      return true;
  case SEND:
    {
      double backoff=m_rand->GetValue()*BC_BACKOFF;
      NS_LOG_DEBUG("BACKOFF time:" << backoff << " on node:" << m_device->GetAddress() << "\n");
      //pkt->AddHeader(mach);
      pkt->AddHeader(ash);
      Simulator::Schedule(Seconds(backoff),&AquaSimBroadcastMac::BackoffHandler,this,pkt);
    }
    return true;
      /*pkt=0;*/
  default:
      /*
      * all cases have been processed above, so simply return
      */
    break;
  }
  return true;  //may be bug due to Sleep/default cases
}


void
AquaSimBroadcastMac::BackoffHandler(Ptr<Packet> pkt)
{
  m_backoffCounter++;
  if (m_backoffCounter<BC_MAXIMUMCOUNTER)
    TxProcess(pkt);
  else
    {
      NS_LOG_INFO("BackoffHandler: too many backoffs");
      m_backoffCounter=0;
      DropPacket(pkt);
    }
}

void AquaSimBroadcastMac::DoDispose()
{
  NS_LOG_FUNCTION(this);
  AquaSimMac::DoDispose();
}
