#include "Fpm.h"

void Fpm::run() {
    FcgiRole fcgiRole;
    Fcgi fcgi;
    FcgiRequestType fcgiRequestType;
    int request_id = 5679;
    vector<char> one_packet;
    vector<char> begin_request_body;
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
    vector<char> begin_request_body_packet;
    fcgi.fcgi_packet(fcgiRequestType.FCGI_BEGIN_REQUEST, request_id, begin_request_body, begin_request_body.size(),
                     &begin_request_body_packet);
    one_packet.insert(one_packet.end(), begin_request_body_packet.begin(), begin_request_body_packet.end());
//    one_packet.assign(begin_request_body_packet.begin(), begin_request_body_packet.end());
    //2.params
    map<string, string> params;
    string uri = "/test.php";
    string content_type = "";
    params["GATEWAY_INTERFACE"] = "FastCGI/1.0";
    params["REQUEST_METHOD"] = "GET";
    params["SCRIPT_FILENAME"] = "/Users/cg/data/www/cg/sort.php";
    params["SCRIPT_NAME"] = uri;
    params["QUERY_STRING"] = "";
    params["REQUEST_URI"] = uri;
    params["DOCUMENT_URI"] = uri;
    params["SERVER_SOFTWARE"] = "php/fcgiclient";
    params["REMOTE_ADDR"] = "127.0.0.1";
    params["REMOTE_PORT"] = "9986";
    params["SERVER_ADDR"] = "127.0.0.1";
    params["SERVER_PORT"] = "80";
    params["SERVER_NAME"] = "DESKTOP-NCL22GF";
    params["SERVER_PROTOCOL"] = "HTTP/1.1";
    params["CONTENT_TYPE"] = content_type;
    params["CONTENT_LENGTH"] = "0";
    params["AUTHOR"] = "cg";

    vector<char> pairs;
    for (map<string, string>::iterator it = params.begin(); it != params.end(); it++) {
        fcgi.fcgi_param(it->first, it->second, &pairs);
    }

    vector<char> fcgi_params;
    fcgi.fcgi_packet(fcgiRequestType.FCGI_PARAMS, request_id, pairs, pairs.size(), &fcgi_params);
    one_packet.insert(one_packet.end(), fcgi_params.begin(), fcgi_params.end());
//    one_packet.assign(fcgi_params.begin(), fcgi_params.end());

    vector<char> fcgi_params_end;
    vector<char> zero;
//    zero.push_back(0);
    fcgi.fcgi_packet(fcgiRequestType.FCGI_PARAMS, request_id, zero, zero.size(), &fcgi_params_end);
    one_packet.insert(one_packet.end(), fcgi_params_end.begin(), fcgi_params_end.end());
//    one_packet.assign(fcgi_params_end.begin(), fcgi_params_end.end());

    vector<char> fcgi_stdin;
    // test
    fcgi.fcgi_packet(fcgiRequestType.FCGI_STDIN, request_id, zero, zero.size(), &fcgi_stdin);
    one_packet.insert(one_packet.end(), fcgi_stdin.begin(), fcgi_stdin.end());
//    one_packet.assign(fcgi_stdin.begin(), fcgi_stdin.end());

    vector<char> end_request_body;
    end_request_body.push_back((int) ((0 >> 24) & 0xff));
    end_request_body.push_back((int) ((0 >> 16) & 0xff));
    end_request_body.push_back((int) ((0 >> 8) & 0xff));
    end_request_body.push_back((int) (0 & 0xff));
    end_request_body.push_back(0);
    for (int i = 0; i < 3; i++) {
        end_request_body.push_back(0);
    }
    vector<char> end_request;
    fcgi.fcgi_packet(fcgiRequestType.FCGI_END_REQUEST, request_id, end_request_body, end_request_body.size(), &end_request);
    one_packet.insert(one_packet.end(), end_request.begin(), end_request.end());
//    one_packet.assign(end_request.begin(), end_request.end());

    Network network;
    int socket = network.client_socket("127.0.0.1", CLIENT_PORT);
//    FILE *fp = fopen("/Users/cg/data/code/wheel/java/demo/html/log/data2.log", "w");
    for (int i = 0; i < one_packet.size(); i++) {
        char bit = one_packet[i];
//        printf("%d\n", bit);
//        fprintf(fp, "%d,", bit);
        write(socket, &bit, sizeof(char));
    }
//    fclose(fp);
//    string str = "hello";
//    char one_packet2[435] = {1,1,22,46,0,8,0,0,0,1,0,0,0,0,0,0,1,4,22,46,1,122,0,0,17,11,71,65,84,69,87,65,89,95,73,78,84,69,82,70,65,67,69,70,97,115,116,67,71,73,47,49,46,48,14,3,82,69,81,85,69,83,84,95,77,69,84,72,79,68,71,69,84,15,30,83,67,82,73,80,84,95,70,73,76,69,78,65,77,69,47,85,115,101,114,115,47,99,103,47,100,97,116,97,47,119,119,119,47,99,103,47,115,111,114,116,46,112,104,112,11,9,83,67,82,73,80,84,95,78,65,77,69,47,115,111,114,116,46,112,104,112,12,0,81,85,69,82,89,95,83,84,82,73,78,71,11,9,82,69,81,85,69,83,84,95,85,82,73,47,115,111,114,116,46,112,104,112,12,9,68,79,67,85,77,69,78,84,95,85,82,73,47,115,111,114,116,46,112,104,112,15,14,83,69,82,86,69,82,95,83,79,70,84,87,65,82,69,112,104,112,47,102,99,103,105,99,108,105,101,110,116,11,9,82,69,77,79,84,69,95,65,68,68,82,49,50,55,46,48,46,48,46,49,11,4,82,69,77,79,84,69,95,80,79,82,84,57,57,56,53,11,9,83,69,82,86,69,82,95,65,68,68,82,49,50,55,46,48,46,48,46,49,11,2,83,69,82,86,69,82,95,80,79,82,84,56,48,11,15,83,69,82,86,69,82,95,78,65,77,69,68,69,83,75,84,79,80,45,78,67,76,50,50,71,70,15,8,83,69,82,86,69,82,95,80,82,79,84,79,67,79,76,72,84,84,80,47,49,46,49,12,0,67,79,78,84,69,78,84,95,84,89,80,69,14,1,67,79,78,84,69,78,84,95,76,69,78,71,84,72,48,6,2,65,85,84,72,79,82,99,103,1,4,22,46,0,0,0,0,1,5,22,46,0,1,0,0,0,1,3,22,46,0,8,0,0,0,0,0,0,0,0,0,0};
//    for(int i = 0; i < 435;i++){
//        char bit = one_packet2[i];
////        printf("%d",bit);
//        write(socket, &bit, sizeof(bit));
//    }
//    return;
    //    write(socket, str.c_str(), str.size());


    fd_set fdSet;
    struct timeval tv;
    FD_ZERO(&fdSet);

    while (1) {
        FD_SET(socket, &fdSet);
        sleep(1);
        tv.tv_sec = 0;
        tv.tv_usec = 0;
        int h = 0;
        h = select(socket + 1, &fdSet, NULL, NULL, &tv);
        if (h == 0) {
            continue;
        }
        if (h < 0) {
            close(socket);
            perror("select");
            break;
        }
        char buf[8];
//        memset(buf, 0, 8 * sizeof(char));
        if (h > 0) {
//            cout << "\n========================start===================" << endl;
//            read(socket, buf, 8 * sizeof(char));
            recv(socket, buf, 8 * sizeof(char), 0);
            cout << buf;
//            cout << "========================end===================" << endl;
        }
    }
}