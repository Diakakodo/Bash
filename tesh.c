#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <winsock2.h>

#define MAX_INSTRUCTION_LENGTH 256
#define MAX_HOST_LENGTH 64
#define MAX_PATH_LENGTH 256
#define BUFSIZE 256

int cdFonction(char** args) {
  if(args[1] == NULL)
    fprintf(stderr, "Argument manquant\n");
  else {
    if(chdir(args[1]) != 0)
      perror("Dossier inexistant");
  }

  return 1;
}

int exitFonction(char** args) {
	return 0;
}

char* builtin[] = {"cd", "exit"};
int (*builtin_func[])(char**) = {&cdFonction, &exitFonction};

int nbBuiltins() {
  return sizeof(builtin) / sizeof(char*);
}

// Renvoie la longueur d'une chaîne de caractères
int stringLength(char* string) {
	int length = 0;

	while (string[length] != '\0') {
		length++;
	}

	return length;
}

//Renvoie le nombre d'occurences de car dans string
int number_of_occurences(char* string, char car) {
	int n = stringLength(string);
	int count = 0;

	for (int i = 0; i < n; i++) {
		if (string[i] == car) {
			count++;
		}
	}

	return count;
}

// On met les commandes et arguments dans un tableau
char** parseSentence(char* sentence) {
	int length = stringLength(sentence);
	char** tabSentence = malloc((length / 2) * sizeof(char*));
	const char delimiter[2] = " ";
	char* word;
	int count = 0;

	word = strtok(sentence, delimiter);

	while(word != NULL) {
		tabSentence[count] = word;
		count++;
		word = strtok(NULL, delimiter);
	}

	tabSentence[count] = NULL;

	//Affichage pour test
	printf("Tableau final :\n");

	for (int i = 0; i < count; i++) {
		printf("%s\n", tabSentence[i]);
	}

	return tabSentence;
}

/*
//On exécute la commande qui n'est pas un symbole
int execOperation(char** args) {
	pid_t pid, wpid;
	int status;

	for(i = 0; i < nbBuiltins(); i++)
    	if(strcmp(args[0], builtin[i]) == 0)
    		return (*builtin_func[i])(args);

	pid = fork();

	if(pid == 0) {
		execvp(args[0], args);
	} else {
		do {
	  		wpid = waitpid(pid, &status, WUNTRACED);
		} while(!WIFEXITED(status) && !WIFSIGNALED(status));
	}

	return 1;
}*/

int analyseInstruction(char* sentence) {
	//Création du tableau des mots
	int nb = number_of_occurences(sentence, ';');
	char** tabSentence;
	int i;

	//S'il y a un espace au début on le supprime
	if(sentence[0] == ' ') {
		sentence = sentence + 1;
	}

	if(nb) {
		char sub_sentence[MAX_INSTRUCTION_LENGTH];
		i = 0;

		while (sentence[i] != ';') {
			sub_sentence[i] = sentence[i];
			i++;
		}

		//S'il y a un espace à la fin on le supprime
		if (sentence[i - 1] == ' ') {
			sub_sentence[i - 1] = '\0';
		} else {
			sub_sentence[i] = '\0';
		}

		tabSentence = parseSentence(sub_sentence);
	} else {
		tabSentence = parseSentence(sentence);
	}

	printf("Appel d'analyse\n");

	//Exécution de l'instruction
	//if(execOperation(tabSentence))
	//	return 0;

	//Appel récursif des autres instructions
	if (nb && i + 1 < stringLength(sentence)) {
		analyseInstruction(sentence + i + 1);
	}

	return 1;
}

char* getInstruction(char* sentence) {
	char* endOfLine = NULL;

	if(fgets(sentence, MAX_INSTRUCTION_LENGTH, stdin) != NULL) {
		endOfLine = strchr(sentence, '\n');

		if(endOfLine != NULL)
			*endOfLine = '\0';

		return sentence;
	} else return NULL;

	analyseInstruction(sentence);
}

/*
// < > >>
int readOrCreateFile(char* symbol, char** args1, char** args2) {
	int pid, fd;

	if((pid = fork()) < 0)
		perror("fork failed");
	else if(pid == 0) {
		if(symbol[0] == '<') {
			if((fd = open(*args2, O_RDONLY, 0)) < 0)
				perror("open failed");

			dup2(fd, 0);
		} else if(symbol[0] == '>') {
			if((fd = creat(*args2, O_WRONLY)) < 0)
				perror("creat failed");

			dup2(fd, 1);
		} else if(strcmp(symbol, ">>")) {
			if((fd = open(*args2, O_WRONLY | O_APPEND)) < 0)
				perror("open failed");

			dup2(fd, 1);
		}

		close(fd);
		execlp(*args1, *args1, NULL);
		fprintf(stderr, "Failed to execute %s\n", args1[0]);
	} else {
		waitpid(pid, 0, 0);
		free(args1);
	}

	return 1;
}

// |
void pipeOperation(char** args1, char** args2) {
	int fd[2];
	pipe(fd);

	if(fork() == 0) {
		dup2(fd[1], 1);
		close(fd[0]);
		close(fd[1]);
		execlp(*args1, *args1, NULL);
	}

	if(fork() == 0) {
		dup2(fd[0], 0);
		close(fd[0]);
		close(fd[1]);
		execlp(*args2, *args2, NULL);
	}

	wait(NULL);
	wait(NULL);
}


// &&
void logicalAnd(char** args1, char** args2) {
	int status;

	if(fork() == 0)
		execlp(args[0], args[0], NULL);

	wait(&status);




	wait(NULL);
}

// ||
void logicalOr(char** args1, char** args2) {
	int status;

	if(fork() == 0)
		execlp(*args1, *args1, NULL);

	wait(&status);

	if((WEXITED(status) && (WEXITSTATUS(status) != 0)) && fork() == 0)
		execlp(*args1, *args1, NULL);

	wait(NULL);
}
*/

int main() {
	char sentence[MAX_INSTRUCTION_LENGTH];
	const char* user = getenv("USERNAME");
	char host[MAX_HOST_LENGTH];
	char path[MAX_PATH_LENGTH];
	int status = 1;

    gethostname(host, MAX_HOST_LENGTH);
    chdir(getenv("USERPROFILE"));			//chdir(getenv("HOME"));

    while(status) {
    	getcwd(path, sizeof(path));
    	printf("%s@%s:%s$ ", user, host, path);
    	getInstruction(sentence);

    	if(strcmp(sentence, ""))
    		status = analyseInstruction(sentence);
    }

    return 0;
}
