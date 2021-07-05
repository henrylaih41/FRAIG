/****************************************************************************
  FileName     [ cirMgr.h ]
  PackageName  [ cir ]
  Synopsis     [ Define circuit manager ]
  Author       [ Yen-Li (Henry) Laih, Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_MGR_H
#define CIR_MGR_H

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

#include "cirDef.h"

extern CirMgr *cirMgr;

class CirMgr {
   public:
    CirMgr(){};
    ~CirMgr(){};

    // Access functions
    CirGate *getGate(int);
    // Member functions about circuit construction
    void init(int, int, int);
    bool readCircuit(const string &);
    // Used when we want to clear the current circuit and free the resource
    void reset();
    // Member functions about circuit optimization
    void sweep();
    void optimize();

    // Member functions about simulation
    void randomSim();
    void fileSim(ifstream &);
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
    void writeAag(ostream &);
    void writeGate(ostream &, CirGate *) const;
    void dfs(int, int *, int&, int init_run = 0, ostream& out = cout);
    
    // utils
    void getFanins(int idx, int& left, int& right) const;
    // The whole circuit is stored as an array of gates, 
    // where each gates connects to two input, left and right
    // and output stored in fanOuts[gateID]
    int AIGnum;
    int GateNum;
    // uninitialize gates has gateID = -1
    // stores the gate objects, 0 is constant 0.
    CirGate *allGates;  
    ofstream *_simLog; 
    vector<int> *fanOuts; // fanOuts[gateID] is the output of gateID, format: gateID*2 + inv
    vector<int> inputID;  // format: gateID
    vector<int> outputID;  // format: gateID
    vector<int> dfsList;  // format: gateID

};

#endif  // CIR_MGR_H
