#ifndef __ADD_MSG_AMAZON_HPP__
#define __ADD_MSG_AMAZON_HPP__

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

//using namespace ups_protocol;

void add_UtoAConnect(UtoACommand& utoa, int seq, int worldid){
  UtoAConnect * t = utoa.add_connection();
  t->set_seqnum(seq);
  t->set_worldid(worldid);
}

void add_UserValidationResponse(UtoACommand& utoa, int seq, bool res, int shipid){
  UserValidationResponse *t = utoa.add_usrvlid();
  t->set_seqnum(seq);
  t->set_result(res);
  t->set_shipid(shipid);
}

void add_UtoALoadRequest(UtoACommand& utoa, int seq, int wareid, int truckid, vector<int>& shipid){
  UtoALoadRequest *t = utoa.add_loadreq();
  t->set_seqnum(seq);
  t->set_warehouseid(wareid);
  t->set_truckid(truckid);
  for(size_t i = 0 ;i < shipid.size(); i++){
    t->add_shipid(shipid[i]);
  }
}

void add_Delivery(UtoACommand& utoa, int seq, int shipid){
  Delivery* t = utoa.add_delivery();
  t->set_seqnum(seq);
  t->set_shipid(shipid);
}

void add_ErrorMessage(UtoACommand& utoa, string err, int originseq, int seq){
  ErrorMessage* t = utoa.add_errmsg();
  t->set_err(err);
  t->set_originseqnum(originseq);
  t->set_seqnum(seq);
}

void add_ack(UtoACommand& utoa, int ack){
  utoa.add_ack(ack);
}
#endif
