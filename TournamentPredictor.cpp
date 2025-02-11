//**********************************************************************************************************
// Computer Architecture project 
// Tournament predictor simulation comprising:
//      i.  Global predictor: (2,2) correlating predictor &
//      ii. 2-Level local predictor: Level1: 1024 10-bit entries, Level2: 1024 2-bit saturating counters
//
// By Ayotunde Odejayi
//**********************************************************************************************************

#include <iostream>
#include <sstream>
#include <bitset>
#include <string>
#include <fstream>
#include <math.h>
#include <algorithm>
using namespace std;

// Global variables
// -------- CONTROL PARAMETERS (1) -----------
const int entries = 1024, localHistoryDepth = 10, lastmBranchesUsed = 2, globPredwidth = 4; 

const int GHRsize = lastmBranchesUsed; 
int GHR[GHRsize], pcLowerTenBits, globalIndex = 0, predictorArray[1][2], globPredArray[entries][globPredwidth], globPredStrengths[entries][globPredwidth]; // globPredArray: Used in nBitCounter, stores prediction and globPredStrengths stores corresponding pred_strength from each n-bit counter 
int localIndex = 0, levelOne[entries][localHistoryDepth], levelTwo[entries][entries], levelTwoStrengths[entries][entries];

// Helper functions
int globalPred(int, int);
int localPred(int, int);
void nBitCounter(int, int, bool);
int saturatingCounter(int, int, int);
long long GetpcLowerTenBits(string);



int main() {
    // -------- CONTROL PARAMETERS (2) ----------
    int currPredictor = 1, no_CounterBits = 2, no_SaturatingCounterBits = 2; //Initial predictor (1 - Global Predictor), no_CounterBits- no. of wrong predictions before nbit counter/saturating counter switch

    // Set-up variables
    int execution = 0;
    float no_branches = 0.0, mispred_rate = 0.0, mispred_count = 0.0;
    bool tournament_prediction = 0, globalPrediction = 0, localPrediction = 0;  //where 0-Not Taken & 1-Taken
    string instr_info[4], pc, dummy; 
    ifstream reader;
    reader.open("largetrace.txt");     

    // Initialize Data Arrays
    for (int i=0; i<GHRsize; i++)
       GHR[i] = 0;
    for (int i=0; i<entries; i++) { 
        for (int j=0; j<globPredwidth; j++) {  
            globPredStrengths[i][j] = no_CounterBits; // Initialize prediction  strengths to strongly taken/strongly not taken
            globPredArray[i][j] = 0; // Initialize predictions to all Not taken
        }
        for (int k=0; k<localHistoryDepth; k++) 
            levelOne[i][k] = 0;
        for (int l=0; l<entries; l++) {
           levelTwo[i][l] = 0; 
           levelTwoStrengths[i][l] = no_CounterBits; 
        }
    }
    predictorArray[0][0] = currPredictor; // Initialize selected predictor to currPredictor 
    predictorArray[0][1] = no_SaturatingCounterBits; // Initialize selected predictor strength to 2; i.e 2 misses before switch 


    // Core functionality
    while (!reader.eof()) {
        // Parse input trace file
        for (int i=0; i<4; i++){
            reader >> instr_info[i];
            if(i<3) reader>>dummy;    
        }

        // check if it's a Branch instruction
        if (instr_info[2] == "B") {

            no_branches++;

            // Grab info (Instr PC and execution info)
            execution = atoi(instr_info[3].c_str()); //Get Execution result. Recall, atoi doesn't work on strings
            pc = instr_info[1];
            pcLowerTenBits = GetpcLowerTenBits(pc);

            // Get result from local & global predictors, then update them 
            globalPrediction = globalPred(execution, no_CounterBits);
            localPrediction = localPred(execution, no_CounterBits);

            // Use current selected predictor, then update current predictor if necessary
            (currPredictor == 1) ? tournament_prediction = globalPrediction : tournament_prediction = localPrediction;
            currPredictor = saturatingCounter(no_SaturatingCounterBits, execution, tournament_prediction);                

            // Determine if misprediction
            if (tournament_prediction != execution) 
               mispred_count++; 
            
        }
    }
    cout << "\nMisprediction rate: " << (mispred_count*100)/no_branches << "%\n";

    return 0;
}

int localPred(int execution, int no_CounterBits) {
    // Compute local Pred Index
    localIndex = 0;
    for (int i=localHistoryDepth-1; i>=0; i--) 
        localIndex += levelOne[pcLowerTenBits][i]*pow(2,localHistoryDepth-(i+1)); 
    
    int localPrediction = levelTwo[pcLowerTenBits][localIndex];

    //Implement shift-left register to track last 10 executions of particular branch
    for (int j=1; j<localHistoryDepth; j++) 
        levelOne[pcLowerTenBits][j-1] = levelOne[pcLowerTenBits][j]; 
    levelOne[pcLowerTenBits][localHistoryDepth-1] = execution; 
    
    // Update levelTwoArray and levelTwoStrengths
    nBitCounter(no_CounterBits, execution, false);

    return localPrediction;
}

void nBitCounter(int n, int execution, bool isGlobalPredictor) {
    int nBitPrediction, pred_strength;

    if (isGlobalPredictor) 
         nBitPrediction = globPredArray[pcLowerTenBits][globalIndex], pred_strength = globPredStrengths[pcLowerTenBits][globalIndex]; // For better readability
    else
         nBitPrediction = levelTwo[pcLowerTenBits][localIndex], pred_strength = levelTwoStrengths[pcLowerTenBits][localIndex]; // For better readability


    if (execution == nBitPrediction){
        if(pred_strength < n) pred_strength++;    
    } else{
        pred_strength--;
        if (pred_strength == 0) {
            nBitPrediction = execution;
            pred_strength = n;
        }
    }


    if (isGlobalPredictor) 
        globPredArray[pcLowerTenBits][globalIndex] = nBitPrediction, globPredStrengths[pcLowerTenBits][globalIndex] = pred_strength;
    else
        levelTwo[pcLowerTenBits][localIndex] = nBitPrediction, levelTwoStrengths[pcLowerTenBits][localIndex] = pred_strength;
}

int globalPred(int execution, int no_CounterBits) { 
       // Compute glob. Pred Index
       globalIndex = 0;
       for (int i=GHRsize-1; i>=0; i--) 
           globalIndex += GHR[i]*pow(2,GHRsize-(i+1)); 

       // Access PHR (globPredArray), global prediction value
       int globalPrediction = globPredArray[pcLowerTenBits][globalIndex];

       //Implement shift-left register to track execution of last 2 branches
       for (int j=1; j<GHRsize; j++) 
            GHR[j-1] = GHR[j]; 
       GHR[GHRsize-1] = execution; 

       // Update globPredArray by implementing n=2 bit counter
       nBitCounter(no_CounterBits, execution, true);

    return globalPrediction;
}

int saturatingCounter (int n, int execution, int curr_prediction) {
    int selPredictor = predictorArray[0][0], strength = predictorArray[0][1];

    if (execution == curr_prediction) {
        if (strength < n) strength++;
    } else { // Consider other predictor
        strength--;
        if (strength == 0) {
            if (selPredictor == 1) selPredictor = 2;
            else selPredictor = 1;
            strength = n;
        }
    }
    //cout << strength  << endl << endl;
    predictorArray[0][0] = selPredictor;
    predictorArray[0][1] = strength;

    return selPredictor;
}

long long GetpcLowerTenBits(string pc){
    long long lowerTenBits;
    unsigned n;

    pc = "0x" + pc; 
    stringstream ss;
    ss << hex << pc;
    ss >> n;
    bitset<10> binary_val(n);
    bitset<10> mask(0x3FF);
    binary_val = binary_val & mask; // Get rid of still present bits after desired 10

    int bin_int = (int)(binary_val.to_ulong());
    lowerTenBits = stol(to_string(bin_int));

    return lowerTenBits;
}
