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
int (*logicalsAndPipe_func[])(char**, char**, int) = {&executeIfFirstSucceeds, &logicalOr, &pipeOperation};
 
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
int pipeOperation(char** args1, char** args2, int errorMode) {
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
        execvp(args1[0], args1);
 
        if(errorMode)
            return errno;
        else fprintf(stderr, "Failed to execute %s\n", args1[0]);
    }
 
    if((pid = fork()) < 0) {
        if(errorMode)
            return errno;
        else perror("fork failed");
    } else if(pid == 0) {
        dup2(fd[0], 0);
        close(fd[0]);
        close(fd[1]);
        execvp(args1[0], args1);
 
        if(errorMode)
            return errno;
        else fprintf(stderr, "Failed to execute %s\n", args1[0]);
    }
 
    wait(NULL);
    wait(NULL);
 
    return 0;
}
 
// ||
int logicalOr(char** args1, char** args2, int errorMode) {
    int status;
    pid_t pid;
 
    if((pid = fork()) < 0) {
        if(errorMode)
            return errno;
        else perror("fork failed");
    } else if(pid == 0) {
        execvp(args1[0], args1);
 
        if(errorMode)
            return errno;
        else fprintf(stderr, "Failed to execute %s\n", args1[0]);
    }
 
    wait(&status);
 
    if(WIFEXITED(status) && (WEXITSTATUS(status) != 0)) {
        if((pid = fork()) < 0) {
            if(errorMode)
                return errno;
            else perror("fork failed");
        } else if(pid == 0) {
            execvp(args1[0], args1);
 
            if(errorMode)
                return errno;
            else fprintf(stderr, "Failed to execute %s\n", args1[0]);
        }
    }
 
    wait(NULL);
 
    return 0;
}
 
//Fonction &&
int executeIfFirstSucceeds(char** args1, char** args2, int errorMode) {
    if(!analyseInstruction(*args1, errorMode))
        return analyseInstruction(*args2, errorMode);
    else return errno;
}
 
//Renvoie le nombre d'occurences de car dans string
int numberOfOccurences(char* string, char car) {
    int n = strlen(string);
    int count = 0;
 
    for (int i = 0; i < n; i++)
        if (string[i] == car)
            count++;
 
    return count;
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
 
    //Affichage pour test
    /*printf("Tableau final :\n");
 
    for (int i = 0; i < count; i++) {
        printf("%s\n", tabSentence[i]);
    }*/
 
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
        else fprintf(stderr, "Failed to execute %s\n", args[0]);
    } else {
        do {
            waitpid(pid, &status, WUNTRACED);
        } while(!WIFEXITED(status) && !WIFSIGNALED(status));
    }
 
    return 0;
}
 
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
        if (sentence[i - 1] == ' ')
            sub_sentence[i - 1] = '\0';
        else sub_sentence[i] = '\0';
 
        tabSentence = parseSentence(sub_sentence);
    } else tabSentence = parseSentence(sentence);
 
    for(int i = 0; i < nbBuiltins(); i++)
        if(strcmp(tabSentence[0], builtin[i]) == 0)
            return (*builtin_func[i])(tabSentence, errorMode);

    if(tabSentence[1] == NULL) {
    	if(strcmp(tabSentence[0], "false") == 0)
	        return 10;
	    else if(strcmp(tabSentence[0], "true") == 0)
	        return 0;
	}

    //Exécution de l'instruction
    if(execOperation(tabSentence, errorMode))
        return errno;
 
    //Appel récursif des autres instructions
    if (nb && i + 1 < strlen(sentence))
        analyseInstruction(sentence + i + 1, errorMode);
 
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
        endOfLine = strchr(sentence, '\n');
 
        if(endOfLine != NULL)
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
        int filePos = 1;
        char* line = malloc(MAX_LINE_LENGTH * sizeof(char));
 
        if(argc != 2 && posErrorMode == 1)
            filePos = 2;
 
        file = fopen(argv[filePos], "r");
 
        if(file != NULL) {
            while(fgets(line, MAX_LINE_LENGTH, file) != NULL && !status)
                analyseInstruction(line, errorMode);
 
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
 
    while (i < m && i + index < n && sentence[i + index] == subs[i]) {
        i++;
    }
 
    return i == m;
}
 
int testMethod(char* sentence, char* operation, char args1[100], char args2[100]) {
    int i = 0;
    int j = 0;
    int n = strlen(sentence);
    int m = strlen(operation);
    char arg1[n];
    char arg2[n];
 
    while (i < n && !isSubstringHere(sentence, operation, i)) {
        arg1[i] = sentence[i];
        i++ ;
    }
 
    i += m;
 
    while (i + j < n) {
        arg2[j] = sentence[i + j];
        j++;
    }
 
    strcpy(args1, arg1);
 
    if(i == n + m)
        strcpy(args2, "");
    else strcpy(args2, arg2);
 
    return strcmp(arg2, "") == 0;
}
 
int main(int argc, char** argv) {
    char* sentence = malloc(MAX_INSTRUCTION_LENGTH * sizeof(char));
    const char* user = getenv("USERNAME");
    char host[MAX_HOST_LENGTH];
    char path[MAX_PATH_LENGTH];
    void *handle;
    char *error;
    int errorMode = 0;
    int readLineMode = 0;
    int status = 0;
    int count = 0;
    int posErrorMode = 0;
    //int (*rlBindKey)(int, rl_command_func_t *function);
 
    gethostname(host, MAX_HOST_LENGTH);
    chdir(getenv("HOME"));
 
    if(argc > 1) {
        for(int i = 1; i < argc; i++) {
            if(strcmp(argv[i], "-e")) {
                errorMode = 1;
                posErrorMode = i;
                count++;
            }
 
            if(strcmp(argv[i], "-r")) {
                if(!(handle = dlopen("libreadline.so.6", RTLD_LAZY | RTLD_LOCAL))) {
                    fputs (dlerror(), stderr);
                    status = 1;
                }
 
                if ((error = dlerror()) != NULL)  {
                    fputs(error, stderr);
                    status = 1;
                }
 
                //rlBindKey = dlsym(handle, "rl_bind_key");
 
                readLineMode = 1;
                count++;
                //rlBindKey('\t', dlsym(handle, "rl_complete"));
            }
        }
 
        status = readFromFile(argc, argv, count, posErrorMode, status, errorMode);
    }
 
    while(!status || status == 10) {
        getcwd(path, sizeof(path));
 
        if(isatty(fileno(stdin)))
            printf("%s@%s:%s$ ", user, host, path);
 
        status = getInstruction(sentence, readLineMode, handle);
 
        if(sentence && *sentence) {
            status = analyseInstruction(sentence, errorMode);
            /*char arg1[100], arg2[100];
            int i = 0;
 
            while (i < nbOperations() && testMethod(sentence, operations[i], arg1, arg2)) {
                i++;
            }
 
            if(i == nbOperations()) {
                status = analyseInstruction(sentence, errorMode);
            } else {
                if (i < 3) {
                    status = readOrCreateFile(operations[i], &arg1, &arg2, errorMode);
                } else {
                    status = logicalsAndPipe_func[i - 3](&arg1, &arg2, errorMode);
                }
            }*/
        }
    }
 
    if(sentence)
        free(sentence);
 
    return 0;
}