// 타이머를 구현하기 위해 pthread를 활용해서 개발하였습니다.
// 컴파일 후 실행시 아래 명령어로 실행 부탁드립니다.
// ./unix_omok_client -pthread 

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>

#define PORTNUM 9000

// 클라이언트가 1명일 때 소켓기술자의 값은 3
#define CLIENTNUM 3 
// user구조체 배열의 갯수
#define USERNUM 2	


// NONE, BLACK, WHITE 바둑판의 값들을 설정하기 위한 매크로
#define NONE 0		
#define BLACK 1
#define WHITE 2

// 타이머 구현을 위한 전역변수
int timer = 0;
int start = 0;				


// 자신이 흑인지 백인지 구분하는 변수
int player = 0;

int sockfd;




// 오목판을 출력하는 함수
void print_board(int board[][14]){
	// 판을 그리기 전에, 터미널의 내용을 모두 지운다.ㅋ
	system("clear");
	// 모양대로 그리기
	printf("| A │ B │ C │ D │ E │ F │ G │ H │ I │ J │ K │ L │ M │ N │\n");
	printf("┌───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┐───\n");

	for(int i = 0; i< 14; i++) {
		for(int j = 0; j<14; j++) {
			if (board[i][j] % 3 == BLACK) {
            printf("│ ●"); //흑독
         }

         else if (board[i][j] % 3 == WHITE) {
            printf("│ ○"); //백돌
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


//들어온 입력을 좌표로 변환하고, 잘못도니 입력의 경우 다시 입력받도록 구현
void put_board(char * buf, int board[][14], int *i,int *j){
	char temp[BUFSIZ];
	char *temp_ptr;
	int n,m;
	
	while(1){
		// strtok() 함수는 원래의 문자열을 수정하기 때문에 temp변수에 복사해준다.
		strcpy(temp, buf);		
		// 문자열이 'ㅇ,ㅇ'로 이루어지지 않았다면 다시 입력을 받는다.		

		// 보낸 내용이 quit이라면 서버에 quit 메세지를 보내고 종료한다.	
		if(!strcmp(temp, "quit")){				
			printf("기권하였습니다.\n");
			break ;
		}

		else if(!(temp[1] == ',')){	
			printf("','를 이용해 띄어쓰기 없이 입력해 주세요.\n");
			printf("입력 [ex) H,8] : ");
			scanf("%s", buf);
			continue;
		}

		// 앞에 값은 A~N 까지 1byte이기 때문에, m은 1byte n은 이후 나머지 byte 를 사용하는 형식으로 만들 었다. 
		m = buf[0] - 65;
		n = (int)strtol(buf+2, &temp_ptr, 10);
		
		
		n--;	// 배열은 0~13의 값을 사용하기 때문에 하나씩 값을 빼준다.


		if(n < 0 || n > 14){
			printf("입력한 값이 가능한 범위를 초과하였습니다.\n");
    		printf("알맞은 범위 내에서 착수하여 주세요. 열: A~N   행: 1~14\n");
    		printf("입력 [ex) H,8] : ");
    		scanf("%s", buf);
			continue;
		}
		if(m < 0 || m > 14){
			printf("입력한 값이 가능한 범위를 초과하였습니다.\n");
    		printf("알맞은 범위 내에서 착수하여 주세요. 열: A~N   행: 1~14\n");
    		printf("입력 [ex) H,8] : ");
    		scanf("%s", buf);
			continue;
		}
		// 이미 board에 BLACK이나 WHITE가 존재한다면 다시 입력을 받는다.
		if(board[n][m] != NONE){					
			printf("돌이 이미 존재합니다. 다른 곳에 놓아주세요.\n");
    		printf("알맞은 위치 착수하여 주세요. 열: A~N   행: 1~14\n");
    		printf("입력 [ex) H,8] : ");
    		scanf("%s", buf);
			continue;
		}
		break;
	}
	// set_board()함수가 i, j값을 사용하기 때문에 call by reference로 값을 바꿔준다.
	*i = n;
	*j = m;
}


//i, j의 값으로 board의 값을 세팅한다.
//count가 짝수라면 BLACK, 홀수라면 WHITE값을 board에 넣는다.
void set_board(int board[][14],int count, int i, int j){
	
	if ((count % 2) == 0) {
        	if (board[i][j] == NONE) {
            	board[i][j] = BLACK;
        	}
    	}
    	else {
        	if (board[i][j] == NONE) {
            	board[i][j] = WHITE;
        	}
    	}
}


//send의 예외처리와 quit이 입력되면 종료하는 함수.
void my_send(int sock, char* msg){			// 소켓기술자 저장
	if(send(sock, msg, BUFSIZ , 0)==-1){	// 메세지를 보낸다.
		perror("send");
		exit(1);
	}
}


//recv의 예외처리를 포함한 함수
//받은 메세지가 경기가 끝났음을 알리는 경우, 1을 리턴한다.
int my_recv(int sock, char *buf){
	int n;
	if((n = recv(sock, buf, BUFSIZ, 0)) == -1){
		perror("recv");
		exit(1);
	}
	buf[n] = '\0';

	if(!strcmp(buf, "quit")){
		printf("상대방이 기권하였습니다.\n");
		printf("승리하셨습니다.\n");
		return 1 ;
	}
	else if(!strcmp(buf, "winner")){		
		printf("승리하셨습니다.\n");		
		return 1;
	}
	else if(!strcmp(buf, "looser")){				
		printf("패배하셨습니다.\n");	
		return 1;
	}

	return 0;
}

//소켓을 얻어오고 connect()함수를 모아놓은 함수
void connect_request(int *sockfd, struct sockaddr_in *server_addr)
{
	// socket을 얻어옴
	if ((*sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("Socket");
		exit(1);
	}
	memset(server_addr->sin_zero, 0, sizeof(server_addr->sin_zero));
	
	server_addr->sin_family = AF_INET;
	server_addr->sin_port = htons(PORTNUM);
	server_addr->sin_addr.s_addr = inet_addr("127.0.0.1");
	
	if(connect(*sockfd, (struct sockaddr *)server_addr, sizeof(struct sockaddr)) == -1) {
		perror("connect");
		exit(1);
	}
}

// 실제 오목을 실행하는 함수, thread로 실행
void * omok(void* sd){
	int sockfd = (intptr_t)sd;
	char buf[BUFSIZ], name[BUFSIZ], pwd[BUFSIZ];
	// 오목판 배열 선언
	int board[14][14];								
	// count : 흑돌 백돌을 정하기 위한 변수, i,j는 좌표값
	int count = 0, j = 0, i = 0;					

	
	//바둑판 초기화
	for (i = 0; i < 14; i++) {
		for (j = 0; j < 14; j++) {
			board[i][j] = 0;
		}
	}
	system("clear");

	// client 0 : 상대방을 기다리는 중입니다. | client 1 : 대국을 시작합니다.
	my_recv(sockfd, buf);					
	printf("%s",buf);
	//	백인 경우,
	if(strcmp(buf,"상대방을 기다려주세요.\n")){ 
		print_board(board);						
		printf("상대방의 차례입니다.\n");	
		// client 1 이기 때문에 player를 1로 셋한다.
		start = 1;
		player = 1;								
	}

	// client 0 : 대국을 시작합니다 .	| client 1 : 상대방의 메세지
	my_recv(sockfd, buf);
	start = 1;

	print_board(board);
	if(player){		
		// 입력받은 값을 기반으로 연산						
		// put_board() 와 set_board()를 해준다.
		put_board(buf, board, &i, &j);			
		set_board(board, count, i, j);
		// client의 차례가 지났으니 count하나 더해준다.(다음 차례에 다른 사람이 둔다는 것을 의미)			
		count++;								
	}else{
		// client 0인경우 "대국을 시작합니다."라는 메세지를 출력한다.
		printf("%s",buf);						
	}
	
	while(1){
		print_board(board);
		
		printf("사용자의 차례입니다.\n");	
		printf("종료를 원하시면 quit을 적어주세요\n");
		printf("돌을 둘 좌표를 입력하세요 열: A~N   행: 1~14\n");
		printf("입력 [ex) H,8] : ");
		// 보낼 내용을 입력받는다.
		scanf("%s",buf);						
		
		put_board(buf, board,&i, &j);
		set_board(board, count, i, j);
		count++;
		
		print_board(board);

		// 보낸 내용이 quit이라면 서버에 quit 메세지를 보내고 종료한다.	
		if(!strcmp(buf, "quit")){				
			printf("기권하셨습니다.\n");
			printf("패배하셨습니다.\n");	
			start = 2;
			my_send(sockfd, buf);
			break ;
		}	
		
		printf("상대방의 차례입니다.\n");	
		my_send(sockfd, buf);

										
		
		if(my_recv(sockfd, buf)){
			start = 2;
			break;
		}
		put_board(buf, board,&i, &j);
		set_board(board, count, i, j);
		count++;
	}


	sleep(1);

	return 0;
}


// 타이머를 쓰레드를 활용하여 구현,
void * counting(void * arg){
	

	while(1){
		if(start == 1){
			timer++;
			sleep(1);
		}
		else if(start == 2){
			break;
		}
	}
	printf("총 대국 시간 : %d초\n",timer);
	
	return 0;
}


int main(int argc, char *argv[])
{
	
	struct sockaddr_in server_addr;
	pthread_t omok_thread, counting_thread;
	int thread_result;
	char buf[BUFSIZ], id[BUFSIZ], time[BUFSIZ];
	int i = 0;
	int c = 0;


	printf("		---------------------------------------------------\n\n");

	printf("    			유닉스 2021학년도 2학기 프로젝트2 과제\n");
	printf("		socket network를 활용한 실시간 오목게임 클라이언트 입니다.\n\n");

	printf("		---------------------------------------------------\n\n\n");


	printf("id는 이후 web에서 승패를 확인하는 용도로만 사용됩니다.\n");
	printf("id 를 입력해 주세요 : ");
	scanf("%s",id);

	
	connect_request(&sockfd, &server_addr);

	my_send(sockfd, id);

	// thread를 만들어서 timer와 omok 로직을 분리실행 시킵니다.
	pthread_create(&omok_thread, NULL, omok, (void *)(intptr_t)sockfd);
	pthread_create(&counting_thread, NULL, counting, NULL);

	pthread_join(omok_thread, (void **)&thread_result);
	pthread_join(counting_thread, (void **)&thread_result);

	// 모두 종료된 후 타이머로 계산된 시간 전송
	if(player) {
		sprintf(time,"%d",timer);
		my_send(sockfd, time);
	}
	
	my_recv(sockfd,buf);

	printf("client-quited\n");
	close(sockfd);
	return 0;
}