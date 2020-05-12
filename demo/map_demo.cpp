#include <map>
#include <iostream>

using namespace std;

int main() {
    map<string, string> params;
    string uri = "";
    string content_type = "";
    params["GATEWAY_INTERFACE"] = "FastCGI/1.0";
    params["REQUEST_METHOD"] = "GET";
    params["SCRIPT_FILENAME"] = "/Users/cg/data/code/wheel/cpp/http-server/demo/test.php";
    params["SCRIPT_NAME"] = uri;
    params["QUERY_STRING"] = "";
    params["REQUEST_URI"] = uri;
    params["DOCUMENT_URI"] = uri;
    params["SERVER_SOFTWARE"] = "php/fcgiclient";
    params["REMOTE_ADDR"] = "127.0.0.1";
    params["REMOTE_PORT"] = "9985";
    params["SERVER_ADDR"] = "127.0.0.1";
    params["SERVER_PORT"] = "80";
    params["SERVER_NAME"] = "DESKTOP-NCL22GF";
    params["SERVER_PROTOCOL"] = "HTTP/1.1";
    params["CONTENT_TYPE"] = content_type;
    params["CONTENT_LENGTH"] = "0";
    params["AUTHOR"] = "cg";

    map<string,string>::iterator iter;
    for (iter = params.begin(); iter != params.end(); iter++) {
        printf("%s\t:\t%s\n", iter->first.c_str(), iter->second.c_str());
    }

//    // C++11使用
//    for (auto &key : params) {
//        cout << key << params[key] << endl;
//    }

    return 0;
}