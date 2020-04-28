#ifndef __WORLD_TO_UPS_HPP__
#define __WORLD_TO_UPS_HPP__
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
#include "util.hpp"
#include "world_ups.pb.h"
#include "ups.pb.h"
#include "SocketProg.hpp"
#include "add_msg_amazon.hpp"
#include "add_msg_world.hpp"
#include "sql.hpp"
#include "util.hpp"
#include <thread>
#include <unordered_set>
#include <pqxx/pqxx>
#include <exception>
#include <time.h>

using namespace std;
//using namespace ups_protocol;
using namespace pqxx;

//reply ack to world
void ack_to_world(int world_fd, int seqnum){
  UCommands ucomm; 
  ucomm.add_acks(seqnum);
  sendCommandMsg(world_fd,ucomm);
}

//send UtoAConnect
void deal_UtoAConnect(int amazon_fd, int worldid, global_data* data){
  //  cout<<"send world id: "<<worldid<<endl;
  cout<<"sending world i to amazon"<<endl;
  int amazon_seq = data->read_amazon_seq();
  cout<<"amazon_seq:"<<amazon_seq<<endl;
  UtoACommand utoa;
  add_UtoAConnect(utoa,amazon_seq,worldid);
  sendUtoAMsg(amazon_fd, utoa);
  /*while(true){
    if(data->read_send_amazon_set(amazon_fd)){
      break;
    }
    sendUtoAMsg(amazon_fd, utoa);
    usleep(10000);
    }*/
}


//deal with completion
void deal_completion(int amazon_fd, int world_fd, const UFinished& completion,global_data* data){
  cout<<"handle completion"<<endl;
  int seq = completion.seqnum();
  ack_to_world(world_fd, seq);
  if(data->read_recv_world_set(completion.seqnum())){
    return;
  }
  int amazon_seq = data->read_amazon_seq();
  cout<<"amazon_seq:"<<amazon_seq<<endl;
  //return ack to world  
  
  data->insert_recv_world_set(seq);
  //connect to db
  connection* C = connect_db();

  int truck_id = completion.truckid();
  //update_truck_location(C, truck_id, completion.status(), completion.x(), completion.y());
  //int x = completion.x();
  //int y = completion.y();
  vector<int>shipid_vec = getPackageId(C,truck_id,0);
  
  string status = completion.status();
  int whid = getWarehouse(C,truck_id);
  if(status == "IDLE"){
    update_truck_location(C, truck_id, 0, completion.x(), completion.y());
  }
  else{
    update_truck_location(C, truck_id, 2, completion.x(), completion.y());
    UtoACommand utoa;
    time_t rawtime;
    time(&rawtime);
    for(size_t i = 0; i < shipid_vec.size();i++){
      addLoadHistory(C, shipid_vec[i], ctime(&rawtime));
    }
    add_UtoALoadRequest(utoa,amazon_seq,whid,truck_id, shipid_vec);
      //amazon_seq = data->read_amazon_seq();
   
    while(true){
      usleep(500000);
      if(data->read_send_amazon_set(amazon_seq)){
	break;
      }
      sendUtoAMsg(amazon_fd,utoa);
      //sleep for 0.1 second
     
    }
    //finish this completion, so next the world request to do this completion just skip this step
    data->insert_recv_world_set(completion.seqnum());
    cout<<"send info to Amazon to notify the truck has arrived at warehouse"<<endl;
  }
  (*C).disconnect();
}

//deal with delivered
void deal_delivered(int amazon_fd, int world_fd, const UDeliveryMade& Ud, global_data*data){
  cout<<"deal with delivered"<<endl;
  ack_to_world(world_fd, Ud.seqnum());
  if(data->read_recv_world_set(Ud.seqnum())){
    return;
  }
  
  int amazon_seq = data->read_amazon_seq();
  cout<<"amazon_seq: "<<amazon_seq<<endl;
  int truck_id = Ud.truckid();
  int pkid = Ud.packageid();
  //update truck status to idle
  connection* C = connect_db();
  update_truck(C, truck_id, 0);
  
  update_package(C,pkid,2);

  UtoACommand utoa;
  time_t rawtime;
  time (&rawtime);
  addDeliveredHistory(C, pkid, ctime(&rawtime));
  add_Delivery(utoa, amazon_seq, pkid);
  while(true){
    usleep(500000);
    if(data->read_send_amazon_set(amazon_seq)){
      break;
    }
    sendUtoAMsg(amazon_fd,utoa);
    //sleep for 0.1 second
    
  }
  data->insert_recv_world_set(Ud.seqnum());
  cout<<"send info to Amazon Package has been delivered."<<endl;
  (*C).disconnect();
}


int get_truck_status(const UTruck& utruck){
  if(utruck.status() == "idle"){
    return 0;
  }
  if(utruck.status() == "traveling"){
    return 1;
  }
  if(utruck.status() == "arrive_warehouse"){
    return 2;
  }
  if(utruck.status() == "loading"){
    return 3;
  }
  if(utruck.status() == "delivering"){
    return 4;
  }
  else{return -1;}
}
//deal with trucks status 
void deal_trucksstatus(int world_fd, const UTruck& utruck, global_data* data){
  cout<<"deal with truckstatus"<<endl;
  connection* C = connect_db();
  ack_to_world(world_fd,utruck.seqnum());
  if(data->read_recv_world_set(utruck.seqnum())){
     return;
  }
  data->insert_recv_world_set(utruck.seqnum());
  update_truck_location(C, utruck.truckid(), get_truck_status(utruck), utruck.x(), utruck.y());
  (*C).disconnect();
}

//deal with err
void deal_err(int world_fd, const UErr& err, global_data* data){
  cout<<"deal with err"<<endl;
  ack_to_world(world_fd,err.seqnum());
  if(data->read_recv_world_set(err.seqnum())){
     return;
  }
  data->insert_recv_world_set(err.seqnum());
}


//world_to_ups
void world_to_ups(int world_socket, int amazon_socket, global_data* data){
  cout<<"receive UResponses from world"<<endl;
  UResponses UResp;
  int world_fd = world_socket;
  int amazon_fd = amazon_socket;
  recvResponseMsg(world_fd,UResp);
  //  connection* C = connect_db();
  for(int i = 0; i < UResp.completions_size();i++){
    const UFinished& temp_completion = UResp.completions(i);
    std::thread completion_thread(deal_completion, amazon_fd, world_fd, temp_completion, data);
    completion_thread.detach();
  }
  for(int i = 0; i< UResp.delivered_size(); i++){
    const UDeliveryMade& temp_dlv = UResp.delivered(i);
    std::thread delivered_thread(deal_delivered, amazon_fd, world_fd, temp_dlv, data);
    delivered_thread.detach();
  }
  for(int i = 0; i < UResp.acks_size();i++){
    data->insert_send_world_set(UResp.acks(i));
  }
  for(int i = 0; i < UResp.truckstatus_size(); i++){
    const UTruck& temp_truck = UResp.truckstatus(i);
    std::thread truck_thread(deal_trucksstatus, world_fd, temp_truck,data);
    truck_thread.detach();
  }
  for(int i = 0; i < UResp.error_size(); i++){
    const UErr& temp_err = UResp.error(i);
    std::thread err_thread(deal_err, world_fd, temp_err,data);
    err_thread.detach();
  }
}

#endif
