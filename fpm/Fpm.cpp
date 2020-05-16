#include "Fpm.h"

DataFromFastCGIServer *Fpm::run(ParamsFromWebServer params_from_web_server) {
    cout << "============params_from_web_server start============" << endl;
    if (params_from_web_server.http_body.size()) {
        char *data = params_from_web_server.http_body.data();
        cout << data << endl;
    } else {
        cout << "没有数据" << endl;
    }
    cout << "============params_from_web_server end============" << endl;
    vector<char> one_packet;
    create_packet(&one_packet, params_from_web_server);
    Network network;
    int socket = network.client_socket("127.0.0.1", CLIENT_PORT);
//    send_packet(socket, one_packet);
    send_packet_test(socket);
    DataFromFastCGIServer *data_from_fast_cgi_server = receive_data_from_server(socket);
    /******************************************************************
     * todo
     * 不理解。
     * 没有close(socket)，就会出现：在第二次请求PHP文件时，被卡住。
     * 原因是什么？
     * 喜欢这种注释方式，开始不喜欢//这种风格的注释方式了。
     ******************************************************************/
    close(socket);
    return data_from_fast_cgi_server;
}

void Fpm::create_packet(vector<char> *one_packet, ParamsFromWebServer params_from_web_server) {
    FcgiRole fcgiRole;
    Fcgi fcgi;
    FcgiRequestType fcgiRequestType;
    // todo 有空，再弄成由web服务器产生
    int request_id = 5679;
    vector<char> body = params_from_web_server.http_body;
    // 1.FCGI_BEGIN_REQUEST，请求开始
    vector<char> begin_request_body;
    // todo 对vector能这样赋值吗？对空的，似乎不能吧？有空再验证。
//    begin_request_body[0] = 0; // roleB1
//    begin_request_body[1] = fcgiRole.FCGI_RESPONDER; //roleB0
//    begin_request_body[2] = 0; // flags
    begin_request_body.push_back(0);
    begin_request_body.push_back(fcgiRole.FCGI_RESPONDER);
    begin_request_body.push_back(0);
    for (int i = 0; i < 5; i++) {
        begin_request_body.push_back(0);
    }
    // todo 当时没找到使用函数返回局部变量的方法，估使用引用传值获取函数处理结果
    // &begin_request_body_packet
    vector<char> begin_request_body_packet;
    fcgi.fcgi_packet(fcgiRequestType.FCGI_BEGIN_REQUEST, request_id, begin_request_body, begin_request_body.size(),
                     &begin_request_body_packet);
    // todo 拼接多个vector的方法
    one_packet->insert(one_packet->end(), begin_request_body_packet.begin(), begin_request_body_packet.end());
    // 下面写法是用begin_request_body_packet覆盖one_packet，与意图不符合
    //    one_packet->assign(begin_request_body_packet.begin(), begin_request_body_packet.end());
    //2.params
    map<string, string> params;
    string uri = params_from_web_server.uri;
    string content_type = params_from_web_server.content_type;
    params["GATEWAY_INTERFACE"] = "FastCGI/1.0";
    params["REQUEST_METHOD"] = params_from_web_server.method;
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
    params["CONTENT_LENGTH"] = body.size();//params_from_web_server.content_length;
    params["AUTHOR"] = "cg";

    cout<<"============ debug log start=============="<<endl;
    cout<<"length:"<<params_from_web_server.content_length<<endl;
    cout<<"actual_len:"<<params_from_web_server.http_body.size()<<endl;
    cout<<"============ debug log end=============="<<endl;

    vector<char> pairs;
    // todo 遍历map的方法，与遍历vector相似，差异在获取单元值
    for (map<string, string>::iterator it = params.begin(); it != params.end(); it++) {
        fcgi.fcgi_param(it->first, it->second, &pairs);
    }

    vector<char> fcgi_params;
    fcgi.fcgi_packet(fcgiRequestType.FCGI_PARAMS, request_id, pairs, pairs.size(), &fcgi_params);
    one_packet->insert(one_packet->end(), fcgi_params.begin(), fcgi_params.end());
    //    3. 发送参数结束
    // todo 构造数据报时，在这里犯了错误，耗时十几个小时，才低效地发现错误。
    // todo 原因是对FastCGI协议不熟悉，看Java代码又不仔细，FastCGI服务器又是个黑盒，面对错误数据报，什么也不输出。
    vector<char> fcgi_params_end;
    vector<char> zero;
//    zero.push_back(0);    // todo 致命耗时错误
    fcgi.fcgi_packet(fcgiRequestType.FCGI_PARAMS, request_id, zero, zero.size(), &fcgi_params_end);
    one_packet->insert(one_packet->end(), fcgi_params_end.begin(), fcgi_params_end.end());

    // 4. 实体主体，在这里传递
    vector<char> fcgi_stdin;
    fcgi.fcgi_packet(fcgiRequestType.FCGI_STDIN, request_id, body, body.size(), &fcgi_stdin);
    one_packet->insert(one_packet->end(), fcgi_stdin.begin(), fcgi_stdin.end());

    vector<char> fcgi_stdin_end;
    fcgi.fcgi_packet(fcgiRequestType.FCGI_STDIN, request_id, zero, 0, &fcgi_stdin_end);
    one_packet->insert(one_packet->end(), fcgi_stdin_end.begin(), fcgi_stdin_end.end());

    // 5.发送实体主体结束标志数据报
    vector<char> end_request_body;
    // todo 作用是什么？不都是存储0吗？为何要做移位运算？
//    end_request_body.push_back((int) ((0 >> 24) & 0xff));
//    end_request_body.push_back((int) ((0 >> 16) & 0xff));
//    end_request_body.push_back((int) ((0 >> 8) & 0xff));
//    end_request_body.push_back((int) (0 & 0xff));
    end_request_body.push_back((char) ((0 >> 24) & 0xff));
    end_request_body.push_back((char) ((0 >> 16) & 0xff));
    end_request_body.push_back((char) ((0 >> 8) & 0xff));
    end_request_body.push_back((char) (0 & 0xff));
    end_request_body.push_back(0);
    for (int i = 0; i < 3; i++) {
        end_request_body.push_back(0);
    }
    vector<char> end_request;
    fcgi.fcgi_packet(fcgiRequestType.FCGI_END_REQUEST, request_id, end_request_body, end_request_body.size(),
                     &end_request);
    // 一个完整数据报
    one_packet->insert(one_packet->end(), end_request.begin(), end_request.end());
}

void Fpm::send_packet(int socket_fd, vector<char> packet) {
    for (int i = 0; i < packet.size(); i++) {
        char bit = packet[i];
        write(socket_fd, &bit, sizeof(char));
    }
}

DataFromFastCGIServer *Fpm::receive_data_from_server(int socket_fd) {
    DataFromFastCGIServer *data_from_fast_cgi_server = new DataFromFastCGIServer;
    fd_set fdSet;
    struct timeval tv;
    while (1) {
        FD_ZERO(&fdSet);
        FD_SET(socket_fd, &fdSet);
        tv.tv_sec = 0;
        tv.tv_usec = 0;
        int h = 0;
        h = select(socket_fd + 1, &fdSet, NULL, NULL, &tv);
        if (h == 0) {
            continue;
        }
        if (h < 0) {
            perror("select");
            return data_from_fast_cgi_server;
        }
        char c;
        string head = "";
        string content;
        int counter = 0;
        int type;
        while ((recv(socket_fd, &c, sizeof(char), 0)) != -1) {
            // 忽略数据报的head
            if (counter++ < 8) {
                // 第二位是数据报type
                if (counter == 2) {
                    type = (int) c;
                }
                continue;
            }
            if (c == '\0') {
                recv(socket_fd, &c, 1, MSG_PEEK);
                if (c == '\0') {
                    recv(socket_fd, &c, 1, 0);
                    recv(socket_fd, &c, 1, MSG_PEEK);
                    if (c == '\0') {
                        recv(socket_fd, &c, 1, 0);
                        recv(socket_fd, &c, 1, MSG_PEEK);
                        if (c == '\0') {
                            recv(socket_fd, &c, 1, 0);
                            recv(socket_fd, &c, 1, MSG_PEEK);
                            if (c == '\0') {
                                recv(socket_fd, &c, 1, 0);
                                recv(socket_fd, &c, 1, MSG_PEEK);
                                if (c == '\0') {
                                    cout << "last" << endl;
                                    break;
                                }
                            }
                        }
                    }
                }
            }
            if (c == '\0' || c < 0) {
                continue;
            }
            content += c;
        }

        if (type == FcgiRequestType::FCGI_STDERR) {
            data_from_fast_cgi_server->body = content;

            return data_from_fast_cgi_server;
        }

        int content_length = content.size();

        int i = 0;
        for (i = 0; i < content_length; i++) {
            char c = content[i];
            head += c;
            // 分离FastCIG服务器返回数据中的响应行与实体主体，耗费了非常多时间。
            // 细小问题，边断点边调试，解决速度更快。我不应该心算。
            // 若满足这串if条件，\r\n\r\n没有写进head
            // 这个分割响应头与实体主体的操作，我想了大概十多分钟，才认为下面的代码是正确的。
            // 没有运行，心算的。
            // 可以用 strtok 实现需求，不必像我这样写这样麻烦
            // strtok https://www.runoob.com/cprogramming/c-function-strtok.html
            if ('\r' == c) {
                i++;
                c = content[i];
                // 不能漏掉非最后一行响应行的\r\n
//                    head += c;
                if ('\n' == c) {
                    head += c;
                    i++;
                    c = content[i];
                    if ('\r' == c) {
                        i++;
                        c = content[i];
                        if ('\n' == c) {
                            break;
                        }
                    } else {
                        // 防止中间行的非\r\n遗漏
                        head += c;
                    }
                }
            }
        }

        char *body = new char[content_length - i];
        strcpy(body, content.c_str() + i + 1);

        data_from_fast_cgi_server->head_line = head;
        data_from_fast_cgi_server->body = body;

        return data_from_fast_cgi_server;
    }
    /******************************************************
     * todo 这里停止接收数据是否正确？有没有遗漏这种情况：
     * 在一个页面，同时出现错误信息和正常输出结果。这种情况，FastCGI
     * 会先后返回FCGI_STDOUT和FCGI_STDERR数据报吗？
     * 有空再验证并完善。
     ******************************************************/
}

void Fpm::send_packet_test(int socket_fd) {
    char packet[1113] = {1, 1, 22, 46, 0, 8, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 4, 22, 46, 1, -90, 0, 0, 17, 11, 71, 65,
                         84, 69, 87, 65, 89, 95, 73, 78, 84, 69, 82, 70, 65, 67, 69, 70, 97, 115, 116, 67, 71, 73, 47,
                         49, 46, 48, 14, 4, 82, 69, 81, 85, 69, 83, 84, 95, 77, 69, 84, 72, 79, 68, 80, 79, 83, 84, 15,
                         32, 83, 67, 82, 73, 80, 84, 95, 70, 73, 76, 69, 78, 65, 77, 69, 47, 85, 115, 101, 114, 115, 47,
                         99, 103, 47, 100, 97, 116, 97, 47, 119, 119, 119, 47, 99, 103, 47, 115, 101, 114, 118, 101,
                         114, 46, 112, 104, 112, 11, 11, 83, 67, 82, 73, 80, 84, 95, 78, 65, 77, 69, 47, 115, 101, 114,
                         118, 101, 114, 46, 112, 104, 112, 12, 0, 81, 85, 69, 82, 89, 95, 83, 84, 82, 73, 78, 71, 11,
                         11, 82, 69, 81, 85, 69, 83, 84, 95, 85, 82, 73, 47, 115, 101, 114, 118, 101, 114, 46, 112, 104,
                         112, 12, 11, 68, 79, 67, 85, 77, 69, 78, 84, 95, 85, 82, 73, 47, 115, 101, 114, 118, 101, 114,
                         46, 112, 104, 112, 15, 14, 83, 69, 82, 86, 69, 82, 95, 83, 79, 70, 84, 87, 65, 82, 69, 112,
                         104, 112, 47, 102, 99, 103, 105, 99, 108, 105, 101, 110, 116, 11, 9, 82, 69, 77, 79, 84, 69,
                         95, 65, 68, 68, 82, 49, 50, 55, 46, 48, 46, 48, 46, 49, 11, 4, 82, 69, 77, 79, 84, 69, 95, 80,
                         79, 82, 84, 57, 57, 56, 53, 11, 9, 83, 69, 82, 86, 69, 82, 95, 65, 68, 68, 82, 49, 50, 55, 46,
                         48, 46, 48, 46, 49, 11, 2, 83, 69, 82, 86, 69, 82, 95, 80, 79, 82, 84, 56, 48, 11, 15, 83, 69,
                         82, 86, 69, 82, 95, 78, 65, 77, 69, 68, 69, 83, 75, 84, 79, 80, 45, 78, 67, 76, 50, 50, 71, 70,
                         15, 8, 83, 69, 82, 86, 69, 82, 95, 80, 82, 79, 84, 79, 67, 79, 76, 72, 84, 84, 80, 47, 49, 46,
                         49, 12, 33, 67, 79, 78, 84, 69, 78, 84, 95, 84, 89, 80, 69, 97, 112, 112, 108, 105, 99, 97,
                         116, 105, 111, 110, 47, 120, 45, 119, 119, 119, 45, 102, 111, 114, 109, 45, 117, 114, 108, 101,
                         110, 99, 111, 100, 101, 100, 14, 3, 67, 79, 78, 84, 69, 78, 84, 95, 76, 69, 78, 71, 84, 72, 49,
                         55, 51, 6, 2, 65, 85, 84, 72, 79, 82, 99, 103, 1, 4, 22, 46, 0, 0, 0, 0, 1, 5, 22, 46, 2, 123,
                         0, 0, 80, 79, 83, 84, 32, 47, 115, 101, 114, 118, 101, 114, 46, 112, 104, 112, 32, 72, 84, 84,
                         80, 47, 49, 46, 49, 13, 10, 72, 111, 115, 116, 58, 32, 116, 101, 115, 116, 46, 99, 111, 109,
                         58, 50, 48, 48, 48, 13, 10, 67, 111, 110, 110, 101, 99, 116, 105, 111, 110, 58, 32, 107, 101,
                         101, 112, 45, 97, 108, 105, 118, 101, 13, 10, 67, 111, 110, 116, 101, 110, 116, 45, 76, 101,
                         110, 103, 116, 104, 58, 32, 49, 55, 51, 13, 10, 67, 97, 99, 104, 101, 45, 67, 111, 110, 116,
                         114, 111, 108, 58, 32, 109, 97, 120, 45, 97, 103, 101, 61, 48, 13, 10, 85, 112, 103, 114, 97,
                         100, 101, 45, 73, 110, 115, 101, 99, 117, 114, 101, 45, 82, 101, 113, 117, 101, 115, 116, 115,
                         58, 32, 49, 13, 10, 79, 114, 105, 103, 105, 110, 58, 32, 104, 116, 116, 112, 58, 47, 47, 116,
                         101, 115, 116, 46, 99, 111, 109, 58, 50, 48, 48, 48, 13, 10, 67, 111, 110, 116, 101, 110, 116,
                         45, 84, 121, 112, 101, 58, 32, 97, 112, 112, 108, 105, 99, 97, 116, 105, 111, 110, 47, 120, 45,
                         119, 119, 119, 45, 102, 111, 114, 109, 45, 117, 114, 108, 101, 110, 99, 111, 100, 101, 100, 13,
                         10, 85, 115, 101, 114, 45, 65, 103, 101, 110, 116, 58, 32, 77, 111, 122, 105, 108, 108, 97, 47,
                         53, 46, 48, 32, 40, 77, 97, 99, 105, 110, 116, 111, 115, 104, 59, 32, 73, 110, 116, 101, 108,
                         32, 77, 97, 99, 32, 79, 83, 32, 88, 32, 49, 48, 95, 49, 53, 95, 49, 41, 32, 65, 112, 112, 108,
                         101, 87, 101, 98, 75, 105, 116, 47, 53, 51, 55, 46, 51, 54, 32, 40, 75, 72, 84, 77, 76, 44, 32,
                         108, 105, 107, 101, 32, 71, 101, 99, 107, 111, 41, 32, 67, 104, 114, 111, 109, 101, 47, 56, 49,
                         46, 48, 46, 52, 48, 52, 52, 46, 49, 51, 56, 32, 83, 97, 102, 97, 114, 105, 47, 53, 51, 55, 46,
                         51, 54, 13, 10, 65, 99, 99, 101, 112, 116, 58, 32, 116, 101, 120, 116, 47, 104, 116, 109, 108,
                         44, 97, 112, 112, 108, 105, 99, 97, 116, 105, 111, 110, 47, 120, 104, 116, 109, 108, 43, 120,
                         109, 108, 44, 97, 112, 112, 108, 105, 99, 97, 116, 105, 111, 110, 47, 120, 109, 108, 59, 113,
                         61, 48, 46, 57, 44, 105, 109, 97, 103, 101, 47, 119, 101, 98, 112, 44, 105, 109, 97, 103, 101,
                         47, 97, 112, 110, 103, 44, 42, 47, 42, 59, 113, 61, 48, 46, 56, 44, 97, 112, 112, 108, 105, 99,
                         97, 116, 105, 111, 110, 47, 115, 105, 103, 110, 101, 100, 45, 101, 120, 99, 104, 97, 110, 103,
                         101, 59, 118, 61, 98, 51, 59, 113, 61, 48, 46, 57, 13, 10, 82, 101, 102, 101, 114, 101, 114,
                         58, 32, 104, 116, 116, 112, 58, 47, 47, 116, 101, 115, 116, 46, 99, 111, 109, 58, 50, 48, 48,
                         48, 47, 102, 111, 114, 109, 46, 104, 116, 109, 108, 13, 10, 65, 99, 99, 101, 112, 116, 45, 69,
                         110, 99, 111, 100, 105, 110, 103, 58, 32, 103, 122, 105, 112, 44, 32, 100, 101, 102, 108, 97,
                         116, 101, 13, 10, 65, 99, 99, 101, 112, 116, 45, 76, 97, 110, 103, 117, 97, 103, 101, 58, 32,
                         122, 104, 45, 67, 78, 44, 122, 104, 59, 113, 61, 48, 46, 57, 44, 101, 110, 45, 85, 83, 59, 113,
                         61, 48, 46, 56, 44, 101, 110, 59, 113, 61, 48, 46, 55, 44, 106, 97, 59, 113, 61, 48, 46, 54,
                         13, 10, 13, 10, 1, 3, 22, 46, 0, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    for (int i = 0; i < 1113; i++) {
        char bit = packet[i];
        write(socket_fd, &bit, sizeof(char));
    }
}