/****************************************************************************
  FileName     [ cirGate.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define class CirAigGate member functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include "cirGate.h"

#include <stdarg.h>

#include <cassert>
#include <iomanip>
#include <iostream>
#include <sstream>

#include "cirMgr.h"
#include "util.h"

using namespace std;

extern CirMgr* cirMgr;
int CirGate::_globalRef = 1;
// TODO: Implement memeber functions for class(es) in cirGate.h
/**************************************/
/*   class CirGate member functions   */
/**************************************/
void CirGate::setGate(int id, int line, char symbol, int left, int right) {
    gateID = id;
    lineNum = line;
    gateType = symbol;
    left_fanin = left;
    right_fanin = right;
}

void CirGate::reportGate() const {
    stringstream IDs, lineNums;
    IDs << gateID;
    string gateS = IDs.str();
    lineNums << lineNum;
    string lineS = lineNums.str();
    int blankNum = 0;
    blankNum += gateS.length();
    blankNum += lineS.length();
    if (symbol != "") blankNum += symbol.length() + 2;

    if (gateType == 'I') {
        cout << "==================================================" << endl;
        cout << "= PI(" << gateID;
        if (symbol != "")
            cout << ")\"" << symbol << "\", line " << lineNum;
        else
            cout << "), line " << lineNum;
        for (int i = 0; i < 36 - blankNum; i++) cout << ' ';
        cout << '=' << endl;
        cout << "==================================================" << endl;
    } else if (gateType == 'C') {
        cout << "==================================================" << endl;
        cout << "= CONST(" << gateID << "), line " << lineNum;
        for (int i = 0; i < 33 - blankNum; i++) cout << ' ';
        cout << '=' << endl;
        cout << "==================================================" << endl;
    } else if (gateType == 'A') {
        cout << "==================================================" << endl;
        cout << "= AIG(" << gateID << "), line " << lineNum;
        for (int i = 0; i < 35 - blankNum; i++) cout << ' ';
        cout << '=' << endl;
        cout << "==================================================" << endl;
    } else if (gateType == 'O') {
        cout << "==================================================" << endl;
        cout << "= PO(" << gateID;
        if (symbol != "")
            cout << ")\"" << symbol << "\", line " << lineNum;
        else
            cout << "), line " << lineNum;
        for (int i = 0; i < 36 - blankNum; i++) cout << ' ';
        cout << '=' << endl;
        cout << "==================================================" << endl;
    }
}
void CirGate::reportFanin(int level, bool inv) {
    unordered_set<int> visited;
    this->fanin(level, 0, visited);
}

void CirGate::fanin(int max_level, int level, unordered_set<int>& visited, bool inv) {
    if (level > max_level) return;
    // printinf infos
    for (int i = 0; i < level; i++) cout << "  "; 
    if (inv) cout << '!';
    if (gateType == 'O') {
        cout << "PO " << gateID << endl;
    } else if (gateType == 'A') {
        cout << "AIG " << gateID;
        if (visited.find(gateID) != visited.end()) {
            if (level < max_level) cout << " (*)";
            cout << endl;
            return;
        }
        cout << endl;
    } else if (gateType == 'I') {
        cout << "PI " << gateID << endl;
    }

    if (level < max_level) visited.insert(gateID);

    if(left_fanin != -1){
        CirGate* left_gate = cirMgr->getGate(left_fanin / 2);
        if(left_gate == 0){
            for (int i = 0; i < level; i++) cout << "  "; 
            cout << "UNDEF " << left_fanin / 2 << endl;
        }
        else{
            left_gate->fanin(max_level, level+1, visited, left_fanin % 2);
        }
    }

    if(right_fanin != -1){
        CirGate* right_gate = cirMgr->getGate(right_fanin / 2);
        if(right_gate == 0){
            for (int i = 0; i < level; i++) cout << "  "; 
            cout << "UNDEF " << right_fanin / 2 << endl;
        }
        else{
            right_gate->fanin(max_level, level+1, visited, right_fanin % 2);
        }
    }
}

void CirGate::fanout(int max_level, int level, unordered_set<int>& visited, bool inv) {
    if (level > max_level) return;
    for (int i = 0; i < level; i++) cout << "  ";
    if (gateType == 'O') {
        if (inv) cout << '!';
        cout << "PO " << gateID << endl;
        return;
    } else if (gateType == 'A') {
        if (inv) cout << '!';
        cout << "AIG " << gateID;
        // fanout printed, so we return
        if (visited.find(gateID) != visited.end()) {
            if (level < max_level) cout << " (*)";
            cout << endl;
            return;
        }
        cout << endl;
    } else if (gateType == 'I') {
        cout << "PI " << gateID << endl;
    }

    // if level == max_level, the fanout are not printed
    // so we don't mark it 
    if (level < max_level) visited.insert(gateID);
    // Sorted to match ref
    vector<int> fouts = cirMgr->fanOuts[gateID];
    sort(fouts.begin(), fouts.end());
    for (int outs : fouts) {
        bool inv = false;
        if (outs % 2) inv = true;
        cirMgr->getGate(outs / 2)->fanout(max_level, level + 1, visited, inv);
    }
}

void CirGate::reportFanout(int level) {
    unordered_set<int> visited;
    this->fanout(level, 0, visited);
}

void CirGate::printGate() const {
    if (gateType == 'I') {
        cout << "PI  " << gateID;
        if (symbol != "") cout << ' ' << '(' << symbol << ')';
        cout << endl;

    } else if (gateType == 'C') {
        cout << "CONST0" << endl;

    } else if (gateType == 'A') {
        cout << "AIG " << gateID << ' ';

        CirGate* left_gate = cirMgr->getGate(left_fanin / 2);
        CirGate* right_gate = cirMgr->getGate(right_fanin / 2);

        if (left_gate == 0 or left_gate->gateID == -1)
            cout << '*';
        else if (left_fanin % 2)
            cout << '!';
        cout << left_fanin / 2 << ' ';

        if (right_gate == 0 or right_gate->gateID == -1)
            cout << '*';
        else if (right_fanin % 2)
            cout << '!';
        cout << right_fanin / 2 << endl;

    } else if (gateType == 'O') {
        cout << "PO  " << gateID << ' ';
        if (left_fanin % 2) cout << '!';
        cout << left_fanin / 2;
        if (symbol != "")
            cout << " (" << symbol << ')';
        cout << endl;
    }
}
/*
void CirGate::fanin(int firstlevel, int level, bool inv) {
    //cout<<"the level is "<<level<<' ';
    if (level < 0) return;
    assert(level >= 0);
    for (int i = 0; i < firstlevel - level; i++) cout << ' ' << ' ';
    if (inv) cout << '!';
    if (gateType == 'O') {
        cout << "PO " << gateID << endl;
    } else if (gateType == 'A') {
        cout << "AIG " << gateID;
        if (_ref == _globalRef && level != 0) {
            cout << " (*)" << endl;
            return;
        }
        cout << endl;
    } else if (gateType == 'I') {
        cout << "PI " << gateID << endl;
    }
    assert(level >= 0);
    if (level != 0) _ref = _globalRef;
    if (left_fanin != -1) {
        if (left_fanin % 2 == 0) {
            if (cirMgr->getGate(left_fanin / 2) == 0) {
                for (int i = 0; i < firstlevel - level + 1; i++) cout << ' ' << ' ';
                cout << "UNDEF " << left_fanin / 2 << endl;
            } else {
                cirMgr->getGate(left_fanin / 2)->fanin(firstlevel, level - 1);
            }
        } else {
            if (cirMgr->getGate((left_fanin - 1) / 2) == 0) {
                for (int i = 0; i < firstlevel - level + 1; i++) cout << ' ' << ' ';
                cout << "!UNDEF " << (left_fanin - 1) / 2 << endl;
            } else {
                cirMgr->getGate((left_fanin - 1) / 2)->fanin(firstlevel, level - 1, true);
            }
        }
    }
    if (right_fanin != -1) {
        if (right_fanin % 2 == 0) {
            if (cirMgr->getGate(right_fanin / 2) == 0) {
                for (int i = 0; i < firstlevel - level + 1; i++) cout << ' ' << ' ';
                cout << "UNDEF " << right_fanin / 2 << endl;
            } else {
                cirMgr->getGate(right_fanin / 2)->fanin(firstlevel, level - 1);
            }
        } else {
            if (cirMgr->getGate((right_fanin - 1) / 2) == 0) {
                for (int i = 0; i < firstlevel - level + 1; i++) cout << ' ' << ' ';
                cout << "!UNDEF " << (right_fanin - 1) / 2 << endl;
            } else {
                cirMgr->getGate((right_fanin - 1) / 2)->Fanin(firstlevel, level - 1, true);
            }
        }
    }
}*/