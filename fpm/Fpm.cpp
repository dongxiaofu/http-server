#include "Fpm.h"

void Fpm::run() {
    FcgiRole fcgiRole;
    Fcgi fcgi;
    FcgiRequestType fcgiRequestType;
    int request_id = 5678;
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
    fcgi.fcgi_packet(fcgiRequestType.FCGI_BEGIN_REQUEST, request_id, begin_request_body, 8,
                     &begin_request_body_packet);
    one_packet.insert(one_packet.end(), begin_request_body_packet.begin(), begin_request_body_packet.end());
//    one_packet.assign(begin_request_body_packet.begin(), begin_request_body_packet.end());
    //2.params
    map<string, string> params;
    string uri = "/test.php";
    string content_type = "";
    params["GATEWAY_INTERFACE"] = "FastCGI/1.0";
    params["REQUEST_METHOD"] = "GET";
    params["SCRIPT_FILENAME"] = "/Users/cg/data/www/cg/test.php";
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
    map<string, string>::iterator iterator;
    for (iterator = params.begin(); iterator != params.end(); iterator++) {
        fcgi.fcgi_param(iterator->first, iterator->second, &pairs);
    }

    vector<char> fcgi_params;
    fcgi.fcgi_packet(fcgiRequestType.FCGI_PARAMS, request_id, pairs, pairs.size(), &fcgi_params);
    one_packet.insert(one_packet.end(), fcgi_params.begin(), fcgi_params.end());
//    one_packet.assign(fcgi_params.begin(), fcgi_params.end());

    vector<char> fcgi_params_end;
    vector<char> zero;
    zero.push_back(0);
    fcgi.fcgi_packet(fcgiRequestType.FCGI_PARAMS, request_id, zero, 1, &fcgi_params_end);
    one_packet.insert(one_packet.end(), fcgi_params_end.begin(), fcgi_params_end.end());
//    one_packet.assign(fcgi_params_end.begin(), fcgi_params_end.end());

    vector<char> fcgi_stdin(0);
    // test
    fcgi.fcgi_packet(fcgiRequestType.FCGI_STDIN, request_id, zero, 1, &fcgi_stdin);
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
    fcgi.fcgi_packet(fcgiRequestType.FCGI_END_REQUEST, request_id, end_request_body, 8, &end_request);
    one_packet.insert(one_packet.end(), end_request.begin(), end_request.end());
//    one_packet.assign(end_request.begin(), end_request.end());
    int a = 5;

    Network network;
    int socket = network.client_socket("127.0.0.1", 9001);
    FILE *fp = fopen("/Users/cg/data/code/wheel/java/demo/html/log/data2.log", "w");
    for(int i = 0; i < one_packet.size();i++){
        char bit = one_packet[i];
        fprintf(fp,"%d,",bit);
        int n = write(socket, &bit, sizeof(bit));
        cout << "n:" << n << endl;
    }
    fclose(fp);
//    string str = "hello";
//    int one_packet2[868] = {1,1,22,46,0,8,0,0,0,1,0,0,0,0,0,0,1,4,22,46,1,122,0,0,17,11,71,65,84,69,87,65,89,95,73,78,84,69,82,70,65,67,69,70,97,115,116,67,71,73,47,49,46,48,14,3,82,69,81,85,69,83,84,95,77,69,84,72,79,68,71,69,84,15,30,83,67,82,73,80,84,95,70,73,76,69,78,65,77,69,47,85,115,101,114,115,47,99,103,47,100,97,116,97,47,119,119,119,47,99,103,47,116,101,115,116,46,112,104,112,11,9,83,67,82,73,80,84,95,78,65,77,69,47,116,101,115,116,46,112,104,112,12,0,81,85,69,82,89,95,83,84,82,73,78,71,11,9,82,69,81,85,69,83,84,95,85,82,73,47,116,101,115,116,46,112,104,112,12,9,68,79,67,85,77,69,78,84,95,85,82,73,47,116,101,115,116,46,112,104,112,15,14,83,69,82,86,69,82,95,83,79,70,84,87,65,82,69,112,104,112,47,102,99,103,105,99,108,105,101,110,116,11,9,82,69,77,79,84,69,95,65,68,68,82,49,50,55,46,48,46,48,46,49,11,4,82,69,77,79,84,69,95,80,79,82,84,57,57,56,53,11,9,83,69,82,86,69,82,95,65,68,68,82,49,50,55,46,48,46,48,46,49,11,2,83,69,82,86,69,82,95,80,79,82,84,56,48,11,15,83,69,82,86,69,82,95,78,65,77,69,68,69,83,75,84,79,80,45,78,67,76,50,50,71,70,15,8,83,69,82,86,69,82,95,80,82,79,84,79,67,79,76,72,84,84,80,47,49,46,49,12,0,67,79,78,84,69,78,84,95,84,89,80,69,14,1,67,79,78,84,69,78,84,95,76,69,78,71,84,72,48,6,2,65,85,84,72,79,82,99,103,1,4,22,46,0,0,0,0,1,5,22,46,1,-78,0,0,71,69,84,32,47,116,101,115,116,46,112,104,112,32,72,84,84,80,47,49,46,49,13,10,72,111,115,116,58,32,116,101,115,116,46,99,111,109,58,50,48,48,48,13,10,85,115,101,114,45,65,103,101,110,116,58,32,77,111,122,105,108,108,97,47,53,46,48,32,40,77,97,99,105,110,116,111,115,104,59,32,73,110,116,101,108,32,77,97,99,32,79,83,32,88,32,49,48,46,49,53,59,32,114,118,58,55,50,46,48,41,32,71,101,99,107,111,47,50,48,49,48,48,49,48,49,32,70,105,114,101,102,111,120,47,55,50,46,48,13,10,65,99,99,101,112,116,58,32,116,101,120,116,47,104,116,109,108,44,97,112,112,108,105,99,97,116,105,111,110,47,120,104,116,109,108,43,120,109,108,44,97,112,112,108,105,99,97,116,105,111,110,47,120,109,108,59,113,61,48,46,57,44,105,109,97,103,101,47,119,101,98,112,44,42,47,42,59,113,61,48,46,56,13,10,65,99,99,101,112,116,45,76,97,110,103,117,97,103,101,58,32,122,104,45,67,78,44,122,104,59,113,61,48,46,56,44,122,104,45,84,87,59,113,61,48,46,55,44,122,104,45,72,75,59,113,61,48,46,53,44,101,110,45,85,83,59,113,61,48,46,51,44,101,110,59,113,61,48,46,50,13,10,65,99,99,101,112,116,45,69,110,99,111,100,105,110,103,58,32,103,122,105,112,44,32,100,101,102,108,97,116,101,13,10,67,111,110,110,101,99,116,105,111,110,58,32,107,101,101,112,45,97,108,105,118,101,13,10,85,112,103,114,97,100,101,45,73,110,115,101,99,117,114,101,45,82,101,113,117,101,115,116,115,58,32,49,13,10,80,114,97,103,109,97,58,32,110,111,45,99,97,99,104,101,13,10,67,97,99,104,101,45,67,111,110,116,114,111,108,58,32,110,111,45,99,97,99,104,101,13,10,13,10,1,3,22,46,0,8,0,0,0,0,0,0,0,0,0,0,};
//    for(int i = 0; i < 868;i++){
//        int bit = one_packet2[i];
//        printf("%d",bit);
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
        memset(&buf, 0, 8 * sizeof(char));
        if (h > 0) {
            read(socket, buf, 8 * sizeof(char));
            cout << buf << endl;
        }
    }
}