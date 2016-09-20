
/*
 *
 * CS-252 Fall 2013
 * shell.l: lexical analyzer for shell
 * You have to extend it.
 *
 *
 *
 */




%{
#include <sys/types.h>
#include <sys/wait.h>
#include <iostream>
#include <fstream>
#include <string.h>
#include "y.tab.h"
#include <unistd.h>
#include "command.h"
#include <signal.h>

extern "C" char * read_line();
int mygetc(FILE * f) {
	static char *p;
  	char ch;
   	if (!isatty(0)) {
      	// stdin is not a tty. Call real getc
        	 return getc(f);
	}
	   // stdin is a tty. Call our read_line.
	if (p==NULL || *p == 0) {
		char * s = read_line();
	        p = s;
	}
	ch = *p;
	p++;
	return ch;
}
#undef getc
#define getc(f) mygetc(f)
/////////// End added code ///////////
static  void yyunput (int c,char *buf_ptr  );

void myunputc(int c) {
    unput(c);
}

%}

%%

\n 	{
		return NEWLINE;
	}

[ \t] 	{
		/* Discard spaces and tabs */
     	}	

">" 	{
		return GREAT;
	}	

"&"	{
		return AMPERSAND;
	}

"<"	{
		return LESS;
	}

">&"	{
		return GREATAMPERSAND;
	}

">>"	{
		return GREATGREAT;
	}

"|"	{
		return PIPE;
	}

">>&"	{
		return GREATGREATAMPERSAND;
	}

"<<"	{
		return LESSLESS;
	}

[`][^\n`]+[`] {
		//subshell
		char * p = yytext;
		char * str = (char*)malloc(1024);
		char * s = str;
		int index = 0;
		while(*p){
		//	printf("p:%s\n",p);
		//	printf("s:%s\n",s);
			if(*p == '`'){
				p++;
			}
			else{
			//character
				*s = *p;
				s++;
				p++;
			}
		}
		*s = '\n';
		s++;
		*s = 'e';
		s++;
		*s = 'x';
		s++;
		*s = 'i';
		s++;
		*s = 't';
		s++;
		*s = '\n';
		s++;
	
		*s = '\0';
		s = str;
	//	printf("str: %s",str);
		//store initial in/out
		int defaultin = dup(0);
		int defaultout = dup(1);
		//pid for fork
		int pid;
		//two pipes for child and parent process
		int fd1[2];
		int fd2[2];
		pipe(fd1);
		pipe(fd2);
	//	printf("str size: %ld\n",str.length());
		write(fd1[1], str, strlen(str));
	//	write(1,"exit\n",10);
		close(fd1[1]);

		dup2(fd1[0], 0);
		close(fd1[0]);
		dup2(fd2[1], 1);
		close(fd2[1]);
		char buff[3072];
	//	char * b = &buff[0];
	//	printf("before fork\n");
		pid = fork();
	//	fprintf(stderr, "pid %d\n", pid);
		if(pid < 0){
			perror("ERROR: subshell fork\n");
			exit(2);
		}else if(pid == 0){

			//child process
			execvp("/proc/self/exe", NULL);
			perror("ERROR:***Subshell execvp\n");
			exit(2);

		}
		waitpid(pid,NULL,0);
		dup2(defaultin, 0);
		dup2(defaultout, 1);
		close(defaultin);
		close(defaultout);

		int i = 0;

		char c;
//		printf("results read: ");
		while(read(fd2[0], &c, 1) != 0){
//			printf("%c", c);
			buff[i] = c;
			i++;
		}
		buff[i] = '\0';
		close(fd2[0]);	
//		printf("buff: %s\n",buff);

		while (i >= 0) {
			if (buff[i] == '\n'){ 
				buff[i] = ' ';
			}
			unput(buff[i]);
			i--;
		}
	}
[^ \t\n|<>&"]+  {
		/* Assume that file names have only alpha chars */
		yylval.string_val = strdup(yytext);
		return WORD;
	}

["][^\n]+["] {
		//This part for double quotes
		//printf("yytext:%s\n",yytext);
		char * p = yytext;
		char * w = (char*)malloc(1024);
		char * a = w;
		while(*p){
			if(*p == '\"'){
				p++;
				while(*p != '\"'){
					*a = *p;
					p++;
					a++;
				}
			}
			p++;
		}
		*a = '\0';
		yytext = strdup(w);
		yylval.string_val = strdup(yytext);
		return WORD;
	}
[^ \n]*\\[^\n]*\\[^ \n] {
		//This part for backslash
		char * p = yytext;
		char * back_arg = (char*)malloc(1024);
		int index = 0;
		char * mul_back = (char*)malloc(4);
		strcpy(mul_back,"[\\]");
	//	printf("%s\n",mul_back);
		while(*p){
			if(*p == '\\'){
			//if *p == \",skip to next char
				p++;
				if(*p == '\\'){
				//CASE for //////////
					while(*p == '\\'){
						p++;
					}
					back_arg[index++] = '\\';
				}else{
					back_arg[index++] = *p;
					p++;
				}
			}else{
				back_arg[index++] = *p;
				p++;
			}
		}
		back_arg[index] = '\0';
	//	free(back_arg);
		yylval.string_val = strdup(back_arg);
		return WORD;
	}


