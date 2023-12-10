#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define GCCSH_RL_BUFSIZE 1024

char *gccsh_readline(void) {
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
			buffer[position] = '\n';
			return buffer;
		} else {
			buffer[position] = c;
		}
		position++;
		if(position > bufsize) {
			bufsize += GCCSH_RL_BUFSIZE;
			buffer = realloc(buffer, bufsize);
			if(!buffer) {
				fprintf(stderr,"lsh: allocation error");
				exit(EXIT_FAILURE);
			}
		}
	}
}

void gccsh_loop() {
	char *line;
	int status = 1;
	FILE *fptr;
	do {
		printf("> ");
		line = gccsh_readline();
		fptr = fopen(".gccshtemp.c","w");
		if(fptr == NULL) {
			perror("fopen");
			exit(EXIT_FAILURE);
		}
		fprintf(fptr,"#include <stdio.h>\n"
				"#include <unistd.h>\n"
				"int main() { %s }", line);
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
