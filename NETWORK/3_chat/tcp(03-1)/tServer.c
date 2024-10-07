#include "Common.h"

#define SERVERPORT 9004
#define BUFSIZE    4096

int retval;

// 클라이언트로부터 데이터를 수신하는 함수 
void* reader(void* args)
{
    int client_sock = *((int*)args);
    char buf[BUFSIZE];

    // 클라이언트와 데이터 통신
    while (1)
    {
        // 데이터 받기
        retval = recv(client_sock, buf, BUFSIZE, 0);
        if (retval == SOCKET_ERROR) { err_display("recv()"); break; }

        // 받은 데이터 출력
        buf[retval] = '\0';
        printf("client: %s", buf);
    }
}

// 클라이언트로 데이터를 전송하는 함수
void* sender(void* args)
{
    int client_sock = *((int*)args);
    char buf[BUFSIZE];

    // 클라이언트와 데이터 통신
    while (1)
    {
        // 데이터 입력
        fgets(buf, BUFSIZE, stdin);

        // 데이터 보내기
        retval = send(client_sock, buf, strlen(buf), 0);
        if (retval == SOCKET_ERROR) { err_display("send()"); break; }
        printf("server: %s", buf);  
    }
}

int main()
{
    // 소켓 생성
    SOCKET server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock == INVALID_SOCKET) err_quit("socket()");

    // bind()
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(SERVERPORT);
    if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR)
        err_quit("bind()");

    // listen()
    if (listen(server_sock, SOMAXCONN) == SOCKET_ERROR) err_quit("listen()");

    // accept()
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    SOCKET client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &client_addr_len);
    if (client_sock == INVALID_SOCKET) err_quit("accept()");

    // reader 
    pthread_t readerTID;
    if (pthread_create(&readerTID, NULL, reader, (void*)&client_sock) != 0){
        fprintf(stderr, "thread create error\n"); exit(1);
    }

    // sender
    pthread_t senderTID;
    if (pthread_create(&senderTID, NULL, sender, (void*)&client_sock) != 0){
        fprintf(stderr, "thread create error\n"); exit(1);
    }
        
    // resource return
    pthread_join(senderTID, NULL);
    pthread_join(readerTID, NULL);

    // 소켓 닫기
    close(server_sock);
    return 0;
}