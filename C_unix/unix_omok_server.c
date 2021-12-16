
// mysql을 사용하기 위해서 MySQL_c api가 필요합니다. 아래 명령어로 설치 가능합니다.
// sudo apt-get install libmysqlclient-dev

// 컴파일 명령어는 아래와 같습니다. mysql을 활용하기 위해서 해당 문구를 입력해야하니 확인 부탁드립니다.
// gcc unix_omok_server.c -o unix_omok_server -lmysqlclient -L/usr/lib64/mysql 

// 할일 :  

// client 구현 및 sever와 소켓 통신 연결 
// 오목 로직 구현(socket으로 입력 받고, 화면에 reload, 승리판정 logic, 무승부판정 logic)
// mysql table 구성 및 서버 열기
// table 구성에 맞춰 server 수정하
// spring활용 webpage 및 server 구현
// 

#include "/usr/include/mysql/mysql.h" 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#define BUFSIZE 64 
#define PORT 9000

int DB_INDEX=1;
char* Complete="Complete",*Failed="Failed";

//mysql error 발생시 동작하는 logic
void finish_with_error(MYSQL *conn){
	fprintf(stderr,"%s\n",mysql_error(conn));
	mysql_close(conn);
	exit(1);
}

//socket관련 error 발생시 동작하는 logic
void error_handling(char *message) {
	fputs(message, stderr);
	putc('\n', stderr);
	exit(1);
}

int main(int argc, char **argv)

{
	int serv_sock;
      int clnt_sock;
	int nSockOpt;
	char message[BUFSIZE];
	
	char* data[2],*ptr,query[64];
	memset(query,0,sizeof(query));

	int str_len;
	struct sockaddr_in serv_addr;
      struct sockaddr_in clnt_addr;

      int clnt_addr_size;
	int i=0;


	printf("		---------------------------------------------------\n\n");

	printf("    			유닉스 2021학년도 2학기 프로젝트2 과제\n");
	printf("		socket network를 활용한 실시간 오목게임 서버 입니다.\n\n");

	printf("		---------------------------------------------------\n");



	MYSQL *conn = mysql_init(NULL);
		
	if(conn==NULL) {
		fprintf(stderr,"%s\n",mysql_error(conn));
		exit(1);
	}
	




	/*

	
		// 제출시에는, mysql서버로 변경해서 제출할 것!! 
	

	*/
	if(mysql_real_connect(conn,"localhost","root","rootroot","unix_omok",0,NULL,0)==NULL) {
		finish_with_error(conn);
	}

	// 데이터 베이스 생성 부
	// if(mysql_query(conn,"*******")){
	// 	finish_with_error(conn);
	// }
			
	
      serv_sock = socket(PF_INET, SOCK_STREAM, 0);   
      if(serv_sock == -1) {
            error_handling("socket() error");
      }

	memset(&serv_addr, 0, sizeof(serv_addr));
      serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
     	serv_addr.sin_port = htons(9000);

															       
	nSockOpt=1;

	setsockopt(serv_sock, SOL_SOCKET,SO_REUSEADDR,&nSockOpt,sizeof(nSockOpt));

	// 소켓에 주소 할당
	if( bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr) )==-1) {
	 	error_handling("bind() error");
	}	
																       
	// 연결 요청 대기 상태로 진입															       
	if(listen(serv_sock, 5) == -1) {
		error_handling("listen() error");
	}

																	       
	 clnt_addr_size = sizeof(clnt_addr);

	while(1){
	      /* 연결 요청 수락 */
	      clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_size);
	      
	      if(clnt_sock == -1) {
	            error_handling("accept() error");
	      }
	}
																	           

	while( (str_len = read(clnt_sock, message, BUFSIZE)) != 0) {

		ptr=strtok(message," ");
		while(ptr!=NULL) {

			data[i]=ptr;
			ptr=strtok(NULL," ");
			i=i+1;
		}

		// 이후에 수정
		// 넘어가는 데이터는 Index/흑돌,백돌 ID/ 승리한 ID/ 게임 수행 시간/이다.
		snprintf(query,64,"insert into omok_data values(%d,'%s','%s','%s','%d')",DB_INDEX++,"test","test1","test",1234);
		
		// printf 구문은 확인을 위한 부분이라고 보면 됩니다.
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
	}
	close(clnt_sock);
	mysql_close(conn);
	return 0;	
}
		
