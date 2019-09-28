#include<stdio.h> 
#include<string.h> 
#include<stdlib.h> 
#include<unistd.h> 
#include<sys/types.h> 
#include<sys/wait.h> 
#include<readline/readline.h> 
#include<readline/history.h> 
  
#define MAXCOM 1000 // max number of letters to be supported 
#define MAXLIST 100 // max number of commands to be supported 
  
// Clearing the shell using escape sequences 
#define clear() printf("\033[H\033[J")
  
// Greeting shell during startup

void init_shell() {
    clear();
    char* username = getenv("USER");
    
    printf("Hello, @%s\n", username);
    sleep(1);
    clear();
}

char* getDir() {
    char cwd[1024];
    char* dir = malloc(512);
    getcwd(cwd, sizeof(cwd));
    int i = 0, j = 0;
    
    while (i < 512 && cwd[i] != '\0') {
     
        if (cwd[i] == '/')
            
            j = 0;
        
        else
            
            dir[j++] = cwd[i];
        
        i++;
    }
    
    dir[j] = '\0';
    
    return dir;
}
  
// Function to take input 
int takeInput(char* str) {
    char* buf;
    char hostname[1024];
    hostname[1023] = '\0';
    
    gethostname(hostname, 1023);
    char* username = getenv("USER");
    
    printf("\033[0;31m[%s@%s \033[1;31m%s\033[0;31m]\033[0m", username, hostname, getDir());
    
    buf = readline("$ ");
    if (strlen(buf) != 0) {
        add_history(buf);
        strcpy(str, buf);
        return 0;
    } else {
        return 1;
    }
}
  
// Function where the system command is executed 
void execArgs(char** parsed) {
    // Forking a child 
    pid_t pid = fork();

    if (pid == -1) {
        printf("\nFalha ao realizar fork (pid -1)");
        return;
    } else if (pid == 0) {
        if (execvp(parsed[0], parsed) < 0) {
            printf("\nNao foi possivel executar o comando...\n");
        }
        exit(0);
    } else {
        // waiting for child to terminate
        wait(NULL);
        return;
    }
}
  
// Function where the piped system commands is executed 
void execArgsPiped(char** parsed, char** parsedpipe) {
    // 0 is read end, 1 is write end 
    int pipefd[2];
    pid_t p1, p2;
  
    if (pipe(pipefd) < 0) {
        printf("\nFalha ao inicializar pipe");
        return;
    }
    p1 = fork();
    if (p1 < 0) {
        printf("\nFalha ao realizar fork (pid < 0)");
        return;
    }
  
    if (p1 == 0) {
        // Child 1 executing.. 
        // It only needs to write at the write end 
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);
  
        if (execvp(parsed[0], parsed) < 0) {
            printf("\nNao foi possivel executar o comando 1...");
            exit(0);
        }
    } else {
        // Parent executing 
        p2 = fork();
  
        if (p2 < 0) {
            printf("\nFalha ao realizar fork (pid < 0)");
            return;
        }
  
        // Child 2 executing.. 
        // It only needs to read at the read end 
        if (p2 == 0) {
            close(pipefd[1]);
            dup2(pipefd[0], STDIN_FILENO);
            close(pipefd[0]);
            if (execvp(parsedpipe[0], parsedpipe) < 0) {
                printf("\nNao foi possivel executar o comando 2...");
                exit(0);
            }
        } else {
            // parent executing, waiting for two children 
            wait(NULL);
            wait(NULL);
        }
    }
}
  
// Help command builtin 
void openHelp() {
    puts("\nLista de comandos disponiveis:"
        "\n>cd"
        "\n>ls"
        "\n>exit"
        "\n>comandos genericos disponiveis no shell padrao"
        "\n>pipes"
        "\n>tratamento de espacos indevidos");
  
    return;
}
  
// Function to execute builtin commands 
int handler(char** parsed) {
    int n_cmds = 3, i, input = 0;
    char* commands[n_cmds];
  
    commands[0] = "exit";
    commands[1] = "cd";
    commands[2] = "help";
  
    for (i = 0;i < n_cmds;i++) {
        if (strcmp(parsed[0], commands[i]) == 0) {
            input = i + 1;
            break;
        }
    }
  
    switch (input) {
    case 1: 
        printf("\nAdeus ):\n");
        exit(0);
    case 2: 
        chdir(parsed[1]);
        return 1;
    case 3: 
        openHelp();
        return 1;
    default: 
        break;
    }
  
    return 0;
}
  
// function for parsing command words 
void parseSpace(char* str, char** parsed) {
    int i;
  
    for (i = 0;i < MAXLIST;i++) {
        parsed[i] = strsep(&str, " ");
  
        if (parsed[i] == NULL) 
            break;
        if (strlen(parsed[i]) == 0) 
            i--;
    }
}

int parsePipe(char* str, char** strpiped) { 
    int i; 
    for (i = 0; i < 2; i++) { 
        strpiped[i] = strsep(&str, "|"); 
        if (strpiped[i] == NULL) 
            break; 
    } 
  
    if (strpiped[1] == NULL) 
        return 0; // returns zero if no pipe is found. 
    else { 
        return 1; 
    } 
}
  
int processString(char* str, char** parsed, char** parsedpipe) { 
  
    char* strpiped[2]; 
    int piped = 0; 
  
    piped = parsePipe(str, strpiped); 
  
    if (piped) { 
        parseSpace(strpiped[0], parsed); 
        parseSpace(strpiped[1], parsedpipe); 
  
    } else { 
  
        parseSpace(str, parsed); 
    } 
  
    if (handler(parsed)) 
        return 0; 
    else
        return 1 + piped; 
}
  
int main() {
    char inputString[MAXCOM], *parsedArgs[MAXLIST];
    char* parsedArgsPiped[MAXLIST];
    int execFlag = 0;
    init_shell();
  
    while (1) {
        // take input 
        if (takeInput(inputString)) 
            continue;
        // process 
        execFlag = processString(inputString,
        parsedArgs, parsedArgsPiped);
        // execflag returns zero if there is no command 
        // or it is a builtin command, 
        // 1 if it is a simple command 
        // 2 if it is including a pipe. 
  
        // execute 
        if (execFlag == 1)  
            execArgs(parsedArgs);
  
        if (execFlag == 2) 
            execArgsPiped(parsedArgs, parsedArgsPiped);
    } 
    return 0;
}
