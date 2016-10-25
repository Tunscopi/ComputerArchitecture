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

// Helper functions
void nBitCounter(int, int, int, int[][2]);
string GetpcLowerTenBits(string);
int saturatingCounter(int, int, int[][2]);

int main() {
    // Set-up
    int predictorArray[1][2], globPredArray[4][2]; // predictorArray: Selected predictor & strength. globPredArray: Used in nBitCounter. Stores prediction and pred_strength from each n-bit counter 
    const int GHRsize = 2, entries = 50;
    int GHR[GHRsize]; 
    string predictorSelector[entries][2], pcLowerTenBits;
    int curr_prediction = 0, globalIndex = 0, execution = 0, curr_index = 0;
    bool isfound = false, globalPrediction = 0, localPrediction = 0;  //where 0-Not Taken & 1-Taken
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



    // Parse input trace file
    while (!reader.eof()) {
        for (int i=0; i<4; i++){
            reader >> instr_info[i];
            if(i<3) reader>>dummy;    
        }

        // check if it's a Branch instruction
        if (instr_info[2] == "B") {
            // reset params
            globalIndex = 0;

            // Grab and save history (Instr PC and execution info)
            execution = atoi(instr_info[3].c_str()); //Get Execution result. Recall, atoi doesn't work on strings
            pc = instr_info[1];

            // Save history if it doesn't already exist 
            pcLowerTenBits = GetpcLowerTenBits(pc);


            // Select Predictor
         /* if (selectedPredictor == 1) { // Global Predictor (2,2) correlating predictor
             
             Implement shift-left register to track last 2 branches
                for (int j=1; j<GHRsize; j++) 
                     GHR[j-1] = GHR[j]; 
                GHR[GHRsize-1] = execution; 

                // Compute glob. Pred Index
                for (int i=GHRsize-1; i>=0; i--) 
                    globalIndex += GHR[i]* pow(2,i); 

                // Access PHR (globPredArray), global prediction value
                globalPrediction = globPredArray[globalIndex][0];

                // Update globPredArray by implementing n=2 bit counter
                nBitCounter(2, execution, globalIndex, globPredArray);
                
            } else { // Local Predictor 

            } 
            

            // Update Tournament prediction results & selector
            if (predictorArray[0][1] == 1) curr_prediction = globalPrediction;
            else curr_prediction = localPrediction; 

            selectedPredictor = saturatingCounter(execution, curr_prediction, predictorArray);                
            */

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
        }
    }

    return 0;
    }


string GetpcLowerTenBits(string pc){
    string lowerTenBits;

    pc = "0x" + pc; //.substr(2,5); //This will sufficiently contain last 10 bits
    stringstream ss;
    ss << hex << pc;
    unsigned n;
    ss >> n;

    return to_string(n);
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
