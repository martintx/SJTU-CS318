/**********************************************************
 * Author: Yujia Bao
 * Email : yujiabao2510@gmail.com
 * Last modified : 2015-05-24 15:09
 * Filename  : easy_ls.c
 * Description   : Project 2 -- Easy ls tool for CS318 Operating System
 * Copyright(c) 2015, Yujia Bao All Rights Reserved.
 * *******************************************************/
//#define WRITEFILE
#define COLOUR
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>
#include <sys/ioctl.h>  
#include <sys/stat.h>
#include <sys/types.h>
struct paraflag{
	int a;
	int i;
	int l;
	int S;
};

int strlen_custom(char *c){
	//used to deal with string contained with fucking chinese character!
	int num_cc = 0; //number of chinese character
        int len = strlen(c);
	for(int i = 0; i < len; i++)
		if(c[i] > 127 || c[i] < 32) //abnormal character!
			num_cc++;
	return strlen(c)-num_cc/3;
}

int getlength(unsigned long long x){
        if(!x)
                return 1;
	int result = 0;
	while(x){
		x /= 10;
		result++;
	}
	return result;
}

char * splitstr(char *dst,char *src, int n,int m){//n is the length of the substring, m is the position of the beginnng of the substring
	char *p = src;
	char *q = dst;
	int len = strlen(src);
	if(n > len) 
		n = len-m;
	if(m < 0) m = 0;
	if(m > len) return NULL;
	p += m;
	while(n--) 
		*(q++) = *(p++);
	*(q++) = '\0'; 
	return dst;
}

void translate_mode(mode_t x, char result[]){
	char m[10];
	int len = sprintf(m, "%o", x);
	strcpy(result,"----------");
	switch(m[len-1]){
		case '1':
			result[9] = 'x';
			break;
		case '2':
			result[8] = 'w';
			break;
		case '3':
			result[8] = 'w';
			result[9] = 'x';
			break;
		case '4':
			result[7] = 'r';
			break;
		case '5':
			result[7] = 'r';
			result[9] = 'x';
			break;
		case '6':
			result[7] = 'r';
			result[8] = 'w';
			break;
		default:
			result[7] = 'r';
			result[8] = 'w';
			result[9] = 'x';
			break;

	}
	switch(m[len-2]){
		case '1':
			result[6] = 'x';
			break;
		case '2':
			result[5] = 'w';
			break;
		case '3':
			result[5] = 'w';
			result[6] = 'x';
			break;
		case '4':
			result[4] = 'r';
			break;
		case '5':
			result[4] = 'r';
			result[6] = 'x';
			break;
		case '6':
			result[4] = 'r';
			result[5] = 'w';
			break;
		default:
			result[4] = 'r';
			result[5] = 'w';
			result[6] = 'x';
			break;
	}
	switch(m[len-3]){
		case '1':
			result[3] = 'x';
			break;
		case '2':
			result[2] = 'w';
			break;
		case '3':
			result[2] = 'w';
			result[3] = 'x';
			break;
		case '4':
			result[1] = 'r';
			break;
		case '5':
			result[1] = 'r';
			result[3] = 'x';
			break;
		case '6':
			result[1] = 'r';
			result[2] = 'w';
			break;
		default:
			result[1] = 'r';
			result[2] = 'w';
			result[3] = 'x';
			break;
	}
	switch(len){
		case 5:
			switch(m[0]){
				case '1':
					result[0] = 'p';
					return;
				case '2':
					result[0] = 'b';
					return;
				case '4':
					result[0] = 'd';
					return;
				default:
					result[0] = 'b';
					return;
			}
		default:
			if(len == 6){
				switch(m[1]){
					case '0':
						return;
					case '2':
						result[0] = 'l';
						return;
					case '4':
						result[0] = 's';
						return;
					default:
						return;
				}
			}
			return;
	}
}

void qsortchar(int s[], char name[][1000], int l, int r){
	int i, j, x;
	if (l < r){
		i = l;
		j = r;
		x = s[i];
		while (i < j){
			while(i < j &&	(strcmp(name[s[j]],name[x]) > 0)) 
				j--;
			if(i < j) 
				s[i++] = s[j];
			while(i < j && (strcmp(name[s[i]],name[x]) < 0)) 
				i++; 
			if(i < j) 
				s[j--] = s[i];
		}
		s[i] = x;
		qsortchar(s, name, l, i-1); 
		qsortchar(s, name,i+1, r);
	}
}

void qsortint(int s[], unsigned long long t[], int l, int r){//Sort from big to small!!!
	int i, j, x;
	if (l < r){
		i = l;
		j = r;
		x = s[i];
		while (i < j){
			while(i < j &&	(t[s[j]] < t[x])) 
				j--;
			if(i < j) 
				s[i++] = s[j];
			while(i < j && (t[s[i]] > t[x])) 
				i++; 
			if(i < j) 
				s[j--] = s[i];
		}
		s[i] = x;
		qsortint(s, t, l, i-1); 
		qsortint(s, t,i+1, r);
	}
}

int change_vertical(int x, int remainder, int perlen, int lines){
	x++; //make it easier
	int column = (x%perlen) ? (x - (x/perlen)*perlen) : perlen;
	int row = (x%perlen) ? x/perlen + 1 : x/perlen;
	//printf("\n%d %d %d %d", x,remainder,perlen,lines);
	if(column > remainder+1){
	//	printf("\n%d", (lines+1)*remainder + lines * (column-remainder-1) + row -1);
		return (lines+1)*remainder + lines * (column-remainder-1) + row -1;
	}
	else{
	//	printf("\n%d",  row + ((lines+1)*(column-1)) - 1);
		return row + ((lines+1)*(column-1)) - 1;
	}
}

int main(int argc, char *argv[]){
	//get the length of the terminal window to output in a beautiful way ^ ^
	struct winsize size;
	if(ioctl(1,TIOCGWINSZ,&size) == -1){
		fprintf(stderr, "ERROR\n");
		return 1;
	}
	DIR * dp;
	struct dirent *dirp;
	struct paraflag para = {0,0,0,0};	
	//scan the arguments and get the number of directions and get the parameter of ls
	int dir_nums = 0;
	for(int i = 1; i < argc; i++){
		if (argv[i][0] != '-')
			dir_nums++;
		else
			for(int j = 1; j < strlen(argv[i]); j++)
				switch (argv[i][j]){
					case 'a':
						para.a = 1;
						break;
					case 'i':
						para.i = 1;
						break;
					case 'l':
						para.l = 1;
						break;
					case 'S':
						para.S = 1;
						break;
					default:
						printf("Undeifned parameter.\nThe parameter can be -a, -i, -l, -S\n");
						return 1;
				}
	}

	char *dirlist[(dir_nums == 1)?1:dir_nums];
	//ls equals ls ./
	if(dir_nums == 0){
		dirlist[0] = "./";
		dir_nums = 1;
	}
	//copy directory
	for(int i = 1, j = 0; i < argc; i++)
		if (argv[i][0] != '-')
			dirlist[j++] = argv[i];
#ifdef WRITEFILE
	FILE *fp;
	if (0 == (fp = freopen("out.txt","w",stdout))){
		fprintf(stderr, "Cannot open file\n");
		return 1;
	}

#endif
	//loop through all directories
	for(int i = 0; i < dir_nums; i++){
		if(dir_nums != 1){
			if(i != 0)
				printf("\n");	
			printf("%s:\n", dirlist[i]);
		}
		if ((dp = opendir(dirlist[i])) == NULL){
			printf("cannot open %s\n", dirlist[i]);
			return 1;
		}

	//calculate the number of file in the directory
		int num_of_file = 0;
		while ((dirp = readdir(dp)) != NULL)
			num_of_file++;
		if(closedir(dp) == -1)
			return 1;
		
	//get current work path
		char pwdbuf[100];
		getcwd(pwdbuf, sizeof(pwdbuf));
	//Get information of each file
                char namelist[num_of_file][1000];
		char timelist[num_of_file][50];
		char uidlist[num_of_file][50];
		char gidlist[num_of_file][50];
		char protectlist[num_of_file][20];
		unsigned hardlist[num_of_file];
		unsigned long long inodelist[num_of_file];
		unsigned long long sizelist[num_of_file];

		int namemax = 0;
		int uidmax = 0;
		int gidmax = 0;
		int num_to_display = 0;
		unsigned maxhard = 0;
		unsigned long long maxinode = 0;
		unsigned long long maxsize = 0;

		dp = opendir(dirlist[i]);
		dirp = readdir(dp);
		for(int j = 0; dirp != NULL; j++, dirp = readdir(dp)){
			memset(namelist[j],0,1000*sizeof(char));
			strcpy(namelist[j],dirp->d_name);
			if(para.a == 1 || namelist[j][0] != '.'){
				num_to_display++;
				if(namemax < strlen_custom(namelist[j]))
					namemax = strlen_custom(namelist[j]);
				struct stat buf;
				char temp[500];
				if(dirlist[i][0] == '/'){//Input is a restrict directory
					strcpy(temp,dirlist[i]);
					if(dirlist[i][strlen(dirlist[i])-1] != '/')
						strcat(temp, "/");
				}
				else{//Input isnot a restrict directory
					strcpy(temp,pwdbuf);
					strcat(temp, "/");
				}
				strcat(temp,namelist[j]);

				if(lstat(temp, &buf) == -1)
					return 1;
				//initialize the memory space
				memset(protectlist[j],0,20*sizeof(char));
				memset(timelist[j],0,50*sizeof(char));
				memset(uidlist[j],0,50*sizeof(char));
				memset(gidlist[j],0,50*sizeof(char));
				//get the protect mode
				translate_mode(buf.st_mode, protectlist[j]);
				if(protectlist[j][0] == 'l' && para.l == 1){
					char linkname[500];
					memset(linkname,0,500*sizeof(char));
					if(readlink(temp,linkname,sizeof(linkname)) == -1)
						return 1;
					strcat(namelist[j], " -> ");
					strcat(namelist[j], linkname);
				}
				strcpy(temp,ctime(&buf.st_mtime));
				strcpy(timelist[j], splitstr(ctime(&buf.st_mtime),temp,12,4));
				strcpy(uidlist[j], getpwuid(buf.st_uid)->pw_name);
				strcpy(gidlist[j], getgrgid(buf.st_gid)->gr_name);
				hardlist[j] = buf.st_nlink;
				inodelist[j] = buf.st_ino;
				sizelist[j] = buf.st_size;
				if (uidmax < strlen(uidlist[j]))
					uidmax = strlen(uidlist[j]);
				if (gidmax < strlen(gidlist[j]))
					gidmax = strlen(gidlist[j]);
				if(maxhard < hardlist[j])
					maxhard = hardlist[j];
				if(maxinode < inodelist[j])
					maxinode = inodelist[j];
				if(maxsize < sizelist[j])
					maxsize = sizelist[j];
			}
		}
		if(closedir(dp) == -1)
			return 1;
		num_to_display = (para.a == 1) ? num_of_file : num_to_display;
		namemax ++;

		int sortedlist[num_of_file];
		for(int j = 0; j < num_of_file; j++)
			sortedlist[j] = j;
		qsortchar(sortedlist, namelist, 0, num_of_file-1);
		if(para.S)
			qsortint(sortedlist, sizelist, 0, num_of_file-1);
		// Create additonal list to display in a beautiful way
		int vertical[num_to_display];
		for(int j = 0, i = 0; j < num_to_display; j++, i++){
			for(; i < num_of_file; i++)
				if(namelist[sortedlist[i]][0] != '.'){
					vertical[j] = sortedlist[i];
					break;
				}
		}

		int perlen;
		if(para.i == 0)
			perlen = (size.ws_col/namemax == 0) ? 1 : (size.ws_col/namemax);
		else
			perlen = (size.ws_col/(namemax+getlength(maxinode)+1) == 0) ? 1 : (size.ws_col/(namemax+getlength(maxinode+1)));
		int lines = num_to_display/ perlen;
		int remainder = num_to_display - lines * perlen;
		int flag = 1;
		for(int j = 0; j < ((para.a == 1) ? num_of_file : num_to_display) ; j++){
			int temp = j;
			if(para.l == 0)
				j = change_vertical(j,remainder, perlen, lines);
			j = (para.a == 1) ? sortedlist[j] : vertical[j];
			if(para.a == 1 || namelist[j][0] != '.'){
				if(para.i == 1){
					for(int k = 0; k < (getlength(maxinode)-getlength(inodelist[j])); k++, printf(" "));
					printf("%llu ",inodelist[j]);
				}
				if(para.l == 1){
						printf("%s ",protectlist[j]);
						for(int k = 0; k < (getlength(maxhard)-getlength(hardlist[j])); k++, printf(" "));
						printf("%u ",hardlist[j]);
						for(int k = 0; k < (uidmax - strlen(uidlist[j])) ; k++, printf(" "));
						printf("%s ",uidlist[j]);
						for(int k = 0; k < (gidmax - strlen(gidlist[j])) ; k++, printf(" "));
						printf("%s ",gidlist[j]);
						for(int k = 0; k <( getlength(maxsize) - getlength(sizelist[j])); k++, printf(" "));
						printf("%llu ", sizelist[j]);
						printf("%s ", timelist[j]);
#ifndef WRITEFILE
						switch (protectlist[j][0]){
							case 'd':
								printf("\033[;34m%s\033[0m",namelist[j]);
								break;
							case 'l':
								printf("\033[;32m%s\033[0m",namelist[j]);
								break;
							default:
								if(protectlist[j][3] == 'x' && protectlist[j][6] == 'x' && protectlist[j][9] == 'x')
									printf("\033[;31m%s\033[0m",namelist[j]);
								else
									printf("%s",namelist[j]);
						}
#else
						printf("%s",namelist[j]);
#endif
						printf("\n");
				}else{
				//name
#ifndef WRITEFILE
					switch (protectlist[j][0]){
						case 'd':
							printf("\033[;34m%s\033[0m",namelist[j]);
							break;
						case 'l':
							printf("\033[;32m%s\033[0m",namelist[j]);
							break;
						default:
							if(protectlist[j][3] == 'x' && protectlist[j][6] == 'x' && protectlist[j][9] == 'x')
								printf("\033[;31m%s\033[0m",namelist[j]);
							else	
								printf("%s",namelist[j]);
					}
#else
					printf("%s",namelist[j]);
#endif
					int tem_space = namemax - strlen_custom(namelist[j]);
					for(int k = 0; k < tem_space; printf(" "), k++);
					if(!(flag % perlen))
						printf("\n");
					flag++;
				}
			}
			j = temp;
		}
		printf("\n");
	}
#ifdef WRITEFILE
	fclose(fp);
#endif
	return 0;
}
