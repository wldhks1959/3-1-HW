#include "Common.h"

#define SERVERIP   "127.0.0.1"
#define SERVERPORT 9004
#define BUFSIZE    4096

int retval;

// 서버로부터 데이터를 수신하는 함수
void* reader(void* args)
{
    int server_sock = *((int*)args);
    char buf[BUFSIZE];

    // 서버와 데이터 통신
    while (1)
    {
        // 데이터 받기
        retval = recv(server_sock, buf, BUFSIZE, 0);
        if (retval == SOCKET_ERROR) { err_display("recv()"); break; }

        // 받은 데이터 출력
        buf[retval] = '\0';
        printf("server: %s", buf);
    }
}

// 서버로 데이터를 전송하는 함수
void* sender(void* args)
{
    int server_sock = *((int*)args);
    char buf[BUFSIZE];

    // 서버와 데이터 통신
    while (1)
    {
        // 데이터 입력
        fgets(buf, BUFSIZE, stdin);

        // 데이터 보내기
        retval = send(server_sock, buf, strlen(buf), 0);
        if (retval == SOCKET_ERROR) { err_display("send()"); break; }
        printf("client: %s", buf);
    }
}

int main()
{
    // 소켓 생성
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) err_quit("socket()");

    // 소켓 주소 구조체 초기화
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(SERVERIP);
    server_addr.sin_port = htons(SERVERPORT);

    // 서버에 연결
    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR)
        err_quit("connect()");

    // reader 
    pthread_t readerTID;
    if (pthread_create(&readerTID, NULL, reader, (void*)&sock) != 0){
        fprintf(stderr, "thread create error\n"); exit(1);
    }

    // sender
    pthread_t senderTID;
    if (pthread_create(&senderTID, NULL, sender, (void*)&sock) != 0){
        fprintf(stderr, "thread create error\n"); exit(1);
    }
        
    // resource return
    pthread_join(senderTID, NULL);
    pthread_join(readerTID, NULL);

    // 소켓 닫기
    close(sock);
    return 0;
}