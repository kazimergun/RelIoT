/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2010 Network Security Lab, University of Washington, Seattle.
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
 */
#include "ns3/gnuplot.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/config-store-module.h"
#include "ns3/wifi-module.h"
#include "ns3/energy-module.h"
#include "ns3/internet-module.h"
#include "ns3/reliability-module.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

using namespace ns3;

std::string fileNameWithNoExtension1 = "power_plot";
std::string fileNameWithNoExtension2 = "temperature_plot";
std::string fileNameWithNoExtension3 = "totalenergy_plot";
std::string graphicsFileName1        = fileNameWithNoExtension1 + ".png";
std::string graphicsFileName2        = fileNameWithNoExtension2 + ".png";
std::string graphicsFileName3        = fileNameWithNoExtension3 + ".png";
std::string plotFileName1            = fileNameWithNoExtension1 + ".plt";
std::string plotFileName2            = fileNameWithNoExtension2 + ".plt";
std::string plotFileName3            = fileNameWithNoExtension3 + ".plt";
std::string plotTitle1               = "Power vs Time";
std::string plotTitle2               = "Temperature vs Time";
std::string plotTitle3               = "Total Energy Consumption vs Time";
std::string data1Title               = "Power";
std::string data2Title               = "Temperature";
std::string data3Title               = "Total Energy";

// Instantiate the plot and set its title.
Gnuplot plot1 (graphicsFileName1);
Gnuplot plot2 (graphicsFileName2);
Gnuplot plot3 (graphicsFileName3);
// Instantiate the dataset, set its title, and make the points be
// plotted along with connecting lines.
Gnuplot2dDataset dataset1;
Gnuplot2dDataset dataset2;
Gnuplot2dDataset dataset3;



NS_LOG_COMPONENT_DEFINE ("ReliabilityExample");


double totalPower = 0;
double totalTemperature = 0;
double averagePower = 0;
double averageTemperature = 0;
double count = 0;
double totalEnergy = 0;

static inline std::string
PrintReceivedPacket (Address& from)
{
  InetSocketAddress iaddr = InetSocketAddress::ConvertFrom (from);

  std::ostringstream oss;
  oss << "--\nReceived one packet! Socket: " << iaddr.GetIpv4 ()
      << " port: " << iaddr.GetPort ()
      << " at time = " << Simulator::Now ().GetSeconds ()
      << "\n--";

  return oss.str ();
}

/**
 * \param socket Pointer to socket.
 *
 * Packet receiving sink.
 */
void
ReceivePacket (Ptr<Socket> socket)
{
  Ptr<Packet> packet;
  Address from;
  while ((packet = socket->RecvFrom (from)))
    {
      if (packet->GetSize () > 0)
        {
          NS_LOG_UNCOND (PrintReceivedPacket (from));
        }
    }
}


void
Plotter(void)
{
  // Add the dataset to the plot.
  plot1.AddDataset (dataset1);
  plot2.AddDataset (dataset2);
  plot3.AddDataset (dataset3);
  // Open the plot file.
  std::ofstream plotFile1 (plotFileName1.c_str());
  std::ofstream plotFile2 (plotFileName2.c_str());
  std::ofstream plotFile3 (plotFileName3.c_str());
  // Write the plot file.
  plot1.GenerateOutput (plotFile1);
  plot2.GenerateOutput (plotFile2);
  plot3.GenerateOutput (plotFile3);
  // Close the plot file.
  plotFile1.close ();
  plotFile2.close ();
  plotFile3.close ();
}


/**
 * \param socket Pointer to socket.
 * \param pktSize Packet size.
 * \param n Pointer to node.
 * \param pktCount Number of packets to generate.
 * \param pktInterval Packet sending interval.
 *
 * Traffic generator.
 */
static void
GenerateTraffic (Ptr<Socket> socket, uint32_t pktSize, Ptr<Node> n,
                 uint32_t pktCount, Time pktInterval)
{
  if (pktCount > 0)
    {
      socket->Send (Create<Packet> (pktSize));
      Simulator::Schedule (pktInterval, &GenerateTraffic, socket, pktSize, n,
                           pktCount - 1, pktInterval);
    }
  else
    {
      socket->Close ();
    }
}

/// Trace function for remaining energy at node.
void
RemainingEnergy (double oldValue, double remainingEnergy)
{
  NS_LOG_UNCOND (Simulator::Now ().GetSeconds ()
                 << "s Current remaining energy = " << remainingEnergy << "J");
}

/// Trace function for total energy consumption at node.
void
TotalEnergy (double oldValue, double totalEnergy)
{
  NS_LOG_UNCOND (Simulator::Now ().GetSeconds ()
                 << "s Total energy consumed = " << totalEnergy << "J");
}

void
PrintInfo (Ptr<Node> node, Ptr<DeviceEnergyModel> cpuEnergyModel)
{

  std::cout<< "At time "<< Simulator::Now().GetSeconds()<<", NodeId = "<<node->GetId();
  std::cout << " CPU Power = " << node->GetObject<PowerModel>()->GetPower();
  std::cout << " Temperature = " << node->GetObject<TemperatureModel>()->GetTemperature()<<std::endl;
  std::cout << " Reliability = " << node->GetObject<ReliabilityModel>()->GetReliability()<<std::endl;
  //std::cout << " Total Energy Consumption = " << node->GetObject<DeviceEnergyModel>()->GetTotalEnergyConsumption()<<std::endl;
  //std::cout << "Total Energy Consumption = " << cpuEnergyModel->GetTotalEnergyConsumption()<<std::endl;
  std::cout << "Total Energy Consumption = " << totalEnergy << std::endl;
  dataset1.Add(Simulator::Now ().GetSeconds (), node->GetObject<PowerModel>()->GetPower());
  dataset2.Add(Simulator::Now ().GetSeconds (), node->GetObject<TemperatureModel>()->GetTemperature());
  
  totalPower += node->GetObject<PowerModel>()->GetPower();
  totalTemperature += node->GetObject<TemperatureModel>()->GetTemperature();
  totalEnergy += 0.5*(node->GetObject<PowerModel>()->GetPower());
  count +=1;
  dataset3.Add(Simulator::Now ().GetSeconds (), totalEnergy);


  if (!Simulator::IsFinished ())
  {
    Simulator::Schedule (Seconds (0.5),&PrintInfo,node,cpuEnergyModel);
  }
}

void
PrintAverages(void)
{
  
  averagePower = totalPower/count;
  averageTemperature = totalTemperature/count;
  std::cout << "Average power = " << averagePower << std::endl;
  std::cout << "Average temperature = " << averageTemperature << std::endl;
}


int
main (int argc, char *argv[])
{

  plot1.SetTitle (plotTitle1);
  plot2.SetTitle (plotTitle2);
  plot2.SetTitle (plotTitle3);
  // Make the graphics file, which the plot file will create when it
  // is used with Gnuplot, be a PNG file.
  plot1.SetTerminal ("png");
  plot2.SetTerminal ("png");
  plot3.SetTerminal ("png");
  // Set the labels for each axis.
  plot1.SetLegend ("time(s)", "Power(W)");
  plot2.SetLegend ("time(s)", "Temperature(°C)");
  plot3.SetLegend ("time(s)", "Total Energy(J)");

  dataset1.SetTitle (data1Title);
  dataset1.SetStyle (Gnuplot2dDataset::LINES_POINTS);
  dataset2.SetTitle (data2Title);
  dataset2.SetStyle (Gnuplot2dDataset::LINES_POINTS);
  dataset3.SetTitle (data3Title);
  dataset3.SetStyle (Gnuplot2dDataset::LINES_POINTS);
  /*
  LogComponentEnable ("CpuEnergyModel", LOG_LEVEL_DEBUG);
  LogComponentEnable ("PowerLinearModel", LOG_LEVEL_DEBUG);
  LogComponentEnable ("TemperatureSimpleModel", LOG_LEVEL_DEBUG);
  LogComponentEnable ("ReliabilityTDDBModel", LOG_LEVEL_DEBUG);
   */
  //LogComponentEnable ("CpuEnergyModel", LOG_LEVEL_DEBUG);
  LogComponentEnable ("AppPowerModel", LOG_LEVEL_DEBUG);
  //LogComponentEnable ("PowerModel", LOG_LEVEL_DEBUG);
  //LogComponentEnable ("TemperatureSimpleModel", LOG_LEVEL_DEBUG);

  std::string phyMode ("DsssRate1Mbps");
  double Prss = -80;            // dBm
  uint32_t PpacketSize = 2000;   // bytes
  bool verbose = false;
  uint32_t dataSize = 100000;   // bytes
  // simulation parameters
  uint32_t numPackets = 1000000;  // number of packets to send
  double interval = 0.0005;          // seconds
  double startTime = 0.0;       // seconds
  double distanceToRx = 100.0;  // meters
  double Tenv = 25.0;
  /*
   * This is a magic number used to set the transmit power, based on other
   * configuration.
   */
  double offset = 81;

  CommandLine cmd;
  cmd.AddValue ("phyMode", "Wifi Phy mode", phyMode);
  cmd.AddValue ("Prss", "Intended primary RSS (dBm)", Prss);
  cmd.AddValue ("PpacketSize", "size of application packet sent", PpacketSize);
  cmd.AddValue ("numPackets", "Total number of packets to send", numPackets);
  cmd.AddValue ("startTime", "Simulation start time", startTime);
  cmd.AddValue ("distanceToRx", "X-Axis distance between nodes", distanceToRx);
  cmd.AddValue ("verbose", "Turn on all device log components", verbose);
  cmd.Parse (argc, argv);

  // Convert to time object
  Time interPacketInterval = Seconds (interval);

  // disable fragmentation for frames below 2200 bytes
  Config::SetDefault ("ns3::WifiRemoteStationManager::FragmentationThreshold",
                      StringValue ("2200"));
  // turn off RTS/CTS for frames below 2200 bytes
  Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold",
                      StringValue ("2200"));
  // Fix non-unicast data rate to be the same as that of unicast
  Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode",
                      StringValue (phyMode));


  Ptr<Node> sourceNode = CreateObject<Node>();
  Ptr<Node> rpiNode = CreateObject<Node>();
  Ptr<Node> serverNode = CreateObject<Node>();
  NodeContainer networkNodes = NodeContainer(sourceNode);
  networkNodes.Add(rpiNode);
  networkNodes.Add(serverNode);

  // The below set of helpers will help us to put together the wifi NICs we want
  WifiHelper wifi;
  if (verbose)
    {
      wifi.EnableLogComponents ();
    }
  wifi.SetStandard (WIFI_PHY_STANDARD_80211b);

  /** Wifi PHY **/
  /***************************************************************************/
  YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();
  wifiPhy.Set ("RxGain", DoubleValue (-10));
  wifiPhy.Set ("TxGain", DoubleValue (offset + Prss));
  wifiPhy.Set ("CcaMode1Threshold", DoubleValue (0.0));
  /***************************************************************************/

  /** wifi channel **/
  YansWifiChannelHelper wifiChannel;
  wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  wifiChannel.AddPropagationLoss ("ns3::FriisPropagationLossModel");
  // create wifi channel
  Ptr<YansWifiChannel> wifiChannelPtr = wifiChannel.Create ();
  wifiPhy.SetChannel (wifiChannelPtr);

  /** MAC layer **/
  // Add a MAC and disable rate control
  WifiMacHelper wifiMac;
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode",
                                StringValue (phyMode), "ControlMode",
                                StringValue (phyMode));
  // Set it to ad-hoc mode
  wifiMac.SetType ("ns3::AdhocWifiMac");

  /** install PHY + MAC **/
  //NetDeviceContainer devices = wifi.Install (wifiPhy, wifiMac, networkNodes);
  NetDeviceContainer sourceDevice = wifi.Install (wifiPhy, wifiMac, sourceNode);
  NetDeviceContainer rpiDevice = wifi.Install (wifiPhy, wifiMac, rpiNode);
  NetDeviceContainer serverDevice = wifi.Install (wifiPhy, wifiMac, serverNode);
  NetDeviceContainer devices = NetDeviceContainer(sourceDevice, rpiDevice);
  devices.Add(serverDevice);


  /** mobility **/
  MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  positionAlloc->Add (Vector (0.0, 0.0, 0.0));
  positionAlloc->Add (Vector (2 * distanceToRx, 0.0, 0.0));
  mobility.SetPositionAllocator (positionAlloc);
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (networkNodes);

//   /** Energy Model **/
//   /***************************************************************************/
//   /* energy source */
//   BasicEnergySourceHelper basicSourceHelper;
//   // configure energy source
//   basicSourceHelper.Set ("BasicEnergySourceInitialEnergyJ", DoubleValue (100000));
//   // install source
//   EnergySourceContainer rpiSource = basicSourceHelper.Install (rpiNode);
//   /* reliability stack */
//   ReliabilityHelper reliabilityHelper;
//   reliabilityHelper.SetDeviceType("RaspberryPi");
//   reliabilityHelper.SetPowerModel("ns3::AppPowerModel");
//   reliabilityHelper.SetPerformanceModel("ns3::PerformanceSimpleModel");
//   reliabilityHelper.SetTemperatureModel("ns3::TemperatureSimpleModel");
//   reliabilityHelper.SetAmbientTemperature(Tenv);
//   reliabilityHelper.SetReliabilityModel("ns3::ReliabilityTDDBModel");
//   reliabilityHelper.SetApplication("AdaBoost",dataSize,PpacketSize);
//   reliabilityHelper.Install(rpiNode);
//   /* cpu energy model */
//    CpuEnergyModelHelper cpuEnergyHelper;
//    DeviceEnergyModelContainer deviceModels = cpuEnergyHelper.Install(rpiDevice, rpiSource);
//   /***************************************************************************/

//   Ptr<BasicEnergySource> basicSourcePtr = DynamicCast<BasicEnergySource> (rpiSource.Get (0));
//  // basicSourcePtr->TraceConnectWithoutContext ("RemainingEnergy", MakeCallback (&RemainingEnergy));
//   NS_ASSERT (basicSourcePtr != NULL);
//     // device energy model
//   Ptr<DeviceEnergyModel> cpuEnergyModelPtr =
//     basicSourcePtr->FindDeviceEnergyModels ("ns3::CpuEnergyModel").Get (0);
//   NS_ASSERT (cpuEnergyModelPtr != NULL);
//   //cpuEnergyModelPtr->TraceConnectWithoutContext ("TotalEnergyConsumption", MakeCallback (&TotalEnergy));
//   /***************************************************************************/


    /** Energy Model **/
  /***************************************************************************/
  /* energy source */
  BasicEnergySourceHelper basicSourceHelper;
  // configure energy source
  basicSourceHelper.Set ("BasicEnergySourceInitialEnergyJ", DoubleValue (100000));
  // install source
  EnergySourceContainer serverSource = basicSourceHelper.Install (serverNode);
  /* reliability stack */
  ReliabilityHelper reliabilityHelper;
  reliabilityHelper.SetDeviceType("Server");
  reliabilityHelper.SetPowerModel("ns3::AppPowerModel");
  reliabilityHelper.SetPerformanceModel("ns3::PerformanceSimpleModel");
  reliabilityHelper.SetTemperatureModel("ns3::TemperatureSimpleModel");
  reliabilityHelper.SetAmbientTemperature(Tenv);
  reliabilityHelper.SetReliabilityModel("ns3::ReliabilityTDDBModel");
  reliabilityHelper.SetApplication("AdaBoost",dataSize,PpacketSize);
  reliabilityHelper.Install(serverNode);
  /* cpu energy model */
   CpuEnergyModelHelper cpuEnergyHelper;
   DeviceEnergyModelContainer deviceModels = cpuEnergyHelper.Install(serverDevice, serverSource);
  /***************************************************************************/

  Ptr<BasicEnergySource> basicSourcePtr = DynamicCast<BasicEnergySource> (serverSource.Get (0));
 // basicSourcePtr->TraceConnectWithoutContext ("RemainingEnergy", MakeCallback (&RemainingEnergy));
  NS_ASSERT (basicSourcePtr != NULL);
    // device energy model
  Ptr<DeviceEnergyModel> cpuEnergyModelPtr =
    basicSourcePtr->FindDeviceEnergyModels ("ns3::CpuEnergyModel").Get (0);
  NS_ASSERT (cpuEnergyModelPtr != NULL);
  //cpuEnergyModelPtr->TraceConnectWithoutContext ("TotalEnergyConsumption", MakeCallback (&TotalEnergy));


  /** Internet stack **/
  InternetStackHelper internet;
  internet.Install (networkNodes);

  Ipv4AddressHelper ipv4;
  NS_LOG_INFO ("Assign IP Addresses.");
  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer i = ipv4.Assign (devices);

  TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
  Ptr<Socket> recvSink = Socket::CreateSocket (serverNode, tid);  // node 1, receiver
  InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), 80);
  recvSink->Bind (local);
  //recvSink->SetRecvCallback (MakeCallback (&ReceivePacket));

  Ptr<Socket> source = Socket::CreateSocket (sourceNode, tid);    // node 0, sender
  InetSocketAddress remote = InetSocketAddress (Ipv4Address::GetBroadcast (), 80);
  source->SetAllowBroadcast (true);
  source->Connect (remote);


//   PrintInfo (rpiNode,cpuEnergyModelPtr);
  PrintInfo (serverNode,cpuEnergyModelPtr);


  /** simulation setup **/
  // start traffic
  Simulator::Schedule (Seconds (startTime), &GenerateTraffic, source, PpacketSize,
                       networkNodes.Get (0), numPackets, interPacketInterval);

  Simulator::Stop (Seconds (100.0));
  Simulator::Schedule (Seconds(99.0), &Plotter);
  Simulator::Schedule (Seconds(99.9), &PrintAverages);

  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}
