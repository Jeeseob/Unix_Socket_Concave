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

#define CLIENTNUM 3 // 클라이언트가 1명일 때 소켓기술자의 값은 3
#define USERNUM 2	// user구조체 배열의 갯수
#define NONE 0		// NONE, BLACK, WHITE 바둑판의 값들을 설정하기 위한 매크로

#define BLACK 1
#define WHITE 2

struct opt {		// 옵션을 담을 구조체 정의 및 선언
	int u;			// u옵션의 set/clear를 확인해 주는 flag
	int p;			// p옵션의 set/clear를 확인해 주는 flag
}opt;

int start = 0;				// 타이머 구현을 위한 전역변수
int player = 0; 			// 옵션의 구현을 위한 변수 c와 로그인여부를 확인하는 sign변수를 선언한다.
int sockfd;
int timer = 0;


// 오목판을 출력하는 함수이
void print_board(int board[][14]){
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


/*
들어온 입력을 토큰화 시켜 board에 이미 돌이 놓여져 있는 경우 다시 입력을 받는 함수
*/
void put_board(char * buf, int board[][14], int *i,int *j){
	char temp[BUFSIZ];
	char *temp_ptr;
	int n,m;
	
	while(1){
		strcpy(temp, buf);							// strtok() 함수는 원래의 문자열을 수정하기 때문에 temp변수에 복사해준다.
		if(!(temp[1] == ',')){	// 문자열이 'ㅇ,ㅇ'로 이루어지지 않았다면 다시 입력을 받는다.
			if(!strcmp(temp, "quit")){				// 문자열이 quit이라면 종료한다.
				return;
			}
			printf("','를 이용해 띄어쓰기 없이 입력해 주세요.\n");
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
    		scanf("%s", buf);
			continue;
		}
		if(m < 0 || m > 14){
			printf("입력한 값이 가능한 범위를 초과하였습니다.\n");
    		printf("알맞은 범위 내에서 착수하여 주세요. 열: A~N   행: 1~14\n");
    		scanf("%s", buf);
			continue;
		}
		if(board[n][m] != NONE){					// 이미 board에 BLACK이나 WHITE가 존재한다면 다시 입력을 받는다.
			printf("돌이 이미 존재합니다. 다른 곳에 놓아주세요.\n");
    		printf("알맞은 위치 착수하여 주세요. 열: A~N   행: 1~14\n");
    		scanf("%s", buf);
			continue;
		}
		break;
	}
	// set_board()함수가 i, j값을 사용하기 때문에 call by reference로 값을 바꿔준다.
	*i = n;
	*j = m;
}

/*
i, j의 값으로 board의 값을 세팅한다.
count가 짝수라면 BLACK, 홀수라면 WHITE값을 board에 넣는다.
*/
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

/*
send의 예외처리와 quit이 입력되면 종료하는 함수.
*/
void my_send(int sock, char* msg){			// 소켓기술자 저장
	if(send(sock, msg, BUFSIZ , 0)==-1){	// 메세지를 보낸다.
		perror("send");
		exit(1);
	}
}

/*
recv의 예외처리를 포함한 함수
방은 메세지값이 quit이라면 1을 리턴한다.
*/
int my_recv(int sock, char *buf){
	int n;
	if((n = recv(sock, buf, BUFSIZ, 0)) == -1){
		perror("recv");
		exit(1);
	}
	buf[n] = '\0';
	if(!strcmp(buf, "quit")){
		printf("상대방이 기권하였습니다.\n");
		return 1 ;
	}
	else if(!strcmp(buf, "winner")){				
		return 1;
	}
	else if(!strcmp(buf, "looser")){				
		return 1;
	}

	return 0;
}

/*
소켓을 얻어오고 connect()함수를 모아놓은 함수
*/
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

void * omok(void* sd){
	int sockfd = (intptr_t)sd;
	char buf[BUFSIZ], name[BUFSIZ], pwd[BUFSIZ];
	int board[14][14];								// 오목판 배열 선언
	int count = 0, j = 0, i = 0;					// count : 흑돌 백돌을 정하기 위한 변수

	
	//바둑판 초기화
	for (i = 0; i < 14; i++) {
		for (j = 0; j < 14; j++) {
			board[i][j] = 0;
		}
	}
	system("clear");

	my_recv(sockfd, buf);						// client 0 : 상대방을 기다리는 중입니다. | client 1 : 대국을 시작합니다.
	printf("%s",buf);
	if(strcmp(buf,"상대방을 기다려주세요.\n")){ // client 1 인 경우 
		print_board(board);						// 오목판을 프린트한다.
		printf("상대방의 차례입니다.\n");	
		player = 1;								// client 1 이기 때문에 player를 1로 셋한다.
		start = 1;
	}
	if(my_recv(sockfd, buf)){					// client 0 : 대국을 시작합니다 .	| client 1 : 상대방의 메세지
		start = 2;								// timer쓰레드를 종료시키기 위해 start 변수를 2로 설정해 준다.
		return 0;
	}	
	
	print_board(board);
	start = 1;
	if(player){									// client 1인경우 상대방의 메세지를 받기 때문에
		put_board(buf, board, &i, &j);			// put_board() 와 set_board()를 해준다.
		set_board(board, count, i, j);			
		count++;								// client의 차례가 지났으니 count하나 더해준다.
	}else{
		printf("%s",buf);						// client 0인경우 "대국을 시작합니다."라는 메세지를 출력한다.
	}
	
	while(1){
		print_board(board);
		
		printf("사용자의 차례입니다.\n");	
		printf("종료를 원하시면 quit을 적어주세요\n");
		printf("돌을 둘 좌표를 입력하세요 열: A~N   행: 1~14\n");
		printf("입력 [ex) H,8] : ");
		scanf("%s",buf);						// 보낼 내용을 입력받는다.
		
		put_board(buf, board,&i, &j);
		set_board(board, count, i, j);
		count++;
		
		print_board(board);
		
		printf("상대방의 차례입니다.\n");	
		my_send(sockfd, buf);	
		if(!strcmp(buf, "quit")){				// 보낸 내용이 quit이라면 서버에 quit 메세지를 보내고 종료한다.
			printf("기권하였습니다.\n");
			break ;
		}									// seg fault발생한다. 따라서 send() 뒤에서 체크한다. 
		
		if(my_recv(sockfd, buf)){
			break;
		}
		put_board(buf, board,&i, &j);
		set_board(board, count, i, j);
		count++;
	}


	sleep(1);

	return 0;
}

/*
타이머를 구현한 쓰레드
*/
void * counting(void * arg){
	char time[BUFSIZ];

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
	char buf[BUFSIZ], id[BUFSIZ];
	int i = 0;
	int c = 0;

	extern char* optarg;
	extern int optind;

	printf("		---------------------------------------------------\n\n");

	printf("    			유닉스 2021학년도 2학기 프로젝트2 과제\n");
	printf("		socket network를 활용한 실시간 오목게임 클라이언트 입니다.\n\n");

	printf("		---------------------------------------------------\n");


	printf("id는 이후 web에서 승패를 확인하는 용도로만 사용됩니다.\n");
	printf("id 를 입력해 주세요 : ");
	scanf("%s",id);

	
	connect_request(&sockfd, &server_addr);

	my_send(sockfd, id);


	pthread_create(&omok_thread, NULL, omok, (void *)(intptr_t)sockfd);
	pthread_create(&counting_thread, NULL, counting, NULL);

	pthread_join(omok_thread, (void **)&thread_result);
	pthread_join(counting_thread, (void **)&thread_result);





	printf("client-quited\n");
	close(sockfd);
	return 0;
}