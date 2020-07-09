#pragma warning(4:4996)
#include <string.h>
#include <stdio.h>
#include <winsock2.h>
#include <Windows.h>

#pragma comment(lib,"ws2_32.lib")
#define PORT 10001
#define IPADDR "127.0.0.1"
#define BUFSIZE 100

int main() {
	int c_socket;
	struct sockaddr_in c_addr;
	int n,i;
	char rcvBuffer[BUFSIZE];//서버에서 보내준 메세지를 저장하는 변수
	char sendBuffer[BUFSIZE];
	WSADATA wsadata;

	WSAStartup(MAKEWORD(2, 0), &wsadata);
	//1. 클라이언트 소켓 생성
	c_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED); //서버와 동일한 설정으로 생성
	//2.소켓 정보 초기화s
	memset(&c_addr, 0, sizeof(c_addr));
	c_addr.sin_addr.s_addr = inet_addr(IPADDR); //접속할 IP 설정 (127.0.0.1)
	c_addr.sin_family = AF_INET;
	c_addr.sin_port = htons(PORT);

	//3. 서버에 접속
	if (connect(c_socket, (struct sockaddr*)&c_addr, sizeof(c_addr)) == SOCKET_ERROR) {
		//서버 접속에 실패하면 
		printf("Cannot Connect\n"); //Cannot connect 메세지 출력
		//closesocket(c_socket); //자원 회수
		return -1;  //프로세스 종료
	}
	
	//5. 서버에서 보낸 메시지 읽기 
	recv(c_socket,&i, sizeof(int), 0);
	recv(c_socket, rcvBuffer, i, 0);
	rcvBuffer[i] = '\0';
	//rcvBuffer마지막 i번지에 문자열 깨짐 방지
	printf("received data: %s\n",rcvBuffer); //서버에서 받은 메세지 출력
	printf("received data: %s\n", rcvBuffer); //서버에서 받은 메세지 출력
	
	closesocket(c_socket);
	return 0;
}