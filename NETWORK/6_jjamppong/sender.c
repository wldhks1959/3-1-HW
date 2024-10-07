#include "Common.h"

#define SERVERIP "127.0.0.1"
#define SERVERPORT 9015
#define BUFSIZE 50
#define PACKET_SIZE 9
#define PAYLOAD_SIZE 6

// 전역 변수
pthread_mutex_t mutex;                 
SOCKET sock;                           // 소켓 디스크립터
char payload[1024][PAYLOAD_SIZE + 1];  // 페이로드 데이터를 저장할 배열
int payload_cnt = 0;                   // 페이로드의 개수를 저장할 변수
int ack[1024] = {0};                   // ACK를 저장할 배열
int window_size = 4;                   // 윈도우 크기
int base = 0;                          // 윈도우의 base 인덱스

// 체크섬 계산 함수
us checksum(const char *data) 
{
    ui sum = 0;
    for (int i = 0; i < PAYLOAD_SIZE; i++) 
        sum += (uc)data[i];
    
    return (sum & 0xFFFF);
}

// 페이로드 준비 함수
void prepare_payload(const char *data) 
{
    int len = strlen(data);
    for (int i = 0; i < len; i += PAYLOAD_SIZE) 
    {
        strncpy(payload[payload_cnt], &data[i], PAYLOAD_SIZE);
        payload[payload_cnt][PAYLOAD_SIZE] = '\0';
        payload_cnt++;
    }
}

// sender
void* sender(void *args) 
{
    char packet[PACKET_SIZE];
    int nextseqnum = 0;  // 다음 패킷 번호
    struct timeval startT, curT;
    long timeout_interval = 500000; // 타임아웃 간격 (0.5초)

    FILE *file = fopen("text.txt", "r");
    if (file == NULL) { perror("Failed to open file"); return NULL; }

    char data[4096]; 
    fread(data, sizeof(char), 4096, file);
    fclose(file);

    prepare_payload(data); // 페이로드 준비
    gettimeofday(&startT, NULL);  // 시작 시간 기록

    // 모든 페이로드 전송까지 반복 
    while (base < payload_cnt) 
    {
        pthread_mutex_lock(&mutex);
        // 윈도우 내에서 전송 가능한 모든 패킷 전송
        while (nextseqnum < base + window_size && nextseqnum < payload_cnt) 
        {
            us chk = checksum(payload[nextseqnum]); // 체크섬 계산
            packet[0] = nextseqnum % 10 + '0';  // 패킷 번호

            memcpy(packet + 1, payload[nextseqnum], PAYLOAD_SIZE); 
            memcpy(packet + 1 + PAYLOAD_SIZE, &chk, sizeof(us)); 

            // send 
            send(sock, packet, PACKET_SIZE, 0);
            printf("packet %d is transmitted. (%.*s)\n", 
                    nextseqnum, PAYLOAD_SIZE, payload[nextseqnum]);
           
            if (base == nextseqnum) gettimeofday(&startT, NULL);  // 시작 시간 갱신
            nextseqnum++;
            usleep(50000);  // 0.05초 대기
        }
        pthread_mutex_unlock(&mutex);

        gettimeofday(&curT, NULL); // 현재 시간 기록 

        // 경과 시간 계산
        long elapsed_time = (curT.tv_sec - startT.tv_sec) * 1000000 
                                + (curT.tv_usec - startT.tv_usec);

        // 타임아웃 처리
        if (elapsed_time > timeout_interval) 
        {   
            pthread_mutex_lock(&mutex);
            nextseqnum = base;
            printf("packet %d is timeout.\n", base);
            gettimeofday(&startT, NULL);  // 시작 시간 갱신
            pthread_mutex_unlock(&mutex);
        }
        usleep(10000);  // 0.01초 대기
    }
    return NULL;
}

// receiver 
void* receiver(void *args) 
{
    char ack_msg[BUFSIZE];
    while (1) 
    {
        int len = recv(sock, ack_msg, BUFSIZE, 0);
        if (len > 0) 
        {
            ack_msg[len] = '\0';
            int ack_num;
            sscanf(ack_msg, "ACK = %d", &ack_num);
            pthread_mutex_lock(&mutex);

            ack[ack_num / 7] = 1;  // 실제 패킷 번호에 따라 인덱스 조정
            while (ack[base]) base++; // base 업데이트 -> window 이동
            
            pthread_mutex_unlock(&mutex);
            printf("(ACK = %d) is received.\n", ack_num);
        }
    }
    return NULL;
}

int main() 
{
    pthread_mutex_init(&mutex, NULL);  // 뮤텍스 초기화

    // 소켓 생성
    sock = socket(AF_INET, SOCK_STREAM, 0);  
    if (sock == -1) err_quit("socket()");

    // 소켓 주소 구조체 초기화
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(SERVERIP);
    server_addr.sin_port = htons(SERVERPORT);

    // 서버 연결 
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) 
    { perror("connect()"); exit(1); }

    // sender 생성 
    pthread_t senderTID;
    if (pthread_create(&senderTID, NULL, sender, NULL) != 0)
    { fprintf(stderr, "thread create error\n"); exit(1); }

    // receiver 생성
    pthread_t receiverTID;
    if (pthread_create(&receiverTID, NULL, receiver, NULL) != 0)
    { fprintf(stderr, "thread create error\n"); exit(1); }

    // 자원 반환 
    pthread_join(senderTID, NULL);
    pthread_join(receiverTID, NULL);

    pthread_mutex_destroy(&mutex);  // 뮤텍스 파괸

    close(sock);
    return 0;
}
