#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/energy-module.h" 
#include "ns3/aqua-sim-ng-module.h"
#include "ns3/applications-module.h"
#include "ns3/log.h"
#include "ns3/callback.h"

using namespace ns3;
NS_LOG_COMPONENT_DEFINE("DBR");
int
main (int argc, char *argv[])
{
  double m_simStart = 0;
  double m_simStop = 3000; //seconds
  double m_preStop = 50;

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

	int seed = 2;
	RngSeedManager::SetSeed (seed);

std::cout << "-----------Initializing simulation-----------\n";

	NodeContainer glidersCon, sensorsCon, auvsSingleCon, auvsGroupCon;
	glidersCon.Create (m_gliders);
	sensorsCon.Create (m_sensors);
	auvsSingleCon.Create (m_auvsSingle);
	auvsGroupCon.Create (m_auvsGroup);


  /*LogComponentEnable ("AquaSimDBR", LOG_DEBUG);
  LogComponentEnable ("AquaSimNetDevice", LOG_DEBUG);
  LogComponentEnable ("AquaSimBroadcastMac", LOG_DEBUG);
  LogComponentEnable ("AquaSimMac", LOG_DEBUG);
  LogComponentEnable ("AquaSimHeader", LOG_DEBUG);
  LogComponentEnable ("AquaSimRangePropagation", LOG_DEBUG);
  LogComponentEnable ("DRoutingHeader", LOG_DEBUG);*/
  LogComponentEnable ("DBR", LOG_DEBUG);

  AquaSimChannelHelper channel = AquaSimChannelHelper::Default();
  channel.SetPropagation("ns3::AquaSimRangePropagation");
  channel.SetNoiseGenerator ("ns3::AquaSimConstNoiseGen","Noise",DoubleValue(115)); 
// Sound Power Level Lw, Lw = 10lg(W/W_0), W is Power(W), W_0 = 1pW
  AquaSimHelper asHelper = AquaSimHelper::Default();
  asHelper.SetChannel(channel.Create());
  asHelper.SetMac("ns3::AquaSimBroadcastMac");
  asHelper.SetRouting("ns3::AquaSimDBR");


std::set<Ptr<AquaSimNetDevice> > ASDeviceSet;
  std::cout << "Creating Nodes\n";

/* ============ Gliders Settings ============*/
	std::cout << "Creating surface gliders..." << std::endl;
	NetDeviceContainer gliderDevices;

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

	for (NodeContainer::Iterator i = glidersCon.Begin(); i != glidersCon.End(); i++)
    {
		Ptr<AquaSimNetDevice> newDevice = CreateObject<AquaSimNetDevice> ();
		newDevice = asHelper.Create(*i, newDevice);
		gliderDevices.Add(newDevice);
		newDevice->GetPhy()->SetTransRange(m_range);
		newDevice->SetSinkTeam(1);
		ASDeviceSet.insert (newDevice);
		Ptr<RandomWalk2dMobilityModel> model = (*i)->GetObject<RandomWalk2dMobilityModel>();
		NS_LOG_DEBUG(" - Glider: " << (*i)->GetId() << ", Position: (" << model->GetPosition().x
				<< ", " << model->GetPosition().y << ", " << model->GetPosition().z << ")"
				<<" , address "<< newDevice->GetAddress());
    }

/* =========== Sensors Settings ============ */
	std::cout << "Creating underwater static sensor nodes..." << std::endl;

	std::cout << "- Set position for every static sensor node:" << std::endl;
	MobilityHelper sensorsMobility;
	Ptr<ListPositionAllocator> sensorPosition = CreateObject<ListPositionAllocator> ();
	sensorsMobility.SetPositionAllocator (sensorPosition);
	sensorPosition->Add (m_sensor0Pos);
	sensorPosition->Add (m_sensor1Pos);
	sensorsMobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
	sensorsMobility.Install(sensorsCon);
		for (NodeContainer::Iterator i = sensorsCon.Begin(); i != sensorsCon.End(); i++)
    {
		Ptr<AquaSimNetDevice> newDevice = CreateObject<AquaSimNetDevice> ();
		newDevice = asHelper.Create(*i, newDevice);
		newDevice->GetPhy()->SetTransRange(m_range);
		ASDeviceSet.insert (newDevice);
		Ptr<ConstantPositionMobilityModel> model = (*i)->GetObject<ConstantPositionMobilityModel>();
		NS_LOG_DEBUG (" - Static bottom: " << (*i)->GetId() << ", Position: (" << model->GetPosition().x
				<< ", " << model->GetPosition().y << ", " << model->GetPosition().z << ")"
				<<" , address "<< newDevice->GetAddress());
    }

/*============= AUV Settings ==============*/
	std::cout << "Creating underwater single auvs..." << std::endl;

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
		Ptr<AquaSimNetDevice> newDevice = CreateObject<AquaSimNetDevice> ();
		newDevice = asHelper.Create (*i, newDevice);
		newDevice->GetPhy()->SetTransRange(m_range);
		ASDeviceSet.insert (newDevice);
		Ptr<ConstantVelocityMobilityModel> model = (*i)->GetObject<ConstantVelocityMobilityModel> ();
		model->SetVelocity(Vector(0, -2, 0));
		NS_LOG_DEBUG (" - AUV single: " << (*i)->GetId() << ", Position: (" << model->GetPosition().x
				<< ", " << model->GetPosition().y << ", " << model->GetPosition().z << "), "
				<< "Velocity: (" << model->GetVelocity().x<< ", " << model->GetVelocity().y<< ", " << model->GetVelocity().z << ")"
				<<" , address "<< newDevice->GetAddress());
	}



	std::cout << "Creating underwater group auvs..." << std::endl;
	NetDeviceContainer auvGroupDevices;

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

	for (NodeContainer::Iterator i = auvsGroupCon.Begin(); i != auvsGroupCon.End(); i++)
	{
		Ptr<AquaSimNetDevice> newDevice = CreateObject<AquaSimNetDevice> ();
		newDevice = asHelper.Create (*i, newDevice);
		auvGroupDevices.Add(newDevice);
		newDevice->GetPhy()->SetTransRange(m_range);
		ASDeviceSet.insert (newDevice);
		Ptr<RandomWalk2dMobilityModel> model = (*i)->GetObject<RandomWalk2dMobilityModel> ();
		Vector pos = model->GetPosition();
		NS_LOG_DEBUG (" - AUV: " << (*i)->GetId() << ", Position: (" << pos.x << ", " << pos.y << ", " << pos.z << ")"
		<<" , address "<< newDevice->GetAddress());
	}


/*============= Socket Settings ==============*/	


        PacketSocketHelper socketHelper;
	socketHelper.Install (glidersCon);
	socketHelper.Install (sensorsCon);
	socketHelper.Install (auvsSingleCon);
	socketHelper.Install (auvsGroupCon);

        PacketSocketAddress longDataSocket;
	longDataSocket.SetAllDevices ();
        longDataSocket.SetPhysicalAddress (gliderDevices.Get(0)->GetAddress());  
	longDataSocket.SetProtocol (0); 
        
	Ptr<Node> longDataSinkNode = glidersCon.Get(0);
	TypeId psfid = TypeId::LookupByName ("ns3::PacketSocketFactory");
	Ptr<Socket> longDataSinkSocket = Socket::CreateSocket (longDataSinkNode, psfid);
	longDataSinkSocket->Bind (longDataSocket);

	PacketSocketAddress innerClusterSocket;
	innerClusterSocket.SetAllDevices ();
	innerClusterSocket.SetPhysicalAddress (auvGroupDevices.Get(1)->GetAddress());
	innerClusterSocket.SetProtocol (0);

	Ptr<Node>  innerClusterSinkNode = auvsGroupCon.Get(1);
	TypeId psfid2 = TypeId::LookupByName ("ns3::PacketSocketFactory");
	Ptr<Socket> innerClusterSinkSocket = Socket::CreateSocket (innerClusterSinkNode, psfid2);
	innerClusterSinkSocket->Bind(innerClusterSocket);



/*============= Application Settings ==============*/	
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
//onoff-application.cc
	longDataApp.SetAttribute ("DstSinkTeam", UintegerValue (1));


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
		lDataApp.Start (Seconds (m_simStart));
		lDataApp.Stop (Seconds (m_simStop - m_preStop));
	}


	std::cout << "Set application for single AUVs..." << std::endl;
	for (int i = 0; i < m_auvsSingle; ++i)
	{
		ApplicationContainer lDataApp = longDataApp.Install (auvsSingleCon.Get(i));
		lDataApp.Start (Seconds (m_simStart));
		lDataApp.Stop (Seconds (m_simStop - m_preStop));
	}

	std::cout << "Set application for the cluster head AUV..." << std::endl;
	for (int i = 1; i < (m_auvsGroup - 1); ++i)
	{
		ApplicationContainer lDataApp = longDataApp.Install (auvsGroupCon.Get(i));
		lDataApp.Start (Seconds (m_simStart));
		lDataApp.Stop (Seconds (m_simStop - m_preStop));
	}

	// ------------- Inner Cluster Upload----------
	std::cout << "Set application for the cluster AUVs..." << std::endl;
	for (int i = 0; i < m_auvsGroup; ++i)
	{
		if (i != 1)
		{
			ApplicationContainer sDataApp = shortDataApp.Install (auvsGroupCon.Get(i));
			sDataApp.Start (Seconds (m_simStart));
			sDataApp.Stop (Seconds (m_simStop - m_preStop));
		}
	}


  Packet::EnablePrinting ();  //for debugging purposes
  std::cout << "-----------Running Simulation-----------\n";
  Simulator::Stop(Seconds(m_simStop));
  Simulator::Run();
  asHelper.GetChannel()->PrintCounters();
  Simulator::Destroy();
 


  std::cout << "fin.\n";
  return 0;
}
