#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LENGTH_INSTRUCTION 128

char* getInstruction(char* sentence) {
	char* endOfLine = NULL;

	if(fgets(sentence, MAX_LENGTH_INSTRUCTION, stdin) != NULL) {
		endOfLine = strchr(sentence, '\n');

		if(endOfLine != NULL) {
			*endOfLine = '\0';
		}

		return sentence;
	} else return NULL;
}

int main() {
    printf("Hello, world!\n");
	char sentence[MAX_LENGTH_INSTRUCTION];

    while(1) {
    	printf("%s", getInstruction(sentence));
    }
    return 0;
}
