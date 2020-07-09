#pragma warning(4:4996)
#include <string.h>
#include <stdio.h>
#include <winsock2.h>
#include <Windows.h>
#include <stdlib.h>

#pragma comment(lib,"ws2_32.lib")
#define PORT 10001
#define IPADDR "127.0.0.1"
#define BUFSIZE 100

int main() {
	int c_socket;
	struct sockaddr_in c_addr;
	int n;
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
	while (1) {
		//5. �������� ���� �޽��� �б� 
		n = recv(c_socket, rcvBuffer, sizeof(rcvBuffer), 0);
		rcvBuffer[n] = '\0';
		printf("server: %s\n", rcvBuffer); //�������� ���� �޼��� ���
		//printf("rcvBuffer length: %d\n", n); //3-2. �������� ���� �޼����� ���� ��� 
		//�������� ���� �޼����� ���� ������
		printf("client: ");
		gets(sendBuffer);
		send(c_socket, sendBuffer, (int)strlen(sendBuffer),0);
		//�������� ���� �޽��� �б�
		n = recv(c_socket, rcvBuffer, sizeof(rcvBuffer), 0);
		rcvBuffer[n] = '\0'; //���ڿ� �޺κ� ���� ����
		printf("server: %s\n", rcvBuffer); //�������� ���� �޼��� ���
		closesocket(c_socket);
	}
	return 0;
}