#ifndef __ADD_MSG_WORLD_HPP__
#define __ADD_MSG_WORLD_HPP__
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
#include "SocketProg.hpp"
//#include "world.hpp"

//using namespace ups_protocol;

void add_Upickups(UCommands& comm, int truckid, int whid, int seqnum){
  UGoPickup* t = comm.add_pickups();
  t->set_truckid(truckid);
  t->set_whid(whid);
  t->set_seqnum(seqnum);
}

void add_delivery(UCommands& comm, int truckid, int pkid, int x, int y, int seq){
  UGoDeliver* t = comm.add_deliveries();
  t->set_truckid(truckid);
  UDeliveryLocation* loc = t->add_packages();
  loc->set_packageid(pkid);
  loc->set_x(x);
  loc->set_y(y);
  t->set_seqnum(seq);
}

void add_query(UCommands& comm, int truckid, int seqnum){
  UQuery* t = comm.add_queries();
  t->set_truckid(truckid);
  t->set_seqnum(seqnum);
}

void add_ack(UCommands& comm, int seqNum){
  comm.add_acks(seqNum);
}
#endif
