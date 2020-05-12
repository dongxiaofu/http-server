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

    void fcgi_param(string name, string value, vector<int> *pairs);

    void get_bytes(int name_length, vector<int> *bytes);

    void fcgi_packet(int type, int id, vector<int> content, int content_size, vector<int> *packet);
};