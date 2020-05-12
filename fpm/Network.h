#include <netinet/in.h>
#include <string>
#include <arpa/inet.h>
#include <netdb.h>

class Network {
public:
    int client_socket(const char *host, int client_port);
};

