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

extern CirMgr *cirMgr;
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
    CirGate buf = *this;
    buf.Fanin(level, level, inv);
    _globalRef++;
    if (_globalRef > 60000) _globalRef = 0;
}

void CirGate::Fanin(int firstlevel, int level, bool inv) {
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
                cirMgr->getGate(left_fanin / 2)->Fanin(firstlevel, level - 1);
            }
        } else {
            if (cirMgr->getGate((left_fanin - 1) / 2) == 0) {
                for (int i = 0; i < firstlevel - level + 1; i++) cout << ' ' << ' ';
                cout << "!UNDEF " << (left_fanin - 1) / 2 << endl;
            } else {
                cirMgr->getGate((left_fanin - 1) / 2)->Fanin(firstlevel, level - 1, true);
            }
        }
    }
    if (right_fanin != -1) {
        if (right_fanin % 2 == 0) {
            if (cirMgr->getGate(right_fanin / 2) == 0) {
                for (int i = 0; i < firstlevel - level + 1; i++) cout << ' ' << ' ';
                cout << "UNDEF " << right_fanin / 2 << endl;
            } else {
                cirMgr->getGate(right_fanin / 2)->Fanin(firstlevel, level - 1);
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

}

//記得最後一個level若走過 不要紀錄走過 (標記那些fanout過的）

void CirGate::Fanout(int firstlevel, int level, bool inv) {
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
    if (level != 0 && fanoutList.size() != 0) _ref = _globalRef;
    for (int i = 0; i < fanoutList.size(); i++) {
        if (fanoutList[i] % 2 == 0) {
            cirMgr->getGate(fanoutList[i] / 2)->Fanout(firstlevel, level - 1);
        } else {
            cirMgr->getGate(fanoutList[i] / 2)->Fanout(firstlevel, level - 1, true);
        }
    }
}

void CirGate::reportFanout(int level, bool inv) {
    assert(level >= 0);
    CirGate buf = *this;
    buf.Fanout(level, level, inv);
    _globalRef++;
    if (_globalRef > 60000) _globalRef = 0;
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
        if(left_fanin % 2) cout << '!';
        cout << left_fanin / 2  << ' ';
        if(right_fanin % 2) cout << '!';
        cout << right_fanin / 2<< endl;
        /*
        if (left_fanin % 2 == 0) {
            if (cirMgr->getGate(left_fanin / 2) == 0) cout << '*';
            cout << left_fanin / 2 << ' ';
        } else {
            if (cirMgr->getGate((left_fanin - 1) / 2) == 0) cout << '*';
            cout << '!' << (left_fanin - 1) / 2 << ' '; 
        }
        if (right_fanin % 2 == 0) {
            if (cirMgr->getGate(right_fanin / 2) == 0) cout << '*';
            cout << right_fanin / 2; 
        } else {
            if (cirMgr->getGate((right_fanin - 1) / 2) == 0) cout << '*';
            cout << '!' << (right_fanin - 1) / 2;
        }
        cout << endl;
        */
    } else if (gateType == 'O') {
        cout << "PO  " << gateID << ' ';
        if(left_fanin % 2) cout << '!';
        cout << left_fanin / 2;
        if (symbol != "")
            cout << " (" << symbol << ')';
        cout << endl;
        /*
        if (left_fanin % 2 == 0) {
            if (cirMgr->getGate(left_fanin / 2) == 0) cout << '*';
            cout << left_fanin / 2;
            if (symbol != "")
                cout << ' ' << '(' << symbol << ')';
            cout << endl;
        } else {
            if (cirMgr->getGate((left_fanin - 1) / 2) == 0) cout << '*';
            cout << '!' << (left_fanin - 1) / 2;
            if (symbol != "")
                cout << ' ' << '(' << symbol << ')';
            cout << endl;
        }*/
    }
}
