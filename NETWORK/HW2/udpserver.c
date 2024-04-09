#include "Common.h"

#define SERVERPORT 9000
#define BUFSIZE    4096

int main(int argc, char *argv[]) {
    int retval;
    FILE *fp = NULL;

    // 소켓 생성
    SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == INVALID_SOCKET) err_quit("socket()");

    // bind()
    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(SERVERPORT);
    retval = bind(sock, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
    if (retval == SOCKET_ERROR) err_quit("bind()");

    // 데이터 통신에 사용할 변수
    struct sockaddr_in clientaddr;
    socklen_t addrlen;
    char buf[BUFSIZE + 1];

    // 클라이언트와 데이터 통신
    while (1) {
        // 데이터 받기
        addrlen = sizeof(clientaddr);
        retval = recvfrom(sock, buf, BUFSIZE, 0,
                          (struct sockaddr *)&clientaddr, &addrlen);
        if (retval == SOCKET_ERROR) {
            err_display("recvfrom()");
            break;
        }

        buf[retval] = '\0';
		// buf 확인 후 요청 확인 
        if (strcmp(buf, "request \"novel.txt\"") == 0) {
			printf("The server received a request from a client\n");
            // 파일 열기
            fp = fopen("novel.txt", "rb");
            if (fp == NULL) {
                printf("파일 열기 실패!\n");
                break;
            }

            // 파일 크기 체크
            fseek(fp, 0, SEEK_END);
            size_t fsize = ftell(fp);
            fseek(fp, 0, SEEK_SET);

            // 파일 송신
            while (!feof(fp)) {
                size_t bytes_read = fread(buf, 1, BUFSIZE, fp);
                sendto(sock, buf, bytes_read, 0, (struct sockaddr *)&clientaddr, addrlen);
            }

            fclose(fp);

            printf("The server sent \"novel.txt\" to the client\n");
        }
    }

    // 소켓 닫기
    close(sock);
    return 0;
}
