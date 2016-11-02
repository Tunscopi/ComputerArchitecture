#include <iostream>
#include <fstream>
using namespace std;

int main() {
string instr_info[4];
string dummy;
ifstream reader;
reader.open("smalltrace.txt");

while(!reader.eof()) {
    for (int i=0; i<4; i++){
        reader >> instr_info[i];
        if (i<3) reader >> dummy;
    }
    if (instr_info[2] == "B") {
        for (int j=0; j<4; j++)
            cout << instr_info[j] << " ";
        cout << endl;
    }
}

    return 0;
}; 
