/****************************************************************************
  FileName     [ cirGate.h ]
  PackageName  [ cir ]
  Synopsis     [ Define basic gate data structures ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_GATE_H
#define CIR_GATE_H

#include <iostream>
#include <string>
#include <vector>

#include "cirDef.h"
#include <bits/stdc++.h>

using namespace std;

class CirGate;

//------------------------------------------------------------------------
//   Define classes
//------------------------------------------------------------------------
// TODO: Define your own data members and member functions, or classes
class CirGate {
   public:
    bool in_dfs;
    char gateType;
    int gateID;  //don't know if necessary
    int lineNum;
    int _ref;
    int left_fanin;
    int right_fanin;
    //vector<int> fanoutList;
    string symbol;

    CirGate(){
      gateType = 'U';
      in_dfs = 0;
      left_fanin = -1; // to induce segfault if accessing undefined gate
      right_fanin = -1;
    }
    ~CirGate() {}
    void setGate(int idx, int line, char symbol, int left = -1, int right = -1);
    // Basic access methods
    string getTypeStr() const { return ""; }
    unsigned getLineNo() const { return 0; }

    // Printing functions
    void printGate() const;
    void reportGate() const;
    void reportFanin(int level, bool inv = false);
    void reportFanout(int level);
    void fanin(int, int, unordered_set<int>&, bool inv = false);
    void fanout(int, int, unordered_set<int>&, bool inv = false);
    virtual bool isAig() const { return false; }
};

#endif  // CIR_GATE_H
