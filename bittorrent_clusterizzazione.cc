/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
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
#include "stdlib.h"
#include "math.h"
#include "iostream"
#include <sstream>
#include "string"

#include "ns3/trace-helper.h"
#include "ns3/BitTorrentTracker.h"
#include "ns3/BitTorrentVideoClient.h"
#include "ns3/GlobalMetricsGatherer.h"
 
using namespace ns3;
using namespace std;
using namespace bittorrent;

ofstream miofiledilog;
string tempo;
double finito=0.0;
ApplicationContainer bitTorrentClients;
 
 // OGNI LOCAZIONE DELL'ARRAY È IL NUMERO DI NODI NEL LIVELLO I+1 ESIMO
//int levels[4]={10, 100, 334, 1002}; // Modello 1
int levels[4]={5, 50, 100, 200};      // Modello 2

// Permette di stampare secondo su secondo un Timer: tale da capire l'andamento dell'esecuzione corrente
void ShowTimePeriodic ()
{
  finito=Simulator::Now ().GetSeconds ();
  cout << "It is now " <<  Simulator::Now ().GetSeconds () << "s (" << GlobalMetricsGatherer::GetWallclockTime () << ")" << endl;
  Simulator::Schedule (Seconds (1.0), ShowTimePeriodic);
}

// Metodo per stampare il filelog dei PeerSet
void PeerSet()
{
	 miofiledilog.open("output/PeerSet.txt", ios_base::app);
	  for(unsigned int i=0; i< bitTorrentClients.GetN();i++){
	    vector<Ptr<Peer>> peerset= DynamicCast<BitTorrentClient> (bitTorrentClients.Get(i))->GetActivePeers();
	    if(peerset.empty()==false){
        //miofiledilog<<"_"<< endl;
	      // Ip del Client corrente
	      Ptr<Ipv4> x=bitTorrentClients.Get(i)->GetNode()->GetObject<Ipv4>();
	      Ipv4Address addr=x->GetAddress(1,0).GetLocal();
	      //miofiledilog<<"It is now " <<  Simulator::Now ().GetSeconds () << "s (" << GlobalMetricsGatherer::GetWallclockTime () << ") "<< "Client Ip: "<<addr<< endl;
	      //miofiledilog<< Simulator::Now().GetSeconds () << "s " << "Client Ip: "<<addr<< endl;
        stringstream ss;
        ss << lexical_cast<string>(Simulator::Now().GetSeconds()) << "s Client Ip: " << lexical_cast<string>(addr)<< " : ";
        std::string strss = ss.str();
        //string  pippomomentaneo = lexical_cast<string>(Simulator::Now().GetSeconds()) + "s Client Ip: " + lexical_cast<string>(addr);
        //cout << s;
        
	      //vector<Ptr<Peer>> peerset= DynamicCast<BitTorrentClient> (bitTorrentClients.Get(i))->GetActivePeers();
	      for (unsigned int j=0;j< peerset.size();j++){
          stringstream ciao;
          ciao << strss;
	        ciao << peerset[j]->GetRemoteIp ()<<" DOWN: "<< peerset[j]->GetBpsDownload()<<" UP: "<< peerset[j]->GetBpsUpload  () << endl;
          std::string s = ciao.str();
          //cout << s;
          miofiledilog<< s;
	      }
	    }
	  }
	  miofiledilog.close();
	  Simulator::Schedule (Seconds (1.0), PeerSet);
}

int LinkNodeLevels(NodeContainer **link, NetDeviceContainer **netdev, Ipv4InterfaceContainer **interfaces, 
  NodeContainer **level, PointToPointHelper p2p, Ipv4AddressHelper ipv4, int *count1, int *count2, int numFirstLevel, int numNextLevel);

// Assegnazione applicazioni Torrent: **level-conteiner nodi, indice-nodi effettivamente collegati [in media 670 su Modello 1]
void OurTorrentApplication(NodeContainer **level, int indice);

NS_LOG_COMPONENT_DEFINE ("bittorrent_clusterizzazione-2");
 
int main (int argc, char *argv[])
{
  int i,j;  
  char str[16];
  CommandLine cmd;
  cmd.Parse (argc, argv);
  srand(time(NULL));

  Time::SetResolution (Time::NS);
  /*  Inutili per il fattore ns3.20
  LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
  */
 
  // container con tutti i nodi
  NodeContainer *level[4];
  InternetStackHelper stack;
 
  for (i = 0; i < 4; ++i)
  {
    level[i] = new NodeContainer;
    level[i]->Create(levels[i]);
    stack.Install(*level[i]);
    cout << levels[i] << endl;
    cout << "Ho creato " << levels[i] << " nodi al livello "<< i+1 << endl;
  }

//=====================================LIVELLO 1================================================

  //crea tre matrici triangolari di puntatori
  NodeContainer *linklv1[levels[0]][levels[0]];
  NetDeviceContainer *netdevlv1[levels[0]][levels[0]];
  Ipv4InterfaceContainer *interfaceslv1[levels[0]][levels[0]];
 
  PointToPointHelper p2plv1;
  p2plv1.SetDeviceAttribute ("DataRate", StringValue ("1000Mbps"));
  p2plv1.SetChannelAttribute ("Delay", StringValue ("1ms"));
 
  Ipv4AddressHelper ipv4;
 
 
  int count1=0;
  int count2=0;
  for (i=0; i<levels[0]-1; ++i){
    for (j=i+1;j<levels[0];++j)
      {
          linklv1[i][j] = new NodeContainer(level[0]->Get(i), level[0]->Get(j));
          netdevlv1[i][j] = new NetDeviceContainer;
          interfaceslv1[i][j] = new Ipv4InterfaceContainer;
          *netdevlv1[i][j]= p2plv1.Install(*linklv1[i][j]);
          sprintf(str,"1.1.%d.%d", count2, count1);
          cout << "link L1-L1 creato: " << str <<"/30\n";
          ipv4.SetBase (str, "255.255.255.252");
          *interfaceslv1[i][j] = ipv4.Assign (*netdevlv1[i][j]);
          count1=count1+4;
          
        }
      }

 //==========================LIVELLO 2===============================
//creare matrici di link come per il livello 1.
  NodeContainer *linklv2[levels[1]][levels[1]];
  NetDeviceContainer *netdevlv2[levels[1]][levels[1]];
  Ipv4InterfaceContainer *interfaceslv2[levels[1]] [levels[1]];

// creiamo il link p2p per il livello 2 (capacità minore del livello 1)
  PointToPointHelper p2plv2;
  p2plv2.SetDeviceAttribute("DataRate", StringValue("500Mbps"));
  p2plv2.SetChannelAttribute("Delay", StringValue("5ms"));

  for (i = 0; i < levels[1]; ++i)
  {
    for (j = i+1; j < levels[1]; ++j)
    {
      if (j==i+1)
      {
        if (count1 % 256 ==0)
        {
          count1=0;
          count2++;
        }

        linklv2[i][j]=new NodeContainer(level[1]-> Get(i), level[1]-> Get(j));
        netdevlv2[i][j]=new NetDeviceContainer;
        interfaceslv2[i][j]= new Ipv4InterfaceContainer;
        *netdevlv2[i][j]=p2plv2.Install(*linklv2[i][j]);
        sprintf(str, "1.1.%d.%d", count2, count1);
        cout << "link L2-L2 creato: " << str <<"/30\n";
        ipv4.SetBase (str, "255.255.255.252");
        *interfaceslv2[i][j] = ipv4.Assign (*netdevlv2[i][j]);
        count1=count1+4;
      }
      else {
        linklv2[i][j]=NULL;
        netdevlv2[i][j]=NULL;
        interfaceslv2[i][j]=NULL;
      }
    }
  }

  linklv2[levels[1]-2][levels[1]-1]=new NodeContainer(level[1]-> Get(0), level[1]-> Get(levels[1]-1));
  netdevlv2[levels[1]-2][levels[1]-1]=new NetDeviceContainer;
  interfaceslv2[levels[1]-2][levels[1]-1]= new Ipv4InterfaceContainer;
  *netdevlv2[levels[1]-2][levels[1]-1]=p2plv2.Install(*linklv2[levels[1]-2][levels[1]-1]);
  sprintf(str, "1.1.%d.%d", count2, count1);
  cout << "link L2-L2 creato: " << str <<"/30\n";
  ipv4.SetBase (str, "255.255.255.252");
  *interfaceslv2[levels[1]-2][levels[1]-1] = ipv4.Assign (*netdevlv2[levels[1]-2][levels[1]-1]);
  count1=count1+4;


  //==========================  LIVELLO 2-1  ===============================
//creare matrici di link come per il livello 1.
  cout << "NODI 2-1" << endl;
  NodeContainer *link2to1[levels[1]];
  NetDeviceContainer *netdev2to1[levels[1]];
  Ipv4InterfaceContainer *interfaces2to1[levels[1]];

// creiamo il link p2p per il livello 2 (capacità minore del livello 1)
  PointToPointHelper p2p2to1;
  p2p2to1.SetDeviceAttribute("DataRate", StringValue("500Mbps"));
  p2p2to1.SetChannelAttribute("Delay", StringValue("5ms"));

  LinkNodeLevels(link2to1, netdev2to1, interfaces2to1, level, 
    p2p2to1, ipv4, &count1, &count2, 0,1); //collega i nodi tra L1-L2



    //==========================  LIVELLO 3-2  ===============================

  cout << "NODI 3-2" << endl;
  NodeContainer *link3to2[levels[2]];
  NetDeviceContainer *netdev3to2[levels[2]];
  Ipv4InterfaceContainer *interfaces3to2[levels[2]];

// creiamo il link p2p per il livello 2 (capacità minore del livello 1)
  PointToPointHelper p2p3to2;
  p2p3to2.SetDeviceAttribute("DataRate", StringValue("400Mbps"));
  p2p3to2.SetChannelAttribute("Delay", StringValue("5ms"));

  LinkNodeLevels(link3to2, netdev3to2, interfaces3to2, level, 
    p2p3to2, ipv4, &count1, &count2, 1,2); //collega i nodi tra L2-L3
  


//==========================  LIVELLO 4-3  ===============================
//creare matrici di link come per il livello 1.
  cout << "NODI 4-3" << endl;
  NodeContainer *link4to3[levels[3]];
  NetDeviceContainer *netdev4to3[levels[3]];
  Ipv4InterfaceContainer *interfaces4to3[levels[3]];

// creiamo il link p2p per il livello 2 (capacità minore del livello 1)
  PointToPointHelper p2p4to3;
  int n;
  n= LinkNodeLevels(link4to3, netdev4to3, interfaces4to3, level, 
    p2p4to3, ipv4, &count1, &count2, 3,2); //collega i nodi tra L4-L3

    
  //==========================  LIVELLO 4  ===============================
// Associamo l'applicazione BitTorrent ai vari nodi del livello 4
// Con i corrispettivi Tracker, ClientTorrent e Seed

  tempo =GlobalMetricsGatherer::GetWallclockTime(); // Tutti i filelog inizieranno a crearsi da qui e avranno stesso riferimento di tempo
  cout<<"Inizio BitTorrent Application"<<endl;  
  OurTorrentApplication(level,n);
  
  

  cout<<"Popolo le Tabelle di Routing..."<<endl;
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
  cout<<"...Fatto."<<endl;


  //Start the simulation
  std::cout << "Avvio simulazione..." << std::endl;
  Simulator::ScheduleNow (ShowTimePeriodic);
  Simulator::ScheduleNow(PeerSet);
 
  Simulator::Run ();
  Simulator::Destroy ();

  // Salviamo il tempo massimo di esecuzione avvenuta
  cout << finito << "Simulazione terminata. =)" <<endl;
  miofiledilog.open("output/PeerSet.txt", ios_base::app);
  miofiledilog<<finito;
  miofiledilog.close();
 
  return 0;
}


int LinkNodeLevels(NodeContainer **link, NetDeviceContainer **netdev, Ipv4InterfaceContainer **interfaces, 
  NodeContainer **level, PointToPointHelper p2p, Ipv4AddressHelper ipv4, int *count1, int *count2, 
  int numFirstLevel, int numNextLevel){
  int bitRate [6] = { 1, 8, 20, 35, 50};   // Inizializza il BitRate 
  int rtt [4] = { 20, 15, 10, 5};          // e Delay dei link del livello LV3-LV4
  int l;
  int effectiveBtr;
  int n=0;

  for (int i = 0; i < (int)(level[numNextLevel]->GetN()); ++i)
  {
    char str[16];
    int r=1+rand()%3;
    // cout << "RAND" << r << endl; 
    int nlink[r];
    if(numFirstLevel != 3){
      for (int j = 0; j < r; ++j)
        nlink[j] = -1;
    }
    
    link[i]=new NodeContainer[r];
    netdev[i]=new NetDeviceContainer[r];
    interfaces[i]=new Ipv4InterfaceContainer[r]; 
     
    for (int j = 0; j < r; ++j) //questo for decide il numero di link 2to1 e i loro nodi
    {
      
      if(numFirstLevel != 3){
        n=rand()%((int)level[numFirstLevel]->GetN());
        int k=0;
        while(k<r){
          if (n!=nlink[k])
          {
            k++;
          }
          else{
            k=0;
            n=rand()%((int)level[numFirstLevel]->GetN());
          }
        }
        nlink[j] = n;
      }
      
      if (*count1 % 256 ==0)
      {
        *count1=0;
        *count2+=1;
      }

      if (n<levels[3]) { // Controllo per garantire che gli effettivi nodi da assegnare gli indirizzi siano effettivamente presenti nel LV4 [SIGIOT error] Modello 2 (opportunamente gestito invece in Modello 1)  
        link[i][j].Add(level[numFirstLevel]->Get(n));
        link[i][j].Add(level[numNextLevel]->Get(i));
        if(numFirstLevel == 3){ // LV3-Lv4
          l = rand()%4; // Scelta del Rapporto BitRate & Delay associato
          //cout << "Random bitrate & delay "<< l << endl;
          effectiveBtr = bitRate[l] + (rand()% (bitRate[l+1]-bitRate[l]+1));
          p2p.SetDeviceAttribute("DataRate", StringValue(to_string(effectiveBtr)+"Mbps"));
          p2p.SetChannelAttribute("Delay", StringValue(to_string(rtt[l])+"ms"));
          
        }
        netdev[i][j] = p2p.Install(link[i][j]);
        sprintf(str, "1.1.%d.%d", *count2, *count1);
        cout << "link L" << numNextLevel+1 << "-L" << numFirstLevel+1 << " creato: " << str <<"/30 tra i nodi " << i+1 << "<->" << n+1 << endl;
        if (numFirstLevel== 3) {
          cout << "Eff  BRT: "<< effectiveBtr << " Delay: " << rtt[l] << endl;
        }
        
        ipv4.SetBase (str, "255.255.255.252");
        interfaces[i][j] = ipv4.Assign (netdev[i][j]);
        *count1=*(count1)+4;
        n++;
      }
    }
  }
  return n;
}

void OurTorrentApplication(NodeContainer **level, int indice){

  //string tempo =GlobalMetricsGatherer::GetWallclockTime(); // viene def univocamente prima del :run

   // 1) Install a BitTorrentTracker application (with default values) on one of the nodes
  Ptr<BitTorrentTracker> bitTorrentTracker = Create<BitTorrentTracker> ();
  level[3]->Get (0)->AddApplication (bitTorrentTracker);
  level[3]->Get (1)->AddApplication (bitTorrentTracker);
  level[3]->Get (2)->AddApplication (bitTorrentTracker);
  level[3]->Get (3)->AddApplication (bitTorrentTracker);
  level[3]->Get (4)->AddApplication (bitTorrentTracker);

  // 2) Load a torrent file via the tracker application
  Ptr<Torrent> sharedTorrent = bitTorrentTracker->AddTorrent ("input/bittorrent/torrent-data", "input/bittorrent/torrent-data/ourtorrent.dat.torrent");

  // 3) Install BitTorrentClient applications on the desired number of nodes
  for (int i = 5; i < indice; ++i)
    {
      // Install BitTorrentClient
      Ptr<BitTorrentClient> client = Create<BitTorrentClient> ();
      client->SetTorrent (sharedTorrent);
      client->SetMaxPeers(5);
      level[3]->Get (i)->AddApplication (client);
      bitTorrentClients.Add (client);
      

      // Recupero Ip associato al client per lavorare su opportuni file di LOG
      Ptr<Ipv4> x=level[3]->Get(i)->GetObject<Ipv4>();
      Ipv4Address addr=x->GetAddress(1,0).GetLocal();
      
      cout<<"IP E BANDA ASSOCIATA: "<<addr<<" <-> "<<client->GetBpsUplink()<<" (UP) "<<client->GetBpsDownlink()<< " (DOWN)"<<endl;
      miofiledilog.open("output/ip-bande.txt", ios_base::app);
      miofiledilog<< "Client Ip: "<<addr<<" <-> "<<client->GetBpsUplink()<<" (UP) "<<client->GetBpsDownlink()<< " (DOWN)" << endl;
      miofiledilog.close();
    }
  // Make the application node a seeder
  DynamicCast<BitTorrentClient> (level[3]->Get (10)->GetApplication (0))->SetInitialBitfield ("full");
  DynamicCast<BitTorrentClient> (level[3]->Get (20)->GetApplication (0))->SetInitialBitfield ("full");

  // 4) Set up the BitTorrent metrics gatherer for output handling (here, we just log to the screen)
  GlobalMetricsGatherer* gatherer = GlobalMetricsGatherer::GetInstance ();
  gatherer->SetFileNamePrefix ("output/RESULTS", true);
  gatherer->RegisterWithApplications (bitTorrentClients);
  gatherer->SetStopFraction (1.0, 1.0); // Stops the simulation when all nodes have finished downloading
}
