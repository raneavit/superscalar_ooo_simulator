#include "includes.h"

// structures.h
#ifndef structures_H
#define structures_H

typedef 
struct {
   int opType;
   uint32_t id;
   int actualRd;
   int actualRs1;
   int actualRs2;
   int rd;
   int rs1;
   int rs2;

   bool isRs1Rdy;
   bool isRs2Rdy;

   uint32_t instAge;
   uint32_t executeTime;

   uint32_t enterFE;
   uint32_t cyclesInFE;
   uint32_t enterDE;
   uint32_t cyclesInDE;
   uint32_t enterRR;
   uint32_t cyclesInRR;
   uint32_t enterRN;
   uint32_t cyclesInRN;
   uint32_t enterDI;
   uint32_t cyclesInDI;
   uint32_t enterIS;
   uint32_t cyclesInIS;
   uint32_t enterEX;
   uint32_t cyclesInEX;
   uint32_t enterWB;
   uint32_t cyclesInWB;
   uint32_t enterRT;
   uint32_t cyclesInRT;
} instructionStruct;

typedef 
struct {
   bool validBit;
   uint32_t robTag;
} rmtBlock;

// typedef 
// struct {
//    uint32_t dstTag;
//    uint32_t rs1TagVal;
//    uint32_t rs2TagVal;
//    instructionStruct iqInstructionStruct;
// } issueQueueBlock;

typedef 
struct {
   int robTag;
   bool valueAvail;
   instructionStruct robInstructionStruct;   
} robBlock;




#endif