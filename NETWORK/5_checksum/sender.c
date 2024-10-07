#include "Common.h"

#define SERVERIP "127.0.0.1"
#define SERVERPORT 9009
#define BUFSIZE 4096

typedef unsigned short us;
typedef unsigned int ui;
typedef unsigned char uc;

char *packets[] = 
{
    "I am a boy.",
    "You are a girl.",
    "There are many animals in the zoo.",
    "철수와 영희는 서로 좋아합니다!",
    "나는 점심을 맛있게 먹었습니다."
};

// 전역 변수
int acks[5] = {0};
int sock;
bool flag = true;
pthread_mutex_t mutex;

/* [8비트 이진수로 변환하는 함수]
    주어진 수를 이진수로 변환하여 문자열로 저장.
    us num : 변환할 16비트 정수 
    char * binary_str : 변환된 이진수를 저장할 9바이트 크기의 문자열 
*/
void to_binary(us num, char* binary_str) 
{
    for (int i = 7; i >= 0; i--) 
    {
        // 가장 우측 비트를 추출한다. 비트 연산자 & : 두 비트가 모두 1일 때만 1을 반환 
        binary_str[i] = (num & 1) ? '1' : '0'; 
        // num을 오른쪽으로 1비트 이동시킨다.
        num >>= 1; 
    }
    binary_str[8] = '\0'; // 변환된 이진수 문자열의 끝에 널 종료 문자를 추가.
}

// 체크섬 계산 함수
us checksum(const char *data) 
{
    ui sum = 0; // 누적 합계를 저장하는 변수  
    for (int i = 0; i < strlen(data); i++) 
        sum += (uc)data[i]; // 각 반복에서 문자열의 각 문자를 (uc)변환하여 sum에 누적 합.
    return (us)(sum & 0xFF); // 하위 8비트만을 반환
}

// 1의 보수를 계산하는 함수 
us ones_complement(us num)
{
    return ~num & 0xFF; // 8비트의 1의 보수를 계산
}

void *sender(void *args) 
{
    char buf[BUFSIZE];
    char chk_binary[9]; // 8비트 이진수 문자열
    for (int i = 0; i < 5; i++) 
    {
        us chk = checksum(packets[i]);
        chk = ones_complement(chk); // 1의 보수 적용
        to_binary(chk, chk_binary); // 체크섬을 이진수 문자열로 변환
        sprintf(buf, "%s|%s", packets[i], chk_binary); // 패킷에 이진수 체크섬 추가
        if (i == 1 && flag == true) 
        {
            flag = false;
            printf("packet %d is transmitted. (%s)\n", i, packets[i]);
            continue; 
        }
        send(sock, buf, strlen(buf), 0);
        printf("packet %d is transmitted. (%s)\n", i, packets[i]);
        sleep(1);  
    }

    // ACK 기다림 
    sleep(1);  

    // 패킷 1이 확인되지 않았는지 체크 
    if (acks[1] == 0) 
    {
        us chk = checksum(packets[1]);
        chk = ones_complement(chk); // 1의 보수 적용 
        to_binary(chk, chk_binary); // 체크섬을 이진수 문자열로 변환
        sprintf(buf, "%s|%s", packets[1], chk_binary); // 패킷에 이진수 체크섬 추가
        send(sock, buf, strlen(buf), 0);
        printf("packet 1 is retransmitted. (%s)\n", packets[1]);
    }
    return NULL;
}

void *receiver(void *args) 
{
    char buf[BUFSIZE];
    while (1) 
    {
        int len = recv(sock, buf, BUFSIZE, 0);
        if (len > 0) 
        {
            buf[len] = '\0';
            int ackNum;
            sscanf(buf, "ACK = %d", &ackNum);
            pthread_mutex_lock(&mutex);
            acks[ackNum]++;
            pthread_mutex_unlock(&mutex);
            printf("(ACK = %d) is received.\n", ackNum);
        }
    }
    return NULL;
}

int main() 
{
    pthread_mutex_init(&mutex, NULL); // 뮤텍스 초기화 

    // 소켓 생성
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) { perror("socket()"); exit(1); }

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

    pthread_mutex_destroy(&mutex); // 뮤텍스 파괴

    close(sock);
    return 0;
}