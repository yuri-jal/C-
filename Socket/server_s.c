//#include <stdio.h>
#include <windows.h>
#include <string.h>

#pragma comment(lib,"ws2_32.lib")
#define PORT 10001

char rcvbuffer[100] = "Hi, Server.";

int main(void) {
	int c_socket, s_socket;
	struct sockaddr_in so_addr, c_addr;
	int n;
	int len;
	int c_socket_size;
	WSADATA wsadata;

	WSAStartup(MAKEWORD(2, 2), &wsadata);
	// 1. 서버 소켓 생성
	//서버 소켓 = 클라이언트의 접속 요청을 처리(허용)해 주기 위한 소켓
	s_socket = socket(PF_INET, SOCK_STREAM, 0); //TCP/IP 통신을 위한 서버 소켓 생성

	//2. 서버 소켓 주소 설정
	memset(&so_addr, 0, sizeof(so_addr)); //so_addr의 값을 모두 0으로  초기화
	so_addr.sin_addr.s_addr = htonl(INADDR_ANY); //IP 주소 설정
	so_addr.sin_family = AF_INET;
	so_addr.sin_port = htons(PORT);

	//3. 서버 소켓바인딩
	if (bind(s_socket, (struct sockaddr*)&so_addr, sizeof(so_addr)) == -1) {
		//바인딩 작업 실패 시, Cannot Bind 메시지 출력 후 프로그램 종료
		printf("Cannot Bind\n");
		printf("%d", errno);
		return -1;
	}

	//4.listen() 함수 실행
	if (listen(s_socket, 5) == -1) {
		printf("listen Fail\n");
		return -1;
	}

	//5. 클라이언트 요청 처리
	// 요청을 허용한 후, Hello World 메세지를 전송함
	while (1) { //무한 루프
		len = sizeof(c_addr);
		printf("클라이언트 접속을 기다리는 중....\n");
		c_socket = accept(s_socket, (struct sockaddr*)&c_addr, &len);
		//클라이언트의 요청이 오면 허용(accept)해 주고, 해당 클라이언트와 통신할 수 있도록 클라이언트 소켓(c_socket)을 반환함.
		printf("/client is connected\n");
		printf("클라이언트 접속 허용\n");
		//n = recv(c_socket, rcvbuffer, sizeof(rcvbuffer),0);
		send(c_socket, rcvbuffer, strlen(rcvbuffer), 0);

		closesocket(c_socket);
	}
	close(s_socket);
	return 0;
}
