#include <string>
#include <iostream>

using namespace std;

void to_binary(string str, int res[]);

int main() {
    string data = "A";
    int res[data.size() * 8];
    cout << "data.size:" << data.size() << endl;
    to_binary(data, res);
    for (int i = 0; i < data.size() * 8; i++) {
        cout << res[i];
    }
    return 0;

}

void to_binary(string str, int res[]) {
    int str_size = str.size();
    int k = 0;
    for (int j = 0; j < str_size; j++) {
        char c = str[j];
        for (int i = 7; i >= 0; i--) {
            int bit = ((c & 1 << i) != 0);
            res[k++] = bit;
        }
    }
}