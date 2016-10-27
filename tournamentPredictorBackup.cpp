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
const int GHRsize = 2, entries = 1024, lastmBranchesUsed = 2, globPredwidth = 4; 
int GHR[GHRsize];
int pcLowerTenBits, globalIndex = 0, predictorArray[1][2], globPredArray[entries][globPredwidth], globPredStrengths[entries][globPredwidth]; // predictorArray: Selected predictor & strength. globPredArray: Used in nBitCounter. Stores prediction and pred_strength from each n-bit counter 

// Helper functions
void nBitCounter(int, int, int, int[][globPredwidth], int[][globPredwidth]);
long long GetpcLowerTenBits(string);
int saturatingCounter(int, int, int[][2]);
int globalPred(int, int, int[][globPredwidth], int[][globPredwidth]);
int localPred(int, int);


int main() {
    // Set-up
    string predictorSelector[entries][2];
    int curr_prediction = 0, execution = 0, curr_index = 0, currPredictor = 1;
    bool isfound = false, tournament_prediction = 0, globalPrediction = 0, localPrediction = 0;  //where 0-Not Taken & 1-Taken
    string instr_info[4], pc, dummy; 
    ifstream reader;
    reader.open("smalltrace.txt");     

    // Initialize GHR, predictorSelector and globPredArrays to zeros
    for (int i=0; i<GHRsize; i++) {
       GHR[i] = 0;
    }
    for (int i=0; i<entries; i++) {
        for (int j=0; j<globPredwidth; j++) 
            globPredArray[i][j] = 0; // Initialize predictions to all Not taken
    }
    for (int i=0; i<entries; i++) {
        for (int j=0; j<globPredwidth; j++) 
            globPredStrengths[i][j] = 2; // Initialize prediction  strengths to strongly taken/strongly not taken
    }
    for (int i=0; i<entries; i++) 
        predictorSelector[i][0] = predictorSelector[i][1] = "1"; // Start off w/ global predictor (Predictor 1)
    
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

            // Get result from local & global predictors, then update them 
            globalPrediction = globalPred(execution, globalIndex, globPredArray, globPredStrengths);

            //localPrediction = localPred(execution, entries);

            // Use current selected predictor, then update current predictor if necessary
            (currPredictor == 1) ? tournament_prediction = globalPrediction : tournament_prediction = localPrediction;
            currPredictor = saturatingCounter(execution, curr_prediction, predictorArray);                

            // Determine if misprediction
            cout << "Branch: " << pcLowerTenBits << endl;
            cout << "global Prediction: " << globalPrediction << "      Execution: " << execution << endl;

            // Debug
            cout << "globPred: " << globPredArray[pcLowerTenBits][globalIndex] << "   globPredStrength: " << globPredStrengths[pcLowerTenBits][globalIndex] << endl << endl;
        }
    }

    return 0;
}

int globalPred(int execution, int globalIndex, int globPredArray[][globPredwidth], int globPredStrengths[][globPredwidth]) { 
       // Access PHR (globPredArray), global prediction value
       int globalPrediction = globPredArray[pcLowerTenBits][globalIndex];

       //Implement shift-left register to track last 2 branches
       for (int j=1; j<GHRsize; j++) 
            GHR[j-1] = GHR[j]; 
       GHR[GHRsize-1] = execution; 

       // Compute glob. Pred Index
       for (int i=GHRsize-1; i>=0; i--) 
           globalIndex += GHR[i]*pow(2,i); 

       // Update globPredArray by implementing n=2 bit counter
       nBitCounter(2, execution, globalIndex, globPredArray, globPredStrengths);
       //cout << globPredStrengths[pcLowerTenBits][globalIndex] << endl;

    return globalPrediction;
}

void nBitCounter(int n, int newvalue, int globalIndex, int globPredArray[][globPredwidth], int globPredStrengths[][globPredwidth]) {
    int nBitPrediction = globPredArray[pcLowerTenBits][globalIndex], pred_strength = globPredStrengths[pcLowerTenBits][globalIndex]; // For better readability

    if (newvalue == nBitPrediction){
        if(pred_strength < n) pred_strength++;    
        cout << "Correct pred! " << nBitPrediction << " " << pred_strength << endl;
    } else {
        pred_strength--;
        if (pred_strength == 0) {
            nBitPrediction = newvalue;
            pred_strength++;
        }
        cout << "Wrong, penalty! " << nBitPrediction << " " << pred_strength << endl;
    }

    globPredArray[pcLowerTenBits][globalIndex] = nBitPrediction;
    globPredStrengths[pcLowerTenBits][globalIndex] = pred_strength;
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

int localPred(int execution, int entries) {
    int localPrediction = execution;

    /* Determine which predictor 
    for (int i=0; i<curr_index; i++) { // Search for record
          if (predictorSelector[i][0] == pcLowerTenBits) {
              predictorSelector[i][1] = currPredictor;
              isfound = true;
              break;
          }
    } 
    if (!isfound) { // Record not found, insert
          predictorSelector[curr_index][0] = pcLowerTenBits;  
          predictorSelector[curr_index][1] = currPredictor; 
          curr_index++;
    }
    isfound = false;
    */
    return localPrediction;
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
