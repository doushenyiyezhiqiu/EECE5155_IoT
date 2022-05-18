/*
 * Copyright (c) 2009 The Boeing Company
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
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/csma-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("LAB2 TASK1");

int main (int argc, char **argv) {
  bool verbose = false;
  // bool tracing = false;
  uint32_t nWifi = 5;

  CommandLine cmd (__FILE__);
  cmd.Parse (argc, argv);
  cmd.AddValue ("nWifi", "Number of wifi Ad-hoc devices", nWifi);
  cmd.AddValue ("verbose", "Tell echo applications to log if true", verbose);
  // cmd.AddValue ("tracing", "Enable pcap tracing", tracing);
  cmd.Parse (argc, argv);

  // The underlying restriction of 18 is due to the grid position
  // allocator's configuration; the grid layout will exceed the
  // bounding box if more than 18 nodes are provided.
  if (nWifi > 18) {
    std::cout << "nWifi should be 18 or less; otherwise grid layout exceeds the bounding box"
      << std::endl;
    return 1;
  }

  Time::SetResolution (Time::NS);
  LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);

  NodeContainer wifiNodes;
  wifiNodes.Create (nWifi);

  YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
  YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();
  phy.SetChannel (channel.Create ());

  WifiHelper wifi;
  wifi.SetRemoteStationManager ("ns3::AarfWifiManager");
  wifi.SetStandard (WIFI_PHY_STANDARD_80211g);
  
  if (verbose) {
    wifi.EnableLogComponents ();  // Turn on all Wifi logging
  }

  // set WIFI standard to 802.11G
  // Adhoc mode WIFI MAC
  WifiMacHelper mac;
  mac.SetType("ns3::AdhocWifiMac");
  
  NetDeviceContainer devices = wifi.Install (phy, mac, wifiNodes);

  MobilityHelper mobility;

  // mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
  //                                "MinX", DoubleValue (0.0),
  //                                "MinY", DoubleValue (0.0),
  //                                "DeltaX", DoubleValue (5.0),
  //                                "DeltaY", DoubleValue (10.0),
  //                                "GridWidth", UintegerValue (3),
  //                                "LayoutType", StringValue ("RowFirst"));

  mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
                             "Bounds", 
                             RectangleValue (Rectangle (-50, 50, -50, 50)));
  
  mobility.Install (wifiNodes);

  InternetStackHelper internet;
  internet.Install (wifiNodes);

  Ipv4AddressHelper address;
  address.SetBase ("192.168.1.0", "255.255.255.0");

  Ipv4InterfaceContainer interface;
  interface = address.Assign (devices);

  UdpEchoServerHelper echoServer (20);

  ApplicationContainer serverApp = echoServer.Install (wifiNodes.Get (0));
  serverApp.Start (Seconds (0.0));
  serverApp.Stop (Seconds (6.0));

  // Node 3 UDP echo client
  UdpEchoClientHelper echoClient0 (interface.GetAddress(0), 20);
  echoClient0.SetAttribute ("MaxPackets", UintegerValue (2));
  echoClient0.SetAttribute ("Interval", TimeValue (Seconds (2.0)));
  echoClient0.SetAttribute ("PacketSize", UintegerValue (512));

  // Node 4 UDP echo client
  UdpEchoClientHelper echoClient1 (interface.GetAddress (0), 20);
  echoClient1.SetAttribute ("MaxPackets", UintegerValue (2));
  echoClient1.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient1.SetAttribute ("PacketSize", UintegerValue (512));

  // ApplicationContainer clientApp = echoClient0.Install (wifiNodes.Get (3));
  // clientApp.Start (Seconds (2.0));
  // clientApp.Stop (Seconds (6.0));

  ApplicationContainer clientApps[2];
  clientApps[0] = echoClient0.Install (wifiNodes.Get (3));
  clientApps[0].Start (Seconds (2.0));
  clientApps[0].Stop (Seconds (6.0));

  clientApps[1] = echoClient1.Install (wifiNodes.Get (4));
  clientApps[1].Start (Seconds (1.0));
  clientApps[1].Stop (Seconds (6.0));


  // Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  Simulator::Stop (Seconds (6.0));

  phy.EnablePcap("lab2_task1", devices.Get(2));

  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}