#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
#include "/usr/include/mysql/mysql.h"  

#define PORTNUM 9000
#define CLIENTNUM 2 

// NONE, BLACK, WHITE 바둑판의 값들을 설정하기 위한 매크로
#define NONE 0		
#define BLACK 1
#define WHITE 2


int i;                                          
int j;

int count;
int board[14][14];

int game = 1;




int Win_Check() {

	// 가로 또는 세로에 5줄이 있는지 확인
   for (i = 2; i < 13; i++) {                     
      for (j = 0; j < 15; j++) {                  
      	 // 가로
         if (board[j][i - 2] == 1 && board[j][i - 1] == 1 && board[j][i] == 1 && board[j][i + 1] == 1 && board[j][i + 2] == 1) { 
            printf("*** 흑돌 승리 ***\n");
            return 1;
         }
         else if (board[j][i - 2] == 2 && board[j][i - 1] == 2 && board[j][i] == 2 && board[j][i + 1] == 2 && board[j][i + 2] == 2) {
            printf("*** 백돌 승리 ***\n");
            return 1;
         }
         // 세로
         else if (board[i - 2][j] == 1 && board[i - 1][j] == 1 && board[i][j] == 1 && board[i + 1][j] == 1 && board[i + 2][j] == 1) {   
            printf("*** 흑돌 승리 ***\n");
            return 1;
         }
         else if (board[i - 2][j] == 2 && board[i - 1][j] == 2 && board[i][j] == 2 && board[i + 1][j] == 2 && board[i + 2][j] == 2) {
            printf("*** 백돌 승리 ***\n");
            return 1;
         }
      }
   }
	// 대각선 중에 5줄이 있는지 확인
   for (i = 2; i < 13; i++) {                  
      for (j = 2; j < 13; j++) {               
      	//왼쪽 위에서 오른쪽 밑으로 내려가는 대각선
         if (board[j - 2][i - 2] == 1 && board[j - 1][i - 1] == 1 && board[j][i] == 1 && board[j + 1][i + 1] == 1 && board[j + 2][i + 2] == 1) {
            return 1;
         }
         else if (board[j - 2][i - 2] == 2 && board[j - 1][i - 1] == 2 && board[j][i] == 2 && board[j + 1][i + 1] == 2 && board[j + 2][i + 2] == 2) {
            printf("*** 백돌 승리 ***\n");
            return 1;
         }
         // 왼쪽 아래에서 오른쪽 위로 올라가는 대각선
         else if (board[j + 2][i - 2] == 1 && board[j + 1][i - 1] == 1 && board[j][i] == 1 && board[j - 1][i + 1] == 1 && board[j - 2][i + 2] == 1) { 
            printf("*** 흑돌 승리 ***\n");
            return 1;
         }
         else if (board[j + 2][i - 2] == 2 && board[j + 1][i - 1] == 2 && board[j][i] == 2 && board[j - 1][i + 1] == 2 && board[j - 2][i + 2] == 2) {
            printf("*** 백돌 승리 ***\n");
            return 1;
         }
      }
   } 
   // 승리한 돌이 있는경우, 1 없는 경우 0 반환
   return 0;
}






// socket으로 데이터를 보낼 때, 오류처리
void my_send(int sock, char* msg){
	if(send(sock, msg, BUFSIZ, 0) == -1){
		perror("send");
		exit(0);
	}
}


// socket으로 데이터를 받을 때, 오류처리
void my_recv(int sock, char * buf)
{
	int n;
	
	if((n = recv(sock, buf, BUFSIZ, 0)) == -1){
		perror("recv");
		exit(1);
	}
}


//mysql error 발생시 동작하는 logic
void finish_with_error(MYSQL *conn){
	fprintf(stderr,"%s\n",mysql_error(conn));
	mysql_close(conn);
	exit(1);
}



// board의 좌표를 기반으로, 바둑판을 그려준다.
void print_board(){
	// 판을 그리기 전에, 터미널의 내용을 모두 지운다.
	system("clear");
	// 모양대로 그리기
	printf("| A │ B │ C │ D │ E │ F │ G │ H │ I │ J │ K │ L │ M │ N │\n");
	printf("┌───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┐───\n");

	for(int i = 0; i< 14; i++) {
		for(int j = 0; j<14; j++) {
			if (board[i][j] % 3 == BLACK) {
            printf("│ ● ");
         }

         else if (board[i][j] % 3 == WHITE) {
            printf("│ ○ ");
         }
         else {
				printf("│   "); // bord[i][j]
			}
		}
		printf("│ %d\n",i+1);

		if(i!=13) {
			printf("├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤─── \n");
		}		
	}
	printf("└───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┘───\n");

}


// 입력받은  값을 좌표값으로 변환한다.
void put_board(char * buf){
	char temp[BUFSIZ];
	char *temp_ptr;

	//char 형에서 65를 배면, 해당하는 숫자값이 나온다.
	j = buf[0] - 65;
	i = (int)strtol(buf+2, &temp_ptr, 10);
		
	// 배열은 0~13의 값을 사용하기 때문에 하나씩 값을 빼준다.
	i--;	
}

// 입력받은 값을 기반으로 board에 흑돌, 백돌을 어디에 위치시킬지 입력으로 넣는다.
void set_board(){
	if ((count % 2) == 0) {
        board[i][j] = BLACK;
    }
    else {
        board[i][j] = WHITE;
    }
}









void connect_request(int *sockfd, struct sockaddr_in *my_addr)
{
	int yes = 1;
		
	if ((*sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("Socket");
		exit(1);
	}
	
	// 소켓 연결
	memset(my_addr->sin_zero, 0, sizeof my_addr->sin_zero);
	
	my_addr->sin_family = AF_INET;
	my_addr->sin_port = htons(PORTNUM);
	my_addr->sin_addr.s_addr = htonl(INADDR_ANY);			
	
	
	if (setsockopt(*sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
		perror("setsockopt");
		exit(1);
	}
	
	// 소켓에 이름지정
	if (bind(*sockfd, (struct sockaddr *)my_addr, sizeof(struct sockaddr)) == -1) {
		perror("Unable to bind");
		exit(1);
	}
	// 클라이언트 연결대기. 큐의 길이 5
	if (listen(*sockfd, 5) == -1) {
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

	// 데이터를 보내고 받을 문자열들
	char wait_msg[BUFSIZ], buf[BUFSIZ], idBlack[BUFSIZ], idWhite[BUFSIZ], winner[BUFSIZ],time[BUFSIZ], query[BUFSIZ];
	int msglen, n;
	int member_num =0;
	int member_sd[CLIENTNUM];
	
	int once = 1;
	count = 0;




	MYSQL *conn = mysql_init(NULL);
		
	// if(conn==NULL) {
	// 	fprintf(stderr,"%s\n",mysql_error(conn));
	// 	exit(1);
	// }

	/*

	
		// 제출시에는, mysql서버로 변경해서 제출할 것!! 
	

	*/

	if(mysql_real_connect(conn,"localhost","root","1234","unix_omok",3306,NULL,0)==NULL) {
		finish_with_error(conn);
	}






	connect_request(&sockfd, &my_addr);
	


	printf("		---------------------------------------------------\n\n");

	printf("    			유닉스 2021학년도 2학기 프로젝트2 과제\n");
	printf("		socket network를 활용한 실시간 오목게임 서버 입니다.\n\n");

	printf("		---------------------------------------------------\n");


	
	while(game){
		FD_ZERO(&read_fds);

		if(member_num > 0) {						
			nfds = member_sd[member_num -1] + 1;	
		}
		else {										
			nfds = sockfd + 1;						
		}

		// read_fds에 sockfd 추가
		FD_SET(sockfd, &read_fds);					
		
		// 클라이언트들을 read_fds에 추가
		for(i=0;i < member_num ; i++){				
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
			
			// 연결 요청한 클라이언트 소켓 기술자를 배열에 추가
			member_sd[member_num++] = clisd; 
			printf("%d번 사용자 연결\n", clisd);
			
			

			//초기에 상대방이 없을경우 기다려달라는 메세지 출력
			if(member_num == 1 && once){

				// 먼저 들어온 client가 흑돌을 잡게되고, 흑 id로 저장
				my_recv(member_sd[0], idBlack);
				printf("%s\n",idBlack);
				
				strcpy(wait_msg, "상대방을 기다려주세요.\n");
				msglen = strlen(wait_msg);
				my_send(member_sd[0], wait_msg);
				once = 0;
			}


			// 클라이언트가 2명이 되면 대국을 시작
			if(member_num >= 2 ) {	
				if(!once){		
					// 대국을 시작한다는 메세지는 시작 1번만 나오므로 once 변수로 단 1번만 출력한다.

					// 나중에 들어온 client가 백돌을 잡게되고, 백 id로 저장
					my_recv(member_sd[1], idWhite);
					printf("%s\n",idWhite);

					strcpy(wait_msg, "대국을 시작합니다..\n");
					msglen = strlen(wait_msg);

					// 클라이언트 둘에게 시작 메세지를 보낸다.
					my_send(member_sd[0], wait_msg);
					my_send(member_sd[1], wait_msg);

					once = 1;
				
				}

				/*
				클라이언트 0에게 수신한 것을 클라이언트 1에게 발송
				클라이언트 1에게 수신한 것을 클라이언트 0에게 발송 
				반복
				*/
				int start = 1;	
				while(1){
					my_recv(member_sd[0], buf);
					
					// 입력받은 값이 quit이라면, 기권으로 받아들인다.
					if(!strcmp(buf, "quit")) {
						strcpy(winner,idWhite);
						start = 2;	
						my_send(member_sd[1], buf);
						game = 0;
						break;
					}
			
					put_board(buf);
					set_board();
					print_board();

					// 입력받은 값을 기준으로 승부가 결정 났는지 여부를 판단한다.
					if(Win_Check()){
						start = 2;
						my_send(member_sd[0], "winner");
						my_send(member_sd[1], "looser");
						strcpy(winner,idBlack);
						game = 0;
						break;
					}
					
					my_send(member_sd[1], buf);
					count ++;
					//printf("%s\n", buf);

					// 흑과 백을 바꿔서 위와 같은 로직 그대로 실행

					my_recv(member_sd[1], buf);
					if(!strcmp(buf, "quit")) {
						strcpy(winner,idBlack);
						start = 2;
						my_send(member_sd[0], buf);	
						game = 0;
						break;
					}
					

					put_board(buf);
					set_board();
					print_board();
					Win_Check();
					if(Win_Check()){
						my_send(member_sd[0], "winner");
						my_send(member_sd[1], "looser");
						strcpy(winner,idWhite);
						start = 2;
						game = 0;
						break;
					}

					my_send(member_sd[0], buf);
					count ++;
					//printf("%s\n", buf);
				}
			}
		}
	}

	my_recv(member_sd[0],time);


	strcpy(query,"insert into omok_data('id_black','id_white','winner','play_time') values('");
	strcat(query,idBlack);
	strcat(query,"','");
	strcat(query,idWhite);
	strcat(query,"','");
	strcat(query,winner);
	strcat(query,"','");
	strcat(query,time);
	strcat(query,"')");
	//sprintf(query,"insert into omok_data('id_black','id_white','winner','play_time') values('%s','%s','%s','%s')",idBlack,idWhite,winner,time);
		
	// printf 구문은 확인을 위한 부분이라고 보면 됩니다.
	printf("-----------------------------------------\n");
	printf("%s ",query);

	//sql_query는 정상일 경우 0 아닐 경우엔 !=0
	if(mysql_query(conn,query)) {
		printf("is Failed\n");
		printf("-----------------------------------------\n");
	}

	else {
		printf("is Complete\n");
		printf("-----------------------------------------\n");
	}

	printf("winner is %s\n",winner);

	printf("play time is %ssec",time);




	return 0;
}