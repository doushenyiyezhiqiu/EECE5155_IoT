
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("SecondScriptExample");

int 
main (int argc, char *argv[])
{
  bool verbose = true;
  uint32_t nCsma1 = 2;
  uint32_t nCsma2 = 2;

  CommandLine cmd (__FILE__);
  cmd.AddValue ("nCsma1", "Number of \"extra\" CSMA1 nodes/devices", nCsma1);
  cmd.AddValue ("nCsma2", "Number of \"extra\" CSMA2 nodes/devices", nCsma2);
  cmd.AddValue ("verbose", "Tell echo applications to log if true", verbose);

  cmd.Parse (argc,argv);

  if (verbose)
    {
      LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
      LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
    }

  nCsma1 = nCsma1 == 0 ? 1 : nCsma1;
  nCsma2 = nCsma2 == 0 ? 1 : nCsma2;
  
    
  NodeContainer p2pNodes;
  p2pNodes.Create(2);

  NodeContainer csmaNodes1;
  csmaNodes1.Add (p2pNodes.Get(0));
  csmaNodes1.Create (nCsma1);
  
  NodeContainer csmaNodes2;
  csmaNodes2.Add (p2pNodes.Get(1));
  csmaNodes2.Create (nCsma2);

  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("10Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

  NetDeviceContainer p2pDevices;
  p2pDevices = pointToPoint.Install (p2pNodes);

  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", StringValue ("100Mbps"));
  csma.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (10000)));

  NetDeviceContainer csmaDevices1;
  csmaDevices1 = csma.Install (csmaNodes1);
  
  NetDeviceContainer csmaDevices2;
  csmaDevices2 = csma.Install (csmaNodes2);

  InternetStackHelper stack;
  stack.Install (csmaNodes1);
  stack.Install (csmaNodes2);

  Ipv4AddressHelper address;
  address.SetBase ("192.168.3.0", "255.255.255.0");
  Ipv4InterfaceContainer p2pInterfaces;
  p2pInterfaces = address.Assign (p2pDevices);

  address.SetBase ("192.168.1.0", "255.255.255.0");
  Ipv4InterfaceContainer csmaInterfaces1;
  csmaInterfaces1 = address.Assign (csmaDevices1);
  
   address.SetBase ("192.168.2.0", "255.255.255.0");
  Ipv4InterfaceContainer csmaInterfaces2;
  csmaInterfaces2 = address.Assign (csmaDevices2);

  UdpEchoServerHelper echoServer (21);

  ApplicationContainer serverApps = echoServer.Install (csmaNodes1.Get (nCsma1-1));
  serverApps.Start (Seconds (3.0));
  serverApps.Stop (Seconds (15.0));

  UdpEchoClientHelper echoClient (csmaInterfaces1.GetAddress (nCsma1-1), 21);
  echoClient.SetAttribute ("MaxPackets", UintegerValue (2));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (3.0)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (256));

  ApplicationContainer clientApps = echoClient.Install (csmaNodes2.Get (nCsma2));
  clientApps.Start (Seconds (4.0));
  clientApps.Stop (Seconds (15.0));

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
  
  pointToPoint.EnablePcap("second",p2pDevices.Get(0),true);
  csma.EnablePcap ("second", csmaDevices1.Get (0), true);
  csma.EnablePcap ("second", csmaDevices2.Get (1), true);

  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
  
  }
