#pragma warning(4:4996)

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <winsock2.h>
#include <Windows.h>
#pragma comment(lib, "dsound.lib")

#pragma comment(lib,"ws2_32.lib")
#define PORT 9000
#define IPADDR "192.168.168.49"
#define BUFSIZE 100

int main() {
	int c_socket;
	struct sockaddr_in c_addr;
	int n, i = 0, count=0;
	char rcvBuffer[BUFSIZE];//서버에서 보내준 메세지를 저장하는 변수
	char* sendBuffer[BUFSIZE] = { NULL," " };
	char* sendbuf[BUFSIZE]= { NULL," " };
	char buf[256];
	char buf1[BUFSIZE];

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
	recv(c_socket, &i, sizeof(int), 0);
	printf("received data: %x\n", i);
	recv(c_socket, rcvBuffer, i, 0);
	rcvBuffer[i] = '\0';
	//rcvBuffer마지막 i번지에 문자열 깨짐 방지
	printf("received data: %s\n", rcvBuffer); //서버에서 받은 메세지 출력
	char* token = NULL;
	char* token1 = NULL;
	token = strtok(rcvBuffer, "@");
	int num = 0,m=0;
	while (token != NULL) {
		sendBuffer[num] = token;
		num++;
		token = strtok(NULL, "@");

	}
	printf("구조체명: %s\n", sendBuffer[1]);
	for (num = 0; num < atoi(sendBuffer[0]); num++) {
		printf("%s  %s%s%s%s%s\n",
			sendBuffer[2 + 4 * num],
			*sendBuffer[4 + 4 * num] == '1' ? "*" : "",
			sendBuffer[3 + 4 * num],
			*sendBuffer[5 + 4 * num] == '0' ? "" : "[",
			*sendBuffer[5 + 4 * num] == '0' ? "" : sendBuffer[5 + 4 * num],
			*sendBuffer[5 + 4 * num] == '0' ? "" : "]"
		);
		
	}

	while (1) {
		printf("선택할 변수: ");
		scanf("%s", buf);
		send(c_socket, buf, strlen(buf), 0);
			if (strcmp(buf, sendBuffer[7])==0) {
				printf("입력할 값:");
				scanf(" %s", buf);
				send(c_socket, buf, sizeof(buf), 0);
			}else if (strcmp(buf, sendBuffer[11])==0) {
				printf("인덱스");
				scanf(" %s", buf);
				send(c_socket, buf, strlen(buf), 0);
				printf("입력할 값:");
				scanf(" %s", buf);
				send(c_socket, buf, strlen(buf), 0);
			}else if (strcmp(buf,"st") == 0) {
				recv(c_socket, &count, sizeof(int), 0);
				printf("received data: %x\n", count);
				recv(c_socket, buf1, count, 0);
				buf1[count] = '\0';
				//rcvBuffer마지막 i번지에 문자열 깨짐 방지
				printf("received data: %s\n", buf1); //서버에서 받은 메세지 출력
				
				token1 = strtok(buf1, "@");
				while (token1 != NULL) {
					sendbuf[m] = token1;
					m++;
					token1 = strtok(NULL, "@");
				}
				printf("구조체명: %s\n", sendbuf[1]);
				for (m = 0; m < atoi(sendbuf[0]); m++) {
					printf("%s  %s%s%s%s%s\n",
						sendbuf[2 + 4 * m],
						*sendbuf[4 + 4 * m] == '1' ? "*" : "",
						sendbuf[3 + 4 * m],
						*sendbuf[5 + 4 * m] == '0' ? "" : "[",
						*sendbuf[5 + 4 * m] == '0' ? "" : sendbuf[5 + 4 * m],
						*sendbuf[5 + 4 * m] == '0' ? "" : "]"
					);

				}
				printf("선택할 변수: ");
				scanf("%s", buf);
				send(c_socket, buf, strlen(buf), 0);
				printf("입력할 값:");
				scanf(" %s", buf);
				send(c_socket, buf, sizeof(buf), 0);
			}
		
	}
	closesocket(c_socket);
	
	return 0;
}