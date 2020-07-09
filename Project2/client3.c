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
	char rcvBuffer[BUFSIZE];//�������� ������ �޼����� �����ϴ� ����
	char sendBuffer[BUFSIZE];
	WSADATA wsadata;

	WSAStartup(MAKEWORD(2, 0), &wsadata);
	//1. Ŭ���̾�Ʈ ���� ����
	c_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED); //������ ������ �������� ����
	//2.���� ���� �ʱ�ȭs
	memset(&c_addr, 0, sizeof(c_addr));
	c_addr.sin_addr.s_addr = inet_addr(IPADDR); //������ IP ���� (127.0.0.1)
	c_addr.sin_family = AF_INET;
	c_addr.sin_port = htons(PORT);

	//3. ������ ����
	if (connect(c_socket, (struct sockaddr*)&c_addr, sizeof(c_addr)) == SOCKET_ERROR) {
		//���� ���ӿ� �����ϸ� 
		printf("Cannot Connect\n"); //Cannot connect �޼��� ���
		//closesocket(c_socket); //�ڿ� ȸ��
		return -1;  //���μ��� ����
	}
	
	//5. �������� ���� �޽��� �б� 
	recv(c_socket,&i, sizeof(int), 0);
	recv(c_socket, rcvBuffer, i, 0);
	rcvBuffer[i] = '\0';
	//rcvBuffer������ i������ ���ڿ� ���� ����
	printf("received data: %s\n",rcvBuffer); //�������� ���� �޼��� ���
	printf("received data: %s\n", rcvBuffer); //�������� ���� �޼��� ���
	
	closesocket(c_socket);
	return 0;
}