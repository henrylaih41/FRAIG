### CirParser
This parser is provided as a object file, so there is no source code
The main workflow logic is command in terminal ==> parser by paser ==>
call the corresponding cmd.exec() defined in CirCmd.cpp 
[NOTE] Every valid cmd is registered as class object with three methods
exec, help, and print

### CirCmd.cpp
This file contains the code for each command. 
The list of commands (operations)
-------------------------------------
CIRRead
CIRPrint
CIRGate
CIRSWeep
CIROPTimize
CIRSTRash
CIRSIMULATE
CIRFraig
CIRWrite
-------------------------------------


### CirGate.cpp
Class CirGate defines a single gate object 
A gate object contains all the information of a single AND gate
The printing and logic of the gate is also in this file

### CirMgr.cpp
Class CirMgr is where the whole circuit is stored, all the logic
related to the whole circuit is in this class. The file includes all
the circuit print logics, and also the read/write logics. 

### CirOpt.cpp
The optimization method for CirMgr is in the file, including
sweep() and optimize()

### CirSim.cpp
The simulation logic for CirMgr is in this file, later used in the FRAIG function of CirMgr

### CirFRIAG.cpp
The main logic of the fraig and strash operations is in this file.

