#include "Common.h"

void to_binary(string str, vector<char> *binary) {
    int str_size = str.size();
    for (int j = 0; j < str_size; j++) {
        for (int i = 7; i >= 0; i--) {
            char c = str[j];
            char bit = (char) ((c & 1 << i) != 0);
            binary->push_back(bit);
        }
    }
}
