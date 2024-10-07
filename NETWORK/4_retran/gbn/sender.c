#include "Common.h"
#include <unistd.h>
#include <time.h>

#define SERVERIP   "127.0.0.1"
#define SERVERPORT 9013
#define BUFSIZE    4096
#define WINDOW_SIZE 4
#define TIMEOUT_INTERVAL 5 

int retval;
char packet[6][10] = {"packet 0", "packet 1", "packet 2", "packet 3", "packet 4", "packet 5"};
char ACK[6][10] = {"ACK 0", "ACK 1", "ACK 2", "ACK 3", "ACK 4", "ACK 5"};
int base = 0; 
int next_seqnum = 0;
int cnt = 1;
char last_ack[10]="null";
pthread_mutex_t mutex;

time_t start_time, cur_time;

// 서버로 데이터를 보내는 함수
void* sender(void* args)
{
    int server_sock = *((int*)args);
    char buf[BUFSIZE];

    time(&start_time); // 타이머 시작

    int sent_packets = 0; 

    // 서버와 통신 
    while (base < 6) // 모든 패킷이 확인될 때까지 
    {
        // 윈도우 내에 있고, 모든 패킷이 아직 보내지지 않은 경우 
        if (sent_packets < WINDOW_SIZE && next_seqnum < 6)
        {
            pthread_mutex_lock(&mutex); 

            // 패킷 번호를 문자열로 변환하여 buf에 저장
            sprintf(buf, "%s", packet[next_seqnum]);

            // 패킷이 2이고 처음 전송인 경우, 패킷 손실 
            if (next_seqnum == 2 && cnt == 1) 
            {
                printf("\"packet 2\" is transmitted\n");
                cnt = 0;
            } 
            else 
            {
                retval = send(server_sock, buf, strlen(buf), 0);
                if (retval == SOCKET_ERROR) { err_display("send()"); break; }
                printf("\"%s\" is transmitted\n", buf);
            }
            time(&start_time); // 타이머 시작 

            next_seqnum++; // 다음 패킷 이동
            sent_packets++; 

            pthread_mutex_unlock(&mutex); 
        }

        // 타임 아웃 처리
        time(&cur_time);
        double RTT = difftime(cur_time, start_time);
        if (RTT >= TIMEOUT_INTERVAL)
        {
            pthread_mutex_lock(&mutex); 
            printf("%s is timeout.\n", packet[base]);
            next_seqnum = base; // 확인 안 된 가장 오래된 패킷 전송
            time(&start_time); // 타이머 초기화
            sent_packets = 0; 
            pthread_mutex_unlock(&mutex); 
        }

        // ACK 확인 대기 
        sleep(1); // 1초 대기
    }
}

// 서버로부터 ACK를 수신하는 함수
void* receiver(void* args)
{
    int server_sock = *((int*)args);
    char buf[BUFSIZE];

    while (1)
    {
        retval = recv(server_sock, buf, BUFSIZE, 0);
        if (retval == SOCKET_ERROR) { err_display("recv()"); break; }

        buf[retval] = '\0';

        pthread_mutex_lock(&mutex); 

        // ACK 번호 추출 
        int ack_num = atoi(&buf[4]);
        
        // 수신한 ACK와 마지막 ACK 비교 
        if(strcmp(last_ack, buf) == 0) 
        {
            printf("\"%s\" is received and ignored.\n", last_ack);
        } 
        else 
        {
            // 새로운 ACK로 업데이트 
            strcpy(last_ack, buf);
            
            // 확인 안 된 가장 오래된 패킷에 대한 ACK 수신 시 base 이동 
            if (ack_num == base) {
                base++;
                printf("\"%s\" received.\n", buf);
            }   
        }

        pthread_mutex_unlock(&mutex); // ACK 처리 후 뮤텍스 잠금 해제 
    }
}

int main()
{
    pthread_mutex_init(&mutex, NULL); 

    // 소켓 생성
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) err_quit("socket()");

    // 소켓 주소 구조체 초기화
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(SERVERIP);
    server_addr.sin_port = htons(SERVERPORT);

    // 서버 연결
    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR)
        err_quit("connect()");

    // sender 생성 
    pthread_t senderTID;
    if (pthread_create(&senderTID, NULL, sender, (void*)&sock) != 0){
        fprintf(stderr, "thread create error\n"); exit(1);
    }

    // receiver 생성
    pthread_t receiverTID;
    if (pthread_create(&receiverTID, NULL, receiver, (void*)&sock) != 0){
        fprintf(stderr, "thread create error\n"); exit(1);
    }

    // 자원 반환 
    pthread_join(senderTID, NULL);
    pthread_join(receiverTID, NULL);

    pthread_mutex_destroy(&mutex); 

    // 소켓 닫기
    close(sock);
    return 0;
}