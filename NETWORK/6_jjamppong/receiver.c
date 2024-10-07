#include "Common.h"

#define SERVERPORT 9015
#define BUFSIZE 50
#define PACKET_SIZE 9
#define PAYLOAD_SIZE 6

typedef struct 
{
    char packet[PACKET_SIZE];  // 패킷 데이터를 저장할 배열
    bool using;                // 버퍼 슬롯이 사용 중인지 여부
} bufs;

// 전역 변수
bufs buf[BUFSIZE];   // 버퍼 배열
int buf_start = 0;            // 버퍼 시작 인덱스
int buf_end = 0;              // 버퍼 종료 인덱스
int expect_num = 0;           // 예상 시퀀스 번호

pthread_mutex_t buf_mutex; 

// 체크섬 계산 함수
us checksum(const char *data) 
{
    unsigned int sum = 0;
    for (int i = 0; i < PAYLOAD_SIZE; i++) 
        sum += (uc)data[i];
    
    return (sum & 0xFFFF);
}

// receiver
void* receiver(void* args) 
{
    SOCKET client_sock = *((int*)args);
    char packet[PACKET_SIZE];
    char ack_msg[BUFSIZE];
    int ack_num;

    while (1) 
    {
        memset(packet, 0, sizeof(packet));
        int len = recv(client_sock, packet, PACKET_SIZE, 0); // 패킷 수신
        if (len <= 0) break;

        pthread_mutex_lock(&buf_mutex); // 뮤텍스 잠금

        int seq_num = packet[0] - '0'; // 시퀀스 번호 추출
        char payload[PAYLOAD_SIZE + 1] = {0};
        strncpy(payload, packet + 1, PAYLOAD_SIZE); // 페이로드 추출
        us recv_checksum;
        memcpy(&recv_checksum, packet + 1 + PAYLOAD_SIZE, sizeof(us)); // 체크섬 추출

        // 체크섬 검증 및 버퍼에 공간이 있는지 확인
        if (recv_checksum == checksum(payload) && (buf_end + 1) % BUFSIZE != buf_start) 
        {
            memcpy(buf[buf_end].packet, packet, PACKET_SIZE); // 패킷 버퍼에 복사
            buf[buf_end].using = true;      // 버퍼 슬롯 상태 갱신 
            buf_end = (buf_end + 1) % BUFSIZE; // 버퍼 종료 인덱스 갱신
        }

        // 예상 시퀀스 번호의 패킷이 버퍼에 있는지 확인
        while (buf[buf_start].using && buf[buf_start].packet[0] - '0' == expect_num) 
        {
            // 패킷 수신 올바르면 출력 
            printf("packet %d is received and there is no error. (%.*s) \n", expect_num, PAYLOAD_SIZE, buf[buf_start].packet + 1);
            expect_num = (expect_num + 1) % 10; // 기대하는 시퀀스 번호 갱신
            buf[buf_start].using = false; // 버퍼 슬롯 비우기
            buf_start = (buf_start + 1) % BUFSIZE; // 버퍼 시작 인덱스 갱신

            ack_num = expect_num * 7; // ACK 번호 계산
            sprintf(ack_msg, "ACK = %d", ack_num); // ACK 메시지 생성
            send(client_sock, ack_msg, strlen(ack_msg), 0); // ACK 메시지 전송
            printf("(%s) is transmitted\n", ack_msg);
        }

        pthread_mutex_unlock(&buf_mutex); 
        usleep(100000); // 0.1초 대기
    }
    return NULL;
}

int main() 
{
    pthread_mutex_init(&buf_mutex, NULL); 

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
        err_quit("bind()"); // 소켓 바인딩

    // listen()
    if (listen(server_sock, SOMAXCONN) == -1) 
        err_quit("listen()"); 

    // accept()
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    int client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &client_addr_len);
    if (client_sock == -1) err_quit("accept()");

    // receiver 생성 
    pthread_t receiverTID;
    if (pthread_create(&receiverTID, NULL, receiver, (void*)&client_sock) != 0) 
    { fprintf(stderr, "Thread creation error\n"); exit(1); }

    // 자원 반환
    pthread_join(receiverTID, NULL); 
    pthread_mutex_destroy(&buf_mutex); 

    // 소켓 닫기
    close(server_sock); 
    return 0;
}
