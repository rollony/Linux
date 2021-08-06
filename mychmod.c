#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

char* perm(mode_t mode);

int main(int argc, char * argv[]){
	char *buffer[1];
	struct stat file_info;	
	int fd;
	int temp_fd;

	stat(argv[argc-1], &file_info);	//접근 권한 변경할 파일의 정보를 불러오기
	
	if((fd = open(argv[argc-1], O_RDONLY))<0){	//파일 오픈
		printf("Error opening file1\n");
		return -1;
	}


	int mode = 0000;
	if(argv[1][0]<='9' && '0'<=argv[1][0]){	
//접근 권한을 숫자로만 입력 받았을 경우
		mode = mode + (atoi(argv[1])/100)*8*8;
		
		mode = mode + ((atoi(argv[1])-((atoi(argv[1])/100)*100))/10*8);
		
		mode = mode + (atoi(argv[1])%10);
	
		fchmod(fd, mode);
	}	
	
	

	else{	//접근 권한을 기호와 알파벳으로 입력 받았을 경우
		if(argv[1][0] == '+'){	//첫 기호가 + 일시
			if(argv[1][1] == 'r'){
				file_info.st_mode |= (S_IRUSR|S_IRGRP|S_IROTH);
			}	
			else if(argv[1][1] == 'w'){
				file_info.st_mode |= (S_IWUSR|S_IWGRP|S_IWOTH);
			}
			else if(argv[1][1] == 'x'){
				file_info.st_mode |= (S_IXUSR|S_IXGRP|S_IXOTH);
			}
		}
		
		else if(argv[1][0] == '-'){	//첫 기호가 - 일시
			if(argv[1][1] == 'r'){
				file_info.st_mode &= ~(S_IRUSR|S_IRGRP|S_IROTH);
			}
			else if(argv[1][1] == 'w'){
				file_info.st_mode &= ~(S_IWUSR|S_IWGRP|S_IWOTH);
			}
			else if(argv[1][1] =='x'){
				file_info.st_mode &= ~(S_IXUSR|S_IXGRP|S_IXOTH);
			}
		}	

		else if(argv[1][0] == 'u' || argv[1][0] == 'g' || argv[1][0] == 'o'){
			//첫 기호가 파일 소유자, 그룹 사용자 또는 다른 사용자를 나타낼때
			if(argv[1][0] == 'u'){
				if(argv[1][1] == '+'){
					if(argv[1][2] == 'r'){
					       file_info.st_mode |= (S_IRUSR);
						if(argv[1][3] == 'w') {
							file_info.st_mode |= (S_IWUSR);
							if(argv[1][4] == 'x'){
								file_info.st_mode |= (S_IXUSR);
							}
						}
						else if(argv[1][3] == 'x'){
							file_info.st_mode |= (S_IXUSR);
						}
					}
					else if(argv[1][2] == 'w') {
						file_info.st_mode |= (S_IWUSR);
						if(argv[1][3] =='x'){
							file_info.st_mode |= (S_IXUSR);
						}
						
					}
					else if(argv[1][2] == 'x') file_info.st_mode |= (S_IXUSR);
				}
				
				else if(argv[1][1] == '-'){
					if(argv[1][2] == 'r'){
					       file_info.st_mode &= ~(S_IRUSR);
						if(argv[1][3] == 'w') {
							file_info.st_mode &= ~(S_IWUSR);
							if(argv[1][4] == 'x'){
								file_info.st_mode &= ~(S_IXUSR);
							}
						}
						else if(argv[1][3] == 'x'){
							file_info.st_mode &= ~(S_IXUSR);
						}
					}
					else if(argv[1][2] == 'w') {
						file_info.st_mode &= ~(S_IWUSR);
						if(argv[1][3] =='x'){
							file_info.st_mode &= ~(S_IXUSR);
						}
						
					}
					else if(argv[1][2] == 'x') file_info.st_mode &= ~(S_IXUSR);
				}

			}
			else if(argv[1][0] == 'g'){
				if(argv[1][1] == '+'){
					if(argv[1][2] == 'r'){
					       file_info.st_mode |= (S_IRGRP);
						if(argv[1][3] == 'w') {
							file_info.st_mode |= (S_IWGRP);
							if(argv[1][4] == 'x'){
								file_info.st_mode |= (S_IXGRP);
							}
						}
						else if(argv[1][3] == 'x'){
							file_info.st_mode |= (S_IXGRP);
						}
					}
					else if(argv[1][2] == 'w') {
						file_info.st_mode |= (S_IWGRP);
						if(argv[1][3] =='x'){
							file_info.st_mode |= (S_IXGRP);
						}
						
					}
					else if(argv[1][2] == 'x') file_info.st_mode |= (S_IXGRP);
				}
				
				else if(argv[1][1] == '-'){
					if(argv[1][2] == 'r'){
					       file_info.st_mode &= ~(S_IRGRP);
						if(argv[1][3] == 'w') {
							file_info.st_mode &= ~(S_IWGRP);
							if(argv[1][4] == 'x'){
								file_info.st_mode &= ~(S_IXGRP);
							}
						}
						else if(argv[1][3] == 'x'){
							file_info.st_mode &= ~(S_IXGRP);
						}
					}
					else if(argv[1][2] == 'w') {
						file_info.st_mode &= ~(S_IWGRP);
						if(argv[1][3] =='x'){
							file_info.st_mode &= ~(S_IXGRP);
						}
						
					}
					else if(argv[1][2] == 'x') file_info.st_mode &= ~(S_IXGRP);
				}
			}
			else if(argv[1][0] == 'o'){
				if(argv[1][1] == '+'){
					if(argv[1][2] == 'r'){
					       file_info.st_mode |= (S_IROTH);
						if(argv[1][3] == 'w') {
							file_info.st_mode |= (S_IWOTH);
							if(argv[1][4] == 'x'){
								file_info.st_mode |= (S_IXOTH);
							}
						}
						else if(argv[1][3] == 'x'){
							file_info.st_mode |= (S_IXOTH);
						}
					}
					else if(argv[1][2] == 'w') {
						file_info.st_mode |= (S_IWOTH);
						if(argv[1][3] =='x'){
							file_info.st_mode |= (S_IXOTH);
						}
						
					}
					else if(argv[1][2] == 'x') file_info.st_mode |= (S_IXOTH);
				}
				
				else if(argv[1][1] == '-'){
					if(argv[1][2] == 'r'){
					       file_info.st_mode &= ~(S_IROTH);
						if(argv[1][3] == 'w') {
							file_info.st_mode &= ~(S_IWOTH);
							if(argv[1][4] == 'x'){
								file_info.st_mode &= ~(S_IXOTH);
							}
						}
						else if(argv[1][3] == 'x'){
							file_info.st_mode &= ~(S_IXOTH);
						}
					}
					else if(argv[1][2] == 'w') {
						file_info.st_mode &= ~(S_IWOTH);
						if(argv[1][3] =='x'){
							file_info.st_mode &= ~(S_IXOTH);
						}
						
					}
					else if(argv[1][2] == 'x') file_info.st_mode &= ~(S_IXOTH);
				}
			}

		}
		
	
			
	
		fchmod(fd, file_info.st_mode);
	}

	close(fd);
	return 0;
	
}

char* perm(mode_t mode){
	int i; 
	static char perms[10] = "---------";
	for(i = 0; i<3; i++){
		if(mode & (S_IRUSR >> i*3)) perms[i*3] = 'r';
		if(mode & (S_IWUSR >> i*3)) perms[i*3+1] = 'w';
	       	if(mode & (S_IXUSR >> i*3)) perms[i*3+2] = 'x';	
	}
	return(perms);
}
