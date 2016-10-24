//*****************************************************************************************************
// Computer Architecture project 
// Tournament predictor simulation comprising:
// Global predictor (2,2) correlating predictor and
// 2 Level local predictor: Level1: 1024 10-bit entries and Level2: 1024 2-bit saturating counters
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
// Use 1024 (lower 10 bits of address to index into (m, n) predictors

// Helper functions
void nBitCounter(int, int, int, int[][2]);
string GetpcLowerTenBits(string);


int main() {
    int globPredArray[4][2]; // Used in nBitCounter. Stores prediction and pred_strength from each n-bit counter 
    const int GHRsize = 2, entries = 1024;
    int GHR[GHRsize]; 
    string predictorSelector[entries][2], pcLowerTenBits;
    int globalIndex = 0, execution = 0, curr_index = 0;
    bool isfound = false, globalPrediction = 0, localPrediction = 0, Tournament_prediction = 0;  //where 0-Not Taken & 0-Taken
    string instr_info[4], pc, dummy;
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
        predictorSelector[i][0] = predictorSelector[i][1] = "1"; // Start off w/ Local predictor (Predictor 1)
    }
    

    // Parse input trace file
    while (!reader.eof()) {
        //for (int k =0; k<20; k++){
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
            cout << pcLowerTenBits << endl;


            // Select Predictor


            // Global Predictor 

            /* Implement shift-left register
                for (int j=1; j<GHRsize; j++) {
                     GHR[j-1] = GHR[j]; 
                }
                GHR[GHRsize-1] = execution; 


                // Compute glob. Pred Index
                for (int i=GHRsize-1; i>=0; i--) {
                globalIndex += GHR[i]* pow(2,i); 
                }

                // Access PHR (globPredArray)
                globalPrediction = globPredArray[globalIndex][0];

                // Update globPredArray by implementing n=2 bit counter
                nBitCounter(2, execution, globalIndex, globPredArray);


                // Debug
                cout << "\nPred: " << globalPrediction << "\n";
                for (int i=0; i<GHRsize; i++) {
                cout << GHR[i] << " ";
                }



            // Local Predictor
                */

          

            // Update Tournament prediction results
                


            // Save Prediction records
            for (int i=0; i<curr_index; i++) { // Record not found
                 if (predictorSelector[i][0] == pcLowerTenBits) {
                     predictorSelector[i][1] = Tournament_prediction;
                     isfound = true;
                     break;
                 }
                 i++;
            } 
            if (!isfound) {
                curr_index++;
                predictorSelector[curr_index][0] = pcLowerTenBits;  
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
