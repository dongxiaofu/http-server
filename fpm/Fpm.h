#include <map>
#include <vector>
#include "Fcgi.h"
#include "FcgiRole.cpp"
#include "FcgiRequestType.cpp"
#include "Network.h"
#include <zconf.h>

class Fpm {
public:
    const int CLIENT_PORT = 9000;

    void run();

    void create_packet(vector<char> *one_packet);

    void send_packet(int socket_fd, vector<char> packet);

    char *receive_data_from_server(int socket_fd);
};
