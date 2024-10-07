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
int flag = 1;
char last_ack[10]="null"; // 가장 마지막에 보낸 ack 저장 변수 
pthread_mutex_t mutex;

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

        // 수신한 패킷이 예상대로라면 ACK를 전송하고 다음 순서 번호로 이동
        if (seq_num == base && flag==1) 
        {
            sprintf(last_ack, "%s",ACK[seq_num] );
            send(client_sock, ACK[seq_num], strlen(ACK[seq_num]), 0);

            // 수신한 패킷과 전송된 ACK 출력
            printf("\"%s\" is received. \"%s\" is transmitted.\n", packet[seq_num], ACK[seq_num]);

            base++; // 다음 번호로 이동
        }
        else if (seq_num==base)
        {
            sprintf(last_ack, "%s",ACK[seq_num] );
            send(client_sock, ACK[seq_num], strlen(ACK[seq_num]), 0);

            // Print the received packet and transmitted ACK
            printf("\"%s\" is received and delievered. \"%s\" is transmitted.\n", packet[seq_num], ACK[seq_num]);

            base++; // 다음 번호로 이동
        } 
        else 
        {
            // 수신한 패킷이 순서에 맞지 않는 경우 무시, 가장 최근 ACK를 보냄
            flag = -1;
            send(client_sock, last_ack, strlen(last_ack), 0);
            printf("\"%s\" is received and dropped. \"%s\" is retransmitted.\n", packet[seq_num], last_ack);
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