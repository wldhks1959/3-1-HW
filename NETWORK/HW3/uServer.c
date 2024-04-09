#include "Common.h"

#define SERVERPORT 9000
#define BUFSIZE    4096

struct sockaddr_in clientaddr;
socklen_t addrlen = sizeof(clientaddr);
int retval;

// 클라이언트로부터 데이터를 수신하는 함수 
void* reader(void* args)
{
    SOCKET sock = *((SOCKET*)args);
    // 데이터 통신에 사용할 변수
    char buf[BUFSIZE];

    // 클라이언트와 데이터 통신
    while (1)
    {
        // 데이터 받기
        retval = recvfrom(sock, buf, BUFSIZE, 0,
            (struct sockaddr*)&clientaddr, &addrlen);
        if (retval == SOCKET_ERROR) { err_display("recvfrom()"); break; }

        // 받은 데이터 출력
        buf[retval] = '\0';
        printf("client: %s", buf);
    }
}

void* sender(void* args)
{
    SOCKET sock = *((SOCKET*)args);
    // 데이터 통신에 사용할 변수
    char buf[BUFSIZE];

    // 클라이언트와 데이터 통신
    while (1)
    {
        // 데이터 입력
        fgets(buf, BUFSIZE, stdin);

        // 데이터 보내기
        retval = sendto(sock, buf, BUFSIZE, 0,
            (struct sockaddr*)&clientaddr, addrlen);
        if (retval == SOCKET_ERROR) { err_display("sendto()"); break; }

        printf("server: %s", buf);
    }
}


int main(int argc, char* argv[])
{
    int retval;
    // 소켓 생성
    SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == INVALID_SOCKET) err_quit("socket()");

    // bind()
    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(SERVERPORT);
    retval = bind(sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
    if (retval == SOCKET_ERROR) err_quit("bind()");

    // read thread
    pthread_t readerTID;
    if (pthread_create(&readerTID, NULL, reader, (void*)&sock) != 0) {
        fprintf(stderr, "thread create error\n"); exit(1);
    }

    // send thread
    pthread_t senderTID;
    if (pthread_create(&senderTID, NULL, sender, (void*)&sock) != 0) {
        fprintf(stderr, "thread create error\n"); exit(1);
    }

    // resource return 
    pthread_join(senderTID, NULL);
    pthread_join(readerTID, NULL);

    // 소켓 닫기
    close(sock);
    return 0;
}