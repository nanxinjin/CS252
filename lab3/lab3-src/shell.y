
/*
 * CS-252 Spring 2013
 * shell.y: parser for shell
 *
 * This parser compiles the following grammar:
 *
 *	cmd [arg]* [> filename]
 *
 * you must extend it to understand the complete shell grammar
 *
 */

%token	<string_val> WORD

/*%token 	NOTOKEN LESSLESS GREAT GREATAMPERSAND  NEWLINE GREATGREAT GREATGREATAMPERSAND PIPE AMPERSAND LESS
*/
%token		NOTOKEN GREAT GREATGREAT GREATGREATAMPERSAND NEWLINE AMPERSAND LESS LESSLESS GREATAMPERSAND PIPE

%union	{
		char   *string_val;
	}

%{
//#define yylex yylex
#include <stdio.h>
#include "command.h"
#include <string.h>
#include <sys/types.h>
#include <regex.h>
#include <stdlib.h>
#include <dirent.h>
#include <assert.h>
void yyerror(const char * s);
int yylex();
void expandWildcardsIfNecessary(char * arg);
void expandWildcard(char * prefix, char * suffix);
static int cmpstring(const void *p1, const void *p2);
int maxEntries = 20;
int nEntries = 0;
char ** array;
%}

%%

goal:	
	commands;

arg_list:
	arg_list argument
	| /* can be empty */
	;

argument:
	WORD {
  /*             printf("   Yacc: insert argument \"%s\"\n", $1);*/

/*	       Command::_currentSimpleCommand->insertArgument( $1 );*/
		
		expandWildcardsIfNecessary($1);
	}
	;




command_word:
	WORD {
         /*      printf("   Yacc: insert command \"%s\"\n", $1);*/
	       
	       Command::_currentSimpleCommand = new SimpleCommand();
	       Command::_currentSimpleCommand->insertArgument( $1 );
	}
	;

command_and_args:
	command_word arg_list {
		Command::_currentCommand.
			insertSimpleCommand( Command::_currentSimpleCommand );
	}
	;

pipe_list:
	pipe_list PIPE command_and_args
	| command_and_args
	
	;


/*io_word:
	WORD{
		printf("   Yacc: insert command \"%s\"\n", $2);
		Command::_currentCommand.
			insertSimpleCommand( Command::_currentSimpleCommand );
	}
*/
iomodifier_opt:
	GREAT WORD{
/*		printf("   Yacc: insert output \"%s\"\n", $2);*/
		if(Command::_currentCommand._outFile){
			printf("Ambiguous output redirect\n");
			exit(2);
		}
		Command::_currentCommand._outFile = $2;
	}
	
	
	|
	LESS WORD {
	/*	printf("   Yacc: insert inputput \"%s\"\n", $2);*/
		Command::_currentCommand._inputFile = $2;
	}
	|
	GREATAMPERSAND WORD{
	/*	printf("   Yacc: GREATAMPERSAND \"%s\"\n", $2);*/
		Command::_currentCommand._outFile = $2;
		Command::_currentCommand._errFile = $2;
	}
	
	|
	GREATGREAT WORD{
	/*	printf("   Yacc: GREATGREAT \"%s\"\n", $2);*/
		Command::_currentCommand._append = 1;
		Command::_currentCommand._outFile = $2;
	}
	|
	GREATGREATAMPERSAND WORD{
	/*	printf("   Yacc: GREATGREATAMPERSAND \"%s\"\n", $2);*/
		Command::_currentCommand._errFile = $2;
		Command::_currentCommand._outFile = $2;
		Command::_currentCommand._append = 1;
	}
	|
	LESSLESS WORD{
	/*	printf("   Yacc: GREATGREATAMPERSAND \"%s\"\n", $2);*/
		Command::_currentCommand._inputFile = $2;
	}
	
	;

iomodifier_list:
	iomodifier_list iomodifier_opt
	|
	;


background_opt:
	AMPERSAND{
		Command::_currentCommand._background = 1;
	}
	| /* can be empty */
	;

commands: 
	command
	| commands command 
	;

command: simple_command
        ;



simple_command:	
/*	pipe_list command_and_args iomodifier_list background_opt NEWLINE {*/
	pipe_list iomodifier_list background_opt NEWLINE{
	/*	printf("   Yacc: Execute command\n");*/
		Command::_currentCommand.execute();
	}
	| NEWLINE /* accept empty amd line */
	| error NEWLINE { yyerrok; }
	;








%%



void
yyerror(const char * s)
{
	fprintf(stderr,"%s", s);
}

int cmpstring(const void *p1, const void *p2){
	return strcmp(*(char * const *)p1, *(char * const *) p2);
}

void expandWildcardsIfNecessary(char * arg)
{
	/*arg does nor have any * or ? */
//	printf("very beginning\n");
	char * p = arg;
	if(strchr(p,'*') == NULL && strchr(p,'?') == NULL){
		Command::_currentSimpleCommand->insertArgument(arg);
		return;
	}else{
//		printf("before call function\n");
		expandWildcard("",arg);
		//printf("before\n");
		qsort(array, nEntries,sizeof(char *),cmpstring);
		//printf("%s\n", array[36]);
		for(int i = 0;i < nEntries; i++){
		  //	printf("%d\n", i);
			Command::_currentSimpleCommand->insertArgument(strdup(array[i]));
		}
		//printf("after for()\n");
	//	if(array != NULL){
		free(array);
	//	}
			//printf("array\n");
		array = NULL;
		nEntries = 0;
		//printf("end\n");
		return;
	}
}

void expandWildcard(char * prefix, char * suffix){
	
//	printf("%d prefix:%s\nsuffix:%s\n",nEntries,prefix,suffix);
	if(strlen(suffix) == 0){
		if(array == NULL){
		//	array = (char**)calloc(sizeof(char*)*maxEntries);
			array = (char**)malloc(maxEntries*sizeof(char*));
		}
//		printf("max:%d\n",maxEntries);
		if(nEntries == maxEntries){
			maxEntries *= 2;
			array = (char**)realloc(array, maxEntries*sizeof(char*));
//			printf("success rea\n");
		//	assert(array != NULL);
		}

		array[nEntries] = strdup(prefix);
		nEntries++;
		return;
	}
	char * s = strchr(suffix,'/');
	char component[1024];
	if(s != NULL){
		strncpy(component,suffix,s-suffix);
		component[s-suffix] = '\0';
		suffix = s + 1;
	}else{
		strcpy(component, suffix);
		suffix = suffix + strlen(suffix);
	}

	/* Expand the component*/
	char newPrefix[1024];
	/* Component does not have wildcard*/
	if(strchr(component,'*') == NULL && strchr(component,'?') == NULL){
		if(strcmp(prefix,"/")){
			sprintf(newPrefix,"%s/%s",prefix,component);
		}else{
			sprintf(newPrefix,"/%s",component);
		}
		expandWildcard(newPrefix,suffix);
		return;
	}
//	printf("component:%s\n",component);
	/* Component has wildcards*/

	char * arg = component;
	char * reg = (char*)malloc(2*strlen(arg)+10);
	char * a = arg;
	char * r = reg;
	/*reg[0] = '^'*/

//	printf("arg:%s\n",arg);

	*r = '^';
//	printf("r:%s\n",r);
	r++;
	/*try to find any * and ? in arg*/
	while(*a){
		if(*a == '*'){
			/* transfer * to .*  */
			*r = '.';
			r++;
			*r = '*';
			r++;
		}else if(*a == '?'){
			/* '?' -> '.' */
			*r = '.';
			r++;
		}else if(*a == '.'){
			/* '.' -> '\.'  */
			*r = '\\';
			r++;
			*r = '.';
			r++;
		}else{
			*r = *a;
			r++;
		}
		a++;
	}
	*r = '$';
	r++;
	*r = 0;
//	printf("reg: %s\n",reg);
	/* START to convert regular expression*/
	regex_t re;
	int result = regcomp(&re, reg,REG_EXTENDED|REG_NOSUB);
	if(result != 0){
		perror("ERROR:***regcomp(expand)\n");
		return;
	}
	char *dir;
	if(strlen(prefix) == 0){	
		dir = ".";
	}else{
		dir = prefix;
	}

	DIR * d = opendir(dir);
	if(d == NULL){
	//	perror("ERROR:***opendir\n");
		return;
	}

	regmatch_t match;
	struct dirent * ent;
	while((ent = readdir(d)) != NULL){
		if(regexec(&re , ent->d_name,1,&match,0) == 0){
			if(ent->d_name[0] == '.'){
				if(arg[0] == '.'){				
					if(strcmp(prefix,"")){
						sprintf(newPrefix,"%s/%s",prefix,ent->d_name);
					}else{
						sprintf(newPrefix,"%s",ent->d_name);
					}
					expandWildcard(newPrefix,suffix);
				}
			}else{
				if(!strcmp(prefix,"")){
				  sprintf(newPrefix,"%s",ent->d_name);
				}
				else if(!strcmp(prefix,"/")){
				  sprintf(newPrefix,"/%s", ent->d_name);
				}else{
				  sprintf(newPrefix,"%s/%s",prefix,ent->d_name);
				}
				expandWildcard(newPrefix,suffix);
			}
		}
		/*}else{
			perror("ERROR:***regexec\n");
			return;
		}*/
		
	}
	closedir(d);
	free(reg);


}

#if 0


main()
{
	yyparse();
}
#endif
