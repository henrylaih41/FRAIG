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
void CirMgr::init(int maxIDnum, int POnum, int Anum) {
    allGates = new CirGate[maxIDnum + POnum + 1];  // 1-indexed 0 is for const 0
    fanOuts = new vector<int>[maxIDnum + 1];     
    GateNum = maxIDnum + POnum;
    AIGnum = Anum;
    allGates[0].setGate(0, -1, 'C');
}

void CirMgr::reset() {
    delete[] allGates;
    delete[] fanOuts;
}

CirGate* CirMgr::getGate(int a){
    if(a == -1)
        return 0;
    return allGates + a;
}
/**************************************************************/
/*   class CirMgr member functions for circuit construction   */
/**************************************************************/
bool CirMgr::readCircuit(const string &fileName) {
    // Variables
    //***********************
    int maxIDnum = 0, PInum = 0, Lnum = 0, POnum = 0, Anum = 0;
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
    ss >> row >> maxIDnum >> PInum >> Lnum >> POnum >> Anum;
    // allocate space for all gates and fanout vectors
    init(maxIDnum, POnum, Anum);  // 1-indexed
    // Set up Input Gates
    for (int i = 1; i <= PInum; ++i) {
        getline(filetoRead, row);  // don't need the information in row
        inputID.push_back(i);
        allGates[i].setGate(i, lineNum, 'I');
        ++lineNum;
    }

    // Set up Output Gates
    for (int i = maxIDnum + 1; i <= maxIDnum + POnum; ++i) {
        getline(filetoRead, row);
        outputID.push_back(i);
        allGates[i].setGate(i, lineNum, 'O', stoi(row));
        fanOuts[stoi(row) / 2].push_back(i);
        ++lineNum;
    }

    // Set up AIG
    for (int i = 0; i < Anum; ++i) {
        getline(filetoRead, row);
        ss.str(std::string());
        ss.clear();
        ss << row;
        ss >> N >> left >> right;
        allGates[N / 2].setGate(N / 2, lineNum, 'A', left, right);
        if (left >= 2)
            fanOuts[left / 2].push_back(N / 2);
        if (right >= 2)
            fanOuts[right / 2].push_back(N / 2);
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
void CirMgr::getFanins(int idx, int& left, int& right) const {
    assert(idx != -1);
    left = allGates[idx].left_fanin;
    right = allGates[idx].right_fanin;
}

void CirMgr::printSummary() const {
    cout << "Circuit Statistics" << endl;
    cout << "==================" << endl;
    cout << "  PI" << setw(12) << inputID.size() << endl;
    cout << "  PO" << setw(12) << outputID.size() << endl;
    cout << "  AIG" << setw(11) << AIGnum << endl;
    cout << "------------------" << endl;
    cout << "  Total" << setw(9) << inputID.size() + outputID.size() + AIGnum << endl;
}

void CirMgr::dfs(int idx, int *visited, int& count, int write, ostream& out) {
    int left, right;
    visited[idx] = 1;
    getFanins(idx, left, right);
    if(allGates[idx].gateType == 'A')
        allGates[idx].inNetList = 1;

    if (left != -1 && allGates[left / 2].gateID != -1 && !visited[left / 2]){
        dfs(left / 2, visited, count, write, out);
    }

    if (right != -1 && allGates[right / 2].gateID != -1 && !visited[right / 2]) {
        dfs(right / 2, visited, count, write, out);
    }

    if(!write){
        out << "[" << count++ << "] ";
        allGates[idx].printGate();
    }
    else if(allGates[idx].gateType == 'A')
        out << idx * 2 << ' ' << left << ' ' << right << '\n';
}

void CirMgr::printPIs() const {
    cout << "PIs of the circuit:";
    for (size_t i = 0; i < inputID.size(); i++) {
        cout << ' ' << inputID[i];
    }
    cout << endl;
}

void CirMgr::printPOs() const {
    cout << "POs of the circuit:";
    for (size_t i = 0; i < outputID.size(); i++) {
        cout << ' ' << outputID[i];
    }
    cout << endl;
}

void CirMgr::printNetlist() {
    cout << endl;
    // GateNum + 1 because gates are 1-index 
    int visited[GateNum + 1] = {0};
    int count = 0;
    for (size_t i = 0; i < outputID.size(); i++) {
        dfs(outputID[i], visited, count);
    }
}

void CirMgr::printFloatGates() const {    
    int left, right;
    stringstream fss, uss;
    for (int i = 1; i < GateNum; i++) {
        if((allGates[i].gateID != -1) && (allGates[i].gateType == 'A')){
           getFanins(i, left, right);
           if(allGates[left / 2].gateID == -1 || allGates[right / 2].gateID == -1)
                fss << ' ' << i;
        }
    }
    if(fss.str().length())
        cout << "Gates with floating fanin(s):" << fss.str();
    for (int i = 1; i < GateNum; i++) {
        if ((allGates[i].gateID != -1) && (fanOuts[i].size() == 0) && (allGates[i].gateType != 'O')) 
            uss << ' ' << i; 
    }
    if(uss.str().length())
        cout << "\nGates defined but not used  :" << uss.str() << endl;
}

void CirMgr::writeAag(ostream &outfile) {
    int visited[GateNum + 1] = {0};
    int count = 0;
    outfile << "aag " << GateNum - outputID.size() << ' ' << inputID.size() \
            << ' ' << 0 << ' ' << outputID.size() << ' ' << AIGnum << '\n';

    for (auto i : inputID)
        outfile << i * 2 << '\n';
   
    for (auto i : outputID)
        outfile << allGates[i].left_fanin << '\n';

    for (auto i : outputID)
        dfs(i, visited, count, 1, outfile);

    /*
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
    */
    outfile << "c\n" << "AAG output by Yen-Li (Henry) Laih" << endl;
}

