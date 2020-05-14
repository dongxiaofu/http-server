#include "Fpm.h"

char * Fpm::run(ParamsFromWebServer params_from_web_server) {
    vector<char> one_packet;
    create_packet(&one_packet, params_from_web_server);
    Network network;
    int socket = network.client_socket("127.0.0.1", CLIENT_PORT);
    send_packet(socket, one_packet);
    char *content = receive_data_from_server(socket);
    /******************************************************************
     * todo
     * 不理解。
     * 没有close(socket)，就会出现：在第二次请求PHP文件时，被卡住。
     * 原因是什么？
     * 喜欢这种注释方式，开始不喜欢//这种风格的注释方式了。
     ******************************************************************/
    close(socket);
    return content;
}

void Fpm::create_packet(vector<char> *one_packet, ParamsFromWebServer params_from_web_server) {
    FcgiRole fcgiRole;
    Fcgi fcgi;
    FcgiRequestType fcgiRequestType;
    int request_id = 5679;
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
    one_packet->insert(one_packet->end(), begin_request_body_packet.begin(), begin_request_body_packet.end());
//    one_packet->assign(begin_request_body_packet.begin(), begin_request_body_packet.end());
    //2.params
    map<string, string> params;
    string uri = params_from_web_server.uri;
    string content_type = params_from_web_server.content_type;
    params["GATEWAY_INTERFACE"] = "FastCGI/1.0";
    params["REQUEST_METHOD"] = "GET";
    params["SCRIPT_FILENAME"] = "/Users/cg/data/www/cg" + uri;
    params["SCRIPT_NAME"] = uri;
    params["QUERY_STRING"] = params_from_web_server.query_string;
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
    params["CONTENT_LENGTH"] = params_from_web_server.content_length;
    params["AUTHOR"] = "cg";

    vector<char> pairs;
    for (map<string, string>::iterator it = params.begin(); it != params.end(); it++) {
        fcgi.fcgi_param(it->first, it->second, &pairs);
    }

    vector<char> fcgi_params;
    fcgi.fcgi_packet(fcgiRequestType.FCGI_PARAMS, request_id, pairs, pairs.size(), &fcgi_params);
    one_packet->insert(one_packet->end(), fcgi_params.begin(), fcgi_params.end());
//    one_packet->assign(fcgi_params.begin(), fcgi_params.end());

    vector<char> fcgi_params_end;
    vector<char> zero;
//    zero.push_back(0);
    fcgi.fcgi_packet(fcgiRequestType.FCGI_PARAMS, request_id, zero, zero.size(), &fcgi_params_end);
    one_packet->insert(one_packet->end(), fcgi_params_end.begin(), fcgi_params_end.end());
//    one_packet->assign(fcgi_params_end.begin(), fcgi_params_end.end());

    vector<char> fcgi_stdin;
    // test
    fcgi.fcgi_packet(fcgiRequestType.FCGI_STDIN, request_id, zero, zero.size(), &fcgi_stdin);
    one_packet->insert(one_packet->end(), fcgi_stdin.begin(), fcgi_stdin.end());
//    one_packet->assign(fcgi_stdin.begin(), fcgi_stdin.end());

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
    fcgi.fcgi_packet(fcgiRequestType.FCGI_END_REQUEST, request_id, end_request_body, end_request_body.size(),
                     &end_request);
    one_packet->insert(one_packet->end(), end_request.begin(), end_request.end());
//    one_packet->assign(end_request.begin(), end_request.end());
}

void Fpm::send_packet(int socket_fd, vector<char> packet) {
    for (int i = 0; i < packet.size(); i++) {
        char bit = packet[i];
        write(socket_fd, &bit, sizeof(char));
    }
}

char *Fpm::receive_data_from_server(int socket_fd) {
    fd_set fdSet;
    struct timeval tv;
    FD_ZERO(&fdSet);
    while (1) {
        FD_SET(socket_fd, &fdSet);
//        sleep(1);
        tv.tv_sec = 0;
        tv.tv_usec = 0;
        int h = 0;
        h = select(socket_fd + 1, &fdSet, NULL, NULL, &tv);
        if (h == 0) {
            continue;
        }
        if (h < 0) {
            perror("select");
            return nullptr;
        }
        char buf[8];
//        memset(buf, 0, 8 * sizeof(char));
        int n;
        n = recv(socket_fd, buf, 8 * sizeof(char), 0);
        if (n == -1) {
            return nullptr;
        }
        cout << "n:" << n << endl;
        cout << "version:" << (int) buf[0] << endl;
        cout << "type:" << (int) buf[1] << endl;
        cout << "id:" << (int) ((buf[2] & 0xff) << 8) + (int) ((buf[3] & 0xff)) << endl;
        cout << "content_length:" << (int) ((buf[4] & 0xff) << 8) + (int) ((buf[5] & 0xff)) << endl;
        int type = (int) buf[1];
        if (type != FcgiRequestType::FCGI_STDOUT && type != FcgiRequestType::FCGI_STDERR) {
            return nullptr;
        }
        int content_length = (int) ((buf[4] & 0xff) << 8) + (int) ((buf[5] & 0xff));
        char content[content_length];
        n = read(socket_fd, content, content_length);
        if (n == -1) {
            return nullptr;
        }
//        cout << "content:" << content << endl;
        char *ptr_content = new char[strlen(content)];
        strcpy(ptr_content, content);
        return ptr_content;
    }
    return nullptr;
}