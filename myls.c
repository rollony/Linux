#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <string.h>
#include <utime.h>

char type(mode_t);
char* perm(mode_t);
void printStat(char*, char*, struct stat*);

int main(int argc, char * argv[]){
	DIR * dirp;
	char *dir;
	struct dirent *d;
	struct stat file_info;
	char path[1024];
	struct utimbuf time_buf;


		
		if(argc==1){	//현재 디렉토리에 있는 파일과 디렉토리 출력
			if((dirp = opendir("."))==NULL){
				printf("error opening directory\n");
			}

			while((d = readdir(dirp)) !=NULL){
				lstat(d->d_name, &file_info);
				if(type(file_info.st_mode)=='-' || type(file_info.st_mode)=='d'){
					if(d->d_name[0] != '.'){
						printf("%s ", d->d_name);
					}
				}
			}
			printf("\n");
		}
		


		else if(argc>=2){
			if(argv[1][0] == '-'){	
//만일 기본 명령어에 옵션이 추가 되었을 시 
				if((dirp = opendir("."))==NULL){
					printf("error opening directory\n");
					return -1;
				}
				if(argv[1][1] == 'l'){	
//파일과 함께 권한, 포함된 파일수, 소유자, 크기 등 자세한 내용 출력 (ls -l)
					int total;
					
					if(argc==2){		
						while((d = readdir(dirp)) != NULL){
							sprintf(path, "%s/%s", ".", d->d_name);
							if(lstat(path, &file_info)<0){
								printf("error reading file info\n");
							}
							if(d->d_name[0] != '.'){
								printStat(path, d->d_name, &file_info);
								total = total + file_info.st_blocks;	
							}
							
						}
						printf("합계: %d\n", total/2);
					}
					
					else if(argc>=3){
						for(int i = 2; i<argc; i++){
							sprintf(path, "%s/%s", ".", argv[i]);
							lstat(path, &file_info);
							printStat(path, argv[i], &file_info);
						}
					}

				}
				else if(argv[1][1] == 'i'){	
//파일과 함께 inode 번호 출력 (ls -i)
					while((d = readdir(dirp))!=NULL){
						printf("%ld ", d->d_ino);
						printf("%s\n ", d->d_name);
					}	
					printf("\n");
				}

				else if(argv[1][1] == 't'){	
//파일을 시간 순으로 출력 (ls -t)
					char *seq[1024];
					long long tim[1024];
					int count = 0;
					long long time_temp;
					char *name_temp;
					while((d = readdir(dirp))!=NULL){
						if(d->d_name[0] != '.'){
							lstat(d->d_name, &file_info);
							tim[count] =  file_info.st_mtime;
							seq[count] = d->d_name;	
							count++;
						}
					}				

					for(int i = 0; i<count; i++){	
//시간 순에 따라 정렬
						for(int j = 0; j<count-1; j++){
							if(tim[j]<tim[j+1]){
								time_temp = tim[j];
								tim[j] = tim[j+1];
								tim[j+1] = time_temp;
								name_temp = seq[j];
								seq[j] = seq[j+1];
								seq[j+1] = name_temp;
							}
						}
					}
					for(int i = 0; i<count; i++){
						printf("%s ", seq[i]);
					}
					printf("\n");
				}
				
				else if(argv[1][1] == 'a'){	
//숨긴 파일까지 모두 보여주는 옵션 (ls -a)
					while((d = readdir(dirp))!=NULL){
						printf("%s ", d->d_name);
					}	
					printf("\n");
				}

				else if(argv[1][1] == 'm'){	
//쉼표로 구분하여 출력하는 옵션 (ls -m)
					while((d = readdir(dirp))!=NULL){
						if(d->d_name[0] != '.'){
							printf("%s, ", d->d_name);
						}
					}
					printf("\n");
				}
				
				else if(argv[1][1] == 'R'){	
//하위 디렉토리에 소속된 파일 모두 출력하는 옵션 (ls -R)
					char *seq[1024];
					int count = 1;
					while((d = readdir(dirp))!=NULL){
						lstat(d->d_name, &file_info);
						seq[0] = ".";
						if(type(file_info.st_mode)=='d'){
							if(d->d_name[0] != '.'){
								seq[count] = d->d_name;
								count++;
							}
						}
					}
					
					for(int i = 0; i<count; i++){	
//해당 위치의 모든 디렉토리의 파일 출력
						printf("./%s:\n",seq[i]); 	
						dirp = opendir(seq[i]);
						while((d = readdir(dirp))!=NULL){
							if(d->d_name[0] != '.'){
								printf("%s ", d->d_name);
							}
						}
						printf("\n");
					}
					
				}
		
			}	
			else{	//특정 파일 또는 디렉토리를 인자로 받았을 경우
				for(int i = 1; i < argc; i++){
					if(lstat(argv[i], &file_info)<0){
						printf("myls: '%s'에 접근할 수 없습니다: 그런 파일이나 디렉토리가 없습니다.\n", argv[i]); 
						continue;
					}
					if(S_ISREG(file_info.st_mode)){	
//일반 파일이면 이름만 출력
						printf("%s ", argv[i]);
					}
					else if(S_ISDIR(file_info.st_mode)){ 
//디렉토리이면 하위 파일까지 모두 출력
						printf("%s:\n", argv[i]);
						if((dirp = opendir(argv[i]))==NULL){
							printf("error opening directory\n");
						}
						while((d = readdir(dirp))!=NULL){
							if(d->d_name[0]!='.'){
								printf("%s ", d->d_name);
							}
						}
					
					}	
					printf("\n");
				}
			}
		}

		
		
	


}	


void printStat(char *pathname, char *file, struct stat *st){	//파일 상태 정보를 출력
	printf("%5ld ", st->st_blocks);
	printf("%c%s ", type(st->st_mode), perm(st->st_mode));
	printf("%3ld ", st->st_nlink);
	printf("%s %s ", getpwuid(st->st_uid)->pw_name, getgrgid(st->st_gid)->gr_name);
	printf("%9ld ", st->st_size);
	printf("%.12s ", ctime(&st->st_mtime)+4);
	printf("%s\n", file); 
}

char type(mode_t mode){	//파일의 종류를 반환
	if(S_ISREG(mode)) return ('-');
	if(S_ISDIR(mode)) return ('d');
	if(S_ISCHR(mode)) return ('c');
	if(S_ISBLK(mode)) return ('b');
	if(S_ISLNK(mode)) return ('l');
	if(S_ISFIFO(mode)) return ('p');
	if(S_ISSOCK(mode)) return ('s');
}

char* perm(mode_t mode){	//파일 사용 권한 반환 
	int i; 
	static char perms[10]="---------";
	memset(perms, '-', 9);
	for(i = 0; i<3; i++){
		if(mode & (S_IRUSR>>i*3)) perms[i*3] = 'r';
		if(mode & (S_IWUSR>>i*3)) perms[i*3+1] = 'w';
		if(mode & (S_IXUSR>>i*3)) perms[i*3+2] = 'x';
	}
	return (perms);
}
