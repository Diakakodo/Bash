#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dlfcn.h>
#include "tesh.h"

#define MAX_INSTRUCTION_LENGTH 256
#define MAX_HOST_LENGTH 64
#define MAX_PATH_LENGTH 256
#define MAX_LINE_LENGTH 512

char* builtin[] = {"cd", "exit"};
char* operations[] = {"<", ">", ">>", "&&", "||", "|"};
int (*builtin_func[])(char**, int) = {&cdFonction, &exitFonction};
int (*logicalsAndPipe_func[])(char*, char*, int) = {&executeIfFirstSucceeds, &logicalOr, &pipeOperation};

int nbBuiltins() {
	return sizeof(builtin) / sizeof(char*);
}

int nbOperations() {
	return sizeof(operations) / sizeof(char*);
}

int cdFonction(char** args, int errorMode) {
	if(args[1] == NULL)
		fprintf(stderr, "Argument manquant\n");
	else {
		if(chdir(args[1]) != 0) {
			if(errorMode)
				return errno;
			else perror("Dossier inexistant");
		}
	}

	return 0;
}

int exitFonction(char** args, int errorMode) {
	return 1;
}

// < > >>
int readOrCreateFile(char* symbol, char** args1, char** args2, int errorMode) {
	pid_t pid;
	int fd;

	if((pid = fork()) < 0) {
		if(errorMode)
			return errno;
		else perror("fork failed");
	} else if(pid == 0) {
		if(symbol[0] == '<') {
			if((fd = open(*args2, O_RDONLY, 0)) < 0) {
				if(errorMode)
					return errno;
				else perror("open failed");
			}

			dup2(fd, 0);
		} else if(symbol[0] == '>') {
			if((fd = creat(*args2, O_WRONLY)) < 0) {
				if(errorMode)
					return errno;
				else perror("creat failed");
			}

			dup2(fd, 1);
		} else if(strcmp(symbol, ">>")) {
			if((fd = open(*args2, O_WRONLY | O_APPEND)) < 0) {
				if(errorMode)
					return errno;
				else perror("open failed");
			}

			dup2(fd, 1);
		}

		close(fd);
		execvp(args1[0], args1);

		if(errorMode)
			return errno;
		else fprintf(stderr, "Failed to execute %s\n", args1[0]);
	} else {
		waitpid(pid, 0, 0);
		free(args1);
	}

	return 0;
}

// |
int pipeOperation(char* args1, char* args2, int errorMode) {
	int fd[2];
	pid_t pid;
	pipe(fd);

	if((pid = fork()) < 0) {
		if(errorMode)
			return errno;
		else perror("fork failed");
	} else if(pid == 0) {
		dup2(fd[1], 1);
		close(fd[0]);
		close(fd[1]);
		execute(args1, errorMode);

		if(errorMode)
			return errno;
		else fprintf(stderr, "Failed to execute %s\n", args1);
	}

	if((pid = fork()) < 0) {
		if(errorMode)
			return errno;
		else perror("fork failed");
	} else if(pid == 0) {
		dup2(fd[0], 0);
		close(fd[0]);
		close(fd[1]);
		char** tabSentence = parseSentence(args2);
		execvp(tabSentence[0], tabSentence);

		if(errorMode)
			return errno;
		else fprintf(stderr, "Failed to execute %s\n", tabSentence[0]);
	}

	wait(NULL);
	wait(NULL);

	return 0;
}

// ||
int logicalOr(char* args1, char* args2, int errorMode) {
	if(execute(args1, errorMode))
		return analyseInstruction(args2, errorMode);
	else {
		return errno;
	}
}

//Fonction &&
int executeIfFirstSucceeds(char* args1, char* args2, int errorMode) {
	if(!execute(args1, errorMode))
		return analyseInstruction(args2, errorMode);
	else {
		return errno;
	}
}

//Renvoie le nombre d'occurences de car dans string
int isInString(char* string, char car) {
	int n = strlen(string);
	int i = 0;

	while (i<n && string[i]!=car) {i++;}

	return i!=n;
}

// On met les commandes et arguments dans un tableau
char** parseSentence(char* sentence) {
	int length = strlen(sentence);
	char** tabSentence = malloc((length / 2) * sizeof(char*));
	const char delimiter[2] = " ";
	char* word;
	int count = 0;
	//printf("%s\n", sentence);

	word = strtok(sentence, delimiter);
	//printf("%s\n", sentence);

	while(word != NULL) {
		//printf("%s\n", word);

		tabSentence[count] = word;

		count++;
		word = strtok(NULL, delimiter);
	}

	tabSentence[count] = NULL;

	return tabSentence;
}

//On exécute la commande qui n'est pas un symbole
int execOperation(char** args, int errorMode) {
	pid_t pid;
	int status;

	if((pid = fork()) < 0) {
		if(errorMode)
			return errno;
		else perror("fork failed");
	} else if(pid == 0) {
		execvp(args[0], args);

		if(errorMode)
			return errno;
		else {
			fprintf(stderr, "Failed to execute %s\n", args[0]);
			return exitFonction(args, errorMode);
		}
	} else {
		do {
			waitpid(pid, &status, WUNTRACED);
		} while(!WIFEXITED(status) && !WIFSIGNALED(status));
	}

	return 0;
}

int analyseInstruction(char* sentence, int errorMode) {
	//Création du tableau des mots
	char** tabSentence;

	//S'il y a des espaces au début on les supprime
	while(sentence[0] == ' ') {sentence = sentence + 1;}

	//Création du tableau
	tabSentence = parseSentence(sentence);

	//On cherche les fonctions builtin
	for(int i = 0; i < nbBuiltins(); i++)
		if(strcmp(tabSentence[0], builtin[i]) == 0)
			return (*builtin_func[i])(tabSentence, errorMode);

	//printf("%s\n", tabSentence[0]);

	//On cherche les booléens
	if(strcmp(tabSentence[0], "false") == 0) {
		return 10;
	}
	else if(strcmp(tabSentence[0], "true") == 0) {
		return 0;
	}

	//Exécution de l'instruction
	if(execOperation(tabSentence, errorMode))
		return errno;

	return 0;
}

int getInstruction(char* sentence, int readLineMode, void* handle) {
	char* endOfLine = NULL;
	char* (*readFonction)(char*);
	void* (*addHistory)(char*);

	if(readLineMode) {
		readFonction = dlsym(handle, "readLine");
		addHistory = dlsym(handle, "add_history");

		if((sentence = readFonction("")))
			addHistory(sentence);

		dlclose(handle);
	} else if(fgets(sentence, MAX_INSTRUCTION_LENGTH, stdin) != NULL) {
		if((endOfLine = strchr(sentence, '\n')) != NULL)
			*endOfLine = '\0';
	}

	if(feof(stdin)) {
		printf("\n");
		return 1;
	}

	return 0;
}

int readFromFile(int argc, char** argv, int count, int posErrorMode, int status, int errorMode) {
	if(argc - count == 2) {
		FILE* file = NULL;
		char *endOfLine = NULL;
		int filePos = 1;

		char* line = malloc(MAX_LINE_LENGTH * sizeof(char));

		if(argc != 2 && posErrorMode == 1)
			filePos = 2;

		file = fopen(argv[filePos], "r");

		if(file != NULL) {
			while(fgets(line, MAX_LINE_LENGTH, file) != NULL && !status) {
				if((endOfLine = strchr(line, '\n')) != NULL)
					*endOfLine = '\0';

				status = execute(line, errorMode);
			}

			if(strcmp(line, "") != 0)
				free(line);

			fclose(file);
		} else printf("Impossible d'ouvrir le fichier %s", argv[filePos]);

		return 1;
	}

	return 0;
}

int isSubstringHere(char* sentence, char* subs, int index) {
	int i = 0;
	int n = strlen(sentence);
	int m = strlen(subs);

	while (i < m && i + index < n && sentence[i + index] == subs[i])
		i++;

	return i == m;
}

int testMethod(char* sentence, char args1[100], char args2[100]) {
	int n = strlen(sentence);
	int i = n - 1;
	int op;
	int nbOp = nbOperations();
	int bool = 1;
	char arg1[n];
	char arg2[n];

	while (i >= 0 && bool) {
		op = 0;

		while(op < nbOp && !isSubstringHere(sentence, operations[op], i))
			op++;

		bool = op == nbOp;

		if(bool)
			i-- ;
	}

	//Si on n'a pas trouvé les symboles d'opérations on le signale en renvoyant 0
	if (i == -1)
		return -1;
	else { //Sinon, on modifie args1 et args2 après avoir différencié | et ||
		if (op == 5 && i > 0 && sentence[i - 1] == '|') {
			op = 4;
			i--;
		}

		int j = 0;
		int m = strlen(operations[op]);

		while(i + j + m < n) {
			arg2[j] = sentence[i + j + m];
			j++;
		}

		j = 0;

		while(j <= i - m) {
			arg1[j] = sentence[j];
			j++;
		}

		arg1[j] = '\0';
		arg2[j] = '\0';

		strcpy(args1, arg1);
		strcpy(args2, arg2);

		//Et on renvoie l'indice correspondant à l'opération en question
		return op;
	}
}

int execute(char* sentence, int errorMode) {
	//On commence par traiter les ;
	int bool = isInString(sentence, ';');

	if(bool) {
		char sub_sentence[MAX_INSTRUCTION_LENGTH];
		int i = 0;
		int j = 1;

		while (sentence[i] != ';') {
			sub_sentence[i] = sentence[i];
			i++;
		}

		//S'il y a des espaces à la fin on les supprime
		while(sentence[i - j] == ' ')
			j++;

		sub_sentence[i - j + 1] = '\0';

		//On exécute la première instruction
		execute(sub_sentence, errorMode);
		//Appel récursif des autres instructions s'il y en a
		if(i + 1 < strlen(sentence))
			execute(sentence+i+1, errorMode);
	} else { //On cherche les caractères correspondant aux opérations qu'on traite à part
		char arg1[100], arg2[100];
		int op = testMethod(sentence, arg1, arg2);

		//Si on n'en a pas trouvé, on appelle la procédure générale
		if(op == -1)
			return analyseInstruction(sentence, errorMode);
		else { //Sinon on appelle la fonction qui gère ce symbole
			if(op < 3)
				return 0; //readOrCreateFile(operations[op], arg1, arg2, errorMode);
			else return logicalsAndPipe_func[op - 3](arg1, arg2, errorMode);
		}
	}
	return 0;
}

int main(int argc, char** argv) {
	int sentenceSize = MAX_INSTRUCTION_LENGTH * sizeof(char);
	char* sentence = malloc(sentenceSize);
	const char* user = getenv("USER");
	char host[MAX_HOST_LENGTH];
	char path[MAX_PATH_LENGTH];
	void *handle;
	char *error;
	int errorMode = 0;
	int readLineMode = 0;
	int status = 0;
	int count = 0;
	int posErrorMode = 0;
	int firstNonTerminal = 1;

	gethostname(host, MAX_HOST_LENGTH);

	if(argc > 1) {
		for(int i = 1; i < argc; i++) {
			if(strcmp(argv[i], "-e") == 0) {
				errorMode = 1;
				posErrorMode = i;
				count++;
			}

			if(strcmp(argv[i], "-r") == 0) {
				if(!(handle = dlopen("libreadline.so.6", RTLD_LAZY | RTLD_LOCAL))) {
					fputs(dlerror(), stderr);
					status = 1;
				}

				if ((error = dlerror()) != NULL)  {
					fputs(error, stderr);
					status = 1;
				}

				readLineMode = 1;
				count++;
			}
		}

		status = readFromFile(argc, argv, count, posErrorMode, status, errorMode);
	}

	while((!status || status == 10) && firstNonTerminal) {
		getcwd(path, sizeof(path));

		if(isatty(fileno(stdin)))
			printf("%s@%s:%s$ ", user, host, path);
		else firstNonTerminal = 0;

		memset(sentence, 0, MAX_INSTRUCTION_LENGTH * sizeof(char));
		status = getInstruction(sentence, readLineMode, handle);

		if(sentence && *sentence)
			status = execute(sentence, errorMode);
	}

	if(sentence)
		free(sentence);

	return 0;
}
