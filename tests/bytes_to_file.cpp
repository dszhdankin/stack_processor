#include <iostream>
#include <string>
#include <fstream>

using namespace std;

unsigned char data_out[100000];

void setOne(unsigned char *data, int pos) {
    unsigned char *cur = data + pos/8;
    int pos_in_cur = 7 - pos % 8;
    char mask = 0;
    mask |= (1 << pos_in_cur);
    *cur |= mask;
}

int main() {
    string inPath, outPath, data_in;
    cout << "Enter input path: ";
    cin >> inPath;
    cout << "Enter output path: ";
    cin >> outPath;

    ifstream in(inPath);
    ofstream out(outPath, ios_base::out | ios_base::binary);

    in >> data_in;

    int data_size = data_in.size();

    for (int i = 0; i < data_size; i++) {
        if (data_in[i] == '1')
            setOne(data_out, i);
    }

    out.write(reinterpret_cast<char *>(data_out), data_size / 8);

    return 0;
}