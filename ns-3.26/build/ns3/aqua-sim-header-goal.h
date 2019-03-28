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

#ifndef AQUA_SIM_HEADER_GOAL_H
#define AQUA_SIM_HEADER_GOAL_H

#include "ns3/vector.h"
#include "ns3/header.h"
#include "ns3/nstime.h"

#include "aqua-sim-address.h"

//#include <string>
//#include <iostream>

#define NSADDR_T_SIZE 10

namespace ns3 {

//using implicit ACK.
enum BackoffType{
      VBF,
      HH_VBF
};

/**
 * \ingroup aqua-sim-ng
 *
 * \brief GOAL requester header
 */
class AquaSimGoalReqHeader : public Header {
public:
  AquaSimGoalReqHeader();
  virtual ~AquaSimGoalReqHeader();
  static TypeId GetTypeId(void);

  static uint32_t size(BackoffType type);

  /// Sender / Receiver / Destination addresses
  void SetSA(AquaSimAddress sa);
  void SetRA(AquaSimAddress ra);
  void SetDA(AquaSimAddress da);
  void SetSendTime(Time sendtime);
  void SetTxTime(Time txtime);
  void SetReqID(uint8_t reqid);
  /// Sender's / Sink's / Source's position
  void SetSenderPos(Vector senderPos);
  void SetSinkPos(Vector sinkPos);
  void SetSourcePos(Vector sourcePos);

  AquaSimAddress GetSA();
  AquaSimAddress GetRA();
  AquaSimAddress GetDA();
  Time GetSendTime();
  Time GetTxTime();
  uint8_t GetReqID();
  Vector GetSenderPos();
  Vector GetSinkPos();
  Vector GetSourcePos();

  //inherited methods
  virtual uint32_t GetSerializedSize(void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);
  virtual void Print (std::ostream &os) const;
  virtual TypeId GetInstanceTypeId(void) const;

private:
  AquaSimAddress SA;	//source address        //use 10 bits
  AquaSimAddress RA; //receiver address      //use 10 bits
  AquaSimAddress DA;	//destination address   //use 10 bits  //not useful, so it is not counted in size
  Time		m_SendTime;						//use 2 bytes
  Time		m_TxTime;						//use 1 byte
  uint8_t			m_ReqID;							//use 1 byte

  //following three are for VBF
  Vector		SenderPos;						//3*2bytess
  //following two are for HH-VBF
  Vector		SinkPos;						//3*2bytes
  Vector		SourcePos;						//3*2bytes

}; // class AquaSimGoalReqHeader

/**
 * \brief GOAL replyer header
 */
class AquaSimGoalRepHeader : public Header {
public:
  AquaSimGoalRepHeader();
  virtual ~AquaSimGoalRepHeader();
  static TypeId GetTypeId(void);

  static uint32_t size(BackoffType type);

  void SetSA(AquaSimAddress sa);
  void SetRA(AquaSimAddress ra);
  void SetSendTime(Time sendtime);
  void SetTxTime(Time txtime);
  void SetReqID(uint8_t reqid);
  void SetBackoffTime(Time backoffTime);
  void SetReplyerPos(Vector replyerPos);

  AquaSimAddress GetSA();
  AquaSimAddress GetRA();
  Time GetSendTime();
  Time GetTxTime();
  uint8_t GetReqID();
  Time GetBackoffTime();
  Vector GetReplyerPos();

  //inherited methods
  virtual uint32_t GetSerializedSize(void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);
  virtual void Print (std::ostream &os) const;
  virtual TypeId GetInstanceTypeId(void) const;

private:
  AquaSimAddress SA;	//source address        //use 10 bits
  AquaSimAddress RA; //receiver address      //use 10 bits
  Time		m_SendTime;						//use 2 bytes
  Time		m_TxTime;						//use 1 byte
  uint8_t			m_ReqID;							//use 1 byte
  Time m_BackoffTime;  /* used to decide which replyer is better
							   * This item can be calculated by ReplyerPos and
							   * sender's knowledge. To simplify the code, we include
							   * it in reply packet, so we do not count this item
							   * in the packet size.
							   */  //2 B

  Vector ReplyerPos;  //6 B


}; // class AquaSimGoalRepHeader


/**
 * \brief GOAL ACK header
 */
/*only the destination use this message*/
class AquaSimGoalAckHeader : public Header {
public:
  AquaSimGoalAckHeader();
  virtual ~AquaSimGoalAckHeader();
  static TypeId GetTypeId(void);

  static uint32_t size(BackoffType type);

  void SetSA(AquaSimAddress sa);
  void SetRA(AquaSimAddress ra);
  void SetPush(bool push);
  void SetReqID(uint8_t reqid);

  AquaSimAddress GetSA();
  AquaSimAddress GetRA();
  bool GetPush();
  uint8_t GetReqID();

  //inherited methods
  virtual uint32_t GetSerializedSize(void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);
  virtual void Print (std::ostream &os) const;
  virtual TypeId GetInstanceTypeId(void) const;

private:
  AquaSimAddress SA;	//source address        //use 10 bits
  AquaSimAddress RA; //10 bites, receiver address: BROADCAST because it is broadcast mac
  bool		m_Push;	/*1 bit. if PUSH is set, recver should check the DataSendTimerSet
						   there must be some packets have been sent before.
						*/
  uint8_t			m_ReqID;							//use 1 byte

}; // class AquaSimGoalAckHeader

//treat default packet as a data packet

}  // namespace ns3

#endif /* AQUA_SIM_HEADER_GOAL_H */
