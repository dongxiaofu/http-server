#include "Fcgi.h"

/************************************************************************************************
* FCGI_PARAMS 以键值对的方式传送，键和值之间没有'=',每个键值对之前会分别用1或4个字节来标识键和值的长度
* 例如: \x0B\x02SERVER_PORT80\x0B\x0ESERVER_ADDR199.170.183.42
* 上面的长度是十六进制的  \x0B = 11  正好为字符串 "SERVER_PORT" 的长度， \x02 = 2 为字符串 "80" 的长度。
 * 这段注释，从别人的代码中摘录的。
*************************************************************************************************/
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
    get_chars(name, &binary_name);
    get_chars(value, &binary_value);

    pairs->insert(pairs->end(), name_len_bytes.begin(), name_len_bytes.end());
    pairs->insert(pairs->end(), value_len_bytes.begin(), value_len_bytes.end());
    pairs->insert(pairs->end(), binary_name.begin(), binary_name.end());
    pairs->insert(pairs->end(), binary_value.begin(), binary_value.end());
}

void Fcgi::get_bytes(int name_length, vector<char> *bytes) {
    // todo 数组的长度，不能是变量。因此，下面这段，是危险的错误写法。隐蔽，不容易发现。
//    int size;
//    if (name_length < 128) {
//        size = 1;
//    } else {
//        size = 4;
//    }
//    int name_len[size];
    // 长度小于128，用一个字节存储；大于或等于128，用4个字节存储。
    if (name_length < 128) {
        bytes->push_back((char) name_length);
    } else {
        // todo 移位运算，很有用的常识性但我不知道的操作。
        // 用多个字节存储大数的代码。
        bytes->push_back((char) ((name_length >> 24) & 0xff));
        bytes->push_back((char) ((name_length >> 16) & 0xff));
        bytes->push_back((char) ((name_length >> 8) & 0xff));
        bytes->push_back((char) (name_length & 0xff));
    }
}

void Fcgi::fcgi_packet(int type, int id, vector<char> content, int content_size, vector<char> *packet) {
    // todo 我看到别人用struct存储数据报头
    char header[8];
    header[0] = VERSION;
    header[1] = type;
    header[2] = (id >> 8) & 0xff;   // 高八位
    /*************************************************************************
     * 为什么不是全部，而是低八位呢？
     * 因为0xff是255，它是2的7次方，1个1 + 7个0，共八位。
     * 高于八位的，&运算后，为0。
     ************************************************************************/
    header[3] = id & 0xff;
    header[4] = (content_size >> 8) & 0xff;
    header[5] = content_size & 0xff;
    header[6] = 0;
    header[7] = 0;
    for (int i = 0; i < 8; i++) {
        packet->push_back(header[i]);
    }
    for (vector<char>::iterator it = content.begin(); it < content.end(); it++) {
        // 不清楚为何要加*
        // todo 遍历vector，获取单元值，用*it
        packet->push_back(*it);
    }
}

void Fcgi::get_chars(string str, vector<char> *str_chars) {
    for (int i = 0; i < str.size(); i++) {
        str_chars->push_back(str[i]);
    }
}