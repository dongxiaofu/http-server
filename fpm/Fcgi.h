#include <string>
#include "Common.h"

using namespace std;

typedef struct {
    int size;
    int *data;
} ARRAY;

class Fcgi {
public:
    const int VERSION = 1;

    void fcgi_param(string name, string value, vector<char> *pairs);

    void get_bytes(int name_length, vector<char> *bytes);

    void fcgi_packet(int type, int id, vector<char> content, int content_size, vector<char> *packet);

    void get_chars(string str, vector<char> *str_chars);
};