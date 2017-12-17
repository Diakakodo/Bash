#define MAX_PROMPT_LENGTH 512

int cdFonction(char**, int);
int exitFonction(char**, int);
int nbBuiltins();
int nbLogicalsAndPipe();
int executeIfFirstSucceeds(char*, char*, int, int*);
int readOrCreateFile(char*, char*, char*, int, int*);
int pipeOperation(char*, char*, int, int*);
int logicalOr(char*, char*, int, int*);
int isInString(char*, char);
char** parseSentence(char*);
int execOperation(char**, int, int);
int analyseInstruction(char*, int, int*, int);
int testMethod(char*, char*, char*);
int getInstruction(char*, char[MAX_PROMPT_LENGTH], int, void*);
int readFromFile(int, char*, int, int, int, int*);
int isSubstringHere(char*, char*, int);
int execute(char*, int, int*, int);
void promptString(char[MAX_PROMPT_LENGTH]);
