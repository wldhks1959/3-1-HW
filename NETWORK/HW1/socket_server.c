#include "Common.h"
#include "calender.h"

#define SERVERPORT 9007
#define BUFSIZE    512

int main(int argc, char* argv[])
{
	int retval;
	// 소켓 생성
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock == INVALID_SOCKET) err_quit("socket()");

	// bind()
	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = bind(listen_sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("bind()");

	// listen()
	retval = listen(listen_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR) err_quit("listen()");

	// 데이터 통신에 사용할 변수
	SOCKET client_sock;
	struct sockaddr_in clientaddr;
	socklen_t addrlen;
	char buf[BUFSIZE + 1];

	// accept()
	addrlen = sizeof(clientaddr);
	client_sock = accept(listen_sock, (struct sockaddr*)&clientaddr, &addrlen);
	if (client_sock == INVALID_SOCKET) {
		err_display("accept()");
	}

	// 클라이언트로부터 year and month를 수신
	retval = recv(client_sock, buf, BUFSIZE, 0);
	if (retval == SOCKET_ERROR) err_display("recv()");
	else if (retval == 0) return -1;
	else
	{
		buf[retval] = '\0';

		// year and month를 파싱
		int year, month;
		sscanf(buf, "%d %d", &year, &month);

		char calendarOutput[BUFSIZE];
		execute(year, month, calendarOutput);
		//send
		retval = send(client_sock, calendarOutput, strlen(calendarOutput), 0);
	}
	// 소켓 닫기
	close(client_sock);

	// 소켓 닫기
	printf("\nServer down!\n");
	close(listen_sock);
	return 0;
}