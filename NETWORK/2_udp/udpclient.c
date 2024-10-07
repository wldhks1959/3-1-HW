#include "Common.h"

char *SERVERIP = (char *)"127.0.0.1";
#define SERVERPORT 9000
#define BUFSIZE    4096

int main(int argc, char *argv[]) {
    int retval;
    // 명령행 인수가 있으면 IP 주소로 사용
    if (argc > 1) SERVERIP = argv[1];

    // 소켓 생성
    SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == INVALID_SOCKET) err_quit("socket()");

    // 소켓 주소 구조체 초기화
    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    inet_pton(AF_INET, SERVERIP, &serveraddr.sin_addr);
    serveraddr.sin_port = htons(SERVERPORT);

    // 데이터 통신에 사용할 변수
    struct sockaddr_in peeraddr;
    socklen_t addrlen;
    char buf[BUFSIZE + 1];
    int len;

    // 서버와 데이터 통신
    while (1) {
        // 데이터 입력
        printf("\n[보낼 데이터] ");
        if (fgets(buf, BUFSIZE + 1, stdin) == NULL)
            break;

        // '\n' 문자 제거
        len = (int)strlen(buf);
        if (buf[len - 1] == '\n')
            buf[len - 1] = '\0';
        if (strlen(buf) == 0)
            break;

        // 데이터 보내기
        retval = sendto(sock, buf, (int)strlen(buf), 0,
                        (struct sockaddr *)&serveraddr, sizeof(serveraddr));
        if (retval == SOCKET_ERROR) {
            err_display("sendto()");
            break;
        }
        printf("[UDP 클라이언트] %d바이트를 보냈습니다.\n", retval);

       // 데이터 받기
        addrlen = sizeof(peeraddr);
        retval = recvfrom(sock, buf, BUFSIZE, 0, (struct sockaddr *)&peeraddr, &addrlen);
        
        // 오류 처리
        if (retval == SOCKET_ERROR) {
            err_display("recvfrom()");
            break;
        }
        
        // 파일 생성
        FILE*fp=NULL;   
        fp = fopen("received_novel.txt", "wb");
        if(fp==NULL){
            printf("파일 생성 실패\n");
            break;
        }
        fwrite(buf,1,retval,fp);
        fclose(fp);

        // 파일 읽기
        fp = fopen("received_novel.txt", "rb");
        if(fp==NULL){
            printf("파일 열기 실패\n");
            break;
        }
		
        // 받은 데이터 출력
        printf("The client received \"novel.txt\" from the server.\n");
        printf("[UDP 클라이언트] %d바이트를 받았습니다.\n", retval);
        
        printf("[받은 데이터]: \n");
        while((retval=fread(buf,1,BUFSIZE,fp))>0){
            buf[retval] = '\0';
            printf("%s", buf);
        }
        
        // 송신자의 주소 체크
        if (memcmp(&peeraddr, &serveraddr, sizeof(peeraddr))) {
            printf("[Error] 데이터 잘못 받음!\n");
            break;
        }


    }

    // 소켓 닫기
    close(sock);
    return 0;
}
