#ifndef __AMAZON_TO_UPS_HPP__
#define __AMAZON_TO_UPS_HPP__
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
#include <thread>
#include <time.h>
#include "util.hpp"
#include "world_ups.pb.h"
#include "ups.pb.h"
#include "SocketProg.hpp"
#include "sql.hpp"
#include "add_msg_world.hpp"
#include "add_msg_amazon.hpp"


//using namespace ups_protocol;
using namespace std;

bool validateUser(UserValidationRequest userVReq, connection* C){
  return findUser(C, userVReq.upsaccount());
}

void sendValidateRes(UserValidationRequest userVReq, global_data* data, bool result, int amazon_fd){
  UtoACommand toAmazon;
  int amazon_seqNum = data->read_amazon_seq();
  add_UserValidationResponse(toAmazon, amazon_seqNum, result, userVReq.shipid());
  add_ack(toAmazon, userVReq.seqnum());
  sendUtoAMsg(amazon_fd, toAmazon);
  while(true){
    usleep(500000);
    if(data->read_send_amazon_set(amazon_seqNum)){
      break;
    }
    sendUtoAMsg(amazon_fd, toAmazon);
  }
}

template <typename T>
void sendAck(T Req, int amazon_fd){
  UtoACommand toAmazon;
  add_ack(toAmazon, Req.seqnum());
  sendUtoAMsg(amazon_fd, toAmazon);
}

void sendGoPickup(int truckid, int whid, global_data* data, int world_fd){
  UCommands toWorld;
  int world_seqNum = data->read_world_seq();
  add_Upickups(toWorld, truckid, whid, world_seqNum);
  sendCommandMsg(world_fd, toWorld);
  while(true){
    usleep(500000);
    if(data->read_send_world_set(world_seqNum)){
      break;
    }
    sendCommandMsg(world_fd, toWorld);
  }
}

/*void sendLoadFinishAck(AtoULoadFinishRequest loadFReq, int amazon_fd){
  UtoACommand toAmazon;
  add_ack(toAmazon,loadFReq.seqnum());
  sendUtoAMsg(amazon_fd, toAmazon);
  while(true){
    usleep(10000);
    if(data->read_send_world_set(world_seqNum)){
      break;
    }
    sendCommandMsg(world_fd, toWorld);
  }
}*/


void sendGoDeliver(int truckid, int shipid, int x, int y, global_data* data, int world_fd){
  UCommands toWorld;
  int world_seqNum = data->read_world_seq();
  add_delivery(toWorld, truckid, shipid, x, y, world_seqNum);
  sendCommandMsg(world_fd, toWorld);
  while(true){
    usleep(500000);
    if(data->read_send_world_set(world_seqNum)){
      break;
    }
    sendCommandMsg(world_fd, toWorld);
  }
}

template <typename T>
bool checkAddRead(T msg, global_data* data){
  if(data->read_recv_amazon_set(msg.seqnum())){
    std::cout <<"recv amazon seqnum "<<msg.seqnum()<<" has been processed"<<std::endl;
    return false;
  }
  else{
    std::cout <<"recv amazon seqnum "<<msg.seqnum()<<" is going to be processed"<<std::endl;
    data->insert_recv_amazon_set(msg.seqnum());
    return true;
  }
}

void amazon_ups_world(int amazon_fd, int world_fd,  global_data* data,connection* C){
  AtoUCommand fromAmazon;
  recvAtoUMsg(amazon_fd, fromAmazon);
  cout<<"recv from amazon"<<endl;
  //connection* C = connect_db();
  ///UtoACommand toAmazon;
  ///UCommands toWorld;
  for(int i = 0; i < fromAmazon.usrvlid_size(); i++){
    UserValidationRequest userVReq = fromAmazon.usrvlid(i);
    if(!checkAddRead(userVReq, data)){
      sendAck(userVReq, amazon_fd);
      continue;
    }
    std::cout << "seqNum: "<< userVReq.seqnum() <<std::endl;
    std::cout << "validating UPSaccount: "<< userVReq.upsaccount()<<std::endl;
    bool result = validateUser(userVReq, C);
    std::thread t_VReq(sendValidateRes, userVReq, data, result, amazon_fd);
    t_VReq.detach();
  }
  
  for(int j = 0; j < fromAmazon.pikreq_size(); j++){
    AtoUPickupRequest pickupReq = fromAmazon.pikreq(j);
    sendAck(pickupReq, amazon_fd);
    if(!checkAddRead(pickupReq,data)){
      continue;
    }
    std::cout << "seqNum: "<< pickupReq.seqnum() <<std::endl;

    
    int truckid = getTruck(C,0);
    if(truckid == -1){
      truckid = getTruck(C,4);
    }
    if(truckid ==-1){
      truckid = 0;
    }
    
    update_truck(C, truckid, 1);
    std::cout <<"send truck "<< truckid<< " to warehouse "<<pickupReq.warehouseid()<<std::endl;
    for(int j1 = 0; j1 < pickupReq.shipment_size(); j1++){
      ShipInfo sInfo = pickupReq.shipment(j1);
      if(sInfo.has_upsaccount()){
	init_package(C, sInfo.shipid(), sInfo.upsaccount(), sInfo.destination_x(), sInfo.destination_y(), truckid, pickupReq.warehouseid(), 0);}
      else{
	init_package(C, sInfo.shipid(), "", sInfo.destination_x(), sInfo.destination_y(), truckid, pickupReq.warehouseid(), 0);
      }
      time_t rawtime;
      time (&rawtime);
      initHistory(C, sInfo.shipid(), ctime(&rawtime));
      for(int j2 = 0; j2 < sInfo.products_size(); j2++){
	Product product = sInfo.products(j2);
	init_item(C, sInfo.shipid(), product.description(), product.count());
      }
    }
    std::thread t_goPickup(sendGoPickup, truckid, pickupReq.warehouseid(), data, world_fd);
    t_goPickup.detach();
  }
  
  for(int k = 0; k < fromAmazon.loadreq_size(); k++){
    AtoULoadFinishRequest loadFReq = fromAmazon.loadreq(k);
    std::cout <<"recv load finish req on truck "<< loadFReq.truckid()<< std::endl;
    sendAck(loadFReq, amazon_fd);
    if(!checkAddRead(loadFReq, data)){
      continue;
    }
    for(int k1 = 0; k1 < loadFReq.shipid_size();k1++){
      int shipid = loadFReq.shipid(k1);
      std::cout <<"package "<<shipid<< " has been loaded on truck "<< loadFReq.truckid()<<std::endl;
      std::vector<int> coordinate = getPkgDest(C, shipid);
      update_package(C, shipid, 1);
      update_truck(C, loadFReq.truckid(), 4);
      time_t rawtime;
      time (&rawtime);
      addDeliverHistory(C, shipid, ctime(&rawtime));
      std::thread t_goDeliver(sendGoDeliver, loadFReq.truckid(), shipid, coordinate[0], coordinate[1], data, world_fd);
      t_goDeliver.detach();
    }
  }
  
  for(int l = 0; l < fromAmazon.errmsg_size(); l++){
    ErrorMessage errM = fromAmazon.errmsg(l);
    std::cout <<"errmsg: "<< errM.err()<<std::endl;
    sendAck(errM, amazon_fd);
  }
  for(int m = 0; m < fromAmazon.ack_size(); m++){
    int ackNum = fromAmazon.ack(m);
    data->insert_send_amazon_set(ackNum);
    std::cout << "recv ack to amazon: " << ackNum <<std::endl;
  }
  if(fromAmazon.has_disconnection()){
    std::cout << "lose connection"<<std::endl;
  }
}

#endif
