#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/internet-module.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/ssid.h"
#include "ns3/config.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("ThirdScriptExample");

int 
main (int argc, char *argv[])
{
  bool verbose = true;
  uint32_t nWifi = 5;
  bool useRts = false;
  
  CommandLine cmd (__FILE__);
  cmd.AddValue ("nWifi", "Number of wifi STA devices", nWifi);
  cmd.AddValue ("verbose", "Tell echo applications to log if true", verbose);

  cmd.Parse (argc,argv);
  
    if (nWifi > 18)
    {
      std::cout << "nWifi should be 18 or less; otherwise grid layout exceeds the bounding box" << std::endl;
      return 1;
    }

  if (verbose)
    {
      LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
      LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
    }

  if (useRts)
    {
      Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue ("0"));
    }

  NodeContainer wifiAdhocNodes;
  wifiAdhocNodes.Create (nWifi);
  
  YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
  YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();
  phy.SetChannel (channel.Create ());

  WifiHelper wifi;
  wifi.SetRemoteStationManager ("ns3::AarfWifiManager");

  WifiMacHelper mac;
  Ssid ssid = Ssid ("ns-3-ssid");
  mac.SetType ("ns3::AdhocWifiMac",
               "Ssid", SsidValue (ssid));

  NetDeviceContainer adhocDevices;
  adhocDevices = wifi.Install (phy, mac, wifiAdhocNodes);

  MobilityHelper mobility;

  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (0.0),
                                 "MinY", DoubleValue (0.0),
                                 "DeltaX", DoubleValue (5.0),
                                 "DeltaY", DoubleValue (10.0),
                                 "GridWidth", UintegerValue (3),
                                 "LayoutType", StringValue ("RowFirst"));

  mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
                             "Bounds", RectangleValue (Rectangle (-90, 90, -90, 90)));
  mobility.Install (wifiAdhocNodes);

  InternetStackHelper stack;
  stack.Install (wifiAdhocNodes);
  
  Ipv4AddressHelper ipv4;
 
  ipv4.SetBase ("192.168.1.0", "255.255.255.0");
  Ipv4InterfaceContainer i = ipv4.Assign (adhocDevices);
  
  UdpEchoServerHelper echoServer (20);
    
  ApplicationContainer serverApps = echoServer.Install (wifiAdhocNodes.Get (nWifi-5));
  serverApps.Start (Seconds (0.5));
  serverApps.Stop (Seconds (10.0));
  
  UdpEchoClientHelper echoClient1 (i.GetAddress(nWifi-5), 20);
  echoClient1.SetAttribute ("MaxPackets", UintegerValue (2));
  echoClient1.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient1.SetAttribute ("PacketSize", UintegerValue (512));

  ApplicationContainer clientApps1 = 
    echoClient1.Install (wifiAdhocNodes.Get (nWifi - 1));
  clientApps1.Start (Seconds (1.0));
  clientApps1.Stop (Seconds (10.0));

  UdpEchoClientHelper echoClient2 (i.GetAddress(nWifi-5), 20);
  echoClient2.SetAttribute ("MaxPackets", UintegerValue (2));
  echoClient2.SetAttribute ("Interval", TimeValue (Seconds (2.0)));
  echoClient2.SetAttribute ("PacketSize", UintegerValue (512));

  ApplicationContainer clientApps2 = 
    echoClient2.Install (wifiAdhocNodes.Get (nWifi - 2));
  clientApps2.Start (Seconds (2.0));
  clientApps2.Stop (Seconds (10.0));

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  Simulator::Stop (Seconds (10.0));
  
  phy.EnablePcap ("mythird1", adhocDevices.Get (1));

  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}  
