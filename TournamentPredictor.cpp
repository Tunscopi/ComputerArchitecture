// Computer Architecture project 
// Tournament predictor simulation comprising:
// Global predictor (2,2) correlating predictor and
// 2 Level local predictor: Level1: 1024 10-bit entries and Level2: 1024 2-bit saturating counters
//
// By Ayotunde Odejayi

#include <iostream>
#include <string>
#include <fstream>
#include <math.h>
#include <algorithm>
using namespace std;

void nBitCounter(int, int, int, int[][2]);

int main() {
    int globPredArray[4][2]; // Stores prediction and pred_strength from each n-bit counter 
    const int GHRsize = 2;
    int GHR[GHRsize];
    int globalIndex = 0, result = 0;
    bool globalPrediction = 0, localPrediction = 0, Tournament_prediction = 0;  //where 0-Not Taken & 0-Taken
    string instr_info[4], dummy;
    ifstream reader;
    reader.open("smalltrace.txt");     

    // Initialize GHR and globPredArrays to zeros
    for (int i=0; i<GHRsize; i++) {
       GHR[i] = 0;
    }
    for (int i=0; i<4; i++) {
        for (int j=0; j<2; j++) {
            if (j == 1) globPredArray[i][j] = 2; // Initialize pred_strength;
            else globPredArray[i][j] = 0; // Initialize prediction
        }
    }


    // Parse input trace file
    while (!reader.eof()) {
    //for (int k =0; k<20; k++){
        for (int i=0; i<4; i++){
            reader >> instr_info[i];
            if(i<3) reader>>dummy;    
        }
        result = atoi(instr_info[3].c_str()); //recall, atoi doesn't work on strings
    
    // check if it's a Branch instruction
    if (instr_info[2] == "B") {
        // reset params
        globalIndex = 0;

        // Save history (Instr. and Taken/Not Taken info)
        // Implement shift-left register
        for (int j=1; j<GHRsize; j++) {
            GHR[j-1] = GHR[j]; 
        }
        GHR[GHRsize-1] = result; 


        // Global Predictor
        // Compute glob. Pred Index
        for (int i=GHRsize-1; i>=0; i--) {
            globalIndex += GHR[i]* pow(2,i); 
        }

        // Access PHR (globPredArray)
        globalPrediction = globPredArray[globalIndex][0];

        // Update globPredArray by implementing n=2 bit counter
        nBitCounter(2, result, globalIndex, globPredArray);


        // Debug
        cout << "\nPred: " << globalPrediction << "\n";
        for (int i=0; i<GHRsize; i++) {
            cout << GHR[i] << " ";
        }


        // Local Predictor
        
        // Save accuracy results 
        }
    }

    return 0;
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
