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
	// 1. ���� ���� ����
	//���� ���� = Ŭ���̾�Ʈ�� ���� ��û�� ó��(���)�� �ֱ� ���� ����
	s_socket = socket(PF_INET, SOCK_STREAM, 0); //TCP/IP ����� ���� ���� ���� ����

	//2. ���� ���� �ּ� ����
	memset(&so_addr, 0, sizeof(so_addr)); //so_addr�� ���� ��� 0����  �ʱ�ȭ
	so_addr.sin_addr.s_addr = htonl(INADDR_ANY); //IP �ּ� ����
	so_addr.sin_family = AF_INET;
	so_addr.sin_port = htons(PORT);

	//3. ���� ���Ϲ��ε�
	if (bind(s_socket, (struct sockaddr*)&so_addr, sizeof(so_addr)) == -1) {
		//���ε� �۾� ���� ��, Cannot Bind �޽��� ��� �� ���α׷� ����
		printf("Cannot Bind\n");
		printf("%d", errno);
		return -1;
	}

	//4.listen() �Լ� ����
	if (listen(s_socket, 5) == -1) {
		printf("listen Fail\n");
		return -1;
	}

	//5. Ŭ���̾�Ʈ ��û ó��
	// ��û�� ����� ��, Hello World �޼����� ������
	while (1) { //���� ����
		len = sizeof(c_addr);
		printf("Ŭ���̾�Ʈ ������ ��ٸ��� ��....\n");
		c_socket = accept(s_socket, (struct sockaddr*)&c_addr, &len);
		//Ŭ���̾�Ʈ�� ��û�� ���� ���(accept)�� �ְ�, �ش� Ŭ���̾�Ʈ�� ����� �� �ֵ��� Ŭ���̾�Ʈ ����(c_socket)�� ��ȯ��.
		printf("/client is connected\n");
		printf("Ŭ���̾�Ʈ ���� ���\n");
		//n = recv(c_socket, rcvbuffer, sizeof(rcvbuffer),0);
		send(c_socket, rcvbuffer, strlen(rcvbuffer), 0);

		closesocket(c_socket);
	}
	close(s_socket);
	return 0;
}
