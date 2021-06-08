/****************************************************************************
  FileName     [ cirMgr.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir manager functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <iostream>
#include <iomanip>
#include <cstdio>
#include <ctype.h>
#include <cassert>
#include <cstring>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"
#include <sstream>

using namespace std;

// TODO: Implement memeber functions for class CirMgr

/*******************************/
/*   Global variable and enum  */
/*******************************/
CirMgr* cirMgr = 0;

enum CirParseError {
   EXTRA_SPACE,
   MISSING_SPACE,
   ILLEGAL_WSPACE,
   ILLEGAL_NUM,
   ILLEGAL_IDENTIFIER,
   ILLEGAL_SYMBOL_TYPE,
   ILLEGAL_SYMBOL_NAME,
   MISSING_NUM,
   MISSING_IDENTIFIER,
   MISSING_NEWLINE,
   MISSING_DEF,
   CANNOT_INVERTED,
   MAX_LIT_ID,
   REDEF_GATE,
   REDEF_SYMBOLIC_NAME,
   REDEF_CONST,
   NUM_TOO_SMALL,
   NUM_TOO_BIG,

   DUMMY_END
};

/**************************************/
/*   Static varaibles and functions   */
/**************************************/
static unsigned lineNo = 0;  // in printint, lineNo needs to ++
static unsigned colNo  = 0;  // in printing, colNo needs to ++
static char buf[1024];
static string errMsg;
static int errInt;
static CirGate *errGate;

static bool
parseError(CirParseError err)
{
   switch (err) {
      case EXTRA_SPACE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Extra space character is detected!!" << endl;
         break;
      case MISSING_SPACE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Missing space character!!" << endl;
         break;
      case ILLEGAL_WSPACE: // for non-space white space character
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Illegal white space char(" << errInt
              << ") is detected!!" << endl;
         break;
      case ILLEGAL_NUM:
         cerr << "[ERROR] Line " << lineNo+1 << ": Illegal "
              << errMsg << "!!" << endl;
         break;
      case ILLEGAL_IDENTIFIER:
         cerr << "[ERROR] Line " << lineNo+1 << ": Illegal identifier \""
              << errMsg << "\"!!" << endl;
         break;
      case ILLEGAL_SYMBOL_TYPE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Illegal symbol type (" << errMsg << ")!!" << endl;
         break;
      case ILLEGAL_SYMBOL_NAME:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Symbolic name contains un-printable char(" << errInt
              << ")!!" << endl;
         break;
      case MISSING_NUM:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Missing " << errMsg << "!!" << endl;
         break;
      case MISSING_IDENTIFIER:
         cerr << "[ERROR] Line " << lineNo+1 << ": Missing \""
              << errMsg << "\"!!" << endl;
         break;
      case MISSING_NEWLINE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": A new line is expected here!!" << endl;
         break;
      case MISSING_DEF:
         cerr << "[ERROR] Line " << lineNo+1 << ": Missing " << errMsg
              << " definition!!" << endl;
         break;
      case CANNOT_INVERTED:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": " << errMsg << " " << errInt << "(" << errInt/2
              << ") cannot be inverted!!" << endl;
         break;
      case MAX_LIT_ID:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Literal \"" << errInt << "\" exceeds maximum valid ID!!"
              << endl;
         break;
      case REDEF_GATE:
         cerr << "[ERROR] Line " << lineNo+1 << ": Literal \"" << errInt
              << "\" is redefined, previously defined as "
              << errGate->getTypeStr() << " in line " << errGate->getLineNo()
              << "!!" << endl;
         break;
      case REDEF_SYMBOLIC_NAME:
         cerr << "[ERROR] Line " << lineNo+1 << ": Symbolic name for \""
              << errMsg << errInt << "\" is redefined!!" << endl;
         break;
      case REDEF_CONST:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Cannot redefine constant (" << errInt << ")!!" << endl;
         break;
      case NUM_TOO_SMALL:
         cerr << "[ERROR] Line " << lineNo+1 << ": " << errMsg
              << " is too small (" << errInt << ")!!" << endl;
         break;
      case NUM_TOO_BIG:
         cerr << "[ERROR] Line " << lineNo+1 << ": " << errMsg
              << " is too big (" << errInt << ")!!" << endl;
         break;
      default: break;
   }
   return false;
}

/**************************************************************/
/*   class CirMgr member functions for circuit construction   */
/**************************************************************/
bool
CirMgr::readCircuit(const string& fileName)
{
    
    enum parseType{
        head,
        Input,
        Output,
        AIGType,
        ISymbol,
        OSymbol
    };
    //Variables
    //***********************
    unsigned int PInum = 0;
    unsigned int POnum = 0;
    unsigned int AIGnum = 0;
    unsigned int count = 0;
    unsigned int POcount = 1;
    unsigned int lineNum = 0;
    unsigned int ID = 0;
    unsigned int maxIDnum = 0;
    ifstream filetoRead;
    string row;
    parseType type = head;
    CirGate* AIG = 0;
    //***********************
    
    
    //Check if file is opened
    //***********************
    filetoRead.open(fileName);
    if(!filetoRead){
        cout<<"Cannot open design "<<'\"'<<fileName<<'\"'<<"!!"<<endl;
        return false;
    }
    //***********************
    while(getline(filetoRead,row)){
        string unit;
        istringstream RowtoRead(row);
        unsigned int lineCount = 0;
        lineNum++;
        while(getline(RowtoRead,unit,' ')){
            int N;
            if(unit == "aag") continue;
            if( lineCount == 0 && unit == "c" ) return true;
            if(unit[0] == 'o'){
                type = OSymbol;
                count == 0;
            }
            if(type != ISymbol && type != OSymbol)
            myStr2Int(unit,N);
            else{
                if(lineCount == 0){
                myStr2Int(unit.substr(1),N);
                    ID = N;
                }
                // cout<<"for debugging"<<ID<<' '<<unit.substr(1)<<endl;
            }
            
            if(type == head){
                if(count == 0) maxIDnum = N;
                else if(count == 1) PInum = N;
                else if(count == 2);
                else if(count == 3) POnum = N;
                else if(count == 4) AIGnum = N;
                count++;
            }
            
            else if(type == Input){
                CirGate* input = new CirGate(N/2,lineNum,'I'); //initial Input gate
                cirMgr -> allGates[N/2] = input; //store input to list according to ID;
                cirMgr -> inputID.push_back(N/2);
                count++;
            }
            
            else if(type == Output){
                CirGate* output = new CirGate(maxIDnum + POcount,lineNum,'O'); //initial Output gate
                cirMgr -> allGates[maxIDnum + POcount] = output;
                output -> left_fanin = N;
                cirMgr -> outputID.push_back(maxIDnum + POcount);
                //store output to list according to ID;
                POcount++;
                count++;
            }
            else if(type == AIGType){
                if(lineCount == 0){
                    AIG = new CirGate(N/2,lineNum,'A'); //initial AIG gate
                    cirMgr -> allGates[N/2] = AIG; //store AIG to list according to ID;
                    lineCount++;
                }
                else if(lineCount == 1){
                    AIG -> right_fanin = N; //record AIG fanin ID(literal);
                    if(N % 2 == 0 ){
                        if(cirMgr -> allGates[N/2] != 0)
                        cirMgr -> allGates[N/2] -> fanoutList.push_back((AIG -> gateID)*2);
                        else cirMgr -> AIGtoLinkID.push_back(AIG -> gateID);
                        
                        //record fanin gate's fanout (ID);
                        //cout<<allGates[4]<<endl;
                    }
                    else{
                        if(cirMgr -> allGates[(N - 1)/2] != 0)
                        cirMgr -> allGates[(N - 1)/2] -> fanoutList.push_back((AIG -> gateID)*2 + 1);
                        else cirMgr -> AIGtoLinkID.push_back(AIG -> gateID);
                        //record fanin gate's fanout (ID);
                    }
                    lineCount++;
                }
                else if(lineCount == 2){
                    AIG -> left_fanin = N;
                    if(N % 2 == 0 ){
                        if(cirMgr -> allGates[N/2] != 0)
                        cirMgr -> allGates[N/2] -> fanoutList.push_back((AIG -> gateID)*2);
                        else cirMgr -> AIGtoLinkID.push_back(AIG -> gateID);
                    }
                    else{
                        if(cirMgr -> allGates[(N - 1)/2] != 0)
                        cirMgr -> allGates[(N - 1) /2 ] -> fanoutList.push_back((AIG -> gateID)*2 + 1);
                        else cirMgr -> AIGtoLinkID.push_back(AIG -> gateID);
                    }
                    count++;
                }
                else break;
            }
            else if(type == ISymbol){
                if(lineCount == 1){
                   //cout<<ID<<' '<<unit<<endl;
                    //cout<<cirMgr -> inputID.size()<<endl;
                    //cout<<cirMgr -> inputID[ID]<<endl;
                    //cout<<cirMgr -> allGates[cirMgr -> inputID[ID]]<<endl;
                    cirMgr -> allGates[cirMgr -> inputID[ID]] -> gateName = unit;
                    count++;
                }
                else if(lineCount > 1) cout<<"BUG at ISymbol";
                lineCount++;
            }
            else if(type == OSymbol){
                if(lineCount == 1){
                    //cout<<ID<<' '<<"unit"<<endl;
                    cirMgr -> allGates[cirMgr -> outputID[ID]] -> gateName = unit;
                    count++;
                }
                else if(lineCount > 1) cout<<"BUG at OSymbol";
                lineCount++;
            }
        }
        if(type == head){
            cirMgr = new CirMgr(maxIDnum + POnum + 1,AIGnum);
            CirGate* input = new CirGate(0,0,'C'); //initial Input gate
            cirMgr -> allGates[0] = input; //store input to list according to ID;
            type = Input; count = 0;
        }
        
        if(type == Input && count == PInum){
            type = Output; count = 0;
        }
        
        if(type == Output && count == POnum){
            type = AIGType; count = 0;
        }
        
        if(type == AIGType && count == AIGnum){
         
            for(int i = 0; i < cirMgr -> outputID.size(); i++){
                int N;
                if(cirMgr -> allGates[cirMgr -> outputID[i]] != 0)
                N = cirMgr -> allGates[cirMgr -> outputID[i]] -> left_fanin;
                else continue;
                if(N % 2 == 0 ){
                    if(cirMgr -> allGates[N/2] != 0)
                     cirMgr -> allGates[N/2] -> fanoutList.push_back(cirMgr -> outputID[i]*2);
                }
                else{
                    if(cirMgr -> allGates[(N - 1)/2] != 0)
                     cirMgr -> allGates[(N - 1)/2] -> fanoutList.push_back(cirMgr -> outputID[i]*2 + 1);
                }
            }
            // cout<< cirMgr -> AIGtoLinkID.size() <<"buggggggg " <<endl;
            for(int i = 0; i < cirMgr -> AIGtoLinkID.size(); i++){
                //cout<<cirMgr -> AIGtoLinkID[i]<<endl;
                //cout<<cirMgr -> allGates[cirMgr -> AIGtoLinkID[i]]<<endl;
                if(cirMgr -> allGates[cirMgr -> allGates[cirMgr -> AIGtoLinkID[i]] -> left_fanin/2] != 0){
                   cirMgr -> allGates[cirMgr -> allGates[cirMgr -> AIGtoLinkID[i]] -> left_fanin/2] -> fanoutList.push_back(cirMgr -> AIGtoLinkID[i]);
                }
                else continue;
                
                if(cirMgr -> allGates[cirMgr -> allGates[cirMgr -> AIGtoLinkID[i]] -> right_fanin/2] != 0){
                   cirMgr -> allGates[cirMgr -> allGates[cirMgr -> AIGtoLinkID[i]] -> right_fanin/2] -> fanoutList.push_back(cirMgr -> AIGtoLinkID[i]);
                }
                else continue;
            }
            type = ISymbol; count = 0;
        }
        
        if(type == ISymbol && count == PInum){
            type = OSymbol; count = 0;
        }
        /*for(int i = 0; i < maxIDnum; i++)
        {
            cout<< ' '<<cirMgr -> allGates[i] << endl;
        }*/
    }
    
    filetoRead.close();
    return true;
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
void
CirMgr::printSummary() const
{
    cout<<"Circuit Statistics"<<endl;
    cout<<"=================="<<endl;
    cout<<"  PI"<<setw(12)<<inputID.size()<<endl;
    cout<<"  PO"<<setw(12)<<outputID.size()<<endl;
    cout<<"  AIG"<<setw(11)<<AIGnum<<endl;
    cout<<"------------------"<<endl;
    cout<<"  Total"<<setw(9)<<inputID.size()+outputID.size()+AIGnum<<endl;
}

void
CirMgr::printNetlist()
{   cout<<endl;
    int count = 0;
    for(int i = 0; i<outputID.size(); i++){
        DepthS(allGates[outputID[i]],count);
    }
    CirGate::_globalRef++;
    if(CirGate::_globalRef > 60000) CirGate::_globalRef = 0;
}

void CirMgr::DepthS(CirGate* gate,int& count ,ostream& outfile,int toCount){
    gate -> _ref = gate -> _globalRef;
    if(toCount == 1 && gate -> gateType == 'A') Acount++;
    
    if(gate -> right_fanin != -1){
        if(gate -> right_fanin % 2 == 0){
            if(allGates[gate -> right_fanin/2] == 0); ////// 防未定意
            else if(allGates[gate -> right_fanin/2] -> _ref != gate -> _globalRef)
                DepthS(allGates[gate -> right_fanin/2],count,outfile,toCount);
        }
        else{
            if(allGates[(gate -> right_fanin - 1)/2] == 0); //////防未定意
            else if(allGates[(gate -> right_fanin - 1)/2] -> _ref != gate -> _globalRef)
                DepthS(allGates[(gate -> right_fanin - 1)/2],count,outfile,toCount);
        }
        
    }
    if(gate -> left_fanin != -1){
        if(gate -> left_fanin % 2 == 0){
            if(allGates[gate -> left_fanin/2] == 0);
            ////// 以上防未定意
            else if(allGates[gate -> left_fanin/2] -> _ref != gate -> _globalRef)
                DepthS(allGates[gate -> left_fanin/2],count,outfile,toCount);
        }
        else{
            if(allGates[(gate -> left_fanin - 1)/2] == 0);
            ////// 以上防未定意
            else if(allGates[(gate -> left_fanin - 1)/2] -> _ref != gate -> _globalRef)
                DepthS(allGates[(gate -> left_fanin - 1)/2],count,outfile,toCount);
        }
    }
    
    if(gate -> gateType == 'A' && toCount == 2){
        outfile<< gate -> gateID*2 << ' ' << gate -> right_fanin << ' ' << gate -> left_fanin << '\n';
    }
    
    if(toCount == 0){
        cout<<"["<<count<<"] ";
        count++;
        gate->printGate();
    }
}

void CirMgr::DepthS(CirGate* gate,int& count,int toCount){
    
    gate -> _ref = gate -> _globalRef;
    if(toCount == 1 && gate -> gateType == 'A') Acount++;
    
    if(gate -> right_fanin != -1){
        if(gate -> right_fanin % 2 == 0){
            if(allGates[gate -> right_fanin/2] == 0); ////// 防未定意
            else if(allGates[gate -> right_fanin/2] -> _ref != gate -> _globalRef)
                DepthS(allGates[gate -> right_fanin/2],count,toCount);
        }
        else{
            if(allGates[(gate -> right_fanin - 1)/2] == 0); //////防未定意
            else if(allGates[(gate -> right_fanin - 1)/2] -> _ref != gate -> _globalRef)
                DepthS(allGates[(gate -> right_fanin - 1)/2],count,toCount);
        }

    }
    if(gate -> left_fanin != -1){
        if(gate -> left_fanin % 2 == 0){
            if(allGates[gate -> left_fanin/2] == 0);
            ////// 以上防未定意
            else if(allGates[gate -> left_fanin/2] -> _ref != gate -> _globalRef)
                DepthS(allGates[gate -> left_fanin/2],count,toCount);
        }
        else{
            if(allGates[(gate -> left_fanin - 1)/2] == 0);
            ////// 以上防未定意
            else if(allGates[(gate -> left_fanin - 1)/2] -> _ref != gate -> _globalRef)
                DepthS(allGates[(gate -> left_fanin - 1)/2],count,toCount);
        }
    }

   
    if(toCount == 0){
        cout<<"["<<count<<"] ";
        count++;
        gate->printGate();
    }
   
}
void
CirMgr::printPIs() const
{
   cout << "PIs of the circuit:";
    for(int i = 0; i < inputID.size(); i++){
        cout<<' '<<inputID[i];
    }
   cout << endl;
}

void
CirMgr::printPOs() const
{
   cout << "POs of the circuit:";
    for(int i = 0; i < outputID.size(); i++){
        cout<<' '<<outputID[i];
    }
   cout << endl;
}

void
CirMgr::printFloatGates() const
{
    /*for(int i = 0; i < GateNum; i++)
    {
        cout<< ' '<<cirMgr -> allGates[i] << endl;
    }*/
    cout<<"Gates with floating fanin(s) used:";
    for(int i = 0; i< GateNum ; i++){
        if(allGates[i] != 0){ //某些位置會突然變成0x4 而非 0
            if((checkFLfanin(cirMgr -> allGates[i] -> left_fanin)
               || checkFLfanin(cirMgr -> allGates[i] -> right_fanin)) && allGates[i] -> gateType == 'A'){
                cout<<' '<<allGates[i] -> gateID;
            }
        }
    }
    cout<<endl;
    
    cout<<"Gates defined but not used  :";
    for(int i = 0; i< GateNum ; i++){
        if(cirMgr -> allGates[i] != 0){//某些位置會突然變成0x4 而非 0
            if(cirMgr -> allGates[i] -> fanoutList.size() == 0 && cirMgr -> allGates[i] -> gateType != 'O'){
                if(i != 0) cout<<' '<<cirMgr -> allGates[i] -> gateID;
            }
        }
    }
}

void
CirMgr::writeAag(ostream& outfile)
{
    int count = 0;
    if(Acount == 0){
        for(int i = 0; i<outputID.size(); i++){
            DepthS(allGates[outputID[i]],count,true);
        }
        CirGate::_globalRef++;
        if(CirGate::_globalRef > 60000) CirGate::_globalRef = 0;
    }
    outfile<<"aag "<<GateNum - outputID.size() - 1<<' '<<inputID.size()<<' '<<0<<' '<<outputID.size()<<' '<<Acount<<'\n';
    for(int i = 0; i < inputID.size(); i++){
        outfile<<inputID[i]*2<<'\n';
    }
    for(int i = 0; i < outputID.size(); i++){
        outfile<<getGate(outputID[i]) -> left_fanin <<'\n';
    }
    for(int i = 0; i<outputID.size(); i++){
        DepthS(allGates[outputID[i]],count,outfile,2);
    }
    CirGate::_globalRef++;
    if(CirGate::_globalRef > 60000) CirGate::_globalRef = 0;
    for(int i = 0; i < inputID.size(); i++){
        if(getGate(inputID[i]) -> gateName != ""){
            outfile<< 'i' << i << ' ' <<getGate(inputID[i]) -> gateName <<'\n';
        }
    }
    for(int i = 0; i < outputID.size(); i++){
        if(getGate(outputID[i]) -> gateName != ""){
            outfile<< 'o' << i << ' ' << getGate(outputID[i]) -> gateName <<'\n';
        }
    }
    
    
}

bool
CirMgr::checkFLfanin(int N) const {
    if(N == -1) return true;
    if(N % 2 == 0){
        if(cirMgr -> allGates[N/2] == 0) return true;
        else return false;
    }
    else{
        if(cirMgr -> allGates[(N - 1)/2] == 0) return true;
        else return false;
    }
}
