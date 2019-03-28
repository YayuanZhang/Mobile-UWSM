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

/*
#ifndef AQUA_SIM_NODE_H
#define AQUA_SIM_NODE_H


#include "aqua-sim-energy-model.h"
#include "aqua-sim-net-device.h"

#include "ns3/packet.h"	
#include "ns3/address.h"
#include "ns3/object.h"
#include "ns3/node.h"
#include "ns3/mobility-model.h"
#include "ns3/random-variable-stream.h"
#include "ns3/position-allocator.h"
#include "ns3/application.h"

//#include "trace.h"		//TODO should include trace

// Aqua Sim Node

namespace ns3{

enum TransmissionStatus { SLEEP, NIDLE, SEND, RECV, NStatus };	//idle currently used by ns3:WireState

//class UnderwaterMobilityPattern;	//TODO enhance mobility model with underwater mobility pattern
class MobilityModel;
class AquaSimEnergyModel;
class AquaSimNetDevice;
class AquaSimPhy;

*
class AquaSimPositionHandler : public Handler {
public:
AquaSimPositionHandler(AquaSimNode* n) : Node(n) {}
void Handle(Event*);
private:
AquaSimNode *node;
};
*


class AquaSimNode : public Node, public MobilityModel
{
public:
  AquaSimNode(void);
  virtual ~AquaSimNode(void);
  static TypeId GetTypeId(void);

  //double PropDelay(double); //... remove... not this node's responsibility here
  //bool Move(void);	*start the movement... should be handled within example*
  //void Start(void);
  //void CheckPosition(void);

  //Ptr<CubicPositionAllocator> T(void) { return m_T; }	//mobility model may cover this...

  *coordinates*
  inline double &X(void) { return m_x; }
  inline double &Y(void) { return m_y; }
  inline double &Z(void) { return m_z; }
  *speeds projected to each axis*
  inline double &dX(void) { return m_dX; }
  inline double &dY(void) { return m_dY; }
  inline double &dZ(void) { return m_dZ; }
  inline double &speed(void) { return m_speed; }

  inline Time &PositionUpdateTime(void) { return m_positionUpdateTime; }

  //sink related attributes
  int ClearSinkStatus(void);
  int SetSinkStatus(void);
  inline int GetSinkStatus(void) { return m_sinkStatus; }
  inline double &CX(void) { return m_cX; }
  inline double &CY(void) { return m_cY; }
  inline double &CZ(void) { return m_cZ; }

  inline bool FailureStatus(void) { return m_failureStatus; }
  inline double FailurePro(void) { return m_failurePro; }
  inline double FailureStatusPro(void) { return m_failureStatusPro; }

  inline void SetTransmissionStatus(enum TransmissionStatus status) {
    m_transStatus = status;
  }
  inline enum TransmissionStatus TransmissionStatus(void) { return m_transStatus; }

  inline bool CarrierSense(void) { return m_carrierSense; }
  inline void ResetCarrierSense(void) { m_carrierSense = false; }
  inline void SetCarrierSense(bool f){
    m_carrierSense = f;
    m_carrierId = f;
  }
  inline bool CarrierId(void) { return m_carrierId; }
  inline void ResetCarrierId(void) { m_carrierId = false; }

  int m_nextHop;
  int m_setHopStatus;
  int m_sinkStatus;

  //void UpdatePosition(void);  // UpdatePosition() out of date... should be using ns3's mobility module

  bool IsMoving(void);
  Ptr<AquaSimPhy> GetPhy() const;
  void SetPhy(Ptr<AquaSimPhy> phy);

  *
   * inherited from Node
   *
  virtual uint32_t AddApplication(Ptr<Application> application);
  virtual uint32_t AddDevice(Ptr<AquaSimNetDevice> device);
  virtual Ptr<Application> GetApplication(uint32_t index) const;
  virtual Ptr<AquaSimNetDevice> GetDevice(uint32_t index) const;
  virtual uint32_t GetId(void) const;
  virtual uint32_t GetNApplications(void) const;
  virtual uint32_t GetNDevices(void) const;
  virtual uint32_t GetSystemId(void) const;
  //virtual void RegisterDeviceAdditionListener(DeviceAdditionListener listener);
  //virtual void RegisterProtocolHandler(ProtocolHandler handler, uint16_t protocolType,
  //				Ptr<AquaSimNetDevice> device, bool promiscuous=false);
  //virtual void UnregisterDeviceAdditionLister (DeviceAdditionListener listener);
  //virtual void UnregisterProtocolHandler(ProtocolHandler handler);

  *
   * inherited from MobilityModel
   *
  double DistanceFrom(AquaSimNode * n) const;
  Vector Position(void);
  double RelativeSpeed(AquaSimNode * n);
  Vector Velocity(void);
  void SetNodePosition(const Vector &position);
  //virtual int64_t AssignStreams (int64_t stream);

private:
  enum  TransmissionStatus m_transStatus;
  double m_statusChangeTime;  //the time when changing m_preTransStatus to m_transStatus

  bool	m_failureStatus;// 1 if node fails, 0 otherwise
  double m_failurePro;
  double m_failureStatusPro;

  *
  *  The following indicate the (x,y,z) position of the node on
  *  the "terrain" of the simulation.
  *
  double m_x;
  double m_y;
  double m_z;
  double m_speed;	// meters per second

  *
  *  The following is a unit vector that specifies the
  *  direction of the mobile node.  It is used to update
  *  position
  *
  double m_dX;
  double m_dY;
  double m_dZ;

  //the following attributes are added by Peng Xie for RMAC and VBF
  double m_cX;
  double m_cY;
  double m_cZ;

  bool m_carrierSense;
  bool m_carrierId;

  Time m_positionUpdateTime;

  std::vector< Ptr<Application> > m_applications;
  std::vector< Ptr<AquaSimNetDevice> > m_devices;
  uint32_t m_id;
  uint32_t m_sid;

protected:
  *
  double m_maxSpeed;
  double m_minSpeed;
  void RandomSpeed(void);
  void RandomDestination(void);
  *
  void GenerateFailure(void);

  //fully integrate MobilityModel within ASNode
  //UnderwaterMobilityPattern * MP_; //our new mobility pattern module

  Ptr<MobilityModel> m_MP;

  //Ptr<CubicPositionAllocator> m_T;	//... remove

  void RandomPosition(void);
  int m_randomMotion;	// is mobile

private:
  Ptr<UniformRandomVariable> m_uniformRand;
  Ptr<AquaSimEnergyModel> m_energyModel;
  Ptr<AquaSimPhy> m_phy;

public:
  Ptr<AquaSimEnergyModel> EnergyModel(void) { return m_energyModel; }
};  // class AquaSimNode

}  // namespace ns3

#endif * AQUA_SIM_NODE_H *
*/
