#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <limits.h>

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
int gccsh_exec(char* args[]) {
	pid_t pid, wpid;
	int status;
	pid = fork();
	if(pid == 0) {
		int intexec = execvp(args[0], args);
		if(intexec == -1) perror("gccsh");
		exit(EXIT_FAILURE);
	} else if(pid < 0) {
		perror("gccsh");
	} else {
		do {
			wpid = waitpid(pid, &status, WUNTRACED);
		} while (!WIFEXITED(status) && !WIFSIGNALED(status));
	}
	return 1;
}
void gccsh_loop() {
	char *line;
	int line_len;
	int status = 1;
	char* home_path = getenv("HOME");
	char* c_temp_path = malloc(sizeof(char) * PATH_MAX);
	sprintf(c_temp_path, "%s/.gccshtemp.c", home_path);
	char* bin_temp_path = malloc(sizeof(char) * PATH_MAX);
	sprintf(bin_temp_path, "%s/.gccshtemp.bin", home_path);

	FILE *fptr; // .gcctemp.c
	FILE *fprofile_ptr; // .gccsh_profile

	// initialise profile
	fprofile_ptr = fopen("./gccsh_profile", "r");
	off_t size = lseek(fileno(fprofile_ptr), 0, SEEK_END);
	printf("%ld\n", size);
	rewind(fprofile_ptr);
	char* profile_str = malloc(size);
	char* temp_str = malloc(size);
	while(fgets(temp_str, size, fprofile_ptr)) {
		strcat(profile_str,temp_str);
	}
	fclose(fprofile_ptr);
	free(temp_str);
	do {
		printf("> ");
		line = gccsh_readline(&line_len);
		gccsh_match_builtins(line, line_len);
		fptr = fopen(c_temp_path,"w");
		if(fptr == NULL) {
			perror("fopen");
			exit(EXIT_FAILURE);
		}
		fprintf(fptr, profile_str, line);
		fclose(fptr);
		int compSuccess = gccsh_exec((char*[]) {"gcc", c_temp_path, "-o", bin_temp_path, NULL});
		if(compSuccess == 1) {
		gccsh_exec((char*[]) {bin_temp_path,NULL});
		} else status = 0;
		memset(line,0,strlen(line));
		free(line);
	} while (status);
	free(profile_str);
	free(c_temp_path);
	free(bin_temp_path);
}
int main(int argc, char **argv) {
	if(setenv("GCCSH_VERSION","0.1.3",1)) fprintf(stderr,"gccsh: could not set GCCSH_VERSION environment variable");
	gccsh_loop();
	if(unsetenv("GCCSH_VERSION")) fprintf(stderr,"gccsh: could not remove GCCSH_VERSION environment, please remove manually");
	return 0;
}
