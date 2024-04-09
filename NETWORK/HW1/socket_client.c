#include "Common.h"
#include "calender.h"

char* SERVERIP = (char*)"127.0.0.1";
#define SERVERPORT 9007
#define BUFSIZE    512

int main(int argc, char* argv[])
{
	int retval;

	// 명령행 인수가 있으면 IP 주소로 사용
	if (argc > 1) SERVERIP = argv[1];

	// 소켓 생성
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) err_quit("socket()");

	// connect()
	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	inet_pton(AF_INET, SERVERIP, &serveraddr.sin_addr);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = connect(sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("connect()");

	// 데이터 통신에 사용할 변수
	int year, month;
	printf("Enter year & month: ");
	scanf("%d %d", &year, &month);

	// 데이터 보내기
	char buf[BUFSIZE];
	sprintf(buf, "%d %d", year, month);
	retval = send(sock, buf, strlen(buf), 0);
	if (retval == SOCKET_ERROR) err_display("send()");
	else printf("[Client]:Sent year and month to the server.\n");

	// 서버로부터 수신
	char receivedCalendar[BUFSIZE];
	retval = recv(sock, receivedCalendar, BUFSIZE, 0);
	if (retval == SOCKET_ERROR) err_display("recv()");
	else if (retval == 0) return -1;
	else
	{
		receivedCalendar[retval] = '\0';
		printf("[Client]: Received calendar from server:\n%s\n", receivedCalendar);
	}

	// 소켓 닫기
	close(sock);
	return 0;
}