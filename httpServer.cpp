#include <iostream>
#include <netinet/in.h>
#include <zconf.h>
#include <sys/socket.h>
#include <pthread.h>
#include <unistd.h>
#include <string>
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include "Request.h"
#include "fpm/Fpm.h"

const string HTDOCS = "/Users/cg/data/code/wheel/cpp/http-server/htdocs";
const int SERVER_PORT = 80;


typedef struct {
    int len;
    int data_len;
    string data;
} PICTURE;

typedef struct {
    int size;
    string name;
    const char *suffix;
} FILE_META;

int startup(u_short port);

void *accept_request(void *client_sock);

void get_line(int sockfd, char *buf);

string read_file(string file_path);

inline bool file_exists(const std::string &name);

bool file_is_picture(string name);

PICTURE read_picture(string file_path);

void sleep_ms(unsigned int secs);

string read_body(int socket_fd, int content_length);

bool is_dynamic_request(string filename);

// 检查是不是\r\n
bool is_lf_cr(string line);

int main() {
    int server_sock = -1;
    int client_sock;
    pthread_t newthread;
    struct sockaddr_in client_name;
    int client_name_len = sizeof(client_name);
    server_sock = startup(80);
    std::cout << "httpd running on port 80" << std::endl;
    while (1) {
        try {
            client_sock = accept(server_sock, (struct sockaddr *) &client_name, (socklen_t *) &client_name_len);
        } catch (exception exception3) {
            cout << "error";
            cout << exception3.what();
        }

        if (client_sock < 0) {
            if (errno == EINTR)
                continue;
            else {
                perror("accept error");
                return 0;
            }
        }
        void *tmp = (void *) (long) client_sock;
        if (pthread_create(&newthread, NULL, accept_request, tmp) != 0) {
            perror("pthread_create");
        }
    }

    close(server_sock);
    return 0;
}

int startup(u_short port) {
    int httpd = 0;
    struct sockaddr_in name;

    try {
        httpd = socket(PF_INET, SOCK_STREAM, 0);
    } catch (std::exception exception) {
        cout << exception.what();
    }
    memset(&name, 0, sizeof(name));
    name.sin_family = AF_INET;
    name.sin_port = htons(port);
    name.sin_addr.s_addr = htonll(INADDR_ANY);
    try {
        /******************************************************************
         * 在c++中，bind被重载。耗时几个小时才偶然找到问题所在。
         * 偶然点击bind，才发现是一个与socket完全无关的函数。
         * 之前应该遇到过。又可能是include了其他库导致此函数被重载。
         * 又耗费时间大概2个小时，很低效。
         * 找错思路：
         * 1.断点，发现卡在了accept。
         * 2.怀疑没有监听指定端口。可我无法确定本进程监听哪个端口。使用lsof -i:80
         * 并没有完全输出正在使用80端口的进程。那么，不能根据结果列表无本进程判断
         * 该进程没有监听指定端口。htons(port)后port值发生变化，差点被我当成错误。
         * 3.折腾了一些"查看进程监听端口"、"根据端口查看对应进程"的命令，无确定收获，
         * 不是记忆中那些命令。
         * 4.把其他项目中的正确代码移动到本项目运行，竟然也有同样错误。
         * 5.偶然点击bind，才发现该函数被重载了!!!!ide功不可没，能找到错误。
         * 6.这里，绝对应该对bind的结果做个判断。如果有判断，我绝对不会定位为accept
         * 错误。
         * 7.解决被重载的方法：使用::bind,  给bind函数赋予全局作用域。
         ******************************************************************/
//        bind(httpd, (struct sockaddr *) &name, sizeof(name));
        if (::bind(httpd, (struct sockaddr *) &name, sizeof(name)) < 0) {
            perror("bind ");
        }
    } catch (std::exception exception1) {
        cout << exception1.what();
    }
    listen(httpd, 5);

    return httpd;
}

void *accept_request(void *client_sock) {
    std::cout << "client_sock:" << client_sock << std::endl;

    char buf[4096];
    memset(buf, 0, 4096);
    // todo 会溢出吗？
    int tmp = (long) client_sock;
    get_line(tmp, buf);
    /***************************************
     * 尝试解决没有接收到数据的线程导致进程崩溃问题
     * 仍然无效。手动频繁刷新页面，都导致进程退出。
     * Process finished with exit code 13
     * todo 究竟是在哪里导致退出的？
     * 先写其他业务逻辑，有时间再优化。
     ***************************************/
    if (is_lf_cr(buf)) {
        close(tmp);
        return nullptr;
    }
    int size = strlen(buf);
    Request request;
    char str[64];
    int k = 0;
    /******************************************************************
     * 存储来自客户端的所有数据：请求行 + 实体主体
     * 存储为vector，是为了适配fcgi客户端相关函数对参数的要求
     *****************************************************************/
    vector<char> data_from_client;
    for (int i = 0; i < size; i++) {
        char c = buf[i];
        data_from_client.push_back(c);
        /******************************************************************
         * HTTP/1.1 与 200 之间有多个空格，这段代码能正常运行吗？
         * 在头脑中运行，不能很快得出结果。
         * 从HTTP请求行（专业术语？）第一行（即请求行）解析出请求方式、URI等。
         *****************************************************************/
        if (isspace(c)) {
            str[k] = '\0';
            if (strcasecmp(str, "GET") == 0 || strcasecmp(str, "POST") == 0 || strcasecmp(str, "PUT") == 0) {
                request.method += str;
                k = 0;
                continue;
            }
            if (strcasecmp(str, "HTTP/1.1") == 0) {
                request.http_version += str;
                break;
            }
            if (strncasecmp(str, "/", 1) == 0) {
                /****************************************************
                 * 从RUI中分离出查询字符串，即?t=2这类字符串。
                 * todo 有些URL并不用?t=2这类字符串传递查询字符串，而是用/t/2
                 * 这种形式的字符串传递查询参数，该如何处理？
                 ****************************************************/
                char *query_str = strrchr(str, '?');
                // 这个判断很必要，否则query_str++会越界
                if (query_str != NULL && strlen(query_str) > 1) {
                    // 过滤?，这种获取字符串的方法在本项目多次出现，可以封装了
                    query_str++;
                    request.query_string = query_str;
                }
                // 截取uri中?前面的部分当作file_path
                // 以后自己写代码，写详细注释，可在另外分支去掉详细注释
                for (int i = 0; i < strlen(str); i++) {
                    if (str[i] == '?') {
                        str[i] = '\0';
                        break;
                    }
                }
                request.file_path += str;
                k = 0;
                continue;
            }
            // 处理这种情况：HTTP/1.1 与 200 之间有多个空格
            k = 0;
            continue;
        }
        str[k++] = c;
    }

    std::cout << buf;

    /*****************************************************
     * 读取剩余的所有的请求行。
     * 用不同方法读取请求行与实体主体，有两点原因：
     * 1. 请求行，我过滤掉了原始字符串中的\r和\n，进行了格式化。
     * 2. 实体主体中可能包含\r和\n，get_line不能完整读取实体主体。
     *****************************************************/
    while (1) {
        get_line(tmp, buf);
        // 收集FastCGI服务器需要的数据
        for (int i = 0; i < strlen(buf); i++) {
            data_from_client.push_back(buf[i]);
        }
        string line = string(buf);
        // todo 可以这样初始化数组吗？
        // 反正这里可以执行，无警告
        char tmp[] = "Content-Length";
        if (strncasecmp(line.c_str(), tmp, strlen(tmp)) == 0) {
            const char *content_length = strrchr(line.c_str(), ':');
            // 这里其实应该判断content_length是否为NULL，先算了
            // 忽略冒号:
            content_length++;
            // content_length是字符串形式的数字，过滤Content-Length: 789中可能存在的空字符串
            for (int i = 0; i < strlen(content_length); i++) {
                if (strcasecmp(&content_length[i], "") == 0) {
                    content_length++;
                } else {
                    break;
                }
            }
            request.content_length = content_length;
        }
        // 打印请求行
//        std::cout << line;
        if (is_lf_cr(line)) {
            break;
        }
    }

    string full_file_path = HTDOCS + request.file_path;
    // 读取实体主体
    int content_length = atoi(request.content_length.c_str());
    if (content_length > 0) {
        // content_length == 0时，recv会不停地接收空字符串。原因不明。
        // 如果没有上文的的if，进程会在这里陷入死循环。
        string body = read_body(tmp, content_length);
        for (int i = 0; i < body.size(); i++) {
            data_from_client.push_back(body[i]);
        }
    }

    string requet_method = request.method;
    // 频繁刷新页面，遇到没有请求头的线程导致进程退出，原因不明，用此方法临时规避
    if (requet_method.size() == 0) {
        cout << "没有请求行" << endl;
        return nullptr;
    }
    if (is_dynamic_request(request.file_path)) {
        Fpm fpm;
        ParamsFromWebServer params_from_web_server;
        params_from_web_server.uri = request.file_path;
        params_from_web_server.query_string = request.file_path;
        params_from_web_server.http_body = data_from_client;
        // http 请求中会包含这个请求头吗？
        params_from_web_server.content_type = "";
        params_from_web_server.content_length = request.content_length;
        DataFromFastCGIServer data_from_fast_cgi_server = fpm.run(params_from_web_server);
        string content = data_from_fast_cgi_server.body;
        string head_line = data_from_fast_cgi_server.head_line;
        cout << head_line;
        cout << "=============================" << endl;
        cout << content;    // 打印来自FastCGI server的数据
        // todo 用sizeof，行不行？
        int content_len = content.size();
        /***************************************************************
         * 出现重复代码，不过我没有想到好的处理方法，不愿花太多时间在组织代码上，我的
         * 主要目的是学习C++，而不是组织代码。
         ****************************************************************/
        if ("GET" == requet_method) {
            strcpy(buf, "HTTP/1.1 200 OK\r\n");
            send(tmp, buf, strlen(buf), 0);
            if (content_len > 0) {
                string head =
                        "Server: cg-http-server/0.1\r\n"
                        "Connection: keep-alive\r\n";
                // FastCGI服务器返回数据若有响应行，会返回content-type
                if (head_line.size()) {
                    head += head_line;
                } else {
                    head += "Content-Type:text/html\r\n";
                }
                send(tmp, head.c_str(), head.size(), 0);
                sprintf(buf, "Content-Length:%d\r\n", content_len);
                send(tmp, buf, strlen(buf), 0);

                strcpy(buf, "\r\n");
                send(tmp, buf, strlen(buf), 0);
                send(tmp, content.c_str(), content_len, 0);
            } else {
                string response_line = "HTTP/1.1 204 No Content\r\n";
                response_line += "\r\n";
                send(tmp, response_line.c_str(), response_line.size(), 0);
            }
        } else if ("POST" == requet_method) {
            // 不知道php-fpm会返回什么，暂时只输出
            cout << content;
        }
        close(tmp);
        return nullptr;
    }
    // 静态请求
    /*****************************************************************
     * 404 也有返回实体主体。有实体主体就必须有响应头Content-Length
     ****************************************************************/
    if (!file_exists(full_file_path)) {
        string content = "404";
        string response_line = "HTTP/1.1 404 Not Found\r\n";
        sprintf(buf, "Content-Length: %lu\r\n", content.size());
        response_line += buf;
        response_line += "Server: cg-http-server/0.1\r\n";
        response_line += "\r\n";
        response_line += content;
        send(tmp, response_line.c_str(), response_line.size(), 0);
        close(tmp);
        return nullptr;
    }
    if ("GET" == requet_method) {
        int content_len;
        bool is_picture = file_is_picture(full_file_path);
        const char *content;
        /********************************************************************
         * 文本文件可以逐行读取，图片却不行。与处理HTTP请求行与实体主体一样，用不同方式读取。
         ********************************************************************/
        if (is_picture) {
            std::cout << "picture:" << client_sock << full_file_path << std::endl;
            PICTURE picture = read_picture(full_file_path);
            content = picture.data.c_str();
            content_len = picture.len;
        } else {
            std::cout << "file:" << client_sock << full_file_path << std::endl;
            string file_content = read_file(full_file_path);
            content = file_content.c_str();
//            content_len = sizeof(content);    // todo 这个方式，行不行？strlen呢？
            content_len = file_content.size();
        }

        if (content_len > 0) {
            strcpy(buf, "HTTP/1.1 200 OK\r\n");
            send(tmp, buf, strlen(buf), 0);

            struct stat buf2;
            stat(full_file_path.c_str(), &buf2);
            FILE_META file_meta;
            file_meta.size = buf2.st_size;
            file_meta.name = request.file_path;
            const char *suffix = strrchr(request.file_path.c_str(), '.');
            // todo 也应该需要加判断的，防止指针越界。
            suffix++;
            file_meta.suffix = suffix;

            string head =
                    "Server: cg-http-server/0.1\r\n"
                    "Connection: keep-alive\r\n";
            if (strcasecmp(file_meta.suffix, "jpg") == 0) {
                head += "Content-Type: image/";
                head += "jpeg\r\n";
            } else if (strcasecmp(file_meta.suffix, "png") == 0) {
                head += "Content-Type: image/";
                head += "png\r\n";
            } else {
                head += "Content-Type:text/html\r\n";
                send(tmp, head.c_str(), head.size(), 0);
            }

            if (is_picture) {
                head += "Content-Length: ";
                head += std::to_string(content_len);
                head += "\r\n";
                send(tmp, head.c_str(), head.size(), 0);
            } else {
                sprintf(buf, "Content-Length:%d\r\n", content_len);
                send(tmp, buf, strlen(buf), 0);
            }

            strcpy(buf, "\r\n");
            send(tmp, buf, strlen(buf), 0);
            send(tmp, content, content_len, 0);
        } else {
            string response_line = "HTTP/1.1 204 No Content\r\n";
            response_line += "\r\n";
            send(tmp, response_line.c_str(), response_line.size(), 0);
        }
    } else if ("POST" == requet_method) {
//        std::cout << "==============================body start=====================" << std::endl;
//        std::cout << body << std::endl;
//        std::cout << "==============================body end=====================" << std::endl;
    }

    sleep_ms(2);
    close(tmp);

    return nullptr;
}

/**********************************************************************
 * 只能读取http协议中的请求头
 * 专业术语是叫请求头吗？
 * char *buf能获取到函数内部的值。可是，我遇到过类似情况不能获得函数内部的值。
 * 我当时认为：char *buf 是函数内部的局部变量，所以离开了函数，char *buf的
 * 值就不是函数内部的值了。
 * 现在重看，这个函数，不也符合"当时认为"吗？为什么又可以？记下这个问题。
 **********************************************************************/
void get_line(int sockfd, char *buf) {
    char c;
    // i怎么可能达到8801呢？
    int i = 0;
//    cout << "i init:" << i << "\t" << "pid:" << getpid() << endl;
    /***********************************************************************
     * 这个终止条件竟然不能总是起作用。
     * 加入FastCGI代码后，在请求行接收完毕后，还接收到大量空白字符。
     * 为什么会这样？
     * 补充条件：c != 0。0是空字符。
     **********************************************************************/
    while ((recv(sockfd, &c, 1, 0) != -1) && c != '\n' && c != 0) {
//        cout << "c:" << int(c) << endl;
        if (c == '\r') {
            recv(sockfd, &c, 1, 1);
            if (c == '\n') {
                recv(sockfd, &c, 1, MSG_PEEK);
            }
            continue;
        }
        buf[i++] = c;
//        cout << "i:" << i << "\t" << "pid:" << getpid() << endl;
    }
    buf[i++] = '\r';
    buf[i++] = '\n';
    buf[i] = '\0';
}

string read_file(string file_path) {
    string data;
    std::ifstream infile;
    infile.open(file_path);
    if (!infile.is_open()) {
        return data;
    }
//    assert(infile.is_open());
    string line;
    while (getline(infile, line)) {
        data += line;
    }

    infile.close();
    return data;
}

// 这样能检测文件是否存在吗？
inline bool file_exists(const std::string &name) {
    struct stat buffer;
    return (stat(name.c_str(), &buffer) == 0);
}

bool file_is_picture(string name) {
    const char *name_char = name.c_str();
    const char *suffix = strrchr(name_char, '.');
    suffix++;
    if (strcasecmp(suffix, "jpeg") == 0) {
        return true;
    }
    if (strcasecmp(suffix, "png") == 0) {
        return true;
    }
    if (strcasecmp(suffix, "jpg") == 0) {
        return true;
    }
    return false;
}

PICTURE read_picture(string file_path) {
    string data;
    using namespace std;
    ifstream is(file_path, ios::in);
    // todo 不使用二进制读取也可以，为何要使用二进制读取呢？
//    ifstream is(file_path, ios::in | ios::binary);  // ok
    // 2. 计算图片长度
    is.seekg(0, is.end);
    int len = is.tellg();
    // 将读取指针重置到开头
    is.seekg(0, ios::beg);
    stringstream buffer;
    // todo 这里，究竟是否需要循环读取？最开始，我记得好像需要循环才能读取到全部图片数据。
    buffer << is.rdbuf();
    // todo 这里不是把局部变量当作函数返回值了吗？为何无警告？
    PICTURE picture = {len, len, buffer.str()};
    is.close();
    // 到此，图片已经成功的被读取到内存（buffer）中
    return picture;
}

void sleep_ms(unsigned int secs) {
    struct timeval tval;
    tval.tv_sec = secs / 100000;
    tval.tv_usec = (secs * 100000) % 1000000;
    select(0, NULL, NULL, NULL, &tval);
}

string read_body(int socket_fd, int content_length) {
    string body;
    char data[1024];
    while (recv(socket_fd, data, sizeof(char) * 1024, 0) != -1) {
        body += data;
        if (body.size() == content_length) {
            break;
        }
    }

    return body;
}

bool is_dynamic_request(string filename) {
    // 不明白为何要加上const，ide提示需要这么做。
    const char *suffix = strrchr(filename.c_str(), '.');
    suffix++;
    if (strcasecmp(suffix, "php") == 0) {
        return true;
    }
    return false;
}

bool is_lf_cr(string line) {
    /******************************************************
     * todo 有问题，那么，如何比较buf呢？之前为什么没有问题？
     * 没有加入FastCGI客户端代码前，没有问题。也许是运行次数不够多，
     * 没有暴露出来。
     * char line[3];
     * line[0] = '\r';
     * line[1] = '\n';
     * line[2] = '\0';
     * todo 运行证明，line 不等于 "\r\n"字符串。原因，未知。
     ******************************************************/
//    if (line == "\r\n") {
//         break;
//    }
    char end_flag[] = "\r\n";
    /*************************************************************************
     * strlen(end_flag) 的执行结果是2，这个判断，strlen(end_flag) - 1 ，我期望它
     * 的结果是2，刚好和line的前两个字符比较。实际上，只比较了line的前一个字符。
     * todo 我记得，好像说是字符串会自动给末尾加上'\0'。有时间弄明白这点。
     *************************************************************************/
    if (strncasecmp(line.c_str(), end_flag, strlen(end_flag)) == 0) {
        return true;
    }
    return false;
}

// error
PICTURE read_picture2(string file_path) {
    //图像数据长度
    int length;
    //文件指针
    FILE *fp;
    //输入要读取的图像名
    //以二进制方式打开图像
    if ((fp = fopen(file_path.c_str(), "rb")) == NULL) {
        std::cout << "Open image failed!" << std::endl;
        exit(0);
    }
    //获取图像数据总长度
    fseek(fp, 0, SEEK_END);
    length = ftell(fp);
    rewind(fp);
    //根据图像数据长度分配内存buffer
    char *ImgBuffer = (char *) malloc(length * sizeof(char));
    //将图像数据读入buffer
    fread(ImgBuffer, length, 1, fp);
    fclose(fp);
    PICTURE picture = {length, sizeof(ImgBuffer), ImgBuffer};
    return picture;
}

// error
PICTURE read_picture3(string file_path) {
    string data;
    using namespace std;
    ifstream is(file_path, ios::in);
    // 2. 计算图片长度
//    is.tellg();
    is.seekg(0, is.end);
    int len = is.tellg();
    is.seekg(0, ios::beg);
    // 3. 创建内存缓存区
    char *buffer = new char[1024];
//    string buffer;
    int len_data = 0;
    while (!is.eof()) {
        is.read(buffer, 1024 * sizeof(char));
        len_data += is.gcount();
        data += buffer;
    }
    is.close();
//    // 4. 读取图片
//    while (is.read(buffer, sizeof(buffer))) {
//        data += buffer;
//        if (is.eof()) {
//            break;
//        }
//    }
    PICTURE picture = {len_data, len, data};
    // 到此，图片已经成功的被读取到内存（buffer）中
    delete[] buffer;

    return picture;
}
