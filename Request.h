#include <string>

using std::string;

class Request {

public:
    string file_path;
    string host;
    string user_agent;
    string accept;
    string accept_language;
    string accept_encoding;
    string connection;
    string cookie;
    string upgrade_insecure_requests;
    string cache_control;
};

