#include "Common.h"
#include <unistd.h>
#include <time.h>

#define SERVERPORT 9013
#define BUFSIZE    4096
#define WINDOW_SIZE 4

int retval;
char packet[6][10] = {"packet 0", "packet 1", "packet 2", "packet 3", "packet 4", "packet 5"};
char ACK[6][10] = {"ACK 0", "ACK 1", "ACK 2", "ACK 3", "ACK 4", "ACK 5"};
int base = 0; 
// int flag = 1;
char last_ack[10]="null"; // 가장 마지막에 보낸 ack 저장 변수 
pthread_mutex_t mutex;

char buffering[6][10]={"null, "};

// 클라이언트로부터 데이터를 수신하는 함수
void* reader(void* args)
{
    int client_sock = *((int*)args);
    char buf[BUFSIZE];

    // 클라이언트와 통신
    while (1)
    {
        retval = recv(client_sock, buf, BUFSIZE, 0);
        if (retval == SOCKET_ERROR) { err_display("recv()"); break; }

        buf[retval] = '\0';

        pthread_mutex_lock(&mutex); 

        // 수신한 패킷의 순서 번호 추출
        int seq_num = buf[7] - '0';


        // 수신 받은 데이터의 경우 따라 처리 
        // 1. 예상 패킷이 수신되면 ACK를 전송하고 다음 순서 번호로 이동 
        // 2. 예상 패킷이 수신되지 않는 경우 두 경우로 분리 
        // 2.1 패킷을 수신하여 버퍼링 순서가 잘 맞는 경우 ( 패킷 3,4,5가 버퍼링에 있고, 패킷 2가 온 경우)
        // 2.2 패킷을 수신하여 버퍼링 순서가 맞지 않는 경우 ( 패킷 2가 도착하지 않고, 패킷 3,4,5가 온 경우 )
        if (seq_num == base) 
        { // 예상한 패킷이 수신되면 ACK를 전송하고 다음 순서 번호로 이동 
            send(client_sock, ACK[seq_num], strlen(ACK[seq_num]), 0);
            printf("\"%s\" is received. \"%s\" is transmitted.\n", packet[seq_num], ACK[seq_num]);
            base++; // 다음 번호로 이동
        }
        else if (seq_num!=base)
        { // 예상한 패킷이 수신되지 않으면 패킷을 버퍼링에 저장하고, ACK를 전송하고 다음 번호로 이동 
            
            strcpy(buffering[seq_num],packet[seq_num]); // 버퍼링에 패킷 저장 
            
            // 버퍼링 순서 잘 맞는 경우 
            if(strcmp("packet 2", buffering[seq_num]) == 0 )
            { 
                send(client_sock, ACK[seq_num], strlen(ACK[seq_num]), 0);
                printf("\"%s\" is received. %s, %s, %s, and %s are delievered.\"%s\" is transmitted.\n", 
                        packet[seq_num], buffering[2], buffering[3], buffering[4], buffering[5], ACK[seq_num]);
                base++; // 다음 번호로 이동
            }
            // 버퍼링 순서 잘 맞지 않는 경우 
            else
            {
                printf("\"%s\" is received and buffered. \"%s\" is transmitted.\n", packet[seq_num], ACK[seq_num]);
                send(client_sock, ACK[seq_num], strlen(ACK[seq_num]), 0);
                base++; // 다음 번호로 이동
            }
        } 
        pthread_mutex_unlock(&mutex); 
    }
}

int main()
{
    pthread_mutex_init(&mutex, NULL); 

    // 소켓 주소 구조체 초기화
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(SERVERPORT);

    // 소켓 생성 
    SOCKET server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock == INVALID_SOCKET) err_quit("socket()");

    // bind()
    if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR)
        err_quit("bind()");

    // listen()
    if (listen(server_sock, SOMAXCONN) == SOCKET_ERROR) err_quit("listen()");

    // accept()
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    SOCKET client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &client_addr_len);
    if (client_sock == INVALID_SOCKET) err_quit("accept()");

    // reader 생성
    pthread_t readerTID;
    if (pthread_create(&readerTID, NULL, reader, (void*)&client_sock) != 0){
        fprintf(stderr, "Thread creation error\n"); exit(1);
    }
        
    // 자원 반환 
    pthread_join(readerTID, NULL);

    pthread_mutex_destroy(&mutex); // Destroy mutex

    // 소켓 닫기 
    close(server_sock);
    return 0;
}