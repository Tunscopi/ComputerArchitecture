// Computer Architecture project 
// Implement a simulation of a Tournament predictor comprising:
// Global predictor (2,2) correlating predictor and
// 2 Level local predictor: Level1: 1024 10-bit entries and Level2: 1024 2-bit saturating counters
//

#include <iostream>
#include <string>
#include <ifstream>
#include <math.h>
using namespace std;

int main() {
    int globPredArray[4]; 
    int GHR[2];
    int globalIndex = 0;
    bool globalPrediction = 0, localPrediction = 0, Tournament_prediction = 0;  //where 0-Not Taken & 0-Taken
    

    // Global Predictor
    // Compute glob. Pred Index
    for (int i=0; i<sizeof(GHR)/sizeof(GHR[0]); i++) {
        globalIndex += GHR[i]* pow(2,i); 
    }

    // Access PHR (globPredArray)
    globalPrediction = globPredArray[globalIndex];
    
    // Save history


    // Save accuracy results 

    return 0;
}
