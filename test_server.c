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

#define NONE 0		// NONE, BLACK, WHITE 바둑판의 값들을 설정하기 위한 매크로
#define BLACK 1
#define WHITE 2


int i;                                          
int j;

int count;
int board[14][14];

int game = 1;




int Win_Check() {

   for (i = 2; i < 13; i++) {                     // 가로 또는 세로를 검사하기 위한 반복문
      for (j = 0; j < 15; j++) {                  
         if (board[j][i - 2] == 1 && board[j][i - 1] == 1 && board[j][i] == 1 && board[j][i + 1] == 1 && board[j][i + 2] == 1) {   // 가로방향
            printf("*** 흑돌 승리 ***\n");
            return 1;
         }
         else if (board[j][i - 2] == 2 && board[j][i - 1] == 2 && board[j][i] == 2 && board[j][i + 1] == 2 && board[j][i + 2] == 2) {
            printf("*** 백돌 승리 ***\n");
            return 1;
         }

         else if (board[i - 2][j] == 1 && board[i - 1][j] == 1 && board[i][j] == 1 && board[i + 1][j] == 1 && board[i + 2][j] == 1) {   // 세로방향
            printf("*** 흑돌 승리 ***\n");
            return 1;
         }
         else if (board[i - 2][j] == 2 && board[i - 1][j] == 2 && board[i][j] == 2 && board[i + 1][j] == 2 && board[i + 2][j] == 2) {
            printf("*** 백돌 승리 ***\n");
            return 1;
         }
      }
   }

   for (i = 2; i < 13; i++) {                  // 대각선
      for (j = 2; j < 13; j++) {               
         if (board[j - 2][i - 2] == 1 && board[j - 1][i - 1] == 1 && board[j][i] == 1 && board[j + 1][i + 1] == 1 && board[j + 2][i + 2] == 1) { //왼쪽 위에서 오른쪽 밑으로 내려가는 대각선
            return 1;
         }
         else if (board[j - 2][i - 2] == 2 && board[j - 1][i - 1] == 2 && board[j][i] == 2 && board[j + 1][i + 1] == 2 && board[j + 2][i + 2] == 2) {
            printf("*** 백돌 승리 ***\n");
            return 1;
         }

         else if (board[j + 2][i - 2] == 1 && board[j + 1][i - 1] == 1 && board[j][i] == 1 && board[j - 1][i + 1] == 1 && board[j - 2][i + 2] == 1) { // 왼쪽 아래에서 오른쪽 위로 올라가는 대각선
            printf("*** 흑돌 승리 ***\n");
            return 1;
         }
         else if (board[j + 2][i - 2] == 2 && board[j + 1][i - 1] == 2 && board[j][i] == 2 && board[j - 1][i + 1] == 2 && board[j - 2][i + 2] == 2) {
            printf("*** 백돌 승리 ***\n");
            return 1;
         }
      }
   } //조건이 갖춰지면 1, 아니면 0.
   return 0;
}







void my_send(int sock, char* msg){
	if(send(sock, msg, BUFSIZ, 0) == -1){
		perror("send");
		exit(0);
	}
}
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




void print_board(){
	system("clear");
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



void put_board(char * buf){
	char temp[BUFSIZ];
	char *temp_ptr;
	int n,m;
	 
	m = buf[0] - 65;
	n = (int)strtol(buf+2, &temp_ptr, 10);
		
	
	n--;	// 배열은 0~13의 값을 사용하기 때문에 하나씩 값을 빼준다.

	// set_board()함수가 i, j값을 사용하기 때문에 call by reference로 값을 바꿔준다.
	i = n;
	j = m;
}

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
	char wait_msg[BUFSIZ], buf[BUFSIZ], idBlack[BUFSIZ], idWhite[BUFSIZ], winner[BUFSIZ], query[BUFSIZ];
	int msglen, n;
	int member_num =0;
	int member_sd[CLIENTNUM];

	int thread_result;
	pthread_t counting_thread;
	
	int once = 1;
	count = 0;




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






	connect_request(&sockfd, &my_addr);
	


	printf("		---------------------------------------------------\n\n");

	printf("    			유닉스 2021학년도 2학기 프로젝트2 과제\n");
	printf("		socket network를 활용한 실시간 오목게임 서버 입니다.\n\n");

	printf("		---------------------------------------------------\n");


	
	while(game){
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

				// 먼저 들어온 client가 흑돌을 잡게되고, 흑 id로 저장
				my_recv(member_sd[0], idBlack);
				printf("%s\n",idBlack);
				
				strcpy(wait_msg, "상대방을 기다려주세요.\n");
				msglen = strlen(wait_msg);
				my_send(member_sd[0], wait_msg);
				once = 0;
			}


			
			if(member_num >= 2 ) {	// 클라이언트가 2명이 되면 대국을 시작
				if(!once){			// 대국을 시작한다는 메세지는 시작 1번만 나오므로 once 변수로 단 1번만 출력한다.

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
				// pthread_create(&counting_thread, NULL, counting, NULL);
				// pthread_join(counting_thread, (void **)&thread_result);

				while(1){
					my_recv(member_sd[0], buf);
					
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


	snprintf(query,64,"insert into omok_data('id_black','id_white','winner','play_time') values('%s','%s','%s','%d')",idBlack,idWhite,winner,1234);
		
	// printf 구문은 확인을 위한 부분이라고 보면 됩니다.
	printf("-----------------------------------------\n");
	printf("%s\n",query);

	//sql_query는 정상일 경우 0 아닐 경우엔 !=0
	if(mysql_query(conn,query)) {
		printf("is Failed\n");
		printf("-----------------------------------------\n");
	}

	else {
		printf("is Complete\n");
		printf("-----------------------------------------\n");
	}

	printf("%s\t",idBlack);

	printf("%s\t",idWhite);

	printf("%s\t",winner);

	printf("%d",1234);




	return 0;
}