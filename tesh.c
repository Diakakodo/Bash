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

typedef void (*add_history_t)(char *);
typedef char* (*readline_t)(const char *);

static add_history_t add_history;
static readline_t readline;

char* builtin[] = {"cd", "exit"};
char* operations[] = {"<", ">", ">>", "&&", "||", "|"};
int (*builtin_func[])(char**, int) = {&cdFonction, &exitFonction};
int (*logicalsAndPipe_func[])(char*, char*, int, int*) = {&executeIfFirstSucceeds, &logicalOr, &pipeOperation};

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
int readOrCreateFile(char* symbol, char* args1, char* args2, int errorMode, int* returnCode) {
	pid_t pid;
	int fd;

	while (args2[0] == ' ')
		args2 = args2 + 1;

	if((pid = fork()) < 0) {
		if(errorMode)
			return errno;
		else perror("fork failed");
	} else if(pid == 0) {
		if(strcmp(symbol,"<") == 0) {
			if((fd = open(args2, O_RDONLY, 0)) < 0) {
				if(errorMode)
					return errno;
				else perror("open failed");
			}

			dup2(fd, 0);
		} else if(strcmp(symbol,">") == 0) {
			if((fd = creat(args2, 0644)) < 0) {
				if(errorMode)
					return errno;
				else perror("creat failed");
			}

			dup2(fd, 1);
		} else if(strcmp(symbol, ">>") == 0) {
			if((fd = open(args2, O_WRONLY | O_APPEND)) < 0) {
				if(errorMode)
					return errno;
				else perror("open failed");
			}

			dup2(fd, 1);
		}
		
		close(fd);
		
		execute(args1, errorMode, returnCode, 0);
		
		dup2(0,0);
		dup2(1,1);

		return errno;
	} else waitpid(pid, 0, 0);

	return 0;
}

// |
int pipeOperation(char* args1, char* args2, int errorMode, int*
returnCode) {
	int fd[2];
	pid_t pid;
	pipe(fd);

	if((pid = fork()) < 0) {
		if(errorMode)
			return errno;
		else perror("fork failed");
	}

	if(pid == 0) {
		dup2(fd[1], 1);
		close(fd[0]);
		close(fd[1]);
		//fprintf(stderr, "args1 = %s\n", args1);
		execute(args1, errorMode, returnCode, 0);

		return errno;
	} else {
		waitpid(pid, 0, 0);
		dup2(fd[0], 0);
		close(fd[0]);
		close(fd[1]);
		char** tabSentence = parseSentence(args2);
		//fprintf(stderr, "args2 = %s\n", args2);
		execvp(tabSentence[0], tabSentence);

		return errno;
	}
	
	wait(NULL);
	wait(NULL);

	return 0;
}

// ||
int logicalOr(char* args1, char* args2, int errorMode, int* returnCode) {
	execute(args1, errorMode, returnCode, 1);

	if(!*returnCode) {
		*returnCode = 1;

		return analyseInstruction(args2, errorMode, returnCode, 1);
	} else return 0;
}

//Fonction &&
int executeIfFirstSucceeds(char* args1, char* args2, int errorMode, int* returnCode) {
	execute(args1, errorMode, returnCode, 1);

	if(*returnCode)
		return analyseInstruction(args2, errorMode, returnCode, 1);
	else return 0;
}

//Renvoie le nombre d'occurences de car dans string
int isInString(char* string, char car) {
	int n = strlen(string);
	int i = 0;

	while (i < n && string[i] != car)
		i++;

	return i != n;
}

// On met les commandes et arguments dans un tableau
char** parseSentence(char* sentence) {
	int length = strlen(sentence);
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

	return tabSentence;
}

//On exécute la commande qui n'est pas un symbole
int execOperation(char** args, int errorMode, int doPipe) {
	if (doPipe) {
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
			do
				waitpid(pid, &status, WUNTRACED);
			while(!WIFEXITED(status) && !WIFSIGNALED(status));
		}
	} else {
		execvp(args[0], args);

		printf("Haha\n");

		if(errorMode)
			return errno;
		else {
			fprintf(stderr, "Failed to execute %s\n", args[0]);
			return exitFonction(args, errorMode);
		}
	}

	return 0;
}

int analyseInstruction(char* sentence, int errorMode, int* returnCode, int doPipe) {
	//Création du tableau des mots
	char** tabSentence;

	//S'il y a des espaces au début on les supprime
	while(sentence[0] == ' ')
		sentence = sentence + 1;

	//Création du tableau
	tabSentence = parseSentence(sentence);

	//On cherche les fonctions builtin
	for(int i = 0; i < nbBuiltins(); i++)
		if(strcmp(tabSentence[0], builtin[i]) == 0)
			return (*builtin_func[i])(tabSentence, errorMode);

	//printf("tabSentence[0]: %s\n", tabSentence[0]);

	//On cherche les booléens
	if(strcmp(tabSentence[0], "false") == 0) {
		*returnCode = 0;
		//printf("returnCode: %d\n", *returnCode);
		return 10;
	} else if(strcmp(tabSentence[0], "true") == 0) {
		*returnCode = 1;
		return 0;
	}

	//Exécution de l'instruction
	if(execOperation(tabSentence, errorMode, doPipe))
		return errno;

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
		} else if (op == 1 && i > 0 && sentence[i - 1] == '>') {
			op = 2;
			i--;
		}
		
		//printf("Operation detectee : %s\n", operations[op]);
		
		int j = 0;
		int m = strlen(operations[op]);
		
		while(i + j + m < n) {
			arg2[j] = sentence[i + j + m];
			j++;
		}

		arg2[j] = '\0';

		j = 0;

		while(j < i) {
			arg1[j] = sentence[j];
			j++;
		}

		arg1[j] = '\0';

		strcpy(args1, arg1);
		strcpy(args2, arg2);

		//Et on renvoie l'indice correspondant à l'opération en question
		return op;
	}
}

int execute(char* sentence, int errorMode, int* returnCode, int doPipe) {
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
		execute(sub_sentence, errorMode, returnCode, doPipe);

		//Appel récursif des autres instructions s'il y en a
		if(i + 1 < strlen(sentence))
			execute(sentence + i + 1, errorMode, returnCode, doPipe);
	} else { //On cherche les caractères correspondant aux opérations qu'on traite à part
		char arg1[100], arg2[100];
		int op = testMethod(sentence, arg1, arg2);

		//Si on n'en a pas trouvé, on appelle la procédure générale
		if(op == -1)
			return analyseInstruction(sentence, errorMode, returnCode, doPipe);
		else { //Sinon on appelle la fonction qui gère ce symbole
			if(op < 3)
				return readOrCreateFile(operations[op], arg1, arg2, errorMode, returnCode);
			else return logicalsAndPipe_func[op - 3](arg1, arg2, errorMode, returnCode);
		}
	}
	return 0;
}

int getInstruction(char* sentence, char prompt[MAX_PROMPT_LENGTH], int readLineMode, void* handle) {
	char* endOfLine = NULL;
	char *line;
	promptString(prompt);

	if(!readLineMode) {
		if(isatty(fileno(stdin)))
			printf("%s", (const char*)prompt);

		if(fgets(sentence, MAX_INSTRUCTION_LENGTH, stdin) != NULL) {
			if((endOfLine = strchr(sentence, '\n')) != NULL)
				*endOfLine = '\0';
		}
	} else if((line = readline((const char*)prompt))) {
		if(line && *line) {
			add_history(line);
			strcpy(sentence, line);
		}
	} else return 1;

	if(feof(stdin))
		return 1;

	return 0;
}

int readFromFile(int argc, char* fileName, int posErrorMode, int status, int errorMode, int* returnCode) {
	FILE* file = fopen(fileName, "r");
	char *endOfLine = NULL;
	char* line = malloc(MAX_LINE_LENGTH * sizeof(char));

	if(file != NULL) {
		while(fgets(line, MAX_LINE_LENGTH, file) != NULL && (!status || (status == 10 && !errorMode))) {
			if((endOfLine = strchr(line, '\n')) != NULL)
				*endOfLine = '\0';

			status = execute(line, errorMode, returnCode, 1);
		}

		if(strcmp(line, "") != 0)
			free(line);

		fclose(file);
	} else printf("Impossible d'ouvrir le fichier %s", fileName);

	return 1;
}

void promptString(char prompt[MAX_PROMPT_LENGTH]) {
	char host[MAX_HOST_LENGTH];
	char path[MAX_PATH_LENGTH];
	gethostname(host, MAX_HOST_LENGTH);
	getcwd(path, sizeof(path));

	strcpy(prompt, getenv("USER"));
	strcat(prompt, "@");
	strcat(prompt, host);
	strcat(prompt, ":");
	strcat(prompt, path);
	strcat(prompt, "$ ");
}

int main(int argc, char** argv) {
	char* sentence = malloc(MAX_INSTRUCTION_LENGTH * sizeof(char));
	char prompt[MAX_PROMPT_LENGTH];
	void* handle;
	int errorMode = 0;
	int readLineMode = 0;
	int status = 0;
	int posErrorMode = 0;
	int returnCode = 1;
	int filePos = 0;

	if(argc > 1) {
		for(int i = 1; i < argc; i++) {
			const int compError = strcmp(argv[i], "-e");
			const int compRead = strcmp(argv[i], "-r");

			if(compError == 0) {
				errorMode = 1;
				posErrorMode = i;
			}

			if(compRead == 0) {
				handle = dlopen("libreadline.so", RTLD_LAZY);

				if(handle == NULL)
					handle = dlopen("libreadline.so.6", RTLD_LAZY);
				else if(handle == NULL)
					handle = dlopen("libreadline.so.5", RTLD_LAZY);

				if (dlerror() != NULL)  {
					printf("%s\n", dlerror());
					status = 1;
				}

				readline = (readline_t)dlsym(handle, "readline");
				add_history = (add_history_t)dlsym(handle, "add_history");

				readLineMode = 1;
			}

			if(*argv[i] != '-')
				filePos = i;
			else if(compError != 0 && compRead != 0)
				printf("Option %d non reconnue\n", i);
		}

		if(filePos)
			status = readFromFile(argc, argv[filePos], posErrorMode, status, errorMode, &returnCode);
	}

	while(!status || (status == 10 && !errorMode)) {
		memset(sentence, 0, MAX_INSTRUCTION_LENGTH * sizeof(char));
		status = getInstruction(sentence, prompt, readLineMode, handle);

		if(strlen(sentence) != 0) {
			returnCode = 1;
			status = execute(sentence, errorMode, &returnCode, 1);
		}
	}

	if(readLineMode)
		dlclose(handle);

	if(sentence)
		free(sentence);

	return 0;
}
