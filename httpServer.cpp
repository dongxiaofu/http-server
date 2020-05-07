#include <iostream>
#include <netinet/in.h>
#include <zconf.h>
#include <sys/socket.h>
#include <pthread.h>

int startup(u_short port);

void *accept_request(void *client_sock);

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
        void *tmp = (void *) (long)client_sock;
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

    return nullptr;
}
