#ifndef SERVER_H
#define SERVER_H 
#include <jsonrpccpp/server.h>
#include <jsonrpccpp/server/connectors/httpserver.h>
using namespace std;
using namespace jsonrpc;

class FraigServer: public AbstractServer<FraigServer> {

 public:
    // Constructor
    FraigServer(HttpServer &server) : AbstractServer<FraigServer>(server) {
    this->bindAndAddMethod(
            Procedure("cirread", PARAMS_BY_NAME, JSON_STRING,
                      "name", JSON_STRING, NULL), &FraigServer::readFile);

    this->bindAndAddNotification(
            Procedure("notifyServer", PARAMS_BY_NAME, NULL),
                      &FraigServer::notifyServer);

    this->bindAndAddMethod(
            Procedure("cirwrite", PARAMS_BY_NAME, JSON_STRING, NULL), 
            &FraigServer::getCircuit);
    }

    void getCircuit(const Json::Value &request, Json::Value &response);
    void readFile(const Json::Value &request, Json::Value &response);
    // method
    void sayHello(const Json::Value &request, Json::Value &response);

    // transfer object
    void getObj(const Json::Value &request, Json::Value &response);

    // notification
    void notifyServer(const Json::Value &request);
};

enum CirCmdState
{
   // Order matters! Do not change the order!!
   CIRINIT,
   CIRREAD,
   CIROPT,
   CIRSTRASH,
   CIRSIMULATE,
   CIRFRAIG,
   // dummy end
   CIRCMDTOT
};

#endif
