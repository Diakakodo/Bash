#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_LENGTH_INSTRUCTION 256

char* getInstruction(char* sentence) {
	char* endOfLine = NULL;

	if(fgets(sentence, MAX_LENGTH_INSTRUCTION, stdin) != NULL) {
		endOfLine = strchr(sentence, '\n');

		if(endOfLine != NULL)
			*endOfLine = '\0';

		return sentence;
	} else return NULL;
}

void analyseInstruction(char* sentence, char* path) {
	char** tabSentence = parseSentence(sentence);

	if(!strcmp(sentence, "cd")) {
    	printf("chbj");
	}

	sentence = "";
}

int main() {
	char sentence[MAX_LENGTH_INSTRUCTION];
	char path[256];
    getcwd(path, 256);

    while(1) {
    	printf("%s \n", getInstruction(sentence));

    	if(strcmp(sentence, ""))
    		analyseInstruction(sentence, path);
    }

    return 0;
}
