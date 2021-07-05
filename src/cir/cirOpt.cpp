/****************************************************************************
  FileName     [ cirSim.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir optimization functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <cassert>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"

using namespace std;

// TODO: Please keep "CirMgr::sweep()" and "CirMgr::optimize()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/
extern CirMgr* cirMgr; 
/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/**************************************************/
/*   Public member functions about optimization   */
/**************************************************/
// Remove unused gates
// DFS list should NOT be changed
// UNDEF, float and unused list may be changed
void
CirMgr::sweep(){
  for(size_t i = 1; i <= GateNum; ++i){
    // only sweep UNDEF or AIG gates
    if(allGates[i].in_dfs == 0 && (allGates[i].gateType == 'U' || allGates[i].gateType == 'A')){
      sweepGate(allGates+i);
    }
  } 
}

void
CirMgr::sweepGate(CirGate* gate){
  cout << "Sweeping: ";

  // maintain CirMgr invariants
  // UNDEF gate
  if(gate->gateType == 'U'){
    cout << "UNDEF";
  }
  else{
    cout << "AIG";
    // remove left right fanouts
    vector<size_t> *out;
    size_t remove_id;
    out = &fanOuts[gate->left_fanin / 2];
    remove_id = gate->gateID*2 + gate->left_fanin % 2;
    out->erase(find(out->begin(), out->end(), remove_id));
    out = &fanOuts[gate->right_fanin / 2];
    remove_id = gate->gateID*2 + gate->right_fanin % 2;
    out->erase(find(out->begin(), out->end(), remove_id));
    cirMgr->AIGnum--;
  }

  cout << '(' << gate->gateID << ')' << " removed..." << endl;

  // clear the gate
  gate->gateType = 'U';
  gate->left_fanin = -1;
  gate->right_fanin = -1;
  //fanOuts[gate->gateID].clear();

}

// Recursively simplifying from POs;
// _dfsList needs to be reconstructed afterwards
// UNDEF gates may be delete if its fanout becomes empty...
void
CirMgr::optimize(){

}

/***************************************************/
/*   Private member functions about optimization   */
/***************************************************/
