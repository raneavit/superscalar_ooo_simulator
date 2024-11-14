#include "includes.h"
#include "structures.h"

#ifndef pipeline_H
#define pipeline_H

class Pipeline
{
  string id;
  uint32_t width;
  uint32_t rmtSize;
  uint32_t robSize;
  uint32_t robTail = 0;
  uint32_t iqSize;

  vector<rmtBlock> myRMT;
  
  vector<robBlock> retiredInstructions;
  FILE *FP;
  
  bool traceDepleted;
  

public:
  // Registers to be shifted to private
  vector<instructionStruct> myIssueQueue;
  vector<instructionStruct> DE;
  vector<instructionStruct> RN;
  vector<instructionStruct> RR;
  vector<instructionStruct> DI;
  vector<instructionStruct> execute_list;
  vector<instructionStruct> WB;
  vector<robBlock> myROB;

  uint32_t currentCycle;
  uint32_t sequenceNumber;
  
  Pipeline(uint32_t ROB_SIZE, uint32_t IQ_SIZE, uint32_t WIDTH, char* traceFile);
  void Fetch(int cycle);
  void Decode(int cycle);
  void Rename(int cycle);
  void RegRead(int cycle);
  void Dispatch(int cycle);
  void Issue(int cycle);
  void Execute(int cycle);
  void Writeback(int cycle);
  void Retire(int cycle);
  void PrintInstData(instructionStruct);
  bool Advance_Cycle();
};

void PrintRegister(vector<instructionStruct>& reg, string name);
void PrintRob(vector<robBlock>& rob);


#endif