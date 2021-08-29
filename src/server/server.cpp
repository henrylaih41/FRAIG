#include <iostream>
#include "server.h"
#include "../cir/cirMgr.h"
#include <stdio.h>
#include <string>
#define SERVER_PORT 9123

// Global varialbe resposible for command parsering
extern CirMgr* cirMgr;
static CirCmdState curCmd = CIRINIT;

void FraigServer::readFile(const Json::Value &request, Json::Value &response){
    bool doReplace = request["-r"].asBool();
    string fileName = request["fileName"].asString();
    cout << "Reading File " << fileName << endl;
    if (cirMgr != 0) {
        if (doReplace) {
            error(response, "Note: original circuit is replaced...");
            curCmd = CIRINIT;
            cirMgr->reset();
        }
        else {
            error(response, "Error: circuit already exists!!");
            return;
        }
    }
    else cirMgr = new CirMgr();

    if (cirMgr->readCircuit(fileName)) {
        error(response, "Failed to read file: " + fileName);
        curCmd = CIRINIT;
        delete cirMgr; cirMgr = 0;
        return;
    }

    curCmd = CIRREAD;

}

void FraigServer::getCircuit(const Json::Value &request, Json::Value &response){
    if(cirMgr == 0){
        cerr << "cir not init!" << endl;

        return;
    }
    stringstream ss;
    cirMgr->writeAag(ss);
    response["payload"] = ss.str();
}
// Server procedure calls
void FraigServer::sayHello(const Json::Value &request, Json::Value &response) {
    response = "Hello: " + request["name"].asString();
}

void FraigServer::getObj(const Json::Value &request, Json::Value &response){
  response["gate"] = 1;
  response["name"] = "Henry";
}

// notification
void FraigServer::notifyServer(const Json::Value &request) {
    (void)request;
    cout << "server received some Notification" << endl;
}

void FraigServer::error(Json::Value &response, string error_msg){
    cerr << error_msg << endl;
    response["error"] = error_msg;
}

static void
usage(){
    cout << "Usage: ./server" << endl;
}

static void
myexit(){
   usage();
   exit(-1);
}

int
main(int argc, char **argv)
{
    if (argc != 1) {
      cerr << "Error: illegal number of argument (" << argc << ")!!\n";
      myexit();
   }

    cout << "Server Starting!" << endl;
    try {
        HttpServer server(SERVER_PORT);
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

   return 0;
}
