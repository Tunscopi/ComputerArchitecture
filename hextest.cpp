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
    bitset<32> b(n);
    // outputs "00000000000000000000000000001010"
    cout << b.to_string() << endl;
}
