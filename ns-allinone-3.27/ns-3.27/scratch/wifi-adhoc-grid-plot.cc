/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009 University of Washington
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

//
// This program configures a grid (default 5x5) of nodes on an
// 802.11b physical layer, with
// 802.11b NICs in adhoc mode, and by default, sends one packet of 1000
// (application) bytes to node 1.
//
// The default layout is like this, on a 2-D grid.
//
// n20  n21  n22  n23  n24
// n15  n16  n17  n18  n19
// n10  n11  n12  n13  n14
// n5   n6   n7   n8   n9
// n0   n1   n2   n3   n4
//
// the layout is affected by the parameters given to GridPositionAllocator;
// by default, GridWidth is 5 and numNodes is 25..
//
// There are a number of command-line options available to control
// the default behavior.  The list of available command-line options
// can be listed with the following command:
// ./waf --run "wifi-simple-adhoc-grid --help"
//
// Note that all ns-3 attributes (not just the ones exposed in the below
// script) can be changed at command line; see the ns-3 documentation.
//
// For instance, for this configuration, the physical layer will
// stop successfully receiving packets when distance increases beyond
// the default of 500m.
// To see this effect, try running:
//
// ./waf --run "wifi-simple-adhoc --distance=500"
// ./waf --run "wifi-simple-adhoc --distance=1000"
// ./waf --run "wifi-simple-adhoc --distance=1500"
//
// The source node and sink node can be changed like this:
//
// ./waf --run "wifi-simple-adhoc --sourceNode=20 --sinkNode=10"
//
// This script can also be helpful to put the Wifi layer into verbose
// logging mode; this command will turn on all wifi logging:
//
// ./waf --run "wifi-simple-adhoc-grid --verbose=1"
//
// By default, trace file writing is off-- to enable it, try:
// ./waf --run "wifi-simple-adhoc-grid --tracing=1"
//
// When you are done tracing, you will notice many pcap trace files
// in your directory.  If you have tcpdump installed, you can try this:
//
// tcpdump -r wifi-simple-adhoc-grid-0-0.pcap -nn -tt
//
#include "ns3/gnuplot.h"
#include "ns3/core-module.h"
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h"
#include "ns3/internet-module.h"
#include "ns3/olsr-helper.h"
#include "ns3/reliability-module.h"
#include "ns3/energy-module.h"
using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("WifiSimpleAdhocGrid");

double totalPower = 0;
double totalTemperature = 0;
double averagePower = 0;
double averageTemperature = 0;
double avgcount = 0;

std::string fileNameWithNoExtension1 = "avgpower_plot";
std::string fileNameWithNoExtension2 = "avgtemperature_plot";
std::string graphicsFileName1        = fileNameWithNoExtension1 + ".png";
std::string graphicsFileName2        = fileNameWithNoExtension2 + ".png";
std::string plotFileName1            = fileNameWithNoExtension1 + ".plt";
std::string plotFileName2            = fileNameWithNoExtension2 + ".plt";
std::string plotTitle1               = "Average RPi Power vs Time";
std::string plotTitle2               = "Average RPi Temperature vs Time";
std::string data1Title               = "Average RPi Power";
std::string data2Title               = "Average Rpi Temperature";

// Instantiate the plot and set its title.
Gnuplot plot1 (graphicsFileName1);
Gnuplot plot2 (graphicsFileName2);

// Instantiate the dataset, set its title, and make the points be
// plotted along with connecting lines.
Gnuplot2dDataset dataset1;
Gnuplot2dDataset dataset2;

void ReceivePacket (Ptr<Socket> socket)
{
  while (socket->Recv ())
    {
      NS_LOG_UNCOND ("Received one packet!");
    }
}

void
Plotter(void)
{
  // Add the dataset to the plot.
  plot1.AddDataset (dataset1);
  plot2.AddDataset (dataset2);

  // Open the plot file.
  std::ofstream plotFile1 (plotFileName1.c_str());
  std::ofstream plotFile2 (plotFileName2.c_str());

  // Write the plot file.
  plot1.GenerateOutput (plotFile1);
  plot2.GenerateOutput (plotFile2);

  // Close the plot file.
  plotFile1.close ();
  plotFile2.close ();

}

static void GenerateTraffic (Ptr<Socket> socket, uint32_t pktSize,
                             uint32_t pktCount, Time pktInterval )
{
  if (pktCount > 0)
    {
      socket->Send (Create<Packet> (pktSize));
      Simulator::Schedule (pktInterval, &GenerateTraffic,
                           socket, pktSize,pktCount - 1, pktInterval);
    }
  else
    {
      socket->Close ();
    }
}

void
PrintInfo (NodeContainer nodes)
{
  double count = 0;
  double sum_power = 0;
  double sum_temp = 0;
  double avg_power = 0;
  double avg_temp = 0;

  for(uint32_t i=0;i<nodes.GetN();i++)
  {
    count += 1;
    std::cout<<"At time "<< Simulator::Now().GetSeconds()<<", NodeId = "<<nodes.Get(i)->GetId();
    std::cout << " CPU Power = " << nodes.Get(i)->GetObject<PowerModel>()->GetPower();
    std::cout << " Temperature = " << nodes.Get(i)->GetObject<TemperatureModel>()->GetTemperature();
    std::cout << " Reliability = " << nodes.Get(i)->GetObject<ReliabilityModel>()->GetReliability()<<std::endl;

    sum_power += nodes.Get(i)->GetObject<PowerModel>()->GetPower();
    sum_temp += nodes.Get(i)->GetObject<TemperatureModel>()->GetTemperature();
  }
  avg_power = sum_power/count;
  avg_temp = sum_temp/count;
  totalPower += avg_power;
  totalTemperature += avg_temp;
  avgcount += 1;

  dataset1.Add(Simulator::Now ().GetSeconds (), avg_power);
  dataset2.Add(Simulator::Now ().GetSeconds (), avg_temp);

  if (!Simulator::IsFinished ())
  {
    Simulator::Schedule (Seconds (0.5),&PrintInfo,nodes);
  }
}

void
PrintAverages(void)
{
  
  averagePower = totalPower/avgcount;
  averageTemperature = totalTemperature/avgcount;
  std::cout << "Average power = " << averagePower << std::endl;
  std::cout << "Average temperature = " << averageTemperature << std::endl;
}

int main (int argc, char *argv[])
{

  plot1.SetTitle (plotTitle1);
  plot2.SetTitle (plotTitle2);

  // Make the graphics file, which the plot file will create when it
  // is used with Gnuplot, be a PNG file.
  plot1.SetTerminal ("png");
  plot2.SetTerminal ("png");

  // Set the labels for each axis.
  plot1.SetLegend ("time(s)", "Average Power(W)");
  plot2.SetLegend ("time(s)", "Average Temperature(°C)");

  dataset1.SetTitle (data1Title);
  dataset1.SetStyle (Gnuplot2dDataset::LINES_POINTS);
  dataset2.SetTitle (data2Title);
  dataset2.SetStyle (Gnuplot2dDataset::LINES_POINTS);




  std::string phyMode ("DsssRate1Mbps");
  double distance = 500;  // m
  uint32_t packetSize = 1000; // bytes
  uint32_t dataSize = 100000;   // bytes
  uint32_t numPackets = 1000;
  uint32_t numNodes = 25;  // by default, 5x5
  uint32_t sinkNode = 0;
  uint32_t sourceNode = 24;
  double interval = 0.001; // seconds
  bool verbose = false;
  bool tracing = false;

  CommandLine cmd;

  cmd.AddValue ("phyMode", "Wifi Phy mode", phyMode);
  cmd.AddValue ("distance", "distance (m)", distance);
  cmd.AddValue ("packetSize", "size of application packet sent", packetSize);
  cmd.AddValue ("numPackets", "number of packets generated", numPackets);
  cmd.AddValue ("interval", "interval (seconds) between packets", interval);
  cmd.AddValue ("verbose", "turn on all WifiNetDevice log components", verbose);
  cmd.AddValue ("tracing", "turn on ascii and pcap tracing", tracing);
  cmd.AddValue ("numNodes", "number of nodes", numNodes);
  cmd.AddValue ("sinkNode", "Receiver node number", sinkNode);
  cmd.AddValue ("sourceNode", "Sender node number", sourceNode);

  cmd.Parse (argc, argv);
  // Convert to time object
  Time interPacketInterval = Seconds (interval);

  // disable fragmentation for frames below 2200 bytes
  Config::SetDefault ("ns3::WifiRemoteStationManager::FragmentationThreshold", StringValue ("2200"));
  // turn off RTS/CTS for frames below 2200 bytes
  Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue ("2200"));
  // Fix non-unicast data rate to be the same as that of unicast
  Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode",
                      StringValue (phyMode));

  NodeContainer c;
  c.Create (numNodes);

  // The below set of helpers will help us to put together the wifi NICs we want
  WifiHelper wifi;
  if (verbose)
    {
      wifi.EnableLogComponents ();  // Turn on all Wifi logging
    }

  YansWifiPhyHelper wifiPhy =  YansWifiPhyHelper::Default ();
  // set it to zero; otherwise, gain will be added
  wifiPhy.Set ("RxGain", DoubleValue (-10) );
  // ns-3 supports RadioTap and Prism tracing extensions for 802.11b
  wifiPhy.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11_RADIO);

  YansWifiChannelHelper wifiChannel;
  wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  wifiChannel.AddPropagationLoss ("ns3::FriisPropagationLossModel");
  wifiPhy.SetChannel (wifiChannel.Create ());

  // Add an upper mac and disable rate control
  WifiMacHelper wifiMac;
  wifi.SetStandard (WIFI_PHY_STANDARD_80211b);
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                "DataMode",StringValue (phyMode),
                                "ControlMode",StringValue (phyMode));
  // Set it to adhoc mode
  wifiMac.SetType ("ns3::AdhocWifiMac");
  NetDeviceContainer devices = wifi.Install (wifiPhy, wifiMac, c);

  MobilityHelper mobility;
  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (0.0),
                                 "MinY", DoubleValue (0.0),
                                 "DeltaX", DoubleValue (distance),
                                 "DeltaY", DoubleValue (distance),
                                 "GridWidth", UintegerValue (5),
                                 "LayoutType", StringValue ("RowFirst"));
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (c);

  /** Energy Model **/
  /***************************************************************************/
  /* energy source */
  BasicEnergySourceHelper basicSourceHelper;
  // configure energy source
  basicSourceHelper.Set ("BasicEnergySourceInitialEnergyJ", DoubleValue (100000));
  // install source
  EnergySourceContainer sources = basicSourceHelper.Install (c);
  /* reliability stack */
  ReliabilityHelper reliabilityHelper;
  reliabilityHelper.SetDeviceType("RaspberryPi");
  reliabilityHelper.SetPowerModel("ns3::AppPowerModel");
  reliabilityHelper.SetPerformanceModel("ns3::PerformanceSimpleModel");
  reliabilityHelper.SetTemperatureModel("ns3::TemperatureSimpleModel");
  reliabilityHelper.SetReliabilityModel("ns3::ReliabilityTDDBModel");
  reliabilityHelper.SetApplication("AdaBoost",dataSize,packetSize);
  reliabilityHelper.Install(c);
  /* cpu energy model */
   CpuEnergyModelHelper cpuEnergyHelper;
   cpuEnergyHelper.Install(devices, sources);
  /***************************************************************************/

  // Enable OLSR
  OlsrHelper olsr;
  Ipv4StaticRoutingHelper staticRouting;

  Ipv4ListRoutingHelper list;
  list.Add (staticRouting, 0);
  list.Add (olsr, 10);

  InternetStackHelper internet;
  internet.SetRoutingHelper (list); // has effect on the next Install ()
  internet.Install (c);

  Ipv4AddressHelper ipv4;
  NS_LOG_INFO ("Assign IP Addresses.");
  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer i = ipv4.Assign (devices);

  TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
  Ptr<Socket> recvSink = Socket::CreateSocket (c.Get (sinkNode), tid);
  InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), 80);
  recvSink->Bind (local);
  recvSink->SetRecvCallback (MakeCallback (&ReceivePacket));

  Ptr<Socket> source = Socket::CreateSocket (c.Get (sourceNode), tid);
  InetSocketAddress remote = InetSocketAddress (i.GetAddress (sinkNode, 0), 80);
  source->Connect (remote);

  PrintInfo (c);

  if (tracing == true)
    {
      AsciiTraceHelper ascii;
      wifiPhy.EnableAsciiAll (ascii.CreateFileStream ("wifi-simple-adhoc-grid.tr"));
      wifiPhy.EnablePcap ("wifi-simple-adhoc-grid", devices);
      // Trace routing tables
      Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper> ("wifi-simple-adhoc-grid.routes", std::ios::out);
      olsr.PrintRoutingTableAllEvery (Seconds (2), routingStream);
      Ptr<OutputStreamWrapper> neighborStream = Create<OutputStreamWrapper> ("wifi-simple-adhoc-grid.neighbors", std::ios::out);
      olsr.PrintNeighborCacheAllEvery (Seconds (2), neighborStream);

      // To do-- enable an IP-level trace that shows forwarding events only
    }

  // Give OLSR time to converge-- 30 seconds perhaps
  Simulator::Schedule (Seconds (30.0), &GenerateTraffic,
                       source, packetSize, numPackets, interPacketInterval);

  // Output what we are doing
  NS_LOG_UNCOND ("Testing from node " << sourceNode << " to " << sinkNode << " with grid distance " << distance);

  Simulator::Stop (Seconds (1000.0));
  Simulator::Schedule (Seconds(999.0), &Plotter);
  Simulator::Schedule (Seconds(999.9), &PrintAverages);
  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}

