#include <iostream>
#include <jsonrpccpp/server.h>
#include <jsonrpccpp/server/connectors/httpserver.h>
#include <stdio.h>
#include <string>
#include "util.h"
#include "cmdParser.h"

using namespace jsonrpc;
using namespace std;

// Global varialbe resposible for command parsering
CmdParser* cmdMgr = new CmdParser("fraig> ");
// Global variable that contains the whole graph and operations functions.
// allocated in CirReadCmd::exec(const string& option) in cirCmd.cpp

extern bool initCommonCmd();
extern bool initCirCmd();

class FraigServer: public AbstractServer<FraigServer> {
public:
  FraigServer(HttpServer &server) : AbstractServer<FraigServer>(server) {
    this->bindAndAddMethod(Procedure("sayHello", PARAMS_BY_NAME, JSON_STRING,
				     "name", JSON_STRING, NULL),
			   &FraigServer::sayHello);
    this->bindAndAddNotification(
	Procedure("notifyServer", PARAMS_BY_NAME, NULL),
	&FraigServer::notifyServer);
    this->bindAndAddMethod(Procedure("getObj", PARAMS_BY_NAME, JSON_STRING,
				     "name", JSON_STRING, NULL),
			   &FraigServer::getObj);
  }

  // method
  void sayHello(const Json::Value &request, Json::Value &response) {
    response = "Hello: " + request["name"].asString();
  }

  // transfer object
  void getObj(const Json::Value &request, Json::Value &response){
      response["gate"] = 1;
      response["name"] = "Henry";
  }

  // notification
  void notifyServer(const Json::Value &request) {
    (void)request;
    cout << "server received some Notification" << endl;
  }
};

static void
usage()
{
   cout << "Usage: cirTest [ -File < doFile > ]" << endl;
}

static void
myexit()
{
   usage();
   exit(-1);
}

int
main(int argc, char **argv)
{
    myUsage.reset();
    cout << "Server Starting!" << endl;
    try {
        HttpServer server(9123);
        FraigServer serv(server);
        if (serv.StartListening()) {
            cout << "Server started successfully" << endl;
            getchar();
            serv.StopListening();
        } else {
            cout << "Error starting Server" << endl;
        }
    }

    catch (jsonrpc::JsonRpcException &e) {
        cerr << e.what() << endl;
    }
}
