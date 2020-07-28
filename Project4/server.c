#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include<Windows.h>
#include<process.h>
/*
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
*/
#define SIZE 1000
#define PORT 9000
#define SHSIZE 300   

void* do_send(int, char*);
void Msg(int);
void SHM_Create();
void SHM_Update(int);

char sendBuf[SIZE];
char readBuf[SIZE];

pthread_t thread;
pthread_mutex_t mutex;

typedef struct tag_st {
	int num;
	char ch;
} basic;

typedef struct {
	basic st;   int* p;
	long double ldar[128];
} contain;


int shmid; // 공유메모리 아이디 
key_t key; // 공유메모리 접근 키 
void* addr;

contain data;

void EraseSpace(char str[]) {
	char temp[64];
	int i, index = 0;

	for (i = 0; str[i] != 0; i++) {
		if (str[i] == '\n') {
			//		printf("enter");
			str[i] = '\t';
		}
	}

	for (i = 0; str[i] != 0; i++) {
		if (str[i] != '\t') {
			temp[index] = str[i];
			index++;
			//	printf("find tab");
		}
	}


	temp[index] = 0;

	index = 0;
	for (i = 0; temp[i] != 0; i++) {
		str[index] = temp[i];
		index++;
	}

	str[index] = 0;

	//printf("yup : %s\n", str);
}

char* toknizer(char* dest, const char* tok, char** next)
{
	char* ret = NULL;

	if (dest == NULL)
	{
		dest = *next;
	}

	ret = strstr(dest, tok);
	if (ret)
	{
		*ret = '\0';
		*next = ret + strlen(tok);
	}

	return dest;
}

int ret = 1; // SHMcreate  T/F T=0 F=1

int main(int argc, char* argv[]) {

	int c_socket, s_socket;
	struct sockaddr_in s_addr, c_addr;
	int    len;
	int    i, j, n;
	int    res;


	if (pthread_mutex_init(&mutex, NULL) != 0) {
		printf("Can not create mutex\n");
		return -1;
	}

	s_socket = socket(PF_INET, SOCK_STREAM, 0);
	memset(&s_addr, 0, sizeof(s_addr));
	s_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	s_addr.sin_family = AF_INET;
	s_addr.sin_port = htons(PORT);

	if (bind(s_socket, (struct sockaddr*)&s_addr, sizeof(s_addr)) == -1) {
		printf("Can not Bind\n");
		return -1;
	}

	if (listen(s_socket, 5) == -1) {
		printf("listen Fail\n");
		return -1;
	}

	while (1) {
		len = sizeof(c_addr);
		c_socket = accept(s_socket, (struct sockaddr*)&c_addr, &len); // 클라이언트 접속 허용 

		printf("... server ... \n");

		if (ret == 1) {
			Msg(c_socket);
			SHM_Create();
		}

		pthread_mutex_lock(&mutex); // 락걸기
		pthread_create(&thread, NULL, SHM_Update, (int)c_socket); // 스레드 만들기
		pthread_mutex_unlock(&mutex); // 락 풀기

	//	SHM_Update(c_socket);	
	}

	ret = 0;
	close(c_socket);
	close(s_socket);
	return 0;

}

void SHM_Create() {

	key = 2020;

	//공유 메모리생성
	shmid = shmget(key, SHSIZE, IPC_CREAT | 0666);
	if (shmid < 0) { //error = -1
		perror("shmget");
		exit(0);
	}

	// 공유 메모리를 사용하기 위해 프로세스 메모리에 붙인다. 
	addr = shmat(shmid, NULL, 0);
	if (addr == (char*)-1) {
		perror("shmat");
		exit(1);
	}
	else {
		printf("ShardMemory Create Done..!\n");
		ret = 0;
	}
}

void SHM_Update(int c) {
	int c_socket = c;

	while (1) {
		int n = 0;

		char name[30];
		name[0] = '\0';
		n = read(c_socket, name, sizeof(name));
		printf("client put name : %s\n", name);

		char value[30];
		value[0] = '\0';
		char basic_name[30];
		char basic_value[30];

		if (strstr(name, "st")) {
			printf("client select st\n");

			//printf("sendBuf : %s size : %x\n",sendBuf, sizeof(sendBuf));
			Msg(c_socket);
			printf("message send complete !! \n");


			n = read(c_socket, basic_name, sizeof(basic_name));
			printf("basic name : %s\n", basic_name);


			n = read(c_socket, basic_value, sizeof(basic_value));
			basic_value[n] = 0;
			printf("basic value : %s\n", basic_value);


			if (strstr(basic_name, "num")) {
				int val;
				val = atoi(basic_value);
				(*(contain*)addr).st.num = val;
			}
			else if (strstr(basic_name, "ch")) {
				(*(contain*)addr).st.ch = basic_value;
			}

			basic_name[0] = '\0'; basic_value[0] = '\0';
			//write(c_socket, "c200", );					
		}
		else if (strstr(name, "p")) {
			printf("client select p\n");

			n = read(c_socket, value, sizeof(value));
			value[n] = 0;
			printf("client put value : %s\n", value);

			int k;
			k = atoi(value);

			printf("P value : %d\n", k);
			(*(contain*)addr).p = k;

		}
		else if (strstr(name, "ldar")) {
			char idx[10];
			n = read(c_socket, idx, sizeof(idx));
			idx[n] = 0;
			printf("idx : %s\n", idx);
			//                printf( "%02X%02X%02X%02X\n", idx[0], idx[1], idx[2], idx[3] );

			n = read(c_socket, value, sizeof(value));
			value[n] = 0;
			printf("client put value : %s\n", value);

			int index;
			long double val;
			val = atof(value);
			index = atoi(idx);
			(*(contain*)addr).ldar[index] = val;
		}
	} //end while
	//name[0] = '\0'; value[0] = '\0';
}


void Msg(int n) {
	int c_socket = n;
	FILE* rFILE = fopen("struct.h", "r");
	FILE* wFILE = fopen("struct.log", "w");

	char strline[100];
	char cpline[1000];
	char* cutline;

	char str_read[SIZE];
	char* a = "struct";
	char* b = "}";
	char* c = ";";

	char* token = NULL;
	char* context = NULL;
	char* ptr = NULL;

	const char* arr[3];
	const char* arr2[3];
	const char* word = NULL;
	char tmp[100];

	char buf1[1000];
	char buf2[1000];
	char buf3[1000];
	char buf4[1000];
	char buf5[1000];
	int cnt = 0;
	int sumcnt = 0;
	char TagName[] = "\\";
	char TypeName[20];
	char cntStr[5];
	char Msg[300];
	char Check[300];

	char structName[100]; // 구조체 멤버 체크 
	char* d = "basic";

	cpline[0] = '\0';
	strline[0] = '\0';

	while (fgets(strline, sizeof(strline), rFILE)) {
		// printf("맨 처음 : %s", strline);

	 //    if(strncmp(strline, a, strlen(a) ) == 0 || strncmp(strline, c, strlen(c) ) == 0)
		if (strstr(strline, ";")) {
			strcat(cpline, strline);
			//printf("\n cpline : %s \n", cpline);
		}

		//printf("\ncpline : %s\n", cpline);
		//printf("\nstrline : %s\n", strline);

		if (strstr(strline, a))
		{

			//printf("자르기 전 : %s\n", strline);
			token = strtok(strline, "{");
			ptr = strstr(strline, a);
			token = strtok_r(ptr, " ", &context);
			context[strlen(context) - 1] = '\0';
			//	printf("**context : %s\n", context);

			strcpy(TypeName, context);
			// printf("ptr : %s\n", ptr);
//            printf("@@ \n test : %s\n", context);
			buf1[0] = '\0';
			buf3[0] = '\0';
			buf2[0] = '\0';
			buf4[0] = '\0';
			buf5[0] = '\0';
			//			strcat(buf2, TagName);
			//			strcat(buf2, context);

						//strcat();		
		}


		if (strncmp(strline, b, strlen(b)) == 0)
		{
			//printf("구조체 타입명 : ");
			//printf("자르기 전 : %s\n", strline);

//			buf2[0] = '\0';


			token = strtok(strline, "}");
			token[strlen(token) - 2] = '\0';


			if (token[0] == '\0') {
				//strcat(buf1, "없음\n");
			}
			else {
				//strcat(buf1, token);
				strcpy(structName, token);
			}


			//printf("**struct Name : %s\n\n", structName);

			//sprintf(Msg,"%d" ,sizeof(buf5));
		//	strcat(buf1,Msg);
//			strcat(buf2,"멤버변수의 개수 : ");
			strcat(buf1, "@");
			sprintf(cntStr, "%d", sumcnt);
			strcat(buf1, cntStr);
			cnt = 0;
			sumcnt = 0;

			EraseSpace(buf1);
			EraseSpace(buf2);
			EraseSpace(buf3);
			EraseSpace(buf4);

			strcat(buf5, buf1);
			strcat(buf5, buf2);
			strcat(buf5, buf3);

			EraseSpace(buf5);
			puts(buf5);
		}

		if (strstr(strline, c)) {
			Msg[0] = '\0';
			sumcnt++;
			//printf("/n^^^ test ^^^ : cp :  %s    a : %s\n", cpline, strline);
			strline[strlen(strline) - 2] = '\0';
			if (strstr(strline, structName)) {
				strline[strlen(strline) - 1] = '\0';
			}
			arr[0] = strtok(strline, " ");

			int i = 1;
			for (i = 1; i < 3; i++) {
				arr[i] = strtok(NULL, " ");
				//printf("arr[%d] : %s\n", i, arr[i]);
			}



			Msg[0] = '\0';
			if (strstr(arr[0], structName)) {
				sprintf(Msg, "@/%s", TypeName);
				strcat(buf2, Msg);
			}

			if (arr[2] == NULL) {

				word = arr[1];
				strcpy(tmp, arr[1]);

				if (strstr(arr[1], "]")) {
					token = strtok(arr[1], "[");
					arr[1] = token;
				}

				if (strstr(arr[1], "*")) {
					//printf("*있음 !!! \n");
					token = strtok(arr[1], "*");
					arr[1] = token;
				}

				// 멤버변수 변수명
				sprintf(Msg, "@%s", arr[0]);
				strcat(buf3, Msg);
				sprintf(Msg, "@%s", arr[1]);
				strcat(buf3, Msg);
				strcpy(word, tmp);
			}
			else {
				word = arr[2];
				strcpy(tmp, arr[2]);
				//	printf("word : %s \n arr[2] = %s\n", word, arr[2]);

				if (strstr(arr[2], "]")) {
					token = strtok(arr[2], "[");
					arr[2] = token;
				}

				if (strstr(arr[2], "*")) {
					token = strtok(arr[2], "*");
					arr[2] = token;
					//		   printf("str_r : %s\n\n", arr[2]);
				}

				sprintf(Msg, "@%s %s", arr[0], arr[1]);
				strcat(buf3, Msg);
				sprintf(Msg, "@%s", arr[2]);
				strcat(buf3, Msg);
				strcpy(word, tmp);
			}

			// 배열여부 체크		
//				printf("aaa : %s\n", word);


				//	printf("__word : %s\n", word);
			if (strstr(word, "[")) {
				token = strtok_r(word, "[", &word);
				token = strtok(word, "]");
				sprintf(Check, "@%s", word);
			}
			else {
				sprintf(Check, "@0");
			}
			strcat(buf3, Check);

			// 포인터 체크
			if (strstr(word, "*")) {
				sprintf(Check, "@YES");
			}
			else {
				sprintf(Check, "@NO");
			}
			strcat(buf3, Check);

			word = NULL;
			tmp[0] = '\0';
		}

	}

	fclose(wFILE);
	fclose(rFILE);


	do_send(c_socket, buf5);
}


void* do_send(int arg, char* Data) {

	int c_socket = arg;
	//char SendData[1024];
	int n = strlen(Data);
	printf("%d\n", n);
	//printf("%x\n",n);
	write(c_socket, &n, sizeof(n));
	write(c_socket, Data, strlen(Data));
	EraseSpace(Data);
}