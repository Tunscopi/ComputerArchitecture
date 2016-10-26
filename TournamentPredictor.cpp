//*****************************************************************************************************
// Computer Architecture project 
// Tournament predictor simulation comprising:
// Global predictor: (2,2) correlating predictor &
// 2-Level local predictor: Level1: 1024 10-bit entries, Level2: 1024 2-bit saturating counters
//
// By Ayotunde Odejayi
//*****************************************************************************************************

#include <iostream>
#include <sstream>
#include <bitset>
#include <string>
#include <fstream>
#include <math.h>
#include <algorithm>
using namespace std;

// Global variables
const int GHRsize = 2, entries = 50;
int GHR[2];
int predictorArray[1][2], globPredArray[4][2]; // predictorArray: Selected predictor & strength. globPredArray: Used in nBitCounter. Stores prediction and pred_strength from each n-bit counter 

// Helper functions
void nBitCounter(int, int, int, int[][2]);
long long GetpcLowerTenBits(string);
int saturatingCounter(int, int, int[][2]);
int globalPred(int, int, int[][2]);
int localPred(int, int);


int main() {
    // Set-up
    string predictorSelector[entries][2];
    int curr_prediction = 0, globalIndex = 0, execution = 0, curr_index = 0, pcLowerTenBits;
    bool isfound = false, tournament_prediction = 0, globalPrediction = 0, localPrediction = 0;  //where 0-Not Taken & 1-Taken
    string instr_info[4], pc, dummy, selectedPredictor = "1";
    ifstream reader;
    reader.open("smalltrace.txt");     

    // Initialize GHR, predictorSelector and globPredArrays to zeros
    for (int i=0; i<GHRsize; i++) {
       GHR[i] = 0;
    }
    for (int i=0; i<4; i++) {
        for (int j=0; j<2; j++) {
            if (j == 1) globPredArray[i][j] = 2; // Initialize pred_strength;
            else globPredArray[i][j] = 0; // Initialize prediction
        }
    }
    for (int i=0; i<entries; i++) {
        predictorSelector[i][0] = predictorSelector[i][1] = "1"; // Start off w/ global predictor (Predictor 1)
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
            // reset params
            globalIndex = 0;

            // Grab info (Instr PC and execution info)
            execution = atoi(instr_info[3].c_str()); //Get Execution result. Recall, atoi doesn't work on strings
            pc = instr_info[1];
            pcLowerTenBits = GetpcLowerTenBits(pc);
            //cout << pcLowerTenBits << endl; 

            /* Update Predictors 
            globalPrediction = globalPred(execution, globalIndex, globPredArray);
            localPrediction = localPred(execution, entries);

            (selectedPredictor == 1) ? tournament_prediction = globalPrediction : tournament_predictor = localPrediction;

            selectedPredictor = saturatingCounter(execution, curr_prediction, predictorArray);                

            // Determine which predictor to use with lower 10 bits
            for (int i=0; i<curr_index; i++) { // Search for record
                 if (predictorSelector[i][0] == pcLowerTenBits) {
                     predictorSelector[i][1] = selectedPredictor;
                     isfound = true;
                     break;
                 }
            } 
            if (!isfound) { // Record not found, insert
                predictorSelector[curr_index][0] = pcLowerTenBits;  
                predictorSelector[curr_index][1] = selectedPredictor; 
                curr_index++;
            }
            isfound = false;
            */
        }
    }

    return 0;
    }




int globalPred(int execution, int globalIndex, int globPredArray[][2]) {
       // Access PHR (globPredArray), global prediction value
       int globalPrediction = globPredArray[globalIndex][0];

       //Implement shift-left register to track last 2 branches
       for (int j=1; j<GHRsize; j++) 
            GHR[j-1] = GHR[j]; 
       GHR[GHRsize-1] = execution; 

       // Compute glob. Pred Index
       for (int i=GHRsize-1; i>=0; i--) 
           globalIndex += GHR[i]*pow(2,i); 

       // Update globPredArray by implementing n=2 bit counter
       nBitCounter(2, execution, globalIndex, globPredArray);

    return globalPrediction;
}


int localPred(int execution, int entries) {
    int localPrediction = execution;

    return localPrediction;
}


int saturatingCounter (int newvalue, int selPredictorValue, int predictorArray[][2]) {
    int selPredictor = predictorArray[0][0], strength = predictorArray[0][1];

    if (newvalue == selPredictorValue) {
        if (strength < 2) strength++;
    } else { // Consider other predictor
        strength--;
        if (strength == 0) {
            if (selPredictor == 1) selPredictor = 2;
            else selPredictor = 1;
        }
    }
    predictorArray[0][0] = selPredictor;
    predictorArray[0][1] = strength;

    return selPredictor;
}


void nBitCounter(int n, int newvalue, int globalIndex, int globPredArray[][2]) {
    int nBitPrediction = globPredArray[globalIndex][0], pred_strength = globPredArray[globalIndex][1]; // For better readability

    if (newvalue == nBitPrediction){
        if(pred_strength < n) pred_strength++;    
    } else {
        pred_strength--;
        if (pred_strength == 0) {
            nBitPrediction = newvalue;
            pred_strength++;
        }
    }

    globPredArray[globalIndex][0] = nBitPrediction;
    globPredArray[globalIndex][1] = pred_strength;
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
