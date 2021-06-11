/****************************************************************************
  FileName     [ cirMgr.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir manager functions ]
  Author       [ Yen-Li (Henry), Laih, Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include "cirMgr.h"

#include <ctype.h>

#include <cassert>
#include <cstdio>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <sstream>

#include "cirGate.h"
#include "util.h"

using namespace std;
/*******************************/
/*   Global variable and enum  */
/*******************************/
CirMgr *cirMgr = 0;
/**************************************/
/*   Static varaibles and functions   */
/**************************************/
static unsigned lineNo = 0;  // in printint, lineNo needs to ++
static unsigned colNo = 0;   // in printing, colNo needs to ++
static char buf[1024];
static string errMsg;
static int errInt;
static CirGate *errGate;

static bool
parseError(CirParseError err) {
    switch (err) {
        case EXTRA_SPACE:
            cerr << "[ERROR] Line " << lineNo + 1 << ", Col " << colNo + 1
                 << ": Extra space character is detected!!" << endl;
            break;
        case MISSING_SPACE:
            cerr << "[ERROR] Line " << lineNo + 1 << ", Col " << colNo + 1
                 << ": Missing space character!!" << endl;
            break;
        case ILLEGAL_WSPACE:  // for non-space white space character
            cerr << "[ERROR] Line " << lineNo + 1 << ", Col " << colNo + 1
                 << ": Illegal white space char(" << errInt
                 << ") is detected!!" << endl;
            break;
        case ILLEGAL_NUM:
            cerr << "[ERROR] Line " << lineNo + 1 << ": Illegal "
                 << errMsg << "!!" << endl;
            break;
        case ILLEGAL_IDENTIFIER:
            cerr << "[ERROR] Line " << lineNo + 1 << ": Illegal identifier \""
                 << errMsg << "\"!!" << endl;
            break;
        case ILLEGAL_SYMBOL_TYPE:
            cerr << "[ERROR] Line " << lineNo + 1 << ", Col " << colNo + 1
                 << ": Illegal symbol type (" << errMsg << ")!!" << endl;
            break;
        case ILLEGAL_SYMBOL_NAME:
            cerr << "[ERROR] Line " << lineNo + 1 << ", Col " << colNo + 1
                 << ": Symbolic name contains un-printable char(" << errInt
                 << ")!!" << endl;
            break;
        case MISSING_NUM:
            cerr << "[ERROR] Line " << lineNo + 1 << ", Col " << colNo + 1
                 << ": Missing " << errMsg << "!!" << endl;
            break;
        case MISSING_IDENTIFIER:
            cerr << "[ERROR] Line " << lineNo + 1 << ": Missing \""
                 << errMsg << "\"!!" << endl;
            break;
        case MISSING_NEWLINE:
            cerr << "[ERROR] Line " << lineNo + 1 << ", Col " << colNo + 1
                 << ": A new line is expected here!!" << endl;
            break;
        case MISSING_DEF:
            cerr << "[ERROR] Line " << lineNo + 1 << ": Missing " << errMsg
                 << " definition!!" << endl;
            break;
        case CANNOT_INVERTED:
            cerr << "[ERROR] Line " << lineNo + 1 << ", Col " << colNo + 1
                 << ": " << errMsg << " " << errInt << "(" << errInt / 2
                 << ") cannot be inverted!!" << endl;
            break;
        case MAX_LIT_ID:
            cerr << "[ERROR] Line " << lineNo + 1 << ", Col " << colNo + 1
                 << ": Literal \"" << errInt << "\" exceeds maximum valid ID!!"
                 << endl;
            break;
        case REDEF_GATE:
            cerr << "[ERROR] Line " << lineNo + 1 << ": Literal \"" << errInt
                 << "\" is redefined, previously defined as "
                 << errGate->getTypeStr() << " in line " << errGate->getLineNo()
                 << "!!" << endl;
            break;
        case REDEF_SYMBOLIC_NAME:
            cerr << "[ERROR] Line " << lineNo + 1 << ": Symbolic name for \""
                 << errMsg << errInt << "\" is redefined!!" << endl;
            break;
        case REDEF_CONST:
            cerr << "[ERROR] Line " << lineNo + 1 << ", Col " << colNo + 1
                 << ": Cannot redefine constant (" << errInt << ")!!" << endl;
            break;
        case NUM_TOO_SMALL:
            cerr << "[ERROR] Line " << lineNo + 1 << ": " << errMsg
                 << " is too small (" << errInt << ")!!" << endl;
            break;
        case NUM_TOO_BIG:
            cerr << "[ERROR] Line " << lineNo + 1 << ": " << errMsg
                 << " is too big (" << errInt << ")!!" << endl;
            break;
        default:
            break;
    }
    return false;
}
void CirMgr::init(int maxIDnum, int POnum) {
    cout << "Initializing CirMgr " << maxIDnum << ' ' << POnum << endl;
    allGates = new CirGate[maxIDnum + POnum + 1];  //1-indexed
    fanOuts = new vector<int>[maxIDnum + 1];       //1-indexed
    GateNum = maxIDnum + POnum;
}

void CirMgr::reset() {
    delete[] allGates;
}

CirGate *
CirMgr::getGate(int gid) {
    if (gid >= GateNum)
        return 0;

    else {
        return (cirMgr->allGates + gid);
    }
}
/**************************************************************/
/*   class CirMgr member functions for circuit construction   */
/**************************************************************/
bool CirMgr::readCircuit(const string &fileName) {
    cout << "Reading " << fileName << endl;
    // Variables
    //***********************
    int maxIDnum = 0, PInum = 0, Lnum = 0, POnum = 0, AIGnum = 0;
    int N, left, right, lineNum = 1;
    ifstream filetoRead;
    stringstream ss;
    string row;
    //***********************
    // Check if file is opened
    filetoRead.open(fileName);
    if (!filetoRead) {
        cout << "Cannot open design " << '\"' << fileName << '\"' << "!!" << endl;
        return -1;
    }

    getline(filetoRead, row);  // getting the first row
    ss << row;
    ss >> row >> maxIDnum >> PInum >> Lnum >> POnum >> AIGnum;

    // allocate space for all gates and fanout vectors
    cirMgr->init(maxIDnum, POnum);  // 1-indexed
    // Set up Input Gates
    for (int i = 1; i <= PInum; ++i) {
        cout << i << endl;
        getline(filetoRead, row);  // don't need the information in row
        cirMgr->inputID.push_back(i);
        cirMgr->allGates[i].setGate(i, lineNum, 'I');
        ++lineNum;
    }

    // Set up Output Gates
    for (int i = maxIDnum + 1; i <= maxIDnum + POnum; ++i) {
        cout << i << endl;
        getline(filetoRead, row);
        cirMgr->outputID.push_back(i);
        cirMgr->allGates[i].setGate(i, lineNum, 'O', stoi(row));
        ++lineNum;
    }

    // Set up AIG
    for (int i = 0; i < AIGnum; ++i) {
        getline(filetoRead, row);
        ss.str(std::string());
        ss.clear();
        ss << row;
        ss >> N >> left >> right;
        cirMgr->allGates[N / 2].setGate(N / 2, lineNum, 'A', left, right);
        if (left >= 2)
            cirMgr->fanOuts[left / 2].push_back(N / 2);
        if (right >= 2)
            cirMgr->fanOuts[right / 2].push_back(N / 2);
        ++lineNum;
    }

    return 0;
}

/**********************************************************/
/*   class CirMgr member functions for circuit printing   */
/**********************************************************/
/*********************
Circuit Statistics
==================
  PI          20
  PO          12
  AIG        130
------------------
  Total      162
*********************/
void CirMgr::printSummary() const {
    cout << "Circuit Statistics" << endl;
    cout << "==================" << endl;
    cout << "  PI" << setw(12) << inputID.size() << endl;
    cout << "  PO" << setw(12) << outputID.size() << endl;
    cout << "  AIG" << setw(11) << AIGnum << endl;
    cout << "------------------" << endl;
    cout << "  Total" << setw(9) << inputID.size() + outputID.size() + AIGnum << endl;
}

void CirMgr::printNetlist() {
    cout << endl;
    int count = 0;
    for (int i = 0; i < outputID.size(); i++) {
        DepthS(&allGates[outputID[i]], count);
    }
    CirGate::_globalRef++;
    if (CirGate::_globalRef > 60000)
        CirGate::_globalRef = 0;
}

void CirMgr::DepthS(CirGate *gate, int &count, ostream &outfile, int toCount) {
    gate->_ref = gate->_globalRef;
    if (toCount == 1 && gate->gateType == 'A')
        Acount++;

    if (gate->right_fanin != -1) {
        if (gate->right_fanin % 2 == 0) {
            if (&allGates[gate->right_fanin / 2] == 0)
                ;  ////// 防未定意
            else if (allGates[gate->right_fanin / 2]._ref != gate->_globalRef)
                DepthS(&allGates[gate->right_fanin / 2], count, outfile, toCount);
        } else {
            if ((&allGates[(gate->right_fanin - 1) / 2]) == 0)
                ;  //////防未定意
            else if (allGates[(gate->right_fanin - 1) / 2]._ref != gate->_globalRef)
                DepthS(&allGates[(gate->right_fanin - 1) / 2], count, outfile, toCount);
        }
    }
    if (gate->left_fanin != -1) {
        if (gate->left_fanin % 2 == 0) {
            if (&allGates[gate->left_fanin / 2] == 0)
                ;
            ////// 以上防未定意
            else if (allGates[gate->left_fanin / 2]._ref != gate->_globalRef)
                DepthS(&allGates[gate->left_fanin / 2], count, outfile, toCount);
        } else {
            if (&allGates[(gate->left_fanin - 1) / 2] == 0)
                ;
            ////// 以上防未定意
            else if (allGates[(gate->left_fanin - 1) / 2]._ref != gate->_globalRef)
                DepthS(&allGates[(gate->left_fanin - 1) / 2], count, outfile, toCount);
        }
    }

    if (gate->gateType == 'A' && toCount == 2) {
        outfile << gate->gateID * 2 << ' ' << gate->right_fanin << ' ' << gate->left_fanin << '\n';
    }

    if (toCount == 0) {
        cout << "[" << count << "] ";
        count++;
        gate->printGate();
    }
}

void CirMgr::DepthS(CirGate *gate, int &count, int toCount) {
    gate->_ref = gate->_globalRef;
    if (toCount == 1 && gate->gateType == 'A')
        Acount++;

    if (gate->right_fanin != -1) {
        if (gate->right_fanin % 2 == 0) {
            if (&allGates[gate->right_fanin / 2] == 0)
                ;  ////// 防未定意
            else if (allGates[gate->right_fanin / 2]._ref != gate->_globalRef)
                DepthS(&allGates[gate->right_fanin / 2], count, toCount);
        } else {
            if (&allGates[(gate->right_fanin - 1) / 2] == 0)
                ;  //////防未定意
            else if (allGates[(gate->right_fanin - 1) / 2]._ref != gate->_globalRef)
                DepthS(&allGates[(gate->right_fanin - 1) / 2], count, toCount);
        }
    }
    if (gate->left_fanin != -1) {
        if (gate->left_fanin % 2 == 0) {
            if (&allGates[gate->left_fanin / 2] == 0)
                ;
            ////// 以上防未定意
            else if (allGates[gate->left_fanin / 2]._ref != gate->_globalRef)
                DepthS(&allGates[gate->left_fanin / 2], count, toCount);
        } else {
            if (&allGates[(gate->left_fanin - 1) / 2] == 0)
                ;
            ////// 以上防未定意
            else if (allGates[(gate->left_fanin - 1) / 2]._ref != gate->_globalRef)
                DepthS(&allGates[(gate->left_fanin - 1) / 2], count, toCount);
        }
    }

    if (toCount == 0) {
        cout << "[" << count << "] ";
        count++;
        gate->printGate();
    }
}
void CirMgr::printPIs() const {
    cout << "PIs of the circuit:";
    for (int i = 0; i < inputID.size(); i++) {
        cout << ' ' << inputID[i];
    }
    cout << endl;
}

void CirMgr::printPOs() const {
    cout << "POs of the circuit:";
    for (int i = 0; i < outputID.size(); i++) {
        cout << ' ' << outputID[i];
    }
    cout << endl;
}

void CirMgr::printFloatGates() const {
    /*for(int i = 0; i < GateNum; i++)
    {
        cout<< ' '<<cirMgr -> &allGates[i] << endl;
    }*/
    cout << "Gates with floating fanin(s) used:";
    for (int i = 0; i < GateNum; i++) {
        if (&allGates[i] != 0) {  //某些位置會突然變成0x4 而非 0
            if ((checkFLfanin(cirMgr->allGates[i].left_fanin) || checkFLfanin(cirMgr->allGates[i].right_fanin)) && allGates[i].gateType == 'A') {
                cout << ' ' << allGates[i].gateID;
            }
        }
    }
    cout << endl;

    cout << "Gates defined but not used  :";
    for (int i = 0; i < GateNum; i++) {
        if (&cirMgr->allGates[i] != 0) {  //某些位置會突然變成0x4 而非 0
            if (cirMgr->allGates[i].fanoutList.size() == 0 && cirMgr->allGates[i].gateType != 'O') {
                if (i != 0)
                    cout << ' ' << cirMgr->allGates[i].gateID;
            }
        }
    }
}

void CirMgr::writeAag(ostream &outfile) {
    int count = 0;
    if (Acount == 0) {
        for (int i = 0; i < outputID.size(); i++) {
            DepthS(&allGates[outputID[i]], count, true);
        }
        CirGate::_globalRef++;
        if (CirGate::_globalRef > 60000)
            CirGate::_globalRef = 0;
    }
    outfile << "aag " << GateNum - outputID.size() - 1 << ' ' << inputID.size() << ' ' << 0 << ' ' << outputID.size() << ' ' << Acount << '\n';
    for (int i = 0; i < inputID.size(); i++) {
        outfile << inputID[i] * 2 << '\n';
    }
    for (int i = 0; i < outputID.size(); i++) {
        outfile << getGate(outputID[i])->left_fanin << '\n';
    }
    for (int i = 0; i < outputID.size(); i++) {
        DepthS(&allGates[outputID[i]], count, outfile, 2);
    }
    CirGate::_globalRef++;
    if (CirGate::_globalRef > 60000)
        CirGate::_globalRef = 0;
    for (int i = 0; i < inputID.size(); i++) {
        if (getGate(inputID[i])->gateName != "") {
            outfile << 'i' << i << ' ' << getGate(inputID[i])->gateName << '\n';
        }
    }
    for (int i = 0; i < outputID.size(); i++) {
        if (getGate(outputID[i])->gateName != "") {
            outfile << 'o' << i << ' ' << getGate(outputID[i])->gateName << '\n';
        }
    }
}

bool CirMgr::checkFLfanin(int N) const {
    if (N == -1)
        return true;
    if (N % 2 == 0) {
        if (&cirMgr->allGates[N / 2] == 0)
            return true;
        else
            return false;
    } else {
        if (&cirMgr->allGates[(N - 1) / 2] == 0)
            return true;
        else
            return false;
    }
}
