/****************************************************************************
  FileName     [ cirMgr.h ]
  PackageName  [ cir ]
  Synopsis     [ Define circuit manager ]
  Author       [ Yen-Li (Henry) Laih, Chung-Yang (Ric) Huang ]
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

class CirMgr
{
public:
    CirMgr() {};
   ~CirMgr() {};

   // Access functions
   // return '0' if "gid" corresponds to an undefined gate.
   CirGate* getGate(int gid);

   // Member functions about circuit construction
    void init(int, int);
    bool readCircuit(const string&);
    // Used when we want to clear the current circuit and free the resource
    void reset();
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
    vector<int> *fanOuts;
    int AIGnum;
    int GateNum;
    int Acount;
    CirGate *allGates; //stores gate object pointer
    ofstream *_simLog;
    
};

#endif // CIR_MGR_H
