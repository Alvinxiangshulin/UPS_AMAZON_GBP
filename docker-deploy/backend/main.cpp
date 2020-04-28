#include <netdb.h>
#include <cstdlib>
#include <string>
#include <unistd.h>
#include <iostream>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <pqxx/pqxx>
#include <exception>
#include "util.hpp"
#include "world_ups.pb.h"
#include "ups.pb.h"
#include "SocketProg.hpp"
#include "amazon_to_ups.hpp"
#include "world_to_ups.hpp"
#include <thread>
#include <time.h>

#define TRUCK_NUM 1024
using namespace std;
using namespace pqxx;
//using namespace ups_protocol;

UConnect UConnData()
{
  cout<<"intial 1024 trucks"<<endl;
    UConnect connReq;
    // create a new world, world id leave blank
    // initial 1024 trucks
    connReq.set_isamazon(false);
    for(int i = 0; i < TRUCK_NUM; i++){
      UInitTruck * truckObj = connReq.add_trucks();
      truckObj->set_id(i);
      truckObj->set_x(0);
      truckObj->set_y(0);
    }
    return connReq;
}

void Connect_world_amazon(int amazon_fd, int world_fd, global_data* data){
  
  //inital trucks 
  UConnect Uconn = UConnData();
  
  //add truck info to db
  connection* C = connect_db();
  for(int i = 0; i < Uconn.trucks_size();i++){
    int truck_id = Uconn.trucks(i).id();
    int truck_x = Uconn.trucks(i).x();
    int truck_y = Uconn.trucks(i).y();
    init_truck(C, truck_id, 0, truck_x, truck_y);
  }

  //send UConnect and receive the UConneted
  UConnect connReq = UConnData();
  int worldid = -1;
  sendConnectMsg(world_fd, connReq);
  UConnected Connected;
  recvConnectedMsg(world_fd, Connected);
  cout<<"connected with world and ";
  //maybe while loop here???
  //send world id to amazon side
  worldid = Connected.worldid();
  cout<<"world id to send: "<<worldid<<endl;
  deal_UtoAConnect(amazon_fd, worldid, data);
  (*C).disconnect();
}

int main(int argc, char* argv[]){
  if(argc<5){
    cout<<"Usage ./main <world_ip> <world_port> <amazon_ip> <amazon_port>"<<endl;
    return 0;
  }
  char* world_ip = argv[1];
  char* world_port = argv[2];
  char* amazon_ip = argv[3];
  char* amazon_port = argv[4];
  int world_fd = connectServer(world_ip, world_port);
  int amazon_fd = connectServer(amazon_ip, amazon_port);

  connection* C = connect_db();
  InitTables(C);
  //get world id, connect with world
  global_data data;
  Connect_world_amazon(amazon_fd, world_fd, &data);
  //cout<<"after connected world and amzon"<<endl;
  fd_set socket_set;
  int max_fd = max(world_fd, amazon_fd);

  FD_ZERO(&socket_set);
  FD_SET(world_fd, &socket_set);
  FD_SET(amazon_fd, &socket_set);
  
  fd_set reset = socket_set;
  //for test makefile
  ///int amazon_seqNum = 0;
  ///int world_seqNum = 0;
    
  while(true){
     socket_set = reset;
     int n = select(max_fd + 1, &socket_set, NULL, NULL, NULL);
     if(n < 0){
       cerr<<"ERROR IN SELECT FD"<<endl;
     }
     if (FD_ISSET(world_fd, &socket_set)) {
       world_to_ups(world_fd,amazon_fd, &data);
     }
     if (FD_ISSET(amazon_fd, &socket_set)) {
       amazon_ups_world(amazon_fd, world_fd,&data, C);
     }
  }
  (*C).disconnect();
  close(world_fd);
  close(amazon_fd);
  return 0;
  }

/*int main(){
  connection* C = connect_db();
  init_package(C, 1, "zhm2go", 1, 2, 3, 4, 0);
  init_package(C, 2, "", 5, 6, 5, 6, 0);
  init_truck(C, 1, 0, 0, 0);
  std::cout <<"before getTruck"<<std::endl;
  int tid = getTruck(C, 0);
  std::cout <<"idle truck id is: "<< tid <<std::endl;
  update_truck(C, 1, 1);
  update_package_dest(C, 1, 3, 3);
  update_package(C, 1, 1);
  int truckid = get_package_truckid(C, 1);
  std::cout <<"pkg 1 has truck id: "<< truckid<<std::endl;
  }
*/
/*int main(){
  connection* C = connect_db();
  InitTables(C);
  time_t rawtime;
  time(&rawtime);
  printf ("The current local time is: %s", ctime(&rawtime));
  initHistory(C, 1, ctime(&rawtime));
  sleep(2);
  time_t rawtime1;
  time(&rawtime1);
  printf ("The current local time is: %s", ctime(&rawtime1));
  addLoadHistory(C, 1, ctime(&rawtime1));
  sleep(2);
  time_t rawtime2;
  time(&rawtime2);
  printf ("The current local time is: %s", ctime(&rawtime2));
  addDeliverHistory(C, 1, ctime(&rawtime2));
  sleep(2);
  time_t rawtime3;
  time(&rawtime3);
  printf ("The current local time is: %s", ctime(&rawtime3));
  addDeliveredHistory(C, 1, ctime(&rawtime3));

  }
*/
