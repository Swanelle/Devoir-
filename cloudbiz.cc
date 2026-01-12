/* CloudBiz CM - Simple Version for NS-3.29 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/netanim-module.h"

using namespace ns3;

int main(int argc, char* argv[])
{
    // Configuration simple
    uint32_t nClients = 5;
    double simTime = 15.0;

    std::cout << "\n=== CloudBiz CM Simulation ===\n";
    std::cout << "Clients: " << nClients << "\n";
    std::cout << "Time: " << simTime << "s\n\n";

    // Noeuds
    NodeContainer server, router, ap, clients;
    server.Create(1);
    router.Create(1);
    ap.Create(1);
    clients.Create(nClients);

    // Internet Stack
    InternetStackHelper stack;
    stack.Install(server);
    stack.Install(router);
    stack.Install(ap);
    stack.Install(clients);

    // P2P Links
    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue("100Mbps"));
    p2p.SetChannelAttribute("Delay", StringValue("5ms"));

    NetDeviceContainer d1 = p2p.Install(server.Get(0), router.Get(0));
    NetDeviceContainer d2 = p2p.Install(router.Get(0), ap.Get(0));

    // WiFi
    YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default();
    YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default();
    wifiPhy.SetChannel(wifiChannel.Create());

    WifiHelper wifi;
    wifi.SetStandard(WIFI_PHY_STANDARD_80211b);

    WifiMacHelper wifiMac;
    Ssid ssid = Ssid("CloudBiz");

    wifiMac.SetType("ns3::StaWifiMac", "Ssid", SsidValue(ssid));
    NetDeviceContainer staDevices = wifi.Install(wifiPhy, wifiMac, clients);

    wifiMac.SetType("ns3::ApWifiMac", "Ssid", SsidValue(ssid));
    NetDeviceContainer apDevice = wifi.Install(wifiPhy, wifiMac, ap);

    // Mobilite
    MobilityHelper mobility;
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(server);
    mobility.Install(router);
    mobility.Install(ap);

    mobility.SetPositionAllocator("ns3::GridPositionAllocator",
                                  "MinX", DoubleValue(0.0),
                                  "MinY", DoubleValue(0.0),
                                  "DeltaX", DoubleValue(5.0),
                                  "DeltaY", DoubleValue(10.0),
                                  "GridWidth", UintegerValue(3),
                                  "LayoutType", StringValue("RowFirst"));
    mobility.SetMobilityModel("ns3::RandomWalk2dMobilityModel",
                              "Bounds", RectangleValue(Rectangle(-50, 50, -50, 50)));
    mobility.Install(clients);

    // IP Addresses
    Ipv4AddressHelper address;

    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer i1 = address.Assign(d1);

    address.SetBase("10.1.2.0", "255.255.255.0");
    address.Assign(d2);

    address.SetBase("192.168.1.0", "255.255.255.0");
    address.Assign(staDevices);
    address.Assign(apDevice);

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    // Applications
    UdpEchoServerHelper echoServer(9);
    ApplicationContainer serverApp = echoServer.Install(server.Get(0));
    serverApp.Start(Seconds(1.0));
    serverApp.Stop(Seconds(simTime));

    UdpEchoClientHelper echoClient(i1.GetAddress(0), 9);
    echoClient.SetAttribute("MaxPackets", UintegerValue(100));
    echoClient.SetAttribute("Interval", TimeValue(Seconds(0.1)));
    echoClient.SetAttribute("PacketSize", UintegerValue(1024));

    ApplicationContainer clientApps = echoClient.Install(clients);
    clientApps.Start(Seconds(2.0));
    clientApps.Stop(Seconds(simTime));

    // NetAnim
    AnimationInterface anim("cloudbiz-animation.xml");
    anim.UpdateNodeDescription(server.Get(0), "Server");
    anim.UpdateNodeDescription(router.Get(0), "Router");
    anim.UpdateNodeDescription(ap.Get(0), "WiFi-AP");

    // Run
    std::cout << "Starting simulation...\n";
    Simulator::Stop(Seconds(simTime));
    Simulator::Run();

    std::cout << "\n=== DONE ===\n";
    std::cout << "File created: cloudbiz-animation.xml\n";
    std::cout << "Open with NetAnim!\n\n";

    Simulator::Destroy();
    return 0;
}
