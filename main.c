#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>

#define GCCSH_RL_BUFSIZE 1024

char *gccsh_readline(int *line_len) {
	int bufsize = GCCSH_RL_BUFSIZE;
	int position = 0;
	char *buffer = malloc(sizeof(char) * bufsize);
	int c;

	if(!buffer) {
		fprintf(stderr, "gccsh: allocation error");
		exit(EXIT_FAILURE);
	}

	while(1) {
		c = getchar();
		if( c == '\n') {
			buffer[position] = '\0';
			*line_len = position + 1;
			return buffer;
		} else {
			buffer[position] = c;
		}
		position++;
		if(position > bufsize) {
			bufsize += GCCSH_RL_BUFSIZE;
			buffer = realloc(buffer, bufsize);
			if(!buffer) {
				fprintf(stderr,"gccsh: allocation error");
				exit(EXIT_FAILURE);
			}
		}
	}
}
void gccsh_match_builtins(char* line, int line_len) {
	regex_t regex;
	regmatch_t matches[2];
	int reti;
	char *msgbuf = calloc(sizeof(char), line_len);
	reti = regcomp(&regex, "chdir\((.+));", REG_EXTENDED);
	if (reti) {
		fprintf(stderr, "Could not compile regex\n");
		exit(EXIT_FAILURE);
	}
	reti = regexec(&regex, line, 2, matches, 0);
	if(reti == 0) {
		// Match
		memcpy(msgbuf, line + matches[1].rm_so + 2, matches[1].rm_eo - matches[1].rm_so - 4);
		chdir(msgbuf);
	} else if(reti != REG_NOMATCH) {
		regerror(reti, &regex, msgbuf, sizeof(msgbuf));
		exit(EXIT_FAILURE);
	}
	reti = regcomp(&regex, "exit\(.+);", REG_EXTENDED);
	if (reti) {
		fprintf(stderr, "Could not compile regex\n");
		exit(EXIT_FAILURE);
	}
	reti = regexec(&regex, line, 0, NULL, 0);
	if(reti == 0) {
		exit(EXIT_SUCCESS);
	} else if (reti != REG_NOMATCH) {
		regerror(reti, &regex, msgbuf, sizeof(msgbuf));
		exit(EXIT_FAILURE);
	}
	regfree(&regex);
	free(msgbuf);
}
void gccsh_loop() {
	char *line;
	int line_len;
	int status = 1;
	FILE *fptr;
	do {
		printf("> ");
		line = gccsh_readline(&line_len);
		gccsh_match_builtins(line, line_len);
		fptr = fopen(".gccshtemp.c","w");
		if(fptr == NULL) {
			perror("fopen");
			exit(EXIT_FAILURE);
		}
		fprintf(fptr,"#include <stdio.h>\n"
				"#include <unistd.h>\n"
				"#include <stdlib.h>\n"
				"char* cmd(char* command) {\n"
				"FILE *fp; char buffer[1024]; char* result = NULL; size_t result_size = 0;"
				"fp = popen(command, \"r\");"
				"if (fp == NULL) {"
				"perror(\"Error opening pipe\");"
				"return NULL;"
				"}\n"
				"while (fgets(buffer, sizeof(buffer), fp) != NULL) {"
				"result_size += strlen(buffer) * sizeof(char);"
				"result = realloc(result, result_size);"
				"strcat(result, buffer);"
				"}\n"
				"pclose(fp); return result;"
				"}\n"
				"int main() { char version[] = \"0.1.0\"; %s }", line);
		fclose(fptr);
		int compSuccess = system("gcc .gccshtemp.c -o .gccshtemp.bin");
		if(compSuccess == 0) {
			system("./.gccshtemp.bin");
		} else {
			status = 0;
		}
		memset(line,0,strlen(line));
		free(line);
	} while (status);
}
int main(int argc, char **argv) {
	gccsh_loop();
	return 0;
}
