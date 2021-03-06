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
CirMgr* cirMgr = 0;
/**************************************/
/*   Static varaibles and functions   */
/**************************************/
static unsigned lineNo = 0;  // in printint, lineNo needs to ++
static unsigned colNo = 0;   // in printing, colNo needs to ++
static string errMsg;
static int errInt;
static CirGate* errGate;

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
    GateNum = maxIDnum + POnum;
    AIGnum = Anum;
    allGates = new CirGate[GateNum + 1];  // 1-indexed 0 is for const 0
    for(size_t i = 0; i < GateNum; ++i)
        allGates[i].gateID = i;
    fanOuts = new vector<size_t>[maxIDnum + 1];
}

void CirMgr::reset() {
    delete[] allGates;
    delete[] fanOuts;
    inputID.clear();
    outputID.clear();
    dfsList.clear();
}

CirGate* CirMgr::getGate(size_t a) {
    if (a > GateNum || allGates[a].gateType == 'U')
        return 0;
    return allGates + a;
}

/**************************************************************/
/*   class CirMgr member functions for circuit construction   */
/**************************************************************/
bool CirMgr::readCircuit(const string& fileName) {
    // Variables
    //***********************
    vector<pair<int, int> > to_push_fanouts;
    size_t maxIDnum = 0, PInum = 0, Lnum = 0, POnum = 0, Anum = 0;
    int N, left, right, lineNum = 1; // line-index start from 1
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

    // Read input from file
    getline(filetoRead, row);  // getting the first row
    ss << row;
    ss >> row >> maxIDnum >> PInum >> Lnum >> POnum >> Anum;
    ++lineNum; // read a line, increase counter
    // allocate space for all gates and fanout vectors
    init(maxIDnum, POnum, Anum);  // 1-indexed

    // Setupt CONST 0 gate
    allGates[0].setGate(0, 0, 'C');

    // Set up Input Gates
    for (size_t i = 1; i <= PInum; ++i) {
        getline(filetoRead, row);  // don't need the information in row
        ss.str(std::string());
        ss.clear();
        ss << row;
        ss >> N;
        inputID.push_back(N / 2);
        allGates[N / 2].setGate(N / 2, lineNum, 'I');
        ++lineNum;
    }

    // Set up Output Gates
    for (size_t i = maxIDnum + 1; i <= maxIDnum + POnum; ++i) {
        getline(filetoRead, row);
        ss.str(std::string());
        ss.clear();
        ss << row;
        ss >> N;
        outputID.push_back(i);
        allGates[i].setGate(i, lineNum, 'O', N);
    	to_push_fanouts.push_back(make_pair(N/2, i*2+N%2));
        ++lineNum;
    }

    // Set up AIG
    for (size_t i = 0; i < Anum; ++i) {
        getline(filetoRead, row);
        ss.str(std::string());
        ss.clear();
        ss << row;
        ss >> N >> left >> right;
        assert(N % 2 == 0);
        allGates[N / 2].setGate(N / 2, lineNum, 'A', left, right);
        if (left >= 2)
            fanOuts[left / 2].push_back(N + left % 2);
        if (right >= 2)
            fanOuts[right / 2].push_back(N + right % 2);
        ++lineNum;
    }
    // Read Symbol
    string token[2];
    while (getline(filetoRead, row)) {
        ss.str(std::string());
        ss.clear();
        ss << row;
        ss >> token[0] >> token[1];
        if (token[0][0] == 'i') {
            allGates[inputID[stoi(token[0].substr(1))]].symbol = token[1];
        } else if (token[0][0] == 'o') {
            allGates[outputID[stoi(token[0].substr(1))]].symbol = token[1];
        } else if (token[0][0] == 'c')
            break;
        else {
            cout << "Symbol format error!" << endl;
            return 1;
        }
    }
    // push output to its input gate fanouts (late push to match ref-fraig)
    for (auto p : to_push_fanouts)
	    fanOuts[p.first].push_back(p.second);

    int visited[GateNum + 1] = {0};
    int count = 0;
    // construct dfsList 
    for (auto i : outputID)
        dfs(i, visited, count, 1, cout);
   
    return 0;
}

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

void CirMgr::dfs(size_t idx, int* visited, int& count, int init_run, ostream& out) {
    if(idx < 0 || idx > GateNum) return;
    int left, right;
    visited[idx] = 1;
    getFanins(idx, left, right);
   
    if (left != -1 && allGates[left / 2].gateType != 'U' && !visited[left / 2]) {
        dfs(left / 2, visited, count, init_run, out);
    }

    if (right != -1 && allGates[right / 2].gateType != 'U' && !visited[right / 2]) {
        dfs(right / 2, visited, count, init_run, out);
    }
    
    if (init_run && allGates[idx].gateType == 'A'){
        dfsList.push_back(idx);
        allGates[idx].in_dfs = 1;
    }

    if (!init_run) {
        out << "[" << count++ << "] ";
        allGates[idx].printGate();
    }
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
    for (size_t i = 1; i <= GateNum; i++) {
        if (allGates[i].gateType == 'A') {
            getFanins(i, left, right);
            if (allGates[left / 2].gateType == 'U' || allGates[right / 2].gateType == 'U')
                fss << ' ' << i;
        }
    }
    if (fss.str().length())
        cout << "Gates with floating fanin(s):" << fss.str() << '\n';
    for (size_t i = 1; i <= GateNum; i++) {
        if ((allGates[i].gateType != 'U') && (fanOuts[i].size() == 0) && (allGates[i].gateType != 'O'))
            uss << ' ' << i;
    }
    if (uss.str().length())
        cout << "Gates defined but not used  :" << uss.str() << endl;
}

void CirMgr::writeAag(stringstream& outfile) {

    outfile << "aag " << GateNum - outputID.size() << ' ' << inputID.size()
            << ' ' << 0 << ' ' << outputID.size() << ' ' << dfsList.size() << '\n';

    for (auto i : inputID)
        outfile << i * 2 << '\n';

    for (auto i : outputID)
        outfile << allGates[i].left_fanin << '\n';

    for (auto i : dfsList)
        outfile << i * 2 << ' ' << allGates[i].left_fanin << ' ' << allGates[i].right_fanin << endl;

    for (size_t i = 0; i < inputID.size(); i++) {
        if (allGates[inputID[i]].symbol != "")
            outfile << 'i' << i << ' ' << allGates[inputID[i]].symbol << '\n';
    }

    for (size_t i = 0; i < outputID.size(); i++) {
        if (allGates[outputID[i]].symbol != "")
            outfile << 'o' << i << ' ' << allGates[outputID[i]].symbol << '\n';
    }

    outfile << "c\n"
            << "AAG output by Yen-Li (Henry) Laih" << endl;
}
