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

const string HTDOCS = "/Users/cg/data/code/wheel/cpp/http-server/htdocs";

typedef struct {
    int len;
    int data_len;
    string data;
} PICTURE;

int startup(u_short port);

void *accept_request(void *client_sock);

void get_line(int sockfd, char *buf);

string read_file(string file_path);

inline bool file_exists(const std::string &name);

bool file_is_picture(string name);

PICTURE read_picture(string file_path);

int main() {
    std::cout << "hello,world.I am a web server." << std::endl;
    int server_sock = -1;
    int client_sock;
    pthread_t newthread;
    struct sockaddr_in client_name;
    int client_name_len = sizeof(client_name);
    server_sock = startup(80);
    std::cout << "httpd running on port 80" << std::endl;
    while (1) {
        std::cout << 2 << std::endl;
        client_sock = accept(server_sock, (struct sockaddr *) &client_name, (socklen_t *) &client_name_len);
        std::cout << client_sock << std::endl;
        if (client_sock == -1) {
            perror("accept");
        }
        std::cout << "client_sock:";
        std::cout << client_sock << std::endl;
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
        exception.what();
    }
    memset(&name, 0, sizeof(name));
    name.sin_family = AF_INET;
    name.sin_port = htons(port);
    name.sin_addr.s_addr = htonll(INADDR_ANY);
    try {
        bind(httpd, (struct sockaddr *) &name, sizeof(name));
    } catch (std::exception exception1) {
        exception1.what();
    }
    listen(httpd, 5);
    std::cout << "httpd:";
    std::cout << httpd << std::endl;

    return httpd;
}

void *accept_request(void *client_sock) {
    std::cout << "client_sock ";
    std::cout << client_sock << std::endl;

    char buf[512];
    // todo 会溢出吗？
    int tmp = (long) client_sock;
    get_line(tmp, buf);
    std::cout << buf << std::endl;
    int size = strlen(buf);
    Request request;
    char str[64];
    int k = 0;
    for (int i = 0; i < size; i++) {
        char c = buf[i];
        if (isspace(c)) {
            str[k] = '\0';
            if (strcasecmp(str, "GET") == 0) {
                request.method += str;
                k = 0;
                continue;
            }
            if (strcasecmp(str, "HTTP/1.1") == 0) {
                request.http_version += str;
                break;
            }
            if (strncasecmp(str, "/", 1) == 0) {
                request.file_path += str;
                k = 0;
                continue;
            }
        }
        str[k++] = c;
    }

    std::cout << "method:";
    std::cout << request.method << std::endl;
    std::cout << request.file_path << std::endl;
    std::cout << request.http_version << std::endl;
    string full_file_path = HTDOCS + request.file_path;
    /*****************************************************************
     * 404 也有返回实体主体。有实体主体就必须有响应头Content-Length
     ****************************************************************/
    if (!file_exists(full_file_path)) {
        string content = "404";
        string response_line = "HTTP/1.1 404 Not Found\r\n";
        sprintf(buf, "Content-Length: %lu\r\n", content.size());
        response_line += buf;
        response_line += "\r\n";
        response_line += content;
        std::cout << response_line.c_str() << std::endl;
        send(tmp, response_line.c_str(), response_line.size(), 0);
        return nullptr;
    }

    if (request.method == "GET") {
        int content_len;
        int is_picture = file_is_picture(full_file_path);
        const char *content;
        if (is_picture) {
            PICTURE picture = read_picture(full_file_path);
            content = picture.data.c_str();
            content_len = picture.len;
        } else {
            string file_content = read_file(full_file_path);
            content = file_content.c_str();
            content_len = sizeof(content);
        }

        if (content_len > 0) {
            strcpy(buf, "HTTP/1.1 200 OK\r\n");
            send(tmp, buf, strlen(buf), 0);

            if (is_picture) {
//                strcpy(buf, "Content-Type: image/jpeg\r\n");
//                std::cout << buf;
//                send(tmp, buf, strlen(buf), 0);

                string head =
                        "Server: nginx\n"
                        "Date: Sat, 09 May 2020 03:57:07 GMT\n"
                        "Content-Type: image/jpeg\n";
                head += "Content-Length: ";
                head += std::to_string(content_len);
                head += "\n";
                head += "Last-Modified: Wed, 20 Nov 2019 07:27:46 GMT\n"
                        "Connection: closed\n"
                        "ETag: \"5dd4eaf2-1a28d\"\n"
                        "Accept-Ranges: bytes\n";

                send(tmp, head.c_str(), head.size(), 0);

//                std::cout << "===================== start ===================" << std::endl;
//                std::cout << head.c_str();
//                std::cout << "===================== end ===================" << std::endl;


            } else {
                // do nothing

            }
//            sprintf(buf, "Content-Length:%d\r\n", content_len);
//            std::cout << buf;
//            send(tmp, buf, strlen(buf), 0);
            strcpy(buf, "\r\n");
//            std::cout << buf;
            send(tmp, buf, strlen(buf), 0);
//            std::cout << "content:";
//            std::cout << content;
            send(tmp, content, content_len, 0);
        } else {
            string response_line = "HTTP/1.1 204 No Content\r\n";
            response_line += "\r\n";
            send(tmp, response_line.c_str(), response_line.size(), 0);
        }
    }

    return nullptr;
}

void get_line(int sockfd, char *buf) {
    char c;
    int i = 0;
    while ((recv(sockfd, &c, 1, 0) != -1) && c != '\n') {
        if (c == '\r') {
            recv(sockfd, &c, 1, 1);
            if (c == '\n') {
                recv(sockfd, &c, 1, MSG_PEEK);
            }
            continue;
        }
        buf[i++] = c;
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

    return data;
}

inline bool file_exists(const std::string &name) {
    struct stat buffer;
    return (stat(name.c_str(), &buffer) == 0);
}

bool file_is_picture(string name) {

    return true;
}


PICTURE read_picture(string file_path) {
    string data;
    using namespace std;
    ifstream is(file_path, ios::in);
//    ifstream is(file_path, ios::in | ios::binary);  // ok
    // 2. 计算图片长度
    is.seekg(0, is.end);
    int len = is.tellg();
    is.seekg(0, ios::beg);
    stringstream buffer;
    buffer << is.rdbuf();
    PICTURE picture = {len, len, buffer.str()};
    // 到此，图片已经成功的被读取到内存（buffer）中
    return picture;
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
