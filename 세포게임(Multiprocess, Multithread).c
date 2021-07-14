#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <string.h>
#include <pthread.h>


int di[8] = {0, 1, 1, 1, -1, -1, -1, 0};	//매트릭스 안에서 좌표를 찾기 위한 배열
int dj[8] = {1, 1, 0, -1, 0, 1, -1, -1};	//x축, y축

struct thread_data{	//스레드 생성시 인자로 전달할 구조체 선언
  	FILE *fp;
  	int rem;
  	int temp;
  	int t_num;
  	int m, n;
  	pthread_t tid;
};
char** t_mat;	//스레드 병렬처리 시 사용할 배열 전역 선언
int gen, p_num, t_num;	//세대수, 프로세스 수, 스레드 수
pthread_t glo_tid;	
char *input_name;
void *record(void *arg);	//스레드 생성 시 호출 함수

void sequential(){	//순차처리 방식을 구현한 함수
	FILE *fp;
	char buf;
	int m = 0;
	int n = 0;
	int ele = 0;
	if((fp = fopen(input_name, "r+b"))<0){	//input.matrix 파일 최초 열기 
		fprintf(stderr, "file open failure");
		exit(1);
	}
	
	while((buf = getc(fp))!=EOF){	//매트릭스의 전체 원소 개수 카운트
		if(buf=='1' || buf=='0'){		
			n++;
		}		
	}	
	rewind(fp);
	while((buf = getc(fp))!='\n'){	//매트릭스 한 행의 원소 개수 카운트
		if(buf=='1' || buf=='0') ele++;
	}
	m = n/ele;	//위의 두개 값으로 매트릭스의 행 개수 구하기
	
	char** mat;	//파일에서 읽은 원소들을 저장하기 위한 배열 동적할당
	mat = (char**)malloc(sizeof(char*)*(m+2));
	for(int i = 0; i<m+2; i++){
		mat[i] = (char*)malloc(sizeof(char*)*((n/m)+2));
	}
	
	
	
	int k = 1;
	int l = 1;
	rewind(fp);
	
	for(int i = 0; i<=m+1; i++){	//해당 배열보다 열과 행이 두개씩 많은 배열에 원소를 모두 0으로 초기화
		for(int j = 0; j<=(n/m)+1; j++){
			mat[i][j] = '0';

		}	
	}
	
	
	while((buf = getc(fp))!=EOF){	//바깥쪽 행과 열을 제외한 배열에 파일에서 읽은 원소 저장
		if(buf=='\n'){
			k++;
			l = 1;	
		}
		else if(buf=='1' || buf=='0'){
			mat[k][l] = buf;		
			l++;
		}		
	}

	
	
			
	for(int g = 0; g<gen; g++){	//앞서 입력 받은 세대수 만큼 진행
		FILE *after;	//매트릭스 연산 후에 기록할 파일 포인터
		FILE *before;	//전 세대에 시행한 연산을 기록한 파일 포인터
		char s1[20] = "gen_";	//각 세대마다 이름을 바꿔주기 위한 문자열
		char s2[20];
		char s3[20] = ".matrix";
		sprintf(s2, "%d", g+1);
		strcat(s2,s3);
		strcat(s1, s2);
		
		
		if(g+1==gen){	//연산을 진행할 마지막 세대라면 output에 저장
			if((after = fopen("output.matrix", "w+"))<0){
				printf("file error");
			}
		}
		else{	
			if((after = fopen(s1, "w+"))<0){
				printf("file error");
			}
		}
		
		char s4[20] = "gen_";
		char s5[20];
		char s6[20] = ".matrix";
		sprintf(s5, "%d", g);
		strcat(s5, s6);
		strcat(s4, s5);
		
		
		if(g>0){	//첫번째 세대가 아닐 경우 이전 파일에서 원소들을 불러와 배열에 저장
			if((before = fopen(s4,"r"))<0){
				printf("file error");
			}
			int x = 1;
			int y = 1;
			char buff;
			while((buff = getc(before))!=EOF){
				if(buff=='\n'){
					x++;
					y = 1;	
				}
				else if(buff=='1' || buff=='0'){
					mat[x][y] = buff;
					y++;
				}		
			}
		}
		
		for(int i = 1; i<=m; i++){	//각 행과 열의 원소에 대해 연산 진행
			for(int j = 1; j<=n/m; j++){
				int count = 0;
				for(int n = 0; n<8; n++){
					int ni = i + di[n];	//한 원소를 기준으로 이웃한 8방향의 원소에 대해 탐색
					int nj = j + dj[n];
					if(mat[ni][nj] == '1') count++; //이웃이 1일시 count 변수에 이웃의 수 저장
				}
				
				if(mat[i][j]=='1'){	//게임 규칙에 따른 연산과정
					if(count<=2 || count>=7) {
						fprintf(after, "0");
						if(j!=n/m) fprintf(after, " ");
					}
					else{
						fprintf(after, "1");
						if(j!=n/m) fprintf(after, " ");
					}
				}		
				else if(mat[i][j]=='0'){
					if(count==4){ 
						fprintf(after, "1");
						if(j!=n/m) fprintf(after, " ");
					}
					else{
						fprintf(after, "0");
						if(j!=n/m) fprintf(after, " ");
									
					}	
				}
			}
			if(i!=m) fprintf(after, "\n");
			
		}
	
		fclose(after);
	}	
	free(mat);	//모든 연산 종료 후 동적할당 해제
}

void process(){	//프로세스 병렬처리 방식을 구현한 함수
	FILE *fp;
	char buf;
	int m = 0;
	int n = 0;
	int ele = 0;
	
	if((fp = fopen(input_name, "r+b"))<0){
		printf("file open failure\n");
	}
	
	while((buf = getc(fp))!=EOF){	//매트릭스의 전체 원소 개수 카운트
		if(buf=='1' || buf=='0'){		
			n++;
		}		
	}	
	rewind(fp);
	while((buf = getc(fp))!='\n'){	//매트릭스 한 행의 원소 개수 카운트
		if(buf=='1' || buf=='0') {
			ele++;
		}
	}	
	m = n/ele;	//위의 두개 값으로 매트릭스의 행 개수 구하기
	if(p_num>m){	//입력 받은 프로세스수가 행의 개수를 초과하면 균등한 연산 불가 판정 후 다시 입력 받음
		printf("프로세스 수가 배열의 행의 개수를 초과하였습니다. 초기로 이동합니다\n");
		return;
	}
	char** mat;	
	mat = (char**)malloc(sizeof(char*)*(m+2));	//해당 배열을 동적할당
	for(int i = 0; i<m+2; i++){
		mat[i] = (char*)malloc(sizeof(char*)*((n/m)+2));
	}

	
	int k = 1;
	int l = 1;
	rewind(fp);
	
	for(int i = 0; i<=m+1; i++){	//해당 배열보다 열과 행이 두개씩 많은 배열에 원소를 모두 0으로 초기화
		for(int j = 0; j<=(n/m)+1; j++){
			mat[i][j] = '0';
		}	
	}
	
	
	while((buf = getc(fp))!=EOF){	//바깥쪽 행과 열을 제외한 배열에 파일에서 읽은 원소 저장
		if(buf=='\n'){
			k++;
			l = 1;	
		}
		else if(buf=='1' || buf=='0'){
			mat[k][l] = buf;		
			l++;
		}		
	}

	
	int rem = m%p_num;	//프로세스들이 균등하게 연산하도록 계산하는 변수들
	
	int div = m/p_num;
	
	if(rem!=0) div = div + 1; 
	
	fclose(fp);
	
	
	for(int g = 0; g<gen; g++){	//앞서 입력 받은 세대수 만큼 진행
		FILE *before;	//매트릭스 연산 후에 기록할 파일 포인터
		char s1[20] = "gen_";	//각 세대마다 이름을 바꿔주기 위한 문자열
		char s2[20];
		char s3[20] = ".matrix";
		sprintf(s2, "%d", g+1);
		strcat(s2,s3);
		strcat(s1, s2);
		printf("%d번째 세대에 사용된 프로세스들의 ID : \n", g+1);
		FILE *after[p_num];	//자식 프로세스들이 기록할 각각의 파일 포인터를 배열 형식으로 지정
		for(int i = 0; i<p_num; i++){
			if(g+1==gen){
				if((after[i] = fopen("output.matrix", "w"))<0){	
					printf("file error");
				}
			}
			else{
				if((after[i] = fopen(s1, "w"))<0){
					printf("file error");
				}
			}
		}
		
		char s4[20] = "gen_";
		char s5[20];
		char s6[20] = ".matrix";
		sprintf(s5, "%d", g);
		strcat(s5, s6);
		strcat(s4, s5);
		
		if(g>0){
			if((before = fopen(s4,"r"))<0){
				printf("file error");
			}
			int x = 1;
			int y = 1;
			char buff;
			while((buff = getc(before))!=EOF){
				if(buff=='\n'){
					x++;
					y = 1;	
				}
				else if(buff=='1' || buff=='0'){
					mat[x][y] = buff;
					y++;
				}		
			}
		}
		
		int temp;
		int pid_arr[p_num];
		pid_t pid;
		
		for(int i = 0; i<p_num; i++){	
			if((pid = fork())<0){	//입력 받은 프로세스 수 만큼 자식 프로세스 생성
				printf("fork error");
			}
			if(pid==0){	//자식 프로세스일 시 수행
				printf("%d	", getpid());	//프로세스 아이디 출력
				pid_arr[i] = getpid();
				if(rem==0) fseek(after[i], (i*div)*(n/m)*2, SEEK_SET);
				else if(rem!=0){	//프로세스들을 균등하게 배분하기 위한 연산
					//각각의 프로세스들이 자신의 파일 포인터를 찾아가 오프셋 값 설정
					if(i+1<=rem) fseek(after[i], (i*div)*(n/m)*2, SEEK_SET);
					else if(i+1==rem+1) fseek(after[i], (i*div)*(n/m)*2, SEEK_SET);
					else{
						fseek(after[i], ((rem*div)*(n/m)*2) + (((i-rem)*(div-1))*(n/m)*2), SEEK_SET);
					 
					}
					
				}
				temp = i;
				break;
			}
		}
		
		
		if(pid==0 && (getpid() == pid_arr[temp])){
			if(rem==0){	
				for(int i = (temp*div)+1; i<=(temp+1)*div; i++){
					//자신의 오프셋에 해당하는 원소들에 대한 연산 수행
					for(int j = 1; j<=n/m; j++){
						int count = 0;
						for(int n = 0; n<8; n++){
							int ni = i + di[n];
							int nj = j + dj[n];
							if(mat[ni][nj] == '1') count++; 
						}
						//게임 규칙에 따른 연산 과정
						if(mat[i][j]=='1'){
							if(count<=2 || count>=7) {
								fprintf(after[temp], "0");
								if(j!=n/m) fprintf(after[temp], " ");
							}
							else{
								fprintf(after[temp], "1");
								if(j!=n/m) fprintf(after[temp], " ");
							}
						}		
						else if(mat[i][j]=='0'){
							if(count==4){ 
								fprintf(after[temp], "1");
								if(j!=n/m) fprintf(after[temp], " ");
							}
							else{
								fprintf(after[temp], "0");
								if(j!=n/m) fprintf(after[temp], " ");
									
							}	
						}
					}
					fprintf(after[temp], "\n");		
				}
			}
			else if(rem!=0){					
					if(temp+1<=rem){
						for(int i = (temp*div)+1; i<=(temp+1)*div; i++){
							for(int j = 1; j<=n/m; j++){
								int count = 0;
								for(int n = 0; n<8; n++){
									int ni = i + di[n];
									int nj = j + dj[n];
									if(mat[ni][nj] == '1') count++; 
								}
								
								if(mat[i][j]=='1'){
									if(count<=2 || count>=7) {
										fprintf(after[temp], "0");
										if(j!=n/m) fprintf(after[temp], " ");
									}
									else{
										fprintf(after[temp], "1");
										if(j!=n/m) fprintf(after[temp], " ");
									}
								}		
								else if(mat[i][j]=='0'){
									if(count==4){ 
										fprintf(after[temp], "1");
										if(j!=n/m) fprintf(after[temp], " ");
									}
									else{
										fprintf(after[temp], "0");
										if(j!=n/m) fprintf(after[temp], " ");		
									}	
								}
							}
							fprintf(after[temp], "\n");
						}
					}
					else{
						
						for(int i = (rem*div)+1+((temp-rem)*(div-1)); i<=(rem*div)+1+((temp-rem)*(div-1)) + (div -1) -1; i++){
							for(int j = 1; j<=n/m; j++){
								int count = 0;
								for(int n = 0; n<8; n++){
									int ni = i + di[n];
									int nj = j + dj[n];
									if(mat[ni][nj] == '1') count++; 
								}
								
								if(mat[i][j]=='1'){
									if(count<=2 || count>=7) {
										fprintf(after[temp], "0");
										if(j!=n/m) fprintf(after[temp], " ");
									}
									else{
										fprintf(after[temp], "1");
										if(j!=n/m) fprintf(after[temp], " ");
										}
									}		
								else if(mat[i][j]=='0'){
									if(count==4){ 
										fprintf(after[temp], "1");
										if(j!=n/m) fprintf(after[temp], " ");
									}
									else{
										fprintf(after[temp], "0");
										if(j!=n/m) fprintf(after[temp], " ");
									
									}	
								}
							}
							if(i!=m) fprintf(after[temp], "\n");
						}
					}
				}
				
			exit(0);
		}
		while(wait((int*)0) != -1);	//모든 자식 프로세스들이 종료하도록 부모 프로세스가 wait
		for(int i = 0; i<p_num; i++){
			fclose(after[i]);	//각각의 파일 포인터를 닫아준다
		}
		
		printf("\n");
		
		
	}
	
	free(mat);
}

void thread(){	//스레드 병렬처리를 구현한 함수
	FILE *fp;
	char buf;
	int m = 0;
	int n = 0;
	int ele = 0;

	
	if((fp = fopen(input_name, "r+b"))<0){	//초기 입력할 배열을 파일 열기
		printf("file open failure\n");
	}
	
	while((buf = getc(fp))!=EOF){	//매트릭스의 전체 원소 개수 카운트
		if(buf=='1' || buf=='0'){		
			n++;
		}		
	}	
	rewind(fp);
	while((buf = getc(fp))!='\n'){	//매트릭스 한 열의 원소 개수 카운트
		if(buf=='1' || buf=='0') ele++;
	}
	m = n/ele;	//위의 두값으로 매트릭스의 행의 개수 계산
	if(t_num>m){	//입력 받은 스레드의 개수가 행의 개수 초과 시 균등한 연산 불가 판정 후 입력 다시 받음
		printf("스레드 수가 배열의 행의 개수를 초과하였습니다. 초기로 이동합니다\n");
		return;
	}
	
	t_mat = (char**)malloc(sizeof(char*)*(m+2));	//원소들을 저장할 배열 동적할당
	for(int i = 0; i<m+2; i++){
		t_mat[i] = (char*)malloc(sizeof(char*)*((n/m)+2));
	}
	
	
	int k = 1;
	int l = 1;
	rewind(fp);
	
	for(int i = 0; i<=m+1; i++){	//해당 배열보다 열과 행이 두개씩 많은 배열에 원소를 모두 0으로 초기화
		for(int j = 0; j<=(n/m)+1; j++){
			t_mat[i][j] = '0';
		}	
	}
	
	
	while((buf = getc(fp))!=EOF){	//바깥쪽 행과 열을 제외한 배열에 파일에서 읽은 원소 저장
		if(buf=='\n'){
			k++;
			l = 1;	
		}
		else if(buf=='1' || buf=='0'){
			t_mat[k][l] = buf;		
			l++;
		}		
	}

	
	
	fclose(fp);
	int rem = m%t_num;	//프로세스들이 균등하게 연산하도록 계산하는 변수들
	
	int div = m/t_num;
	
	if(rem!=0) div = div + 1;
	
	for(int g = 0; g<gen; g++){	//앞서 입력 받은 세대수 만큼 진행
		
		FILE *before;	//매트릭스 연산 후에 기록할 파일 포인터
		char s1[20] = "gen_";	//각 세대마다 이름을 바꿔주기 위한 문자열
		char s2[20];
		char s3[20] = ".matrix";
		sprintf(s2, "%d", g+1);
		strcat(s2,s3);
		strcat(s1, s2);
		
		FILE *after[t_num];	//자식 스레드들이 기록할 각각의 파일 포인터를 배열 형식으로 지정
		printf("%d번째 세대에 사용된 스레드들의 ID : \n", g+1);
		for(int i = 0; i<t_num; i++){	
			if(g+1==gen){	
				if((after[i] = fopen("output.matrix", "w"))<0){	
					printf("file error");
				}
			}
			else{
				if((after[i] = fopen(s1, "w"))<0){
					printf("file error");
				}
			}
		}
		

		
		
		char s4[20] = "gen_";
		char s5[20];
		char s6[20] = ".matrix";
		sprintf(s5, "%d", g);
		strcat(s5, s6);
		strcat(s4, s5);
		
		
		if(g>0){
			if((before = fopen(s4,"r"))<0){
				printf("file error");
			}
			
			int x = 1;
			int y = 1;
			char buff;
			while((buff = getc(before))!=EOF){
				
				if(buff=='\n'){
					x++;
					y = 1;	
				}
				else if(buff=='1' || buff=='0'){
					t_mat[x][y] = buff;
					y++;
				}		
			}
			fclose(before);
		}
		
		
		struct thread_data data[t_num];	//스레드 생성 시 함수로 전달할 구조체 배열 선언
		pthread_t tid;
		
		int status;
		for(int i = 0; i<t_num; i++){	//구조체에 값 저장
			data[i].rem = rem;
			data[i].t_num = t_num;
			data[i].m = m;
			data[i].n = n;
			data[i].temp = i;
			data[i].fp = after[i];
			
		}
		
		for(int i = 0; i<t_num; i++){	//입력 받은 스레드 수 만큼 생성
			pthread_create(&tid, NULL, record, (void*)&data[i]);
			data[i].tid = tid;
			printf("%ld	", data[i].tid);
		}
		
		printf("\n");
		for(int i = 0; i<t_num; i++){	//종료된 스레드들을 join
			pthread_join(data[i].tid, (void*)&status);
		}
		
		
		
	}
	free(t_mat);	//배열의 동적할당 해제
	
}

void *record(void *arg){	//스레드 생성 시 호출되는 함수
	struct thread_data *data;
	int rem;
	int temp;
	int t_num;
	int m,n;
	data = (struct thread_data *)arg;	//구조체의 값을 전달받음
	rem = data->rem;
	temp = data->temp;
	t_num = data->t_num;
	m = data->m;
	n = data->n;
	int div = m/t_num;
	if(rem!=0) div = div + 1;
	FILE *after = data->fp;

	
	if(rem==0) fseek(after, (temp*div)*(n/m)*2, SEEK_SET);
	else if(rem!=0){	//스레드 번호에 따라 해당 오프셋을 찾아감
		if(temp+1<=rem) fseek(after, (temp*div)*(n/m)*2, SEEK_SET);
		else if(temp+1==rem+1) fseek(after, (temp*div)*(n/m)*2, SEEK_SET);
		else{
			fseek(after, ((rem*div)*(n/m)*2) + (((temp-rem)*(div-1))*(n/m)*2), SEEK_SET);
		}
			
	}
	
	if(rem==0){	
		for(int i = (temp*div)+1; i<=(temp+1)*div; i++){
			//각 스레드들이 자신의 오프셋에서 연산 진행
			for(int j = 1; j<=n/m; j++){
				int count = 0;
				for(int n = 0; n<8; n++){
					int ni = i + di[n];
					int nj = j + dj[n];
					if(t_mat[ni][nj] == '1') count++; 		
				}
				//게임의 규칙에 맞게 각 원소에 대해 연산 진행
						
				if(t_mat[i][j]=='1'){
					if(count<=2 || count>=7) {
						fprintf(after, "0");
						if(j!=n/m) fprintf(after, " ");
					}
					else{
						fprintf(after, "1");
						if(j!=n/m) fprintf(after, " ");
						}
					}		
				else if(t_mat[i][j]=='0'){
					if(count==4){ 
						fprintf(after, "1");
						if(j!=n/m) fprintf(after, " ");
					}
					else{
						fprintf(after, "0");
						if(j!=n/m) fprintf(after, " ");
									
						}	
					}					
				}
				if(i!=m) fprintf(after, "\n");		
			}
		}
		else if(rem!=0){								
			if(temp+1<=rem){
				for(int i = (temp*div)+1; i<=(temp+1)*div; i++){
					for(int j = 1; j<=n/m; j++){
						
						int count = 0;
						if(i<=m && j<=(n/m)){
						for(int n = 0; n<8; n++){
							int ni = i + di[n];
							int nj = j + dj[n];
							if(t_mat[ni][nj] == '1') count++; 
						}
						
						if(t_mat[i][j]=='1'){
							if(count<=2 || count>=7) {
								fprintf(after, "0");
								if(j!=n/m) fprintf(after, " ");
							}
							else{
								fprintf(after, "1");
								if(j!=n/m) fprintf(after, " ");
							}
						}		
						else if(t_mat[i][j]=='0'){
							if(count==4){ 
								fprintf(after, "1");
								if(j!=n/m) fprintf(after, " ");
							}
							else{
								fprintf(after, "0");
								if(j!=n/m) fprintf(after, " ");
												
							}	
						}
						}
					}
					fprintf(after, "\n");
				}
			}
			else{		
				for(int i = (rem*div)+1+((temp-rem)*(div-1)); i<=(rem*div)+1+((temp-rem)*(div-1)) + (div -1) -1; i++){
					for(int j = 1; j<=n/m; j++){
					
						int count = 0;
						if(i<=m && j<=(n/m)){
						for(int n = 0; n<8; n++){
							int ni = i + di[n];
							int nj = j + dj[n];
							if(t_mat[ni][nj] == '1') count++; 
						}
							
						if(t_mat[i][j]=='1'){
							if(count<=2 || count>=7) {
								fprintf(after, "0");
								if(j!=n/m) fprintf(after, " ");
							}
							else{
								fprintf(after, "1");
								if(j!=n/m) fprintf(after, " ");
								}
							}		
						else if(t_mat[i][j]=='0'){
							if(count==4){ 
								fprintf(after, "1");
								if(j!=n/m) fprintf(after, " ");
							}
							else{
								fprintf(after, "0");
								if(j!=n/m) fprintf(after, " ");
									
							}	
						}
						}
						
					}
					if(i!=m) fprintf(after, "\n");
				}
			}
		}
		fclose(after);
		
		pthread_exit(NULL);	//자신이 맡은 연산 마친 후 해당 자식 스레드는 종료
		return NULL;

}

int main(int argc, char* argv[]){	//입력 값을 받고 수행 시간을 계산할 메인 함수
	_Bool cont = 1;
	int choice = 0;
	struct timeval start_s, end_s, start_p, end_p, start_t, end_t;
	int total_time;
	input_name = argv[1];
	system("clear");
	if(argc<2){	//올바른 인자수를 입력 받지 않았을 경우
		printf("인자로 넘겨 받은 파일이 없습니다.\n");
		exit(1);
	}
	if(access(input_name, F_OK)<0){	//존재하지 않은 파일을 인자로 넘겨 받았을 경우
		printf("입력한 파일이 존재하지 않습니다.\n");
		exit(1);
	}
	do{	//종료에 해당하는 번호를 입력 받기 전까지 계속 진행
		printf("(1)프로그램 종료 (2)순차처리 (3)Process 병렬처리 (4)Thread 병렬처리\n");
		scanf("%d", &choice);
		
		if(choice==1){	//프로그램 종료
			cont = 0;
			break;	
		}
		else if(choice==2){	//순차처리
			int total_time = 0;
			printf("게임을 진행할 세대수를 입력하세요 (최소 1 이상) : ");
			scanf("%d", &gen);
			gettimeofday(&start_s, NULL);
			sequential();
			gettimeofday(&end_s, NULL);
			total_time = ((end_s.tv_sec-start_s.tv_sec)*1000)+((end_s.tv_usec-start_s.tv_usec)/1000);
			printf("Sequential 처리 총 수행시간 : %d ms\n\n", total_time);
		}
		else if(choice==3){	//Process 병렬처리
			int total_time;
			printf("진행할 프로세스 개수를 입력하세요 : ");
			scanf("%d", &p_num);
			printf("게임을 진행할 세대수를 입력하세요 (최소 1 이상) : ");
			scanf("%d", &gen);
			
			gettimeofday(&start_p, NULL);
			process();
			gettimeofday(&end_p, NULL);
			total_time = ((end_p.tv_sec-start_p.tv_sec)*1000)+((end_p.tv_usec-start_p.tv_usec)/1000);
			printf("Process 처리 총 수행시간 : %d ms\n\n", total_time);				
					
		}
		else if(choice==4){	//Thread 병렬처리
			int total_time;
			printf("진행할 스레드 개수를 입력하세요 : ");
			scanf("%d", &t_num);
			printf("게임을 진행할 세대수를 입력하세요 (최소 1 이상) : ");
			scanf("%d", &gen);
			
			gettimeofday(&start_t, NULL);		
			thread();
			gettimeofday(&end_t, NULL);
			total_time = ((end_t.tv_sec-start_t.tv_sec)*1000)+((end_t.tv_usec-start_t.tv_usec)/1000);
			printf("Thread 처리 총 수행시간 : %d ms\n\n", total_time);
		}

	}while(cont);
	exit(0);

}
