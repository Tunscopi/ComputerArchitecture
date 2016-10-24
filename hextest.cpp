#include <iostream>
#include <sstream>
#include <bitset>
#include <string>

using namespace std;

int main()
{
    string s = "0xA";
    stringstream ss;
    ss << hex << s;
    unsigned n;
    ss >> n;
    //bitset<10> b(n);
    // outputs "00000000000000000000000000001010"
    cout << n/15  << endl;

    cout << static_cast<int>(n) << endl;
    
}
