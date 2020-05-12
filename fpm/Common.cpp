#include "Common.h"

void to_binary(string str, vector<int> *binary) {
    int str_size = str.size();
    int k = 0;
    for (int j = 0; j < str_size; j++) {
        for (int i = 7; i >= 0; i--) {
            char c = str[j];
            int bit = (int) ((c & 1 << i) != 0);
            binary->push_back(bit);
        }
    }
    cout << "over";
}
