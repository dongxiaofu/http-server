#include <iostream>
#include <netinet/in.h>
#include <zconf.h>
#include <sys/socket.h>
#include <pthread.h>
#include <unistd.h>
#include <cstring>
#include <fstream>
#include <sys/stat.h>
#include "Request.h"

const string HTDOCS = "/Users/cg/data/code/wheel/cpp/http-server/htdocs";

int startup(u_short port);

void *accept_request(void *client_sock);

void get_line(int sockfd, char *buf);

string read_file(string file_path);

inline bool file_exists(const std::string &name);

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
        string content = read_file(full_file_path);
        int content_len = content.size();
        if (content_len > 0) {
            strcpy(buf, "HTTP/1.1 200 OK\r\n");
            std::cout << buf;
            send(tmp, buf, strlen(buf), 0);
            sprintf(buf, "Content-Length:%d\r\n", content_len);
            std::cout << buf;
            send(tmp, buf, strlen(buf), 0);
            strcpy(buf, "\r\n");
            std::cout << buf;
            send(tmp, buf, strlen(buf), 0);
            send(tmp, content.c_str(), content_len, 0);
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
