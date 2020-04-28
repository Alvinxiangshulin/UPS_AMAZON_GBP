#ifndef __SQL_HPP__
#define __SQL_HPP__

#include <iostream>
#include <pqxx/pqxx>
#include <exception>
#include <fstream>
#include <sstream>
#include <vector>
using namespace std;
using namespace pqxx;
//truck status
//idle = 0;
//traveling = 1;
//arrive_warehouse = 2;
//loading = 3;
//delivering = 4

//package status
//to_delivery =0;
//transion = 1;
//arrived = 2;
//intial database tables
void InitTables(connection* C){
  string sql;
  sql = "DROP TABLE IF EXISTS \"TRUCKS_POOL\";"\
    "DROP TABLE IF EXISTS \"PACKAGES\";"\
    "DROP TABLE IF EXISTS \"ITEMS\";"\
    "DELETE FROM UPS_WEB_PACKAGES;"		\
    "DELETE FROM UPS_WEB_ITEMS;"\
    "DELETE FROM UPS_WEB_TRUCK_POOL;"\
    "DELETE FROM UPS_WEB_PACKAGE_HISTORY;"\
    "ALTER SEQUENCE UPS_WEB_PACKAGES_ID_SEQ RESTART WITH 1;"\
    "ALTER SEQUENCE UPS_WEB_ITEMS_ID_SEQ RESTART WITH 1;"\
    "ALTER SEQUENCE UPS_WEB_TRUCK_POOL_ID_SEQ RESTART WITH 1;"\
    "ALTER SEQUENCE UPS_WEB_PACKAGE_HISTORY_ID_SEQ RESTART WITH 1;"
    ;

  //trucs_pool table
  sql += "CREATE TABLE \"TRUCKS_POOL\"("\
    "TRUCK_ID     INT NOT NULL,"\
    "TRUCK_STATUS INT NOT NULL,"\
    "PRIMARY KEY (TRUCK_ID));";

  //packages table
  sql += "CREATE TABLE \"PACKAGES\"("\
    "PACKAGE_ID   INT NOT NULL,"\
    "ACCOUNT      VARCHAR(50),"\
    "DEST_X       INT NOT NULL,"\
    "DEST_Y       INT NOT NULL,"\
    "WHID         INT,"\
    "TRUCK_ID     INT,"\
    "STATUS       INT,"\
    "PRIMARY KEY (PACKAGE_ID));";
  //item table
  sql += "CREATE TABLE \"ITEMS\"("		\
    "PACKAGE_ID    INT NOT NULL,"\
    "PRODUCT      VARCHAR(50) NOT NULL,"\
    "COUNT        INT NOT NULL,"\
    "PRIMARY KEY (PACKAGE_ID));";
  
  work W(*C);

  /* Execute SQL query */
  W.exec(sql.c_str() );
  W.commit();
}


//connect to database
connection* connect_db(){
  connection* C;
  try{
    C = new connection("dbname=postgres user=postgres password=abc123 host=db");
    if(!(*C).is_open()){
      std::cout<<"Can't open database" <<std::endl;                           
    }
    //return C;
  }
  catch(const std::exception &e){
    std::cerr<<e.what()<<std::endl;
    //return NULL;
  }
  return C;
}


//init truck table
void init_truck(connection* C, int id, int status, int x, int y){
  ostringstream sql;
  sql <<"INSERT INTO UPS_WEB_TRUCK_POOL(TRUCK_ID, TRUCK_STATUS, DEST_X, DEST_Y)";
  sql <<"VALUES("<<id<<", "<<status<<", "<<x<<", "<<y<<");";

  work W(*C);
   
  /* Execute SQL query */
  W.exec( sql.str().c_str());
  W.commit();

}

//update truck table
void update_truck(connection* C, int id, int status){
  cout<<"update truck "<<id<<" ,status "<<status<<endl;
  ostringstream sql;
  sql << "UPDATE UPS_WEB_TRUCK_POOL SET TRUCK_STATUS = "<<status<<" WHERE TRUCK_ID = "<<id<<";";

  work W(*C);
   
  /* Execute SQL query */
  W.exec( sql.str().c_str());
  W.commit();
}

void update_truck_location(connection* C, int id, int status, int x, int y){
  cout<<"update truck "<<id<<" location "<<status<<endl;
  ostringstream sql;
  sql << "UPDATE UPS_WEB_TRUCK_POOL SET TRUCK_STATUS = "<<status<<", DEST_X = "<<x<<", DEST_Y = "<<y<<" WHERE TRUCK_ID = "<<id<<";";

  work W(*C);

  /* Execute SQL query */
  W.exec( sql.str().c_str());
  W.commit();
}

//get truck for picking up
int getTruck(connection* C, int status){
  cout<<"get the first free truck"<<endl;
  ostringstream sql;
  //int idle = 0;
  //int delivering = 4;
  ///int arrive_warehouse=2;
  /*sql<<"SELECT TRUCK_ID FROM UPS_WEB_TRUCK_POOL"			\
    " WHERE TRUCK_STATUS = "<<idle<< " OR TRUCK_STATUS = "<<delivering<<";";*/
  
  sql<<"SELECT TRUCK_ID FROM UPS_WEB_TRUCK_POOL"			\
    " WHERE TRUCK_STATUS = "<<status<<";";
  nontransaction N(*C);
  result R(N.exec( sql.str().c_str()));

  vector<int> free_trucks;
  for (result::const_iterator c = R.begin(); c != R.end(); ++c) {
    free_trucks.push_back(c[0].as<int>());
  }
  //select arrive ware house
  /*sql<<"SELECT TRUCK_ID FROM UPS_WEB_TRUCK_POOL"			\
    " WHERE TRUCK_STATUS = "<<arrive_warehouse<< " AND WHID = "<<whid<<";";
  nontransaction N1(*C);
  result R_2(N1.exec( sql.str().c_str()));
  for (result::const_iterator c = R_2.begin(); c != R_2.end(); ++c) {
    free_trucks.push_back(c[0].as<int>());
    }*/

  
  return free_trucks.size()==0? -1:free_trucks[0];
}



//init package table func
void init_package(connection* C,int pkid, string account,  int x, int y, int truck_id, int whid, int status){
  cout<<"initial package"<<endl;
  ostringstream sql;
  
  sql <<"INSERT INTO UPS_WEB_PACKAGES(PACKAGE_ID,ACCOUNT,  DEST_X, DEST_Y, TRUCK_ID, WHID, STATUS)";
  sql <<"VALUES("<<pkid<<", '"<<account<<"', "<<x<<", "<<y<<", "<<truck_id<<","<<whid<<", "<<status<<");";

  work W(*C);
   
  /* Execute SQL query */
  W.exec(sql.str().c_str());
  W.commit();
}

/*void init_package(connection* C,int pkid, int x, int y, int truck_id, int whid, int status){
  ostringstream sql;
  sql <<"INSERT INTO \"PACKAGES\"(PACKAGE_ID, DEST_X, DEST_Y, TRUCK_ID, WHID, STATUS)";
  sql <<"VALUES("<<pkid<<", "<<x<<", "<<y<<", "<<truck_id<<","<<whid<<", "<<status<<");";

  work W(*C);

   Execute SQL query 
  W.exec(sql.str().c_str());
  W.commit();
}*/
//update package table func
void update_package(connection* C, int pkid, int status){
  ostringstream sql;
  sql << "UPDATE UPS_WEB_PACKAGES SET STATUS = "<<status<<" WHERE PACKAGE_ID = "<<pkid<<";";
  work W(*C);
   
  /* Execute SQL query */
  W.exec(sql.str().c_str());
  W.commit();
}

//change package dest
void update_package_dest(connection* C, int pkid, int x, int y){
  cout<<"update package dest"<<endl;
  ostringstream sql;
  sql << "UPDATE UPS_WEB_PACKAGES SET DEST_X = "<<x<<", DEST_Y = "<<y<<" WHERE PACKAGE_ID = "<<pkid<<";";
  work W(*C);
   
  /* Execute SQL query */
  W.exec(sql.str().c_str());
  W.commit();
}


//update package truckid
void update_package_truckid(connection* C, int pkid, int truckid){
  cout<<"update package:"<<pkid<<" truckid: "<<truckid<<endl;
  ostringstream sql;
  sql << "UPDATE UPS_WEB_PACKAGES SET TRUCK_ID = "<<truckid<<" WHERE PACKAGE_ID = "<<pkid<<";";
  work W(*C);
   
  /* Execute SQL query */
  W.exec(sql.str().c_str());
  W.commit();
}

int get_package_truckid(connection* C, int pkid){
  ostringstream sql;
  //int to_delivery = 0;                                                                                                                                           
  sql << "SELECT TRUCK_ID FROM UPS_WEB_PACKAGES WHERE PACKAGE_ID = "<<pkid<<";";
  nontransaction N(*C);
  result R(N.exec( sql.str().c_str()));

  int tid = R.begin()[0].as<int>();
  /*vector<int> coordinate;
  for (result::const_iterator c = R.begin(); c != R.end(); ++c) {
    coordinate.push_back(c[0].as<int>());
    ///coordinate.push_back(c[1].as<int>());
    }*/
  cout<<"get pakage:"<<pkid << "truckid:"<<tid<<endl;
  return tid;
}

vector<int> getPackageId(connection* C, int truck_id, int status){
  ostringstream sql;
  //int to_delivery = 0;
  sql << "SELECT PACKAGE_ID FROM UPS_WEB_PACKAGES WHERE TRUCK_ID = "<<truck_id<<" AND STATUS = "<<status<<";";
  nontransaction N(*C);
  result R(N.exec( sql.str().c_str()));

  
  vector<int> pkid;
  for (result::const_iterator c = R.begin(); c != R.end(); ++c) {
    pkid.push_back(c[0].as<int>());
  }
  return pkid;
}

vector<int> getPkgDest(connection* C, int shipId){
  ostringstream sql;
  //int to_delivery = 0;
  sql << "SELECT DEST_X, DEST_Y FROM UPS_WEB_PACKAGES WHERE PACKAGE_ID = "<<shipId<<";";
  nontransaction N(*C);
  result R(N.exec( sql.str().c_str()));

  vector<int> coordinate;
  for (result::const_iterator c = R.begin(); c != R.end(); ++c) {
    coordinate.push_back(c[0].as<int>());
    coordinate.push_back(c[1].as<int>());
  }
  
  return coordinate;
}

//get whid
int getWarehouse(connection* C, int truck_id){
  int whid = -1;
  ostringstream sql;
  sql << "SELECT WHID FROM UPS_WEB_PACKAGES WHERE TRUCK_ID = "<<truck_id<<";";
  nontransaction N(*C);
  result R(N.exec( sql.str().c_str()));

  vector<int> wh;
  for (result::const_iterator c = R.begin(); c != R.end(); ++c) {
    wh.push_back(c[0].as<int>());
  }
  
  return wh.size()==0? whid:wh[0];
}
//init item table
void init_item(connection* C, int id, string package, int count){
  ostringstream sql;
  sql <<"INSERT INTO UPS_WEB_ITEMS(PACKAGE_ID, PRODUCT,COUNT)";
  sql <<"VALUES("<<id<<", '"<<package<<"', "<<count<<");";

  work W(*C);
   
  /*Execute SQL query */
  W.exec( sql.str().c_str());
  W.commit();

}

bool findUser(connection*C, string username){
  ostringstream sql;
  sql << "SELECT USERNAME FROM AUTH_USER WHERE USERNAME = '"<<username<<"';";
  nontransaction N(*C);
  result R(N.exec( sql.str().c_str()));
  if(R.size() == 0){
    return false;
  }
  else{
    return true;
  }
}

void initHistory(connection* C, int pkgid, string topickup){
  ostringstream sql;
  sql <<"INSERT INTO UPS_WEB_PACKAGE_HISTORY(PACKAGE_ID, TOPICKUP)";
  sql <<"VALUES("<<pkgid<<", '"<<topickup<<"');";

  work W(*C);

  /*Execute SQL query */
  W.exec( sql.str().c_str());
  W.commit();
}

void addLoadHistory(connection* C, int pkgid, string toload){
  ostringstream sql;
  sql << "UPDATE UPS_WEB_PACKAGE_HISTORY SET TOLOAD= '"<<toload<<"' WHERE PACKAGE_ID = "<<pkgid<<";";
  work W(*C);

  /* Execute SQL query */
  W.exec(sql.str().c_str());
  W.commit();
}

void addDeliverHistory(connection* C, int pkgid, string todeliver){
  ostringstream sql;
  sql << "UPDATE UPS_WEB_PACKAGE_HISTORY SET TODELIVER= '"<<todeliver<<"' WHERE PACKAGE_ID = "<<pkgid<<";";
  work W(*C);

  /* Execute SQL query */
  W.exec(sql.str().c_str());
  W.commit();
}

void addDeliveredHistory(connection* C, int pkgid, string delivered){
  ostringstream sql;
  sql << "UPDATE UPS_WEB_PACKAGE_HISTORY SET DELIVERED= '"<<delivered<<"' WHERE PACKAGE_ID = "<<pkgid<<";";
  work W(*C);

  /* Execute SQL query */
  W.exec(sql.str().c_str());
  W.commit();
}
#endif
