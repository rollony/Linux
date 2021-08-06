#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <utime.h>
#include <time.h>

int main(int argc, char * argv[]){
	int fd; 
	struct stat file_info;
	struct dirent *d;
	DIR *dirp;
	_Bool check = 0;

	if((dirp = opendir("."))==NULL){
		printf("Error opening directory\n");
		return -1;
	}

	if(argc==2){	//입력 인자가 파일 이름 하나인 경우
	 	while((d = readdir(dirp))!=NULL){	//해당 파일이 디렉토리에 있는지 확인
			if(strcmp(d->d_name, argv[1])==0){
				check = 1;
				break;
			}
		}
	
		if(!check){	//디렉토리에 없는 파일일 경우 새로 생성
			if((fd = open(argv[1], O_CREAT|O_EXCL, 0751))<0){
				printf("Error creating file\n");
				return -1;
			}
		}

		else if(check){	//디렉토리에 있는 파일일 경우 접근 시간 갱신
			lstat(argv[1], &file_info);
			utime(argv[1],NULL);
		}
	}
	else if(argc==3){	//입력 인자가 3개, 옵션을 사용하였을 경우
		if(argv[1][1] == 'c'){	//시스템 현재 시간으로 입력 받은 파일의 시간을 변경
			utime(argv[2], NULL);	
		}
	}


	return 0;
}
