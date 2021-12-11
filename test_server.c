// mysql을 사용하기 위해서 MySQL_c api가 필요합니다. 아래 명령어로 설치 가능합니다.
// sudo apt-get install libmysqlclient-dev

// 컴파일 명령어는 아래와 같습니다. mysql을 활용하기 위해서 해당 문구를 입력해야하니 확인 부탁드립니다.
// gcc unix_omok_server.c -o unix_omok_server -lmysqlclient -L/usr/lib64/mysql 

// 할일 :  

// client에서 sever와 소켓 통신으로 승부 결과 및 ID보내기
// mysql table 구성 및 서버 열기
// table 구성에 맞춰 server 수정하기
// spring활용 webpage 및 server 구현
// 
#include "/usr/include/mysql/mysql.h" 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
	
#define PORTNUM 9000
#define CLIENTNUM 2 

void my_send(int sock, char* msg){
	if(send(sock, msg, BUFSIZ, 0) == -1){
		perror("send");
		exit(0);
	}
}

//mysql error 발생시 동작하는 logic
void finish_with_error(MYSQL *conn){
	fprintf(stderr,"%s\n",mysql_error(conn));
	mysql_close(conn);
	exit(1);
}

void my_recv(int sock, char * buf,int *member_num)
{
	int n;
	
	if((n = recv(sock, buf, BUFSIZ, 0)) == -1){
		perror("recv");
		exit(1);
	}
	buf[n] = '\0';
	if(!strcmp(buf, "quit")){
		*member_num--;
	}
}

void connect_request(int *sockfd, struct sockaddr_in *my_addr)
{
	int yes = 1;
		
	if ((*sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("Socket");
		exit(1);
	}
	
	memset(my_addr->sin_zero, 0, sizeof my_addr->sin_zero);
	
	my_addr->sin_family = AF_INET;
	my_addr->sin_port = htons(PORTNUM);
	my_addr->sin_addr.s_addr = htonl(INADDR_ANY);			// 이 컴퓨터에 존재하는 랜카드 중 사용가능한 랜카드의 IP사용
	
	
	if (setsockopt(*sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
		perror("setsockopt");
		exit(1);
	}
		
	if (bind(*sockfd, (struct sockaddr *)my_addr, sizeof(struct sockaddr)) == -1) {	// 소켓에 이름지정
		perror("Unable to bind");
		exit(1);
	}
	if (listen(*sockfd, 5) == -1) {	// 클라이언트 연결대기. 큐의 길이 5
		perror("listen");
		exit(1);
	}
	printf("TCPServer Waiting for client\n");
}
int main()
{
	fd_set read_fds;
	int nfds, i; // nfds : 소켓개수 + 1
	struct sockaddr_in my_addr, client_addr;
	int sockfd= 0, clisd, clilen = sizeof(client_addr);
	char wait_msg[BUFSIZ], buf[BUFSIZ];
	int msglen, n;
	int member_num =0;
	int member_sd[CLIENTNUM];

	int DB_INDEX=1; // database의 pk(index 값)
	char* Complete="Complete",*Failed="Failed";
	int i=0;

	printf("---------------------------------------------------\n\n");

	printf("      	유닉스 2021학년도 2학기 프로젝트2 과제      \n");
	printf("	socket network를 활용한 실시간 오목게임 서버 입니다.\n\n");

	printf("---------------------------------------------------\n");


	MYSQL *conn = mysql_init(NULL);
		
	if(conn==NULL) {
		fprintf(stderr,"%s\n",mysql_error(conn));
		exit(1);
	}
	


	/*

	
		// 제출시에는, mysql서버로 변경해서 제출할 것!! 
	

	*/
	if(mysql_real_connect(conn,"localhost","root","rootroot",NULL,0,NULL,0)==NULL) {
		finish_with_error(conn);
	}





	int once = 1;
	connect_request(&sockfd, &my_addr);
	
	
	while(1){
		FD_ZERO(&read_fds);	// read_fds를 0으로 초기화
		if(member_num > 0) {						// 오목에 참가한 사람이 있으면
			nfds = member_sd[member_num -1] + 1;	// 마지막으로 참가한 클라이언트 기술자 + 1
		}
		else {										// 없으면
			nfds = sockfd + 1;						// 서버 기술자 + 1
		}
		
		FD_SET(sockfd, &read_fds);					// read_fds에 sockfd 추가
		
		for(i=0;i < member_num ; i++){				// 클라이언트들을 read_fds에 추가
			FD_SET(member_sd[i], &read_fds);
		}
		
		// select 함수로 read_fds에 저장되어 있는 기술자를 감시한다.
		if(select(nfds, &read_fds, NULL, NULL, NULL) == -1){ 
			perror("select");
			exit(4);
		}
		
		if(FD_ISSET(sockfd, &read_fds)) {	
			if((clisd = accept(sockfd, (struct sockaddr *)&client_addr, &clilen)) == -1){	// 요청 연결 수락.
				perror("accept");
				exit(1);
			}
			
			member_sd[member_num++] = clisd; // 연결 요청한 클라이언트 소켓 기술자를 배열에 추가
			printf("%d번 사용자 연결\n", clisd);
			
			
			/*
			초기에 상대방이 없을경우 기다려달라는 메세지 출력
			*/
			if(member_num == 1 && once){
				strcpy(wait_msg, "상대방을 기다려주세요.\n");
				msglen = strlen(wait_msg);
				
				my_send(member_sd[0], wait_msg);
				once = 0;
			}
			
			if(member_num >= 2 ) {	// 클라이언트가 2명이 되면 대국을 시작
				if(!once){			// 대국을 시작한다는 메세지는 시작 1번만 나오므로 once 변수로 단 1번만 출력한다.
					strcpy(wait_msg, "대국을 시작합니다..\n");
					msglen = strlen(wait_msg);
					
					for(i =0 ; i < member_num; i++){ // 등록된 모든 클라이언트에게 대국 시작 메세지 발송
						my_send(member_sd[i], wait_msg);
					}
					
					once = 1;
				}
				/*
				클라이언트 0에게 수신한 것을 클라이언트 1에게 발송
				클라이언트 1에게 수신한 것을 클라이언트 0에게 발송 
				반복
				*/
				while(1){
					my_recv(member_sd[0], buf, &member_num);
					my_send(member_sd[1], buf);
					printf("%s", buf);
					
					my_recv(member_sd[1], buf, &member_num);
					my_send(member_sd[0], buf);
					printf("%s", buf);
				}
			}
		}
	}


	/*


	쿼리 날리기전에, 필요한 데이터 소켓으로 받아오는 로직 추가


	*/
	// 입력받은 ID(Pk)(DB)INDEX, 닉네임 (흑/백), 승리한 사람(흑,백), play time 

	snprintf(query,64,"insert into unix_omok values(%d,%d,'%s')", DB_INDEX++, atoi(data[0]),data[1]);
		
	// 쿼리문 확인
	printf("-----------------------------------------\n");
	printf("%s\n",query);

	//sql_query는 정상일 경우 0 아닐 경우엔 !=0
	if(mysql_query(conn,query)) {
		i=0;
		memset(query,0,sizeof(query));
		send(clnt_sock,Failed,sizeof(Failed)+1,0);
		printf("Index Number : %d is Failed\n",DB_INDEX-1);
		DB_INDEX--;
		printf("-----------------------------------------\n");
		continue;
	}

	else {
		i=0;
		send(clnt_sock,Complete,sizeof(Complete)+1,0);
		printf("Index Number : %d is Complete\n",DB_INDEX-1);
		printf("-----------------------------------------\n");
		memset(query,0,sizeof(query));
	}

	mysql_close(conn);

	printf("서버를 종료합니다.\n");
	return 0;
}