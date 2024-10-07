#include "Common.h"

#define SERVERPORT 9009
#define BUFSIZE 4096

typedef unsigned short us;
typedef unsigned int ui;
typedef unsigned char uc;

// 전역 변수
pthread_mutex_t mutex;
bool flag = true;

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

void* receiver(void* args) 
{
    SOCKET client_sock = *((int*)args);
    char buf[BUFSIZE];
    int expect_seq = 0; // 예상 패킷 순서 

    while (1) 
    {
        memset(buf, 0, BUFSIZE);
        int len = recv(client_sock, buf, BUFSIZE, 0);
        if (len <= 0) { perror("recv failed"); break; }
        
        buf[len] = '\0';
        
        // 첫 번째 패킷 1 건너뛰기 처리 
        if(expect_seq == 1 && flag == true) 
        {
            flag = false;
            expect_seq++;
        }

        char data[BUFSIZE];
        char recv_chk[9];
        us calc_chk;
        /* 
            [^|] : | 문자가 나오기 전까지의 모든 문자를 읽어서 data에 저장
            %8s :  | 이후의 최대 8문자까지 문자열을 읽어 recv_chk에 저장
        */
        sscanf(buf, "%[^|]|%8s", data, recv_chk);
        calc_chk = checksum(data);
 
        char calc_chk_binary[9];
        to_binary(calc_chk, calc_chk_binary);

        // test용 코드 
        // printf("Received checksum: %s, Calculated checksum: %s\n", 
        //         recv_chk, calc_chk_binary);

        us recv_chk_val = strtol(recv_chk, NULL, 2); // strtol : 2진법으로 표기된 문자열을 숫자로 변환
        if ((recv_chk_val + calc_chk) == 0xFF) // 0xFF : 255 ==> 11111111 이다.
        {
            printf("packet %d is received and there is no error. (%s)\n", expect_seq, data);
            expect_seq++;
            if (expect_seq == 5) expect_seq = 1;
        } 
        else 
        {
            printf("packet %d is received with errors: (%s)\n", expect_seq, data);
        }

        char ack_msg[BUFSIZE];
        // printf("buf : %s\n", buf);
        if (strcmp(data, "You are a girl.") == 0)
            sprintf(ack_msg, "ACK = 26");
        else
            sprintf(ack_msg, "ACK = 11");
        send(client_sock, ack_msg, strlen(ack_msg), 0);
        printf("(%s) is transmitted\n", ack_msg);
    }
    return NULL;
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
    if (listen(server_sock, SOMAXCONN) == SOCKET_ERROR) 
        err_quit("listen()");

    // accept()
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    SOCKET client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &client_addr_len);
    if (client_sock == INVALID_SOCKET) err_quit("accept()");

    // receiver 생성
    pthread_t receiverTID;
    if (pthread_create(&receiverTID, NULL, receiver, (void*)&client_sock) != 0)
    { fprintf(stderr, "Thread creation error\n"); exit(1); }
        
    // 자원 반환 
    pthread_join(receiverTID, NULL);
    pthread_mutex_destroy(&mutex); // Destroy mutex

    // 소켓 닫기 
    close(server_sock);
    return 0;
}