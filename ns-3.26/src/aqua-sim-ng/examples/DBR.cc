#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/aqua-sim-ng-module.h"
#include "ns3/applications-module.h"
#include "ns3/log.h"
#include "ns3/callback.h"
#include "ns3/rng-seed-manager.h"
#include "ns3/random-variable-stream.h"


/*
* MSVBF + Broadcast MAC
*/

using namespace ns3;


/*int sensor0TxTimes = 0;
int sensor1TxTimes = 0;
int sensor0MacTxBytes = 0;
int sensor1MacTxBytes = 0;

int auv0txTimes = 0;
int auv1txTimes = 0;
int auv0MacTxBytes = 0;
int auv1MacTxBytes = 0;

std::map<int, int> txTimes;
std::map<int, int> txBytes;


int sysTxTimes = 0;

int sink0RxTimes = 0;
int sink1RxTimes = 0;
int sink2RxTimes = 0;
int sink0MacRxBytes = 0;
int sink1MacRxBytes = 0;
int sink2MacRxBytes = 0;

int sysRxTimes = 0;

std::map<int, double> pktFstArvMacTxTime;
std::map<int, double> pktFstArvMacRxTime;
double avgE2EDelay = 0;

double
CalculateAvgE2EDelay (std::map<int, double> pktFstArvMacTxTime,
		std::map<int, double> pktFstArvMacRxTime)
{
	double totalE2EDelay = 0;
	for (auto it = pktFstArvMacRxTime.begin(); it != pktFstArvMacRxTime.end(); ++it)
	{
		totalE2EDelay += (it->second - pktFstArvMacTxTime[it->first]);
	}

	return (totalE2EDelay / pktFstArvMacRxTime.size());
}


// Trace Sink

// m_macTxTrace: A packet has been delivered to the MAC layer for transmission
void
MacRxFromUpper (Ptr<const Packet> m_macTxTrace)
{
//	if (pktFstArvMacTxTime.find(m_macTxTrace->GetUid()) == pktFstArvMacTxTime.end())
//	{
		AquaSimHeader ash;
		m_macTxTrace->PeekHeader(ash);
		// ash.GetSAddr().GetAsInt() is 4
		if(txTimes.find(ash.GetSAddr().GetAsInt()) == txTimes.end())
		{
			txTimes[ash.GetSAddr().GetAsInt()] = 1;
			txBytes[ash.GetSAddr().GetAsInt()] = ash.GetSize();
		}
		else
		{
			txTimes[ash.GetSAddr().GetAsInt()]++;
			txBytes[ash.GetSAddr().GetAsInt()] += ash.GetSize();
		}
//	}
}

// m_macTxTrace: A packet has been delivered to the MAC layer for transmission
void
MacRxFromUpperSensor0 (Ptr<const Packet> m_macTxTrace)
{
	if (pktFstArvMacTxTime.find(m_macTxTrace->GetUid()) == pktFstArvMacTxTime.end())
	{
		pktFstArvMacTxTime[m_macTxTrace->GetUid()] = Simulator::Now().GetSeconds();
		sensor0TxTimes ++;
		AquaSimHeader ash;
		m_macTxTrace->PeekHeader(ash);
		// ash.GetSAddr().GetAsInt() is 4
		sensor0MacTxBytes += ash.GetSize();
	}
}

void
MacRxFromUpperSensor1 (Ptr<const Packet> m_macTxTrace)
{
	if (pktFstArvMacTxTime.find(m_macTxTrace->GetUid()) == pktFstArvMacTxTime.end())
	{
		pktFstArvMacTxTime[m_macTxTrace->GetUid()] = Simulator::Now().GetSeconds();
		sensor1TxTimes ++;
		AquaSimHeader ash;
		m_macTxTrace->PeekHeader(ash);
		// ash.GetSAddr().GetAsInt() is 5
		sensor1MacTxBytes += ash.GetSize();
	}
}

void
MacRxFromUpperAuv0 (Ptr<const Packet> m_macTxTrace)
{
	if (pktFstArvMacTxTime.find(m_macTxTrace->GetUid()) == pktFstArvMacTxTime.end())
	{
		pktFstArvMacTxTime[m_macTxTrace->GetUid()] = Simulator::Now().GetSeconds();
		auv0txTimes ++;
		AquaSimHeader ash;
		m_macTxTrace->PeekHeader(ash);
		// ash.GetSAddr().GetAsInt() is 6
		auv0MacTxBytes += ash.GetSize();
	}
}

void
MacRxFromUpperAuv1 (Ptr<const Packet> m_macTxTrace)
{
	if (pktFstArvMacTxTime.find(m_macTxTrace->GetUid()) == pktFstArvMacTxTime.end())
	{
		pktFstArvMacTxTime[m_macTxTrace->GetUid()] = Simulator::Now().GetSeconds();
		auv1txTimes ++;
		AquaSimHeader ash;
		m_macTxTrace->PeekHeader(ash);
		// ash.GetSAddr().GetAsInt() is 7
		auv1MacTxBytes += ash.GetSize();
	}
}

void
MacRxFromUpperSys (Ptr<const Packet> m_macTxTrace)
{
	sysTxTimes ++;
}



// MacRx: A packet has been received and will be delivered to the MAC layer
void
MacRxFromLowerSink0 (Ptr<const Packet> MacRx)
{
	if (pktFstArvMacRxTime.find (MacRx->GetUid()) == pktFstArvMacRxTime.end())
	{
		pktFstArvMacRxTime[MacRx->GetUid()] = Simulator::Now().GetSeconds();
		sink0RxTimes ++;
		AquaSimHeader ash;
		MacRx->PeekHeader(ash);
		sink0MacRxBytes += ash.GetSize(); // pure data pkt
	}	
}

void
MacRxFromLowerSink1 (Ptr<const Packet> MacRx)
{
	if (pktFstArvMacRxTime.find (MacRx->GetUid()) == pktFstArvMacRxTime.end())
	{
		pktFstArvMacRxTime[MacRx->GetUid()] = Simulator::Now().GetSeconds();
		sink1RxTimes ++;
		AquaSimHeader ash;
		MacRx->PeekHeader(ash);
		sink1MacRxBytes += ash.GetSize(); // pure data pkt
	}
}

void
MacRxFromLowerSink2 (Ptr<const Packet> MacRx)
{
	if (pktFstArvMacRxTime.find (MacRx->GetUid()) == pktFstArvMacRxTime.end())
	{
		pktFstArvMacRxTime[MacRx->GetUid()] = Simulator::Now().GetSeconds();
		sink1RxTimes ++;
		AquaSimHeader ash;
		MacRx->PeekHeader(ash);
		sink2MacRxBytes += ash.GetSize(); // pure data pkt
	}
}

void
MacRxFromLowerSys (Ptr<const Packet> MacRx)
{
	sysRxTimes ++;
}
*/




NS_LOG_COMPONENT_DEFINE ("DBR");

int
main (int argc, char *argv[])
{
	LogComponentEnable ("DBR", LOG_LEVEL_INFO);
	// LogComponentEnable ("AquaSimRangePropagation", LOG_LEVEL_DEBUG);

	double m_simStart = 0;
	double m_simStop = 3000; //sim time (seconds) // original is 500s
	double m_preStop = 50;  //time to prepare to stop


	int m_longDataRate = 1219;	// bps
	int m_longPktSize = 608;	// Bytes
	int m_shortDataRate = 441;	// bps
	int m_shortPktSize = 38;	// Bytes
	int m_controlDataRate = m_shortDataRate;
	int m_controlPktSize = m_shortPktSize;

	int m_gliders = 3;		// number of surface gliders
	int m_sensors = 2;		// number of static sensor nodes
	int m_auvsGroup = 3;	// number of underwater auvs in one group
	int m_auvsSingle = 2;	// number of underwater auvs in a line
	int m_range = 1200;		// max transmission range (meters)
	//double m_width = m_range; //vbf pipeline width

	// Setting for modulation
	//double m_sps = m_longDataRate;	// The number of symbols per second on modulator
	//double m_ber = 0; 	// BER


	// Position of surface gliders
	Vector m_glider0Pos = Vector (0, 500, 0);
	Vector m_glider1Pos = Vector (0, 2000, 0);
	Vector m_glider2Pos = Vector (0, 3500, 0);
	// Position of underwater static sensor nodes
	Vector m_sensor0Pos = Vector (0, 1000, 100);
	Vector m_sensor1Pos = Vector (0, 3000, 100);
	// Position of underwater auvs
	Vector m_auv0SinglePos = Vector (0, 5000, 50);
	Vector m_auv1SinglePos = Vector (0, 6000, 50);
	int m_auvGroupDepth = 80;

	
	

	// //Attribute for MAC protocol - GOAL:
	// int m_maxBurst = 10;
	// Time m_vbfMaxDelay = Seconds(2.5);
	// double m_pipeWidth = 1000;
	// double m_txRadius = m_range; // must equals to the m_range
	// Time m_maxDelay =  Seconds (m_txRadius / 1500);
	// Time m_maxBackoffTime = 4 * m_maxDelay + m_vbfMaxDelay * 1.5 + Seconds(2);

	int seed = 1;
	RngSeedManager::SetSeed (seed);
	std::cout << "-----------Initializing simulation-----------\n";

	NodeContainer glidersCon, sensorsCon, auvsSingleCon, auvsGroupCon;
	glidersCon.Create (m_gliders);
	sensorsCon.Create (m_sensors);
	auvsSingleCon.Create (m_auvsSingle);
	auvsGroupCon.Create (m_auvsGroup);
	

	// Establish layers using helper's pre-build settings
	AquaSimChannelHelper channel = AquaSimChannelHelper::Default();
	channel.SetPropagation ("ns3::AquaSimRangePropagation");// Xia: Important!!! TxRange only functional with propagation model!!!
	channel.SetNoiseGenerator ("ns3::AquaSimConstNoiseGen",
								"Noise", DoubleValue(115)); // Sound Power Level Lw, Lw = 10lg(W/W_0), W is Power(W), W_0 = 1pW
	AquaSimHelper asHelper = AquaSimHelper::Default();
	asHelper.SetChannel(channel.Create());

	asHelper.SetMac ("ns3::AquaSimBroadcastMac");

	asHelper.SetRouting	("ns3::AquaSimDBR");

	asHelper.SetEnergyModel ("ns3::AquaSimEnergyModel",
							"RxPower", DoubleValue(1.3),
							"TxPower", DoubleValue(2.8), // 2.8w@1000m
							"InitialEnergy", DoubleValue(10000),
							"IdlePower", DoubleValue(0.003));

	// MobilityHelper sensorsMobility, auvsMobility, glidersMobility;
	// NetDeviceContainer sensorDevices, auvDevices, gliderDevices;
	std::set<Ptr<AquaSimNetDevice> > ASDeviceSet;


	/*
	* ============ Gliders Settings ============
	*/
	std::cout << "Creating surface gliders..." << std::endl;
	NetDeviceContainer gliderDevices;
	// AquaSimHelper sinkASHelper = asHelper;
	// sinkASHelper.SetRouting("ns3::AquaSimRoutingDummy");
	for (NodeContainer::Iterator i = glidersCon.Begin(); i != glidersCon.End(); i++)
    {
		Ptr<AquaSimNetDevice> newDevice = CreateObject<AquaSimNetDevice> ();
		newDevice = asHelper.Create(*i, newDevice);
		gliderDevices.Add(newDevice);
		newDevice->GetPhy()->SetTransRange(m_range);
		/*std::string name = "default";
		Ptr<AquaSimModulation> module = newDevice->GetPhy()->Modulation(&name);
		module->SetAttribute("CodingEff", DoubleValue (1));
		module->SetAttribute("SPS", UintegerValue (m_sps));
		module->SetAttribute("BER", DoubleValue (m_ber));*/

		ASDeviceSet.insert (newDevice);
    }
	std::cout << "- Set position & velocity for every gliders:" << std::endl;
	MobilityHelper glidersMobility;
	Ptr<ListPositionAllocator> gliderPosition = CreateObject<ListPositionAllocator> ();
	glidersMobility.SetPositionAllocator (gliderPosition);
	gliderPosition->Add (m_glider0Pos);
	gliderPosition->Add (m_glider1Pos);
	gliderPosition->Add (m_glider2Pos);
	glidersMobility.SetMobilityModel("ns3::RandomWalk2dMobilityModel",
			"Mode", StringValue ("Time"),
			"Time", TimeValue (Seconds(5.0)),
			"Speed", StringValue ("ns3::UniformRandomVariable[Min=0.25|Max=0.5]"),
			"Bounds", StringValue ("-500|500|-500|4000"));
	glidersMobility.Install(glidersCon);
	// ----- Print Position -----
	for (NodeContainer::Iterator i = glidersCon.Begin(); i != glidersCon.End(); i++)
	{
		Ptr<RandomWalk2dMobilityModel> model = (*i)->GetObject<RandomWalk2dMobilityModel>();
		NS_LOG_INFO (" - Node: " << (*i)->GetId() << ", Position: (" << model->GetPosition().x
				<< ", " << model->GetPosition().y << ", " << model->GetPosition().z << ")");
	}

	/*
	* =========== Sensors Settings ============
	*/
	std::cout << "Creating underwater static sensor nodes..." << std::endl;
	// NetDeviceContainer sensorDevices;
	for (NodeContainer::Iterator i = sensorsCon.Begin(); i != sensorsCon.End(); i++)
    {
		Ptr<AquaSimNetDevice> newDevice = CreateObject<AquaSimNetDevice> ();
		newDevice = asHelper.Create(*i, newDevice);
		// sensorDevices.Add(newDevice);
		newDevice->GetPhy()->SetTransRange(m_range);

		/*std::string name = "default";
		Ptr<AquaSimModulation> module = newDevice->GetPhy()->Modulation(&name);
		module->SetAttribute("CodingEff", DoubleValue (1));
		module->SetAttribute("SPS", UintegerValue (m_sps));
		module->SetAttribute("BER", DoubleValue (m_ber));*/
		ASDeviceSet.insert (newDevice);
    }
	std::cout << "- Set position for every static sensor node:" << std::endl;
	MobilityHelper sensorsMobility;
	Ptr<ListPositionAllocator> sensorPosition = CreateObject<ListPositionAllocator> ();
	sensorsMobility.SetPositionAllocator (sensorPosition);
	sensorPosition->Add (m_sensor0Pos);
	sensorPosition->Add (m_sensor1Pos);
	sensorsMobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
	sensorsMobility.Install(sensorsCon);
	// -------- Print Position-------------
	for (NodeContainer::Iterator i = sensorsCon.Begin(); i != sensorsCon.End(); i++)
	{
		Ptr<ConstantPositionMobilityModel> model = (*i)->GetObject<ConstantPositionMobilityModel>();
		NS_LOG_INFO (" - Node: " << (*i)->GetId() << ", Position: (" << model->GetPosition().x
				<< ", " << model->GetPosition().y << ", " << model->GetPosition().z << ")");
	}


	/*
	* ============= AUV Settings ==============
	*/
	std::cout << "Creating underwater single auvs..." << std::endl;
	// NetDeviceContainer auvSingleDevices;
	for (NodeContainer::Iterator i = auvsSingleCon.Begin(); i != auvsSingleCon.End(); i++)
	{
		Ptr<AquaSimNetDevice> newDevice = CreateObject<AquaSimNetDevice> ();
		newDevice = asHelper.Create (*i, newDevice);
		// auvSingleDevices.Add (newDevice);
		newDevice->GetPhy()->SetTransRange(m_range);

		/*std::string name = "default";
		Ptr<AquaSimModulation> module = newDevice->GetPhy()->Modulation(&name);
		module->SetAttribute("CodingEff", DoubleValue (1));
		module->SetAttribute("SPS", UintegerValue (m_sps));
		module->SetAttribute("BER", DoubleValue (m_ber));*/
		ASDeviceSet.insert (newDevice);
	}
	std::cout << "- Set position & velocity for every single auvs:" << std::endl;
	MobilityHelper auvsSingleMobility;
	Ptr<ListPositionAllocator> auvSinglePosition = CreateObject<ListPositionAllocator> ();
	auvsSingleMobility.SetPositionAllocator(auvSinglePosition);
	auvSinglePosition->Add (m_auv0SinglePos);
	auvSinglePosition->Add (m_auv1SinglePos);
	auvsSingleMobility.SetMobilityModel("ns3::ConstantVelocityMobilityModel");
	auvsSingleMobility.Install (auvsSingleCon);
	// ---------- Set and Print Position & Velocity------------
	for (NodeContainer::Iterator i = auvsSingleCon.Begin(); i != auvsSingleCon.End(); i++)
	{
		Ptr<ConstantVelocityMobilityModel> model = (*i)->GetObject<ConstantVelocityMobilityModel> ();
		model->SetVelocity(Vector(0, -2, 0));
		// ConstantVelocityHelper m_helper = ConstantVelocityHelper(model->GetPosition(), model->GetVelocity());
		NS_LOG_DEBUG (" - Node: " << (*i)->GetId() << ", Position: (" << model->GetPosition().x
				<< ", " << model->GetPosition().y << ", " << model->GetPosition().z << "), "
				<< "Velocity: (" << model->GetVelocity().x
				  << ", " << model->GetVelocity().y
				  << ", " << model->GetVelocity().z << ")");
	}

	std::cout << "Creating underwater group auvs..." << std::endl;
	NetDeviceContainer auvGroupDevices;
	for (NodeContainer::Iterator i = auvsGroupCon.Begin(); i != auvsGroupCon.End(); i++)
	{
		Ptr<AquaSimNetDevice> newDevice = CreateObject<AquaSimNetDevice> ();
		newDevice = asHelper.Create (*i, newDevice);
		auvGroupDevices.Add(newDevice);
		newDevice->GetPhy()->SetTransRange(m_range);

		/*std::string name = "default";
		Ptr<AquaSimModulation> module = newDevice->GetPhy()->Modulation(&name);
		module->SetAttribute("CodingEff", DoubleValue (1));
		module->SetAttribute("SPS", UintegerValue (m_sps));
		module->SetAttribute("BER", DoubleValue (m_ber));*/

		ASDeviceSet.insert (newDevice);
	}
	std::cout << "- Set position & velocity for every group auvs:" << std::endl;
	MobilityHelper auvsGroupMobility;
	std::ostringstream ossAuvZRange;
	ossAuvZRange << "ns3::UniformRandomVariable[Min=" << m_auvGroupDepth << "|Max=" << m_auvGroupDepth << "]";
	auvsGroupMobility.SetPositionAllocator ("ns3::RandomBoxPositionAllocator",
		"X", StringValue ("ns3::UniformRandomVariable[Min=-500|Max=500]"),
		"Y", StringValue ("ns3::UniformRandomVariable[Min=1500|Max=2500]"),
		"Z", StringValue (ossAuvZRange.str ()));
	auvsGroupMobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
		"Mode", StringValue ("Time"),
		"Time", TimeValue (Seconds (1.0)),
		"Speed", StringValue("ns3::UniformRandomVariable[Min=0.2|Max=2]"),
		"Bounds", StringValue ("-500|500|1500|2500"));
	auvsGroupMobility.Install (auvsGroupCon);
	// ---------- Print Position ------------
	for (NodeContainer::Iterator i = auvsGroupCon.Begin(); i != auvsGroupCon.End(); ++i)
	{
		Ptr<RandomWalk2dMobilityModel> model = (*i)->GetObject<RandomWalk2dMobilityModel> ();
		Vector pos = model->GetPosition();
		NS_LOG_DEBUG (" - AUV: " << (*i)->GetId() << ", Position: (" << pos.x << ", " << pos.y << ", " << pos.z << ")");
	}




	
	/*
	* ============= Socket Settings ==============
	*/		
	// NodeContainer virtualSink;
	// virtualSink.Create(1);
	// Ptr<AquaSimNetDevice> virtualSinkDevice = CreateObject<AquaSimNetDevice> ();
	// virtualSinkDevice = asHelper.Create(virtualSink.Get(0), virtualSinkDevice);


	PacketSocketHelper socketHelper;
	socketHelper.Install (glidersCon);
	socketHelper.Install (sensorsCon);
	socketHelper.Install (auvsSingleCon);
	socketHelper.Install (auvsGroupCon);
	// socketHelper.Install (virtualSink);
	
	PacketSocketAddress longDataSocket;
	longDataSocket.SetAllDevices (); // Set the address to match all the outgoing NetDevice
	longDataSocket.SetPhysicalAddress (gliderDevices.Get(1)->GetAddress()); // Set the destination address
	// socket.SetPhysicalAddress (virtualSinkDevice->GetAddress());
	/* see http://www.iana.org/assignments/protocol-numbers */
	longDataSocket.SetProtocol (0); // 0 : IPv6 Hop-by-Hop Option
	// std::cout << "Bind the socket to sink..." << std::endl;
	Ptr<Node> longDataSinkNode = glidersCon.Get(1);
	// Ptr<Node> sinkNode = virtualSink.Get(0);
	TypeId psfid = TypeId::LookupByName ("ns3::PacketSocketFactory");
	Ptr<Socket> longDataSinkSocket = Socket::CreateSocket (longDataSinkNode, psfid);
	longDataSinkSocket->Bind (longDataSocket);

	PacketSocketAddress innerClusterSocket;
	innerClusterSocket.SetAllDevices ();
	innerClusterSocket.SetPhysicalAddress (auvGroupDevices.Get(1)->GetAddress());
	innerClusterSocket.SetProtocol (0);
	Ptr<Node>  innerClusterSinkNode = auvsGroupCon.Get(0);
	TypeId psfid2 = TypeId::LookupByName ("ns3::PacketSocketFactory");
	Ptr<Socket> innerClusterSinkSocket = Socket::CreateSocket (innerClusterSinkNode, psfid2);
	innerClusterSinkSocket->Bind(innerClusterSocket);





	/*
	* ============= Application Settings ==============
	*/	
	/** 
	* \OnOffHelper (std::string protocol, Address address);
	* \param protocol the name of the protocol to use to send traffic
	*        by the applications. This string identifies the socket
	*        factory type used to create sockets for the applications.
	*        A typical value would be ns3::UdpSocketFactory.
	* \param address the address of the remote node to send traffic
	*        to.
	*/
	
	// every 1 min, 38 Bytes/Block * 16 Blocks = 608 Bytes, 3990 ms, 1219 bps
	OnOffHelper longDataApp ("ns3::PacketSocketFactory", Address (longDataSocket));
	longDataApp.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=3.99]"));
	longDataApp.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=56.01]"));
	longDataApp.SetAttribute ("DataRate", DataRateValue (m_longDataRate));
	longDataApp.SetAttribute ("PacketSize", UintegerValue (m_longPktSize));

	// every 20 s, 38 Bytes, 690 ms, 441 bps
	OnOffHelper shortDataApp ("ns3::PacketSocketFactory", Address (innerClusterSocket));
	shortDataApp.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=0.69]"));
	shortDataApp.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=19.31]"));
	shortDataApp.SetAttribute ("DataRate", DataRateValue (m_shortDataRate));
	shortDataApp.SetAttribute ("PacketSize", UintegerValue (m_shortPktSize));

	// every 10 min, 38 Bytes, 690 ms, 441 bps
	OnOffHelper controlApp ("ns3::PacketSocketFactory", Address (innerClusterSocket));
	controlApp.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=0.69]"));
	controlApp.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=599.31]"));
	controlApp.SetAttribute ("DataRate", DataRateValue (m_controlDataRate));
	controlApp.SetAttribute ("PacketSize", UintegerValue (m_controlPktSize));

	// ---------- Upload -------------
	std::cout << "Set application for sensors..." << std::endl;
	for (int i = 0; i < m_sensors; ++i)
	{
		ApplicationContainer lDataApp = longDataApp.Install (sensorsCon.Get(i));
		Ptr<UniformRandomVariable> uv = CreateObject<UniformRandomVariable> ();
	  	double start1 = uv->GetValue (0, 60);
		lDataApp.Start (Seconds (m_simStart + start1));
		lDataApp.Stop (Seconds (m_simStop - m_preStop));
	}


	std::cout << "Set application for single AUVs..." << std::endl;
	for (int i = 0; i < m_auvsSingle; ++i)
	{
		ApplicationContainer lDataApp = longDataApp.Install (auvsSingleCon.Get(i));
		Ptr<UniformRandomVariable> uv = CreateObject<UniformRandomVariable> ();
		double start = uv->GetValue(0, 60);
		lDataApp.Start (Seconds (m_simStart + start));
		lDataApp.Stop (Seconds (m_simStop - m_preStop));
	}

	std::cout << "Set application for the cluster head AUV..." << std::endl;
	for (int i = 1; i < (m_auvsGroup - 1); ++i)
	{
		ApplicationContainer lDataApp = longDataApp.Install (auvsGroupCon.Get(i));
		Ptr<UniformRandomVariable> uv = CreateObject<UniformRandomVariable> ();
		double start = uv->GetValue (0, 60);
		lDataApp.Start (Seconds (m_simStart + start));
		lDataApp.Stop (Seconds (m_simStop - m_preStop));
	}

	// ------------- Inner Cluster Upload----------
	std::cout << "Set application for the cluster AUVs..." << std::endl;
	for (int i = 0; i < m_auvsGroup; ++i)
	{
		if (i != 1)
		{
			ApplicationContainer sDataApp = shortDataApp.Install (auvsGroupCon.Get(i));
			Ptr<UniformRandomVariable> uv = CreateObject<UniformRandomVariable> ();
			double start = uv->GetValue (0, 20);
			sDataApp.Start (Seconds (m_simStart + start));
			sDataApp.Stop (Seconds (m_simStop - m_preStop));
		}
	}

	// TODO: DownLoad





//delete



	Packet::EnablePrinting ();  //for debugging purposes
	std::cout << "-----------Running Simulation-----------\n";
	Simulator::Stop (Seconds (m_simStop));
	Simulator::Run ();
	asHelper.GetChannel ()->PrintCounters ();

	std::cout << "\nSummary:" << std::endl;
	// std::cout << "SenderNum = " << senders << ", ForwarderNum = " << nodes << ", SinkNum = " << sinks << std::endl;
	// std::cout << "Random seed = " << seed << ", tx_range = " << range << " m\n\n";

	
	/*double totalEnergyConsumption = 0;
	std::map<int, std::pair<int, double> > totalSentPktsRecord;
	std::map<int, std::pair<int, double> > totalSendUpPktsRecord;
	for (auto it = ASDeviceSet.begin (); it != ASDeviceSet.end (); it++)
	{
		totalEnergyConsumption += (*it)->EnergyModel ()->GetTotalEnergyConsumption ();

		std::map<int, std::pair<int, double> > sentPktRecord = (*it)->SentPktRecord();
		for (auto c : sentPktRecord)
		{
			if (totalSentPktsRecord.find(c.first) == totalSentPktsRecord.end())
			{
				totalSentPktsRecord[c.first] = c.second;
			}
			else
			{
				if (c.second.second < totalSentPktsRecord[c.first].second)
				{ // Get the min send time
					totalSentPktsRecord[c.first] = c.second;
				}
			}
		}


		std::map<int, std::pair<int, double> > sendUpPktsRecord = (*it)->GetRouting()->SendUpPktRecord();
		for (auto c : sendUpPktsRecord)
		{
			if(totalSendUpPktsRecord.find(c.first) == totalSendUpPktsRecord.end())
			{
				totalSendUpPktsRecord[c.first] = c.second;
			}
			else
			{
				if(c.second.second < totalSendUpPktsRecord[c.first].second)
				{ // Get the min recv time
					totalSendUpPktsRecord[c.first] = c.second;
				}
			}
		}
	}
	

	int totalSentBytes = 0;
	std::map<int, double> pktFirstTxTime;
	for (auto c : totalSentPktsRecord)
	{
		totalSentBytes += c.second.first;
		pktFirstTxTime[c.first] = c.second.second;
	}
	int totalSentTimes = totalSentPktsRecord.size();
	std::cout << "System Total Upload Tx times = " << totalSentTimes << std::endl;
	std::cout << "System Total Upload Tx Bytes = " << totalSentBytes << " Bytes\n\n";

	double totalRecvBytes = 0;
	std::map<int, double> pktFirstRxTime;
	for (auto c : totalSendUpPktsRecord)
	{
		totalRecvBytes += c.second.first;
		pktFirstRxTime[c.first] = c.second.second;
	}
	double totalRecvTimes = totalSendUpPktsRecord.size();
	std::cout << "System Total Upload Sink Rx times = " << totalRecvTimes << std::endl;
	std::cout << "System Total Upload Sink Rx Bytes = " << totalRecvBytes << " Bytes\n\n";

	std::cout << "System Total Upload Throughput = " << (totalRecvBytes * 8 / (m_simStop * 1.0) ) << " bps\n";

	double PDR = totalRecvTimes / (totalSentTimes * 1.0);
	std::cout << "=>Packet Deliver Ratio = " << PDR * 100 << " %\n";

	// avgE2EDelay = CalculateAvgE2EDelay (pktFstArvMacTxTime, pktFstArvMacRxTime);

	avgE2EDelay = CalculateAvgE2EDelay (pktFirstTxTime, pktFirstRxTime);
	std::cout << "=>Average pkt End-To-End Delay = " << avgE2EDelay << " s\n";

	std::cout << "=>System Total Energy Consumption = " << totalEnergyConsumption << " J" << std::endl;
	std::cout << "=>Energy Consumption per Byte = " << (totalEnergyConsumption / totalRecvBytes) << " J/B\n";*/

	Simulator::Destroy ();

	std::cout << "----------------Fin-----------------\n";
	return 0;
}
