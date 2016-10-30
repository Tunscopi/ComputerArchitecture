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
const int GHRsize = 2, entries = 1024, localHistoryDepth = 10, lastmBranchesUsed = 2, globPredwidth = 4; 
int GHR[GHRsize];
int pcLowerTenBits, globalIndex = 0, predictorArray[1][2], globPredArray[entries][globPredwidth], globPredStrengths[entries][globPredwidth]; // predictorArray: Selected predictor & strength. globPredArray: Used in nBitCounter. Stores prediction and pred_strength from each n-bit counter 
int localIndex = 0, levelOne[entries][localHistoryDepth], levelTwo[entries][1], levelTwoStrengths[entries][1];

// Helper functions
void nBitCounter(int, int, bool);
long long GetpcLowerTenBits(string);
int saturatingCounter(int, int);
int globalPred(int);
int localPred(int);


int main() {
    // -------- CONTROL PARAMETERS ----------
    int currPredictor = 1; //Initial predictor (1 - Global Predictor)

    // Set-up variables
    int execution = 0;
    float no_branches = 0.0, mispred_rate = 0.0, mispred_count = 0.0;
    bool tournament_prediction = 0, globalPrediction = 0, localPrediction = 0;  //where 0-Not Taken & 1-Taken
    string instr_info[4], pc, dummy; 
    ifstream reader;
    reader.open("smalltrace.txt");     

    // Initialize Data Arrays
    for (int i=0; i<GHRsize; i++)
       GHR[i] = 0;

    for (int i=0; i<entries; i++) { 
        levelTwo[i][0] = 0; 
        levelTwoStrengths[i][0] = 2; 

        for (int j=0; j<globPredwidth; j++) {  
            globPredStrengths[i][j] = 2; // Initialize prediction  strengths to strongly taken/strongly not taken
            globPredArray[i][j] = 0; // Initialize predictions to all Not taken
        }
        for (int k=0; k<localHistoryDepth; k++) 
            levelOne[i][k] = 0;
    }
    predictorArray[0][0] = 1; // Initialize selected predictor to 1 (Global Predictor)
    predictorArray[0][1] = 2; // Initialize selected predictor strength to 2; i.e 2 misses before switch 



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
            cout << "Branch: " << pcLowerTenBits << endl;
            globalPrediction = globalPred(execution);
            localPrediction = localPred(execution);


            cout << "Curr Predictor: " << currPredictor << endl;
            cout << "Exe: " << execution << " Pred: " << globalPrediction << " " << localPrediction << endl;
            cout << endl;

            // Use current selected predictor, then update current predictor if necessary
            (currPredictor == 1) ? tournament_prediction = globalPrediction : tournament_prediction = localPrediction;
            currPredictor = saturatingCounter(execution, tournament_prediction);                

            // Determine if misprediction
            if (tournament_prediction != execution) 
               mispred_count++; 
            
        }
    }
    cout << "\nMisprediction rate: " << (mispred_count*100)/no_branches << "%\n";

    return 0;
}

int localPred(int execution) {
    int localPrediction = levelTwo[localIndex][0];

    //Implement shift-left register to track last 10 executions of particular branch
    for (int j=1; j<localHistoryDepth; j++) 
        levelOne[pcLowerTenBits][j-1] = levelOne[pcLowerTenBits][j]; 
    levelOne[pcLowerTenBits][localHistoryDepth-1] = execution; 

    // Compute local Pred Index
    localIndex = 0;
    for (int i=localHistoryDepth-1; i>=0; i--) 
        localIndex += levelOne[pcLowerTenBits][i]*pow(2,i); 

    // Update levelTwoArray and levelTwoStrengths
    nBitCounter(2, execution, false);

    return localPrediction;
}

void nBitCounter(int n, int execution, bool isGlobalPredictor) {
    int nBitPrediction, pred_strength;

    if (isGlobalPredictor) 
         nBitPrediction = globPredArray[pcLowerTenBits][globalIndex], pred_strength = globPredStrengths[pcLowerTenBits][globalIndex]; // For better readability
    else
         nBitPrediction = levelTwo[localIndex][0], pred_strength = levelTwoStrengths[localIndex][0]; // For better readability


    if (execution == nBitPrediction){
        if(pred_strength < n) pred_strength++;    
        //cout << "Correct pred! " << nBitPrediction << " " << pred_strength << endl;
    } else{
        pred_strength--;
        if (pred_strength == 0) {
            nBitPrediction = execution;
            pred_strength = n;
        }
        //cout << "Wrong, penalty! " << nBitPrediction << " " << pred_strength << endl;
    }


    if (isGlobalPredictor) 
        globPredArray[pcLowerTenBits][globalIndex] = nBitPrediction, globPredStrengths[pcLowerTenBits][globalIndex] = pred_strength;
    else
        levelTwo[localIndex][0] = nBitPrediction, levelTwoStrengths[localIndex][0] = pred_strength;
}

int globalPred(int execution) { 
       // Access PHR (globPredArray), global prediction value
       int globalPrediction = globPredArray[pcLowerTenBits][globalIndex];

       //Implement shift-left register to track execution of last 2 branches
       for (int j=1; j<GHRsize; j++) 
            GHR[j-1] = GHR[j]; 
       GHR[GHRsize-1] = execution; 

       // Compute glob. Pred Index
       globalIndex = 0;
       for (int i=GHRsize-1; i>=0; i--) 
           globalIndex += GHR[i]*pow(2,i); 

       // Update globPredArray by implementing n=2 bit counter
       nBitCounter(2, execution, true);

    return globalPrediction;
}

int saturatingCounter (int execution, int curr_prediction) {
    int selPredictor = predictorArray[0][0], strength = predictorArray[0][1];

    if (execution == curr_prediction) {
        //cout << "correct!" << endl;
        if (strength < 2) strength++;
    } else { // Consider other predictor
        //cout << "wrong!" << endl;
        strength--;
        if (strength == 0) {
            if (selPredictor == 1) selPredictor = 2;
            else selPredictor = 1;
            strength = 2;
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

    pc = "0x" + pc; // This will sufficiently contain last 10 bits
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
