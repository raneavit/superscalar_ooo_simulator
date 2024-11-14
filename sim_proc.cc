#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "sim_proc.h"
#include "pipeline.h"

/*  argc holds the number of command line arguments
    argv[] holds the commands themselves

    Example:-
    sim 256 32 4 gcc_trace.txt
    argc = 5
    argv[0] = "sim"
    argv[1] = "256"
    argv[2] = "32"
    ... and so on
*/
int main (int argc, char* argv[])
{
    // File handler
    char *trace_file;       // Variable that holds trace file name;
    proc_params params;       // look at sim_bp.h header file for the the definition of struct proc_params

    Pipeline* myPipeline;
    uint32_t currentCycle = 0;
    
    if (argc != 5)
    {
        printf("Error: Wrong number of inputs:%d\n", argc-1);
        exit(EXIT_FAILURE);
    }
    
    params.rob_size     = strtoul(argv[1], NULL, 10);
    params.iq_size      = strtoul(argv[2], NULL, 10);
    params.width        = strtoul(argv[3], NULL, 10);
    trace_file          = argv[4];

    // params.rob_size     = 60;
    // params.iq_size      = 15;
    // params.width        = 3;
    // trace_file          = "val_trace_gcc1";


    // printf("rob_size:%lu "
    //         "iq_size:%lu "
    //         "width:%lu "
    //         "tracefile:%s\n", params.rob_size, params.iq_size, params.width, trace_file);

    myPipeline = new Pipeline(params.rob_size, params.iq_size, params.width, trace_file);
    
    ///////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // The following loop just tests reading the trace and echoing it back to the screen.
    //
    // Replace this loop with the "do { } while (Advance_Cycle());" loop indicated in the Project 3 spec.
    // Note: fscanf() calls -- to obtain a fetch bundle worth of instructions from the trace -- should be
    // inside the Fetch() function.
    //
    ///////////////////////////////////////////////////////////////////////////////////////////////////////
    // while(fscanf(FP, "%lx %d %d %d %d", &pc, &op_type, &dest, &src1, &src2) != EOF)
    //     printf("%lx %d %d %d %d\n", pc, op_type, dest, src1, src2); //Print to check if inputs have been read correctly

    do{
        // printf("======== CYCLE %lu ========\n", myPipeline->currentCycle);
        myPipeline->Retire(myPipeline->currentCycle);
        
        myPipeline->Writeback(myPipeline->currentCycle);
        // PrintRob(myPipeline->myROB);
        
        myPipeline->Execute(myPipeline->currentCycle);
        // PrintRegister(myPipeline->WB, "WRITEBACK");
        
        myPipeline->Issue(myPipeline->currentCycle);
        // PrintRegister(myPipeline->execute_list, "EXECUTE LIST");
        
        myPipeline->Dispatch(myPipeline->currentCycle);
        // PrintRegister(myPipeline->myIssueQueue, "ISSUE QUEUE");
        
        myPipeline->RegRead(myPipeline->currentCycle);
        // PrintRegister(myPipeline->DI, "DISPATCH");
        
        myPipeline->Rename(myPipeline->currentCycle);
        // PrintRegister(myPipeline->RR, "REG READ");
        // PrintRob(myPipeline->myROB);
        
        myPipeline->Decode(myPipeline->currentCycle);
        // PrintRegister(myPipeline->RN, "RENAME");
        
        myPipeline->Fetch(myPipeline->currentCycle);
        // PrintRegister(myPipeline->DE, "DECODE");

        // TBR
        // if (myPipeline->currentCycle == 20) break;
    }
    while(myPipeline->Advance_Cycle());

    //Print
    cout << "# === Simulator Command =========" << endl;
    cout << "# ./sim " << params.rob_size << " " << params.iq_size << " " << params.width << " " << trace_file << endl;
    cout << "# === Processor Configuration ===" << endl;
    cout << "# ROB_SIZE = " << params.rob_size << endl;
    cout << "# IQ_SIZE  = " << params.iq_size << endl;
    cout << "# WIDTH    = " << params.width << endl;
    cout << "# === Simulation Results ========" << endl;
    cout << "# Dynamic Instruction Count    = " << myPipeline->sequenceNumber << endl;
    cout << "# Cycles                       = " << myPipeline->currentCycle << endl;
    cout << fixed << "# Instructions Per Cycle (IPC) = " << setprecision(2) << (double)myPipeline->sequenceNumber/myPipeline->currentCycle << endl;
    return 0;
}
