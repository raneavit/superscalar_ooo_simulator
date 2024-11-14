#include "includes.h"
#include "structures.h"
#include "pipeline.h"
#include <algorithm>

void PrintRegister(vector<instructionStruct>& reg, string name) {
    printf("========== Register %s ===========\n", name.c_str());
    printf("Size: %d", reg.size());
    for_each(reg.begin(), reg.end(), [=](instructionStruct ins){
        printf("[Instruction: id: %lu, rs1: %d, rs2: %d, dst: %d, rs1(ren): %d, rs2(ren): %d, dst(ren): %d rs1Rdy: %d, rs2Rdy: %d]\n", ins.id, ins.actualRs1, ins.actualRs2, ins.actualRd, ins.rs1, ins.rs2, ins.rd, ins.isRs1Rdy, ins.isRs2Rdy);
    });
    printf("==================================\n", name.c_str());
}

void PrintRob(vector<robBlock>& rob) {
    printf("========== ROB ===========\n");
    for_each(rob.begin(), rob.end(), [=](robBlock robentry){
        printf("[ROB Tag: id: %lu, rd: %d, valueAvail: %d, inst id: %d,]\n", robentry.robTag, robentry.robInstructionStruct.actualRd, robentry.valueAvail, robentry.robInstructionStruct.id );
    });
    printf("==================================\n");
}

Pipeline::Pipeline(uint32_t ROB_SIZE, uint32_t IQ_SIZE, uint32_t WIDTH, char* traceFile){
    rmtSize = 67;
    robSize = ROB_SIZE;
    iqSize = IQ_SIZE;
    width = WIDTH;
    // cout << "CONFIG : " << robSize << " " << iqSize << " " << width << " " << endl;

    myRMT.resize(rmtSize);
    
    currentCycle = 0;
    sequenceNumber = 0;
    traceDepleted = false;

    FP = fopen(traceFile, "r");

    if(FP == NULL)
    {
        // Throw error and exit if fopen() failed
        printf("Error: Unable to open file %s\n", traceFile);
        exit(EXIT_FAILURE);
    }
    

}

void Pipeline::Fetch(int cycle){

    int op_type, dest, src1, src2;  // Variables are read from trace file
    uint32_t pc; // Variable holds the pc read from input file
    
    int i;
    int a;
 
    if(DE.empty()){
        i = 0;
        while(traceDepleted == false && i<width){
        a = fscanf(FP, "%lx %d %d %d %d", &pc, &op_type, &dest, &src1, &src2);
        // printf("Fetched Instruction - %lx %d %d %d %d\n", pc, op_type, dest, src1, src2);

        if(a == -1){
            traceDepleted = true;
            continue;
        }
        instructionStruct temp;
        memset(&temp, 0, sizeof(temp));
        temp.opType = op_type;
        temp.id = sequenceNumber;
        temp.actualRd = dest;
        temp.actualRs1 = src1;
        temp.actualRs2 = src2;
        temp.rd = dest;
        temp.rs1 = src1;
        temp.rs2 = src2;

        temp.instAge = 0;

        switch(op_type){
            case 0:
            temp.executeTime = 1;
            break;

            case 1:
            temp.executeTime = 2;
            break;

            case 2:
            temp.executeTime = 5;
            break;
        }
        temp.enterFE = cycle;
        temp.cyclesInFE = 1;
        temp.enterDE = cycle+1;
        DE.push_back(temp);
        i++;
        sequenceNumber++;
        }
    }
}

void Pipeline::Decode(int cycle){
    if(!DE.empty()){
        // cout << "in decode" << endl;
        if(!RN.empty()){
            //do nothing
            for (int i=0; i<DE.size(); i++){
                DE[i].cyclesInDE++;
                DE[i].instAge++;
            }
        } 
        else{
            for (int i=0; i<DE.size(); i++){
                DE[i].cyclesInDE++;
                DE[i].instAge++;

                DE[i].enterRN = cycle + 1;
                // printf("Pushing to RENAME -> %d\n", cycle);
                RN.push_back(DE[i]);
            }
            DE.clear(); 
        }
    }
}

void Pipeline::Rename(int cycle){
    // cout << "Retired Instructions size" << retiredInstructions.size() << endl;
    if(!retiredInstructions.empty() && !myROB.empty()){
        while(!retiredInstructions.empty()){
            PrintInstData(myROB[0].robInstructionStruct);
            myROB.erase(myROB.begin());
            retiredInstructions.erase(retiredInstructions.begin());
        }
    }

    if(!RN.empty()){
        if(!RR.empty() || (myROB.size() > robSize - RN.size())){
            //do nothing
            for (int i=0; i<RN.size(); i++){
                RN[i].cyclesInRN++;
                RN[i].instAge++;
            }
        }
        else{
            //process
            for(int i=0; i<RN.size(); i++){

                //check if source registers have to be renamed

                if(RN[i].rs1 != -1){
                    if(myRMT[RN[i].rs1].validBit == true){
                        RN[i].isRs1Rdy = false;
                        RN[i].rs1 = myRMT[RN[i].rs1].robTag;
                    }
                    else{RN[i].isRs1Rdy = true;}
                }
                else{RN[i].isRs1Rdy = true;}

                if(RN[i].rs2 != -1){
                    if(myRMT[RN[i].rs2].validBit == true){
                        RN[i].isRs2Rdy = false;
                        RN[i].rs2 = myRMT[RN[i].rs2].robTag;
                    }
                    else{RN[i].isRs2Rdy = true;}
                }
                else{RN[i].isRs2Rdy = true;}
                
                //make rob entry
                robBlock temp;
                temp.valueAvail = false;
                temp.robTag = robTail;
                temp.robInstructionStruct = RN[i];
                robTail++;
                // printf("New ROB Entry Made -> %d\n", cycle);
                myROB.push_back(temp);

                //update RMT
                if(RN[i].actualRd != -1){
                myRMT[RN[i].actualRd].validBit = true;
                myRMT[RN[i].actualRd].robTag = temp.robTag;
                } 

                //Rename Rd
                RN[i].rd = temp.robTag;  
            }

            //Advance from RN to RR
            for (int i=0; i<RN.size(); i++){
                //increment
                RN[i].cyclesInRN++;
                RN[i].instAge++;

                RN[i].enterRR = cycle + 1;
                // printf("Pushing to RR -> %d\n", cycle);
                RR.push_back(RN[i]);
            }

            RN.clear();
        }
    }
}

void Pipeline::RegRead(int cycle){
    if(!RR.empty()){
        // cout << "in Reg Read" << endl;
        if(!DI.empty()){
            //do nothing
            for (int i=0; i<RR.size(); i++){
                RR[i].cyclesInRR++;
                RR[i].instAge++;
            }

             // Evaluate readiness of operands
            for (int i=0; i<RR.size(); i++){

                // if(RR[i].isRs1Rdy == false){
                    for(int j=0; j<myROB.size(); j++){
                        if(RR[i].rs1 == myROB[j].robTag){
                            if(myROB[j].valueAvail == true) RR[i].isRs1Rdy = true;
                        }
                    }
                // }

                // if(RR[i].isRs2Rdy == false){
                    for(int j=0; j<myROB.size(); j++){
                        if(RR[i].rs2 == myROB[j].robTag){
                            if(myROB[j].valueAvail == true) RR[i].isRs2Rdy = true;
                        }
                    }
                // }
            }
        }
        else{
            // Evaluate readiness of operands
            for (int i=0; i<RR.size(); i++){

                // if(RR[i].isRs1Rdy == false){
                    for(int j=0; j<myROB.size(); j++){
                        if(RR[i].rs1 == myROB[j].robTag){
                            if(myROB[j].valueAvail == true) RR[i].isRs1Rdy = true;
                        }
                    }
                // }

                // if(RR[i].isRs2Rdy == false){
                    for(int j=0; j<myROB.size(); j++){
                        if(RR[i].rs2 == myROB[j].robTag){
                            if(myROB[j].valueAvail == true) RR[i].isRs2Rdy = true;
                        }
                    }
                // }
            }

            //Advance from RR to DI
            for (int i=0; i<RR.size(); i++){
                RR[i].cyclesInRR++;
                RR[i].instAge++;
                RR[i].enterDI = cycle + 1;
                // printf("Pushing to DI -> %d\n", cycle);
                DI.push_back(RR[i]);
            }

            RR.clear();
        }
    }
}

void Pipeline::Dispatch(int cycle){
    if(!DI.empty()){
        // cout << "in dispatch" << endl;
        if(myIssueQueue.size() > iqSize - DI.size()){
            //do nothing
            for (int i=0; i<DI.size(); i++){
                DI[i].cyclesInDI++;
                DI[i].instAge++;
            }
        }
        else{
            //Put DI into IQ
            for (int i=0; i<DI.size(); i++){
                //increment cycles
                DI[i].cyclesInDI++;
                DI[i].instAge++;
                DI[i].enterIS = cycle + 1;

                myIssueQueue.push_back(DI[i]);
            }

            DI.clear();
        }
    }
}

void Pipeline::Issue(int cycle){
    vector<instructionStruct> readyInstructions;
    vector<uint32_t> removedIds;
    int i, j;


    if(!myIssueQueue.empty()){
        // cout << "in issue" << endl;
        for(i=0;i<myIssueQueue.size();i++){
        myIssueQueue[i].cyclesInIS++;
        myIssueQueue[i].instAge++;
        }

        for(i=0; i<myIssueQueue.size(); i++){
            if(myIssueQueue[i].isRs1Rdy == true && myIssueQueue[i].isRs2Rdy == true){
                readyInstructions.push_back(myIssueQueue[i]);
            }   
        }

        // std::sort(readyInstructions.begin(), readyInstructions.end(),
        //       []( const instructionStruct &left, const instructionStruct &right )
        //          { return ( left.instAge > right.instAge); } );

        int N = readyInstructions.size();
        int m, n;
        bool swapped;
        for (m = 0; m < N - 1; m++) {
            swapped = false;
            for (n = 0; n < N - i - 1; n++) {
                if (readyInstructions[n].instAge < readyInstructions[n + 1].instAge) {
                    swap(readyInstructions[n], readyInstructions[n + 1]);
                    swapped = true;
                }
                else if(readyInstructions[n].instAge == readyInstructions[n + 1].instAge){
                    if(readyInstructions[n].id > readyInstructions[n + 1].id){
                        swap(readyInstructions[n], readyInstructions[n + 1]);
                    swapped = true;
                    }
                }
            }
    
            // If no two elements were swapped
            // by inner loop, then break
            if (swapped == false)
                break;
        }

        j=0;
        while(readyInstructions.size() > 0 && (execute_list.size() < (width*5)) && (j<width)){
            removedIds.push_back(readyInstructions[0].id);
            readyInstructions[0].enterEX = cycle+1;
            execute_list.push_back(readyInstructions[0]);
            readyInstructions.erase(readyInstructions.begin());
            j++;
        }

        j=0;
        while(j<myIssueQueue.size()){
            if(std::find(removedIds.begin(), removedIds.end(), myIssueQueue[j].id) != removedIds.end()) {
                /* removedIds contains current inst id */
                myIssueQueue.erase(myIssueQueue.begin() + j);
            } else {
                /* removedIds does not contain current inst id */
                j++;
            }
        }

        readyInstructions.clear();
        removedIds.clear();
    }
}

void Pipeline::Execute(int cycle){
    int j,k;
    if(!execute_list.empty()){
        // cout << "in execute" << endl;
        //increment cycles
        for(int i=0; i<execute_list.size(); i++){
            execute_list[i].cyclesInEX++;
            execute_list[i].instAge++;
        }

        j = 0;
        while(j<execute_list.size()){

            if(execute_list[j].cyclesInEX == execute_list[j].executeTime){
                //wake up in IQ
                for(k=0; k<myIssueQueue.size(); k++){
                
                    // if(myIssueQueue[k].isRs1Rdy == false){
                        if(execute_list[j].rd == myIssueQueue[k].rs1){
                            myIssueQueue[k].isRs1Rdy = true;
                        }
                    // }

                    // if(myIssueQueue[k].isRs2Rdy == false){
                        if(execute_list[j].rd == myIssueQueue[k].rs2){
                            myIssueQueue[k].isRs2Rdy = true;
                        }
                    // }
                }

                //wake up in DI
                for(k=0; k<DI.size(); k++){
                    // if(DI[k].isRs1Rdy == false){
                        if(execute_list[j].rd == DI[k].rs1){
                            DI[k].isRs1Rdy = true;
                        }
                    // }

                    // if(DI[k].isRs2Rdy == false){
                        if(execute_list[j].rd == DI[k].rs2){   
                            DI[k].isRs2Rdy = true;
                        }
                    // }
                }

                //wake up in RR
                for(k=0; k<RR.size(); k++){
                    // if(RR[k].isRs1Rdy == false){
                        if(execute_list[j].rd == RR[k].rs1){
                            RR[k].isRs1Rdy = true;
                        }
                    // }

                    // if(RR[k].isRs2Rdy == false){
                        if(execute_list[j].rd == RR[k].rs2){   
                            RR[k].isRs2Rdy = true;
                        }
                    // }
                }

                execute_list[j].enterWB = cycle+1;
                WB.push_back(execute_list[j]);
                execute_list.erase(execute_list.begin() + j);
                continue;
            }
            else j++;
        }
    }
}

void Pipeline::Writeback(int cycle){
    int i;
    bool wbErased;
    if(!WB.empty()){
        //increment cycles
        // cout << "in writeback" << endl;
        for(int i=0; i<WB.size(); i++){
            WB[i].cyclesInWB++;
            WB[i].instAge++;
        }

        i=0;
        while(i<WB.size()){
            wbErased = false;
            for(int j=0; j<myROB.size(); j++){
                if(WB[i].rd == myROB[j].robTag){
                myROB[j].valueAvail = true;
                // cout << "VALUE AVAIL AT ROB TAG " << myROB[j].robTag << endl;
                WB[i].enterRT = cycle + 1;
                myROB[j].robInstructionStruct = WB[i];
                wbErased = true;
                WB.erase(WB.begin() + i);
                break;
                }
            }

            if(wbErased == true) continue;
            else i++;
        }
    }
}

void Pipeline::Retire(int cycle){
    int i;
    bool check;

    if(!myROB.empty()){
        // cout << "in retire" << endl;
        for(int i=0; i<myROB.size(); i++){
            if(myROB[i].valueAvail == true){
                myROB[i].robInstructionStruct.cyclesInRT ++;
            }
        }

        i = 0;
     
        while(myROB[i].valueAvail == true && i<width && i<myROB.size()){
        //update rmt
        if(myROB[i].robInstructionStruct.rd == myRMT[myROB[i].robInstructionStruct.actualRd].robTag){
        myRMT[myROB[i].robInstructionStruct.actualRd].validBit = false;
        }
        //print instruction
        retiredInstructions.push_back(myROB[i]);
        i++;

        }
    }
}

void Pipeline::PrintInstData(instructionStruct retiredInst){
cout << retiredInst.id << " ";
cout << "fu{" << retiredInst.opType << "} ";
cout << "src{" << retiredInst.actualRs1 << "," << retiredInst.actualRs2 << "} ";
cout << "dst{" << retiredInst.actualRd << "} ";
cout << "FE{" << retiredInst.enterFE << "," << retiredInst.cyclesInFE << "} ";
cout << "DE{" << retiredInst.enterDE << "," << retiredInst.cyclesInDE << "} ";
cout << "RN{" << retiredInst.enterRN << "," << retiredInst.cyclesInRN << "} ";
cout << "RR{" << retiredInst.enterRR << "," << retiredInst.cyclesInRR << "} ";
cout << "DI{" << retiredInst.enterDI << "," << retiredInst.cyclesInDI << "} ";
cout << "IS{" << retiredInst.enterIS << "," << retiredInst.cyclesInIS << "} ";
cout << "EX{" << retiredInst.enterEX << "," << retiredInst.cyclesInEX << "} ";
cout << "WB{" << retiredInst.enterWB << "," << retiredInst.cyclesInWB << "} ";
cout << "RT{" << retiredInst.enterRT << "," << retiredInst.cyclesInRT << "} ";
cout << endl;
}

bool Pipeline::Advance_Cycle(){
    // cout << "in advance cycle";
    currentCycle ++;
    // cout << "DE : " << DE.size() << endl;
    // cout << "RN : " << RN.size() << endl;
    // cout << "RR : " << RR.size() << endl;
    // cout << "DI : " << DI.size() << endl;
    // cout << "IQ : " << myIssueQueue.size() << endl;
    // cout << "EX : " << execute_list.size() << endl;
    // cout << "WB : " << WB.size() << endl;
    // cout << "ROB : " << myROB.size() << endl;
  
    if(traceDepleted == true && DE.empty() && RN.empty() && RR.empty() && DI.empty() && myIssueQueue.empty() && execute_list.empty() && WB.empty() && myROB.empty()) return false;
    else return true;

}

