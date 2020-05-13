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
};
