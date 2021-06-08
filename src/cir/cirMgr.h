/****************************************************************************
  FileName     [ cirMgr.h ]
  PackageName  [ cir ]
  Synopsis     [ Define circuit manager ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_MGR_H
#define CIR_MGR_H

#include <vector>
#include <string>
#include <fstream>
#include <iostream>

using namespace std;

#include "cirDef.h"

extern CirMgr *cirMgr;
//CirMgr A(1,2),B; int a; A();
// TODO: Define your own data members and member functions
class CirMgr
{
public:
   CirMgr(int gateNum,int AIGn){
       allGates = new CirGate*[gateNum];
       for(int i = 0; i < gateNum; i++)
           allGates[i] = 0;
       AIGnum = AIGn;
       Acount = 0;
       GateNum = gateNum;
   }
    CirMgr() {};
   ~CirMgr() {
       if(allGates != 0) delete [] allGates;
   }

   // Access functions
   // return '0' if "gid" corresponds to an undefined gate.
   CirGate* getGate(unsigned gid) const {
       if(gid >= GateNum) return 0;
       if(cirMgr -> allGates[gid] == 0) return 0;
       else{
           return cirMgr -> allGates[gid];
       }
   }

   // Member functions about circuit construction
    bool readCircuit(const string&);
    // Member functions about circuit optimization
    void sweep();
    void optimize();
    
    // Member functions about simulation
    void randomSim();
    void fileSim(ifstream&);
    void setSimLog(ofstream *logFile) { _simLog = logFile; }
    
    // Member functions about fraig
    void strash();
    void printFEC() const;
    void fraig();

   // Member functions about circuit reporting
    void printSummary() const;
    void printNetlist();
    void printPIs() const;
    void printPOs() const;
    void printFloatGates() const;
    void printFECPairs() const;
    void writeAag(ostream&);
    void writeGate(ostream&, CirGate*) const;
    void DepthS(CirGate*,int&,int toCount = 0);
    void DepthS(CirGate*,int&,ostream&,int toCount = 0);
    bool checkFLfanin(int n) const;

private:
    vector<int> inputID;
    vector<int> outputID;
    vector<int> AIGtoLinkID;
    int AIGnum;
    int GateNum;
    int Acount;
    CirGate** allGates = 0; //stores gate object pointer
    ofstream           *_simLog;
    
};

#endif // CIR_MGR_H
