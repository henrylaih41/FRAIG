/****************************************************************************
  FileName     [ cirGate.h ]
  PackageName  [ cir ]
  Synopsis     [ Define basic gate data structures ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_GATE_H
#define CIR_GATE_H

#include <string>
#include <vector>
#include <iostream>
#include "cirDef.h"

using namespace std;

class CirGate;

//------------------------------------------------------------------------
//   Define classes
//------------------------------------------------------------------------
// TODO: Define your own data members and member functions, or classes
class CirGate
{
public:
    CirGate(int ID,int line,char type,string name = ""): gateID(ID), lineNum(line),_ref(0), left_fanin(-1), right_fanin(-1),gateName(name){
        gateType = type;
    }
   virtual ~CirGate() {}

   // Basic access methods
   string getTypeStr() const { return ""; }
   unsigned getLineNo() const { return 0; }
   
   // Printing functions
   void printGate() const;
   void reportGate() const;
   void reportFanin(int level,bool inv = false);
   void reportFanout(int level,bool inv = false);
   void Fanin(int,int,bool inv = false);
   void Fanout(int,int,bool inv = false);
   virtual bool isAig() const { return false; }
    
    char gateType;
    static int _globalRef;
    unsigned int gateID;//don't know if necessary
    unsigned int lineNum;
    int _ref;
    int left_fanin;
    int right_fanin;
    string gateName;
    vector<int> fanoutList;
};

#endif // CIR_GATE_H
