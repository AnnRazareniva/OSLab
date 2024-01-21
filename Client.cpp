#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <iostream>
#include <string.h>

using namespace std;

int main()
{
    char message[]="Hi, it's me, Client!";
    int sock;
    struct sockaddr_in addr;

    sock=socket(AF_INET, SOCK_STREAM, 0);
    if (sock<0)
    {
        printf("socket error");
        exit(1);
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    if(connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        printf("connect error");
        exit(2);
    }

    send(sock, message, sizeof(message), 0);

    close(sock);
    return 0;
}
