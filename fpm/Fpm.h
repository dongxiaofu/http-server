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

class Fpm {
public:
    const int CLIENT_PORT = 9000;

    char * run(ParamsFromWebServer params_from_web_server);

    void create_packet(vector<char> *one_packet, ParamsFromWebServer params_from_web_server);

    void send_packet(int socket_fd, vector<char> packet);

    char *receive_data_from_server(int socket_fd);
};
