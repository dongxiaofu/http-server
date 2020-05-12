#include "Fpm.h"

void Fpm::run() {
    FcgiRole fcgiRole;
    Fcgi fcgi;
    FcgiRequestType fcgiRequestType;
    int request_id = 5678;
    vector<int> one_packet;
    vector<int> begin_request_body;
    // 对vector能这样赋值吗？
//    begin_request_body[0] = 0; // roleB1
//    begin_request_body[1] = fcgiRole.FCGI_RESPONDER; //roleB0
//    begin_request_body[2] = 0; // flags
    begin_request_body.push_back(0);
    begin_request_body.push_back(fcgiRole.FCGI_RESPONDER);
    begin_request_body.push_back(0);
    for (int i = 0; i < 5; i++) {
        begin_request_body.push_back(0);
    }
    vector<int> begin_request_body_packet;
    fcgi.fcgi_packet(fcgiRequestType.FCGI_BEGIN_REQUEST, request_id, begin_request_body, 8,
                     &begin_request_body_packet);
    one_packet.insert(one_packet.end(), begin_request_body_packet.begin(), begin_request_body_packet.end());
//    one_packet.assign(begin_request_body_packet.begin(), begin_request_body_packet.end());
    //2.params
    map<string, string> params;
    string uri = "";
    string content_type = "";
//    params["GATEWAY_INTERFACE"] = "FastCGI/1.0";
//    params["REQUEST_METHOD"] = "GET";
//    params["SCRIPT_FILENAME"] = "/Users/cg/data/code/wheel/cpp/http-server/demo/test.php";
//    params["SCRIPT_NAME"] = uri;
//    params["QUERY_STRING"] = "";
//    params["REQUEST_URI"] = uri;
//    params["DOCUMENT_URI"] = uri;
//    params["SERVER_SOFTWARE"] = "php/fcgiclient";
//    params["REMOTE_ADDR"] = "127.0.0.1";
//    params["REMOTE_PORT"] = "9985";
//    params["SERVER_ADDR"] = "127.0.0.1";
//    params["SERVER_PORT"] = "80";
//    params["SERVER_NAME"] = "DESKTOP-NCL22GF";
//    params["SERVER_PROTOCOL"] = "HTTP/1.1";
//    params["CONTENT_TYPE"] = content_type;
//    params["CONTENT_LENGTH"] = "0";
    params["A"] = "cg";

    vector<int> pairs;
    map<string, string>::iterator iterator;
    for (iterator = params.begin(); iterator != params.end(); iterator++) {
        fcgi.fcgi_param(iterator->first, iterator->second, &pairs);
    }

    vector<int> fcgi_params;
    fcgi.fcgi_packet(fcgiRequestType.FCGI_PARAMS, request_id, pairs, pairs.size(), &fcgi_params);
    one_packet.insert(one_packet.end(), fcgi_params.begin(), fcgi_params.end());
//    one_packet.assign(fcgi_params.begin(), fcgi_params.end());

    vector<int> fcgi_params_end;
    vector<int> zero;
    zero.push_back(0);
    fcgi.fcgi_packet(fcgiRequestType.FCGI_PARAMS, request_id, zero, 1, &fcgi_params_end);
    one_packet.insert(one_packet.end(), fcgi_params_end.begin(), fcgi_params_end.end());
//    one_packet.assign(fcgi_params_end.begin(), fcgi_params_end.end());

    vector<int> fcgi_stdin;
    // test
    fcgi.fcgi_packet(fcgiRequestType.FCGI_STDIN, request_id, zero, 1, &fcgi_stdin);
    one_packet.insert(one_packet.end(),fcgi_stdin.begin(), fcgi_stdin.end());
//    one_packet.assign(fcgi_stdin.begin(), fcgi_stdin.end());

    vector<int> end_request_body;
    end_request_body.push_back((int) ((0 >> 24) & 0xff));
    end_request_body.push_back((int) ((0 >> 16) & 0xff));
    end_request_body.push_back((int) ((0 >> 8) & 0xff));
    end_request_body.push_back((int) (0 & 0xff));
    end_request_body.push_back(0);
    for(int i = 0; i< 3;i++){
        end_request_body.push_back(0);
    }
    vector<int> end_request;
    fcgi.fcgi_packet(fcgiRequestType.FCGI_END_REQUEST, request_id, end_request_body, 8, &end_request);
    one_packet.insert(one_packet.end(),end_request.begin(), end_request.end());
//    one_packet.assign(end_request.begin(), end_request.end());
    int a = 5;
}