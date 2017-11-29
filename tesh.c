#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#define MAX_INSTRUCTION_LENGTH 256
#define MAX_HOST_LENGTH 64
#define MAX_PATH_LENGTH 256
#define MAX_FILENAME_LENGTH 64
#define MAX_LINE_LENGTH 512
#define BUFSIZE 256

int cdFonction(char** args, int errorMode) {
  if(args[1] == NULL)
    fprintf(stderr, "Argument manquant\n");
  else {
    if(chdir(args[1]) != 0) {
		if(errorMode)
			return 0;
		else perror("Dossier inexistant");
    }
  }

  return 1;
}

int exitFonction(char** args, int errorMode) {
	return 0;
}

char* builtin[] = {"cd", "exit"};
int (*builtin_func[])(char**, int) = {&cdFonction, &exitFonction};

int nbBuiltins() {
  return sizeof(builtin) / sizeof(char*);
}

/*
// < > >>
int readOrCreateFile(char* symbol, char** args1, char** args2, int errorMode) {
	pid_t pid;
	int fd;

	if((pid = fork()) < 0) {
		if(errorMode)
			return 0;
		else perror("fork failed");
	} else if(pid == 0) {
		if(symbol[0] == '<') {
			if((fd = open(*args2, O_RDONLY, 0)) < 0) {
				if(errorMode)
					return 0;
				else perror("open failed");
			}

			dup2(fd, 0);
		} else if(symbol[0] == '>') {
			if((fd = creat(*args2, O_WRONLY)) < 0) {
				if(errorMode)
					return 0;
				else perror("creat failed");
			}

			dup2(fd, 1);
		} else if(strcmp(symbol, ">>")) {
			if((fd = open(*args2, O_WRONLY | O_APPEND)) < 0)
				if(errorMode)
					return 0;
				else perror("open failed");
			}

			dup2(fd, 1);
		}

		close(fd);
		execvp(args1[0], args1);

		if(errorMode)
			return 0;
		else fprintf(stderr, "Failed to execute %s\n", args1[0]);
	} else {
		waitpid(pid, 0, 0);
		free(args1);
	}

	return 1;
}

// |
int pipeOperation(char** args1, char** args2, int errorMode) {
	int fd[2];
	pid_t pid;
	pipe(fd);

	if((pid = fork()) < 0) {
		if(errorMode)
			return 0;
		else perror("fork failed");
	} else if(pid == 0) {
		dup2(fd[1], 1);
		close(fd[0]);
		close(fd[1]);
		execvp(args1[0], args1);

		if(errorMode)
			return 0;
		else fprintf(stderr, "Failed to execute %s\n", args1[0]);
	}

	if((pid = fork()) < 0) {
		if(errorMode)
			return 0;
		else perror("fork failed");
	} else if(pid == 0) {
		dup2(fd[0], 0);
		close(fd[0]);
		close(fd[1]);
		execvp(args1[0], args1);

		if(errorMode)
			return 0;
		else fprintf(stderr, "Failed to execute %s\n", args1[0]);
	}

	wait(NULL);
	wait(NULL);

	return 1;
}

// ||
int logicalOr(char** args1, char** args2, int errorMode) {
	int status;
	pid_t pid;

	if((pid = fork()) < 0) {
		if(errorMode)
			return 0;
		else perror("fork failed");
	} else if(pid == 0) {
		execvp(args1[0], args1);

		if(errorMode)
			return 0;
		else fprintf(stderr, "Failed to execute %s\n", args1[0]);
	}

	wait(&status);

	if((WEXITED(status) && (WEXITSTATUS(status) != 0)))) {
		if((pid = fork()) < 0) {
			if(errorMode)
				return 0;
			else perror("fork failed");
		} else if(pid == 0) {
			execvp(args1[0], args1);

			if(errorMode)
				return 0;
			else fprintf(stderr, "Failed to execute %s\n", args1[0]);
		}
	}

	wait(NULL);

	return 1;
}
*/

// Renvoie la longueur d'une chaîne de caractères
int stringLength(char* string) {
	int length = 0;

	while (string[length] != '\0') {
		length++;
	}

	return length;
}

//Renvoie le nombre d'occurences de car dans string
int numberOfOccurences(char* string, char car) {
	int n = stringLength(string);
	int count = 0;

	for (int i = 0; i < n; i++)
		if (string[i] == car)
			count++;

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
int execOperation(char** args, int errorMode) {
	pid_t pid, wpid;
	int status;

	for(i = 0; i < nbBuiltins(); i++)
    	if(strcmp(args[0], builtin[i]) == 0)
    		return (*builtin_func[i])(args);

	if((pid = fork()) < 0) {
		if(errorMode)
			return 0;
		else perror("fork failed");
	} else if(pid == 0) {
		execvp(args[0], args);

		if(errorMode)
				return 0;
		else fprintf(stderr, "Failed to execute %s\n", args1[0]);
	} else {
		do {
	  		wpid = waitpid(pid, &status, WUNTRACED);
		} while(!WIFEXITED(status) && !WIFSIGNALED(status));
	}

	return 1;
}*/

int analyseInstruction(char* sentence, int errorMode) {
	//Création du tableau des mots
	int nb = numberOfOccurences(sentence, ';');
	char** tabSentence;
	int i;

	//S'il y a un espace au début on le supprime
	if(sentence[0] == ' ')
		sentence = sentence + 1;

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

		//On teste s'il y a un &&
		if (numberOfOccurences(sub_sentence, '&') > 1) {
			executeIfFirstSucceeds(sub_sentence);
		} else {
			tabSentence = parseSentence(sub_sentence);
		}
	} else {
		//On teste s'il y a un &&
		if (numberOfOccurences(sentence, '&') > 1) {
			return executeIfFirstSucceeds(sentence);
		} else {
			tabSentence = parseSentence(sentence);
		}
	}

	printf("Appel d'analyse\n");

	//Exécution de l'instruction
	//if(execOperation(tabSentence, errorMode))
	//	return 0;

	//Appel récursif des autres instructions
	if (nb && i + 1 < stringLength(sentence)) {
		analyseInstruction(sentence + i + 1, errorMode);
	}

	return 1;
}

//Fonction &&
int executeIfFirstSucceeds(char* sentence, int errorMode) {
	const char delimiter[2] = "&&";
	char* command;

	command = strtok(sentence, delimiter);

	if(analyseInstruction(command, errorMode)) {
		command = strtok(sentence, delimiter);
		return analyseInstruction(command, errorMode);
	} else {
		return 0;
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
}

int main(int argc, char** argv) {
	char sentence[MAX_INSTRUCTION_LENGTH];
	const char* user = getenv("USERNAME");
	char host[MAX_HOST_LENGTH];
	char path[MAX_PATH_LENGTH];
	int errorMode = 0;
	int readLineMode = 0;
	int status = 1;
	int count = 0;
	int posErrorMode = 0;

    //gethostname(host, MAX_HOST_LENGTH);
    chdir(getenv("USERPROFILE"));			//chdir(getenv("HOME"));

    if(argc > 1) {
    	for(int i = 1; i < argc; i++) {
    		if(strcmp(argv[i], "-e")) {
    			errorMode = 1;
    			posErrorMode = i;
    			count++;
    		}

    		if(strcmp(argv[i], "-r")) {
    			readLineMode = 1;
    			count++;
    		}
    	}

    	if(argc - count == 2) {
		    FILE* file = NULL;
		    char line[MAX_LINE_LENGTH] = "";
    		status = 0;

    		if(argc == 2)
    			file = fopen(argv[1], "r");
    		else {
    			if(posErrorMode == 1)
    				file = fopen(argv[2], "r");
    			else file = fopen(argv[1], "r");
    		}

    		file = fopen(argv[2], "r");

			if (file != NULL) {
		        while(fgets(line, MAX_LINE_LENGTH, file) != NULL) {
		        	strcat(sentence, line);
		        	strcat(sentence, " ; ");
		        }

		        analyseInstruction(sentence, errorMode);
			} else printf("Impossible d'ouvrir le fichier %s", argv[2]);
    	}
    }

    while(status) {
    	getcwd(path, sizeof(path));
    	printf("%s@%s:%s$ ", user, host, path);
    	getInstruction(sentence);

    	if(strcmp(sentence, ""))
    		status = analyseInstruction(sentence, errorMode);
    }

    return 0;
}
