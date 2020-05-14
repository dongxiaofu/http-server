#include "Fpm.h"

DataFromFastCGIServer Fpm::run(ParamsFromWebServer params_from_web_server) {
    vector<char> one_packet;
    create_packet(&one_packet, params_from_web_server);
    Network network;
    int socket = network.client_socket("127.0.0.1", CLIENT_PORT);
    send_packet(socket, one_packet);
    DataFromFastCGIServer data_from_fast_cgi_server = receive_data_from_server(socket);
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
    // 下面写法是用begin_request_body_packet覆盖one_packet，与意图符合
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
    fcgi.fcgi_packet(fcgiRequestType.FCGI_STDIN, request_id, zero, zero.size(), &fcgi_stdin);
    one_packet->insert(one_packet->end(), fcgi_stdin.begin(), fcgi_stdin.end());
    // 5.发送实体主体结束标志数据报
    vector<char> end_request_body;
    // todo 作用是什么？不都是存储0吗？为何要做移位运算？
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
    // 一个完整数据报
    one_packet->insert(one_packet->end(), end_request.begin(), end_request.end());
}

void Fpm::send_packet(int socket_fd, vector<char> packet) {
    for (int i = 0; i < packet.size(); i++) {
        char bit = packet[i];
        write(socket_fd, &bit, sizeof(char));
    }
}

DataFromFastCGIServer Fpm::receive_data_from_server(int socket_fd) {
    DataFromFastCGIServer data_from_fast_cgi_server;
    fd_set fdSet;
    struct timeval tv;
    FD_ZERO(&fdSet);
    while (1) {
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
        char buf[8];
//        memset(buf, 0, 8 * sizeof(char));     // todo 似乎可有可无？我执行代码，发现好像是这样。
        int n;
        // todo recv接收完合法数据后，竟然还会不停接收空白字符串。神奇，若不主动断开，陷入死循环。
        n = recv(socket_fd, buf, 8 * sizeof(char), 0);
        if (n == -1) {
            return data_from_fast_cgi_server;
        }
        cout << "n:" << n << endl;
        cout << "version:" << (int) buf[0] << endl;
        cout << "type:" << (int) buf[1] << endl;
        cout << "id:" << (int) ((buf[2] & 0xff) << 8) + (int) ((buf[3] & 0xff)) << endl;
        cout << "content_length:" << (int) ((buf[4] & 0xff) << 8) + (int) ((buf[5] & 0xff)) << endl;
        int type = (int) buf[1];
        // 数据报不上标准输出和标准错误输出，停止接收数据
        if (type != FcgiRequestType::FCGI_STDOUT && type != FcgiRequestType::FCGI_STDERR) {
            return data_from_fast_cgi_server;
        }
        int content_length = (int) ((buf[4] & 0xff) << 8) + (int) ((buf[5] & 0xff));
        char content[content_length];
        n = read(socket_fd, content, content_length);
        if (n == -1) {
            return data_from_fast_cgi_server;
        }
        char *ptr_content = new char[strlen(content)];
        strcpy(ptr_content, content);
        if (FcgiRequestType::FCGI_STDERR == type) {
            // 没有响应头
            data_from_fast_cgi_server.body = content;
        } else if (FcgiRequestType::FCGI_STDOUT == type) {
            string head = "";
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
//                c = content[i];

            }
            // 上面的if语句把最后那行响应行后面的换行符也跳过了，需要补上。
            // 更新，上面那行注释，是错误的。不需要下面两行代码。
//            head += '\r';
//            head += '\n';
            // comparison between NULL and non-pointer ('int' and NULL) [-Wnull-arithmetic]
//            if (i == NULL) {
//                i = 0;
//            }
//            char body[content_length - i];        // todo 不能这样用数组
            char *body = new char[content_length - i];
//            strncpy(body, content, i + 1);    // 错误，最多复制i+1，而不是从i+1开始复制
            strcpy(body, content + i + 1);
            data_from_fast_cgi_server.head_line = head;
            data_from_fast_cgi_server.body = body;
        }

        /******************************************************
         * todo 这里停止接收数据是否正确？有没有遗漏这种情况：
         * 在一个页面，同时出现错误信息和正常输出结果。这种情况，FastCGI
         * 会先后返回FCGI_STDOUT和FCGI_STDERR数据报吗？
         * 有空再验证并完善。
         ******************************************************/
        return data_from_fast_cgi_server;
    }
    return data_from_fast_cgi_server;
}