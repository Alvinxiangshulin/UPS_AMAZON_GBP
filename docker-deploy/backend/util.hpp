#ifndef __UTIL_HPP__
#define __UTIL_HPP__

#include "world_ups.pb.h"
#include "ups.pb.h"
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <mutex>
#include <unordered_set>


using namespace std;
//using namespace ups_protocol;
class global_data{
  std::mutex gl_w_seq;
  std::mutex gl_a_seq;
  std::mutex gl_send_world;
  std::mutex gl_recv_world;
  std::mutex gl_send_amazon;
  std::mutex gl_recv_amazon;
  unordered_set<int> send_world_set;
  unordered_set<int> recv_world_set;
  unordered_set<int> send_amazon_set;
  unordered_set<int> recv_amazon_set;
  int world_seq;
  int amazon_seq;
public:
  //read and add + 1 for world seqnum
  global_data():world_seq(0),amazon_seq(0){}
  int read_world_seq(){
    std::lock_guard<std::mutex> lock(gl_w_seq);
    int temp = world_seq;
    world_seq+=1;
    return temp;
  }

  void add_world_seq(int cnt){
    std::lock_guard<std::mutex> lock(gl_w_seq);
    world_seq+=cnt;
  }
  
  //read and add + 1 for amazon seqnum
  int read_amazon_seq(){
    std::lock_guard<std::mutex> lock(gl_a_seq);
    int temp = amazon_seq;
    amazon_seq+=1;
    return temp;
  }

  void add_amazon_seq(int cnt){
    std::lock_guard<std::mutex> lock(gl_a_seq);
    amazon_seq+=cnt;
  }
  
  //read and insert send_world_set
   bool read_send_world_set(int seq){
    std::lock_guard<std::mutex> lock(gl_send_world);
    if(send_world_set.find(seq)!=send_world_set.end()){
      return true;
    }
    else{
      return false;
    }
    
  }

  void insert_send_world_set(int seq){
    std::lock_guard<std::mutex> lock(gl_send_world);
    send_world_set.insert(seq);
  }

  //read and insert recv_world_set
  bool read_recv_world_set(int seq){
    std::lock_guard<std::mutex> lock(gl_recv_world);
    
    if(recv_world_set.find(seq)!=recv_world_set.end()){
      return true;
    }
    else{
      return false;
    }
    
  }

  void insert_recv_world_set(int seq){
    std::lock_guard<std::mutex> lock(gl_recv_world);
    recv_world_set.insert(seq);
  }

  //read and insert send_amazon_set
  bool read_send_amazon_set(int seq){
    std::lock_guard<std::mutex> lock(gl_send_amazon);
    if(send_amazon_set.find(seq)!=send_amazon_set.end()){
      return true;
    }
    else{
      return false;
    }
    
  }

  void insert_send_amazon_set(int seq){
    std::lock_guard<std::mutex> lock(gl_send_amazon);
    send_amazon_set.insert(seq);
  }

  //read and insert recv_world_set
  bool read_recv_amazon_set(int seq){
    std::lock_guard<std::mutex> lock(gl_recv_amazon);
    if(recv_amazon_set.find(seq)!=recv_amazon_set.end()){
      return true;
    }
    else{
      return false;
    }
    
  }

  void insert_recv_amazon_set(int seq){
    std::lock_guard<std::mutex> lock(gl_recv_amazon);
    recv_amazon_set.insert(seq);
  }
  
};


template<typename T>
bool sendMesgTo(const T & message, google::protobuf::io::FileOutputStream * out) 
{
    { 
        //extra scope: make output go away before out->Flush()
        // We create a new coded stream for each message.
        // Don’t worry, this is fast.
        google::protobuf::io::CodedOutputStream output(out);
        // Write the size.
        const int size = message.ByteSizeLong();
        output.WriteVarint32(size);
        uint8_t* buffer = output.GetDirectBufferForNBytesAndAdvance(size);
        if (buffer != NULL) 
        {
            // Optimization: The message fits in one buffer, so use the faster direct-to-array serialization path.
            message.SerializeWithCachedSizesToArray(buffer);
        } 
        else 
        {
            // Slightly-slower path when the message is multiple buffers.
            message.SerializeWithCachedSizes(&output);
            if (output.HadError()) 
            {
                return false;
            }
        }
    }
    out->Flush();
    return true;
}

template<typename T>
bool recvMesgFrom(T & message, google::protobuf::io::FileInputStream * in)
{
    google::protobuf::io::CodedInputStream input(in);
    uint32_t size;
    if (!input.ReadVarint32(&size)) 
    {
      cout<<"size problem"<<endl;
        return false;
    }
    // Tell the stream not to read beyond that size.
    google::protobuf::io::CodedInputStream::Limit limit = input.PushLimit(size);
    // Parse the message.
    if (!message.MergeFromCodedStream(&input)) 
    {
      cout <<"cannot merge from coded stream"<<endl;
        return false;
    }
    if (!input.ConsumedEntireMessage()) 
    {
      cout <<"cannot consume entire msg"<<endl;
        return false;
    }
    // Release the limit.
    input.PopLimit(limit);
    return true;
}


 //send connect msg
void sendConnectMsg(int server_fd, const UConnect & data)
{
  google::protobuf::io::FileOutputStream * out = new google::protobuf::io::FileOutputStream(server_fd);
  if(!sendMesgTo(data, out)){
      cerr<<"ERROR: SEND CONNECT MSG!"<<endl;
  }
}

//receive connected msg
void recvConnectedMsg(int server_fd, UConnected& connRes)
{ 
  google::protobuf::io::FileInputStream * in = new google::protobuf::io::FileInputStream(server_fd);
  if(!recvMesgFrom(connRes, in)){
    cerr<<"ERROR: RECV CONNECTED MSG!"<<endl;
  }
  //std::cout <<"world id: "<< connRes.worldid() << std::endl;
  //std::cout << "result: "<<connRes.result() << std::endl;
  //return connRes;
}

//send command msg
void sendCommandMsg(int server_fd, const UCommands & data)
{
  google::protobuf::io::FileOutputStream * out = new google::protobuf::io::FileOutputStream(server_fd);
  if(!sendMesgTo(data, out)){
    cerr<<"ERROR: SEND COMMAND MSG!"<<endl;
  }
  //    seqNum++;
}

//receive response msg
void recvResponseMsg(int server_fd, UResponses& connRes)
{
  //UResponses connRes;
  google::protobuf::io::FileInputStream * in = new google::protobuf::io::FileInputStream(server_fd);
  if(!recvMesgFrom(connRes, in)){
    cerr<<"ERROR: RECV RESPONSE MSG!"<<endl;
  }
  //std::cout<<"ack size: "<<connRes.acks_size()<<std::endl;
  //std::cout <<"ack(0): "<< connRes.acks(0) << std::endl;
  //return connRes;
}








 //send UtoACommand msg
void sendUtoAMsg(int server_fd, const UtoACommand & data)
{
  google::protobuf::io::FileOutputStream * out = new google::protobuf::io::FileOutputStream(server_fd);
  if(!sendMesgTo(data, out)){
      cerr<<"ERROR: SEND AMAZON MSG!"<<endl;
  }
}

//receive AtoUCommand
void recvAtoUMsg(int server_fd, AtoUCommand& connRes)
{ 
  google::protobuf::io::FileInputStream * in = new google::protobuf::io::FileInputStream(server_fd);
  if(!recvMesgFrom(connRes, in)){
    cerr<<"ERROR: RECV AMAZON MSG!"<<endl;
  }  
}


#endif
