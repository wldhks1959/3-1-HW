#include "Common.h"
#include <string.h>
char* SERVERIP = (char*)"127.0.0.1";
#define SERVERPORT 9000
#define BUFSIZE    4096

struct sockaddr_in serveraddr;
socklen_t addrlen = sizeof(serveraddr);
int retval;

// 서버로부터 데이터를 수신하는 함수
void* reader(void* args)
{
    SOCKET sock = *((SOCKET*)args);
    // 데이터 통신에 사용할 변수
    char buf[BUFSIZE];

    // 서버와 데이터 통신
    while (1)
    {
        // 데이터 받기
        retval = recvfrom(sock, buf, BUFSIZE, 0,
            (struct sockaddr*)&serveraddr, &addrlen);
        if (retval == SOCKET_ERROR) { err_display("recvfrom()"); break; }

        // 받은 데이터 출력
        buf[retval] = '\0';
        printf("server: %s", buf);
    }
}

// 서버로 데이터를 송신하는 함수
void* sender(void* args)
{
    SOCKET sock = *((SOCKET*)args);
    // 데이터 통신에 사용할 변수
    char buf[BUFSIZE];

    // 서버와 데이터 통신
    while (1)
    {
        // 데이터 입력
        fgets(buf, BUFSIZE, stdin);

        // 데이터 보내기
        retval = sendto(sock, buf, BUFSIZE, 0,
            (struct sockaddr*)&serveraddr, addrlen);
        if (retval == SOCKET_ERROR) { err_display("sendto()"); break; }

        printf("client: %s", buf);
    }
}

int main(int argc, char* argv[])
{
    // 명령행 인수가 있으면 IP 주소로 사용
    if (argc > 1) SERVERIP = argv[1];

    // 소켓 생성
    SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == INVALID_SOCKET) err_quit("socket()");

    // 소켓 주소 구조체 초기화
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    inet_pton(AF_INET, SERVERIP, &serveraddr.sin_addr);
    serveraddr.sin_port = htons(SERVERPORT);

    // send thread
    pthread_t senderTID;
    if (pthread_create(&senderTID, NULL, sender, (void*)&sock) != 0) {
        fprintf(stderr, "thread create error\n"); exit(1);
    }

    // read thread
    pthread_t readerTID;
    if (pthread_create(&readerTID, NULL, reader, (void*)&sock) != 0) {
        fprintf(stderr, "thread create error\n"); exit(1);
    }

    // resource return 
    pthread_join(senderTID, NULL);
    pthread_join(readerTID, NULL);

    // 소켓 닫기
    close(sock);
    return 0;
}