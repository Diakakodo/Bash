#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <winsock2.h>

#define MAX_INSTRUCTION_LENGTH 256
#define MAX_HOST_LENGTH 64
#define MAX_PATH_LENGTH 128
#define BUFSIZE 256

// Renvoie la longueur d'une chaîne de caractères
int string_length(char* string) {

	int length = 0;

	while (string[length] != '\0') {
		length++;
	}

	return length;
}

//Renvoie le nombre d'occurences de car dans string
int number_of_occurences(char* string, char car) {
	int n = string_length(string);
	int count = 0;

	for (int i=0;i<n;i++){
		if (string[i]==car){
			count++;
		}
	}

	return count;
}

// On met les commandes et arguments dans un tableau
char** parseSentence(char* sentence) {

	int length = string_length(sentence);
	char** tabSentence = (char**) malloc((length/2)*sizeof(char*));
	const char delimiter[2] = " ";
	char* word;
	int count = 0;

	word = strtok(sentence, delimiter);

	while (word != NULL) {
		tabSentence[count] = word;
		count++;
		word = strtok(NULL, delimiter);
	}

	//Affichage pour test
	printf("Tableau final :\n");

	for (int i=0; i<count; i++) {
		printf( "%s\n", tabSentence[i]);
	}

	return tabSentence;
}

/*
//On exécute la commande qui n'est ni cd ni un symbole
void execOperation(char** args) {

	pid_t pid, wpid;
	int status;

	pid = fork();
	if (pid == 0) {
		execvp(args[0], args);
	 } else {
		 do {
	  	 wpid = waitpid(pid, &status, WUNTRACED);
		 } while (!WIFEXITED(status) && !WIFSIGNALED(status));
	 }
}*/

void analyseInstruction(char* sentence, char* path) {
	//Création du tableau des mots
	int nb = number_of_occurences(sentence, ';');
	char** tabSentence;
	int i;

	//S'il y a un espace au début on le supprime
	if (sentence[0]==' ') {
		sentence = sentence+1;
	}

	if (nb){
		char sub_sentence[MAX_INSTRUCTION_LENGTH];
		i=0;

		while (sentence[i]!=';'){
			sub_sentence[i]=sentence[i];
			i++;
		}

		//S'il y a un espace à la fin on le supprime
		if (sentence[i-1]==' ') {
			sub_sentence[i-1]='\0';
		}
		else {
			sub_sentence[i]='\0';
		}

		tabSentence = parseSentence(sub_sentence);
	}

	else {
		tabSentence = parseSentence(sentence);
	}

	printf("Appel d'analyse\n");


	//Exécution de l'instruction
	if(!strcmp(tabSentence[0], "cd")) {
		//execOperation(tabSentence);
	}


	//Appel récursif des autres instructions
	if (nb && i+1<string_length(sentence)){
		analyseInstruction(sentence+i+1, path);
	}
}

char* getInstruction(char* sentence) {
	char* endOfLine = NULL;

	if(fgets(sentence, MAX_INSTRUCTION_LENGTH, stdin) != NULL) {
		endOfLine = strchr(sentence, '\n');

		if(endOfLine != NULL)
			*endOfLine = '\0';

		return sentence;
	} else return NULL;

	analyseInstruction(sentence, "");
}

/*
// >>
void appendFile(char** args) {
	int fd[2];
	pipe(fd);
	char bufin[BUFSIZE] = "empty";
	char bufout[ ] = "hello";

	if(!fork()) {
		fd = open(args[1], O_WRONLY | O_APPEND);
		dup2(fd, 1);
		close(fd);
		execlp(args[0], args[0], NULL);
	}
}

// >
void createFile(char** args) {
	int fd[2];
	pipe(fd);

	if(!fork()) {
		fd = open(args[1], O_WRONLY);
		dup2(fd, 1);
		close(fd);
		execlp(args[0], args[0], NULL);
	}
}

// <
void readFromFile(char** args) {
	int fd[2];
	pipe(fd);

	if(!fork()) {
		fd = open(args[1], O_RDONLY);
		dup2(fd, 0);
		close(fd);
		execlp(args[0], args[0], NULL) ;
	}
}

// |
void pipeOperation(char** args) {
	int fd[2];
	pipe(fd);

	if(fork() == 0) {
		dup2(fd[1], 1);
		close(fd[0]);
		close(fd[1]);
		execlp(args[0], args[0], NULL);
	}

	if(fork() == 0) {
		dup2(fd[0], 0);
		close(fd[0]);
		close(fd[1]);
		execlp(args[1], args[1], NULL);
	}

	wait(NULL);
	wait(NULL);
}

// &&
void logicalAnd(char** args) {
	int status;

	if(fork() == 0)
		execlp(args[0], args[0], NULL);

	wait(&status);




	wait(NULL);
}

// ||
void logicalOr(char** args) {
	int status;

	if(fork() == 0)
		execlp(args[0], args[0], NULL);

	wait(&status);

	if((WEXITED(status) && (WEXITSTATUS(status) != 0)) && fork() == 0)
		execpl(args[1], args[1], NULL);

	wait(NULL);
}*/

int main() {
	char sentence[MAX_INSTRUCTION_LENGTH];
	char* user = getenv("USERNAME");
	char host[MAX_HOST_LENGTH];
	char path[MAX_PATH_LENGTH];

    //gethostname(host, MAX_HOST_LENGTH);
    getcwd(path, MAX_PATH_LENGTH);

    while(1) {
    	printf("%s@%s:%s$ ", user, host, path);
    	getInstruction(sentence);

    	if(strcmp(sentence, ""))
    		analyseInstruction(sentence, path);
    }

    return 0;
}
