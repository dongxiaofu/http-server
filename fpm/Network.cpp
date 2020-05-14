#include "Network.h"

/****************************************
 * 连接socket服务器。
 * 我不能看懂这里的所有套路代码，比如，每个变量
 * 的作用是什么，是不是不可或缺。
 * todo 有空再去记忆这些东西。
 ****************************************/
int Network::client_socket(const char *host, int client_port) {
    int sock;
    unsigned long inaddr;
    struct sockaddr_in ad;
    struct hostent *hp;

    memset(&ad, 0, sizeof(ad));
    ad.sin_family = AF_INET;

    inaddr = inet_addr(host);
    if (inaddr != INADDR_NONE)
        memcpy(&ad.sin_addr, &inaddr, sizeof(inaddr));
    else {
        hp = gethostbyname(host);
        if (hp == NULL)
            return -1;
        memcpy(&ad.sin_addr, hp->h_addr, hp->h_length);
    }
    ad.sin_port = htons(client_port);
    /*****************************************
     * 与服务端socket很相似。
     * 给struct sockaddr_in ad赋值,
     * ad.sin_port
     * ad.sin_addr
     * ad.sin_family
     * 然后创建一个socket。
     * 差异是，最后使用connect，而不是bind、listen
     * accept。
     * AF_INET与PF_INET是相等的。
     *****************************************/
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
        return sock;
    if (connect(sock, (struct sockaddr *) &ad, sizeof(ad)) < 0)
        return -1;
    return sock;
}
