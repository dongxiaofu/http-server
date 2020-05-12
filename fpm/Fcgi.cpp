#include "Fcgi.h"

void Fcgi::fcgi_param(string name, string value, vector<char> *pairs) {
    int name_length = name.size();
    int value_length = value.size();
    vector<char> name_len_bytes;
    get_bytes(name_length, &name_len_bytes);
    vector<char> value_len_bytes;
    get_bytes(value_length, &value_len_bytes);
    const int binary_name_len = name_length * 8;
    const int binary_value_len = value.size() * 8;
//    int binary_name[binary_name_len];
//    int binary_value[binary_value_len];
    vector<char> binary_name;//(binary_name_len);
    vector<char> binary_value;//(binary_value_len);
    to_binary(name, &binary_name);
    to_binary(value, &binary_value);

    pairs->insert(pairs->end(),name_len_bytes.begin(), name_len_bytes.end());
    pairs->insert(pairs->end(),value_len_bytes.begin(), value_len_bytes.end());
    pairs->insert(pairs->end(),binary_name.begin(), binary_name.end());
    pairs->insert(pairs->end(),binary_value.begin(), binary_value.end());

//    pairs->assign(name_len_bytes.begin(), name_len_bytes.end());
//    pairs->assign(value_len_bytes.begin(), value_len_bytes.end());
//    pairs->assign(binary_name.begin(), binary_name.end());
//    pairs->assign(binary_value.begin(), binary_value.end());
//
//    for (int i = 0; i < name_len_bytes.size(); i++) {
//        one_pair->push_back(name_len_bytes[i]);
//    }
//    for (int i = 0; i < value_len_bytes.size(); i++) {
//        one_pair->push_back(value_len_bytes[i]);
//    }
//    for (int i = 0; i <binary_name.size() ; i++) {
//        one_pair->push_back(binary_name[i]);
//    }
//    for (int i = 0; i < binary_value.size(); i++) {
//        one_pair->push_back(binary_value[i]);
//    }
}

void Fcgi::get_bytes(int name_length, vector<char> *bytes) {
//    int size;
//    if (name_length < 128) {
//        size = 1;
//    } else {
//        size = 4;
//    }
//    int name_len[size];

    if (name_length < 128) {
        bytes->push_back((char)name_length);
    } else {
        bytes->push_back((char)((name_length >> 24) & 0xff));
        bytes->push_back((char)((name_length >> 16) & 0xff));
        bytes->push_back((char)((name_length >> 8) & 0xff));
        bytes->push_back((char)(name_length & 0xff));
    }
}

void Fcgi::fcgi_packet(int type, int id, vector<char> content, int content_size, vector<char> *packet) {
    char header[8];
    header[0] = VERSION;
    header[1] = type;
    header[2] = (id >> 8) & 0xff;
    header[3] = id & 0xff;
    header[4] = (content_size >> 8) & 0xff;
    header[5] = content_size & 0xff;
    header[6] = 0;
    header[7] = 0;
    for (int i = 0; i < 8; i++) {
        packet->push_back(header[i]);
    }
    for(vector<char>::iterator it = content.begin();it < content.end();it++){
        // 不清楚为何要加*
        packet->push_back(*it);
    }
}