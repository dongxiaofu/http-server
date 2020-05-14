#include <map>
#include <vector>
#include "Fcgi.h"
#include "FcgiRole.cpp"
#include "FcgiRequestType.cpp"
#include "Network.h"
#include <zconf.h>

typedef struct {
    string content_type;
    string content_length;
    string uri;
    vector<char> http_body;
    string query_string;
} ParamsFromWebServer;

typedef struct {
    string head_line;   // http 响应头
    string body;    // http 实体主体
} DataFromFastCGIServer;

/**
 * 参考资料
 * 1.FastCGI官方文档：https://fastcgi-archives.github.io/FastCGI_Specification.html#S5.3
 * 2.别人的博客：https://blog.csdn.net/zhang197093/article/details/78914509
 */
class Fpm {
public:
    const int CLIENT_PORT = 9000;

    DataFromFastCGIServer * run(ParamsFromWebServer params_from_web_server);

    void create_packet(vector<char> *one_packet, ParamsFromWebServer params_from_web_server);

    void send_packet(int socket_fd, vector<char> packet);

    DataFromFastCGIServer *receive_data_from_server(int socket_fd);
};
