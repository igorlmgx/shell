/*
SHELL DESENVOLVIDO EM LINGUAGEM C, 2019
Autores:    Igor Lúcio Manta Guedes RA: 743185
            Karina Mayumi Johansson RA: 758617
*/

#include<stdio.h> 
#include<string.h> 
#include<stdlib.h> 
#include<unistd.h> 
#include<sys/types.h> 
#include<sys/wait.h> 
#include<readline/readline.h> 
#include<readline/history.h> 

#define MAX 100 // número máximo de comandos suportados 
  
#define clear() printf("\033[H\033[J")
  

//inicialização
void init_shell() {
    clear();
    char* username = getenv("USER");
    
    printf("Hello, @%s\n", username);
    sleep(1);
    clear();
}

//retorna diretório atual
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
  
// função para recuperar entrada
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
  
// função onde os comandos de sistema são executados 
void execArgs(char** parsed) {
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
        wait(NULL);
        return;
    }
}
  
// função para executar comandos implementados internamente 
int handler(char** parsed) {
    int n_cmds = 2, i, input = 0;
    char* commands[n_cmds];
  
    commands[0] = "exit";
    commands[1] = "cd";
  
    for (i = 0; i < n_cmds;i++) {
        if (strcmp(parsed[0], commands[i]) == 0) {
            input = i + 1;
            break;
        }
    }
  
    switch (input) {
    case 1:
        printf("Adeus ):\n");
        exit(0);
    case 2:
		i = 2;
		char dir[MAX];
		strcpy(dir, parsed[1]);
		
		while(i < MAX && parsed[i]) {
			strcat(dir, " ");
			strcat(dir, parsed[i]);
			i++;
		}
		
        chdir(dir);
        return 1;
    default:
        break;
    }
  
    return 0;
}
  
// função para analisar e recuperar cada palavra de um comando
void parseSpace(char* str, char** parsed) {
    int i;
  
    for (i = 0;i < MAX;i++) {
        parsed[i] = strsep(&str, " ");
  
        if (parsed[i] == NULL) 
            break;
        if (strlen(parsed[i]) == 0) 
            i--;
    }
}
  
// função que retorna tipo de comando com base nas entradas str, parsed e parsedpipe
int processString(char* str, char** parsed) {
  
    char* strpiped[2]; 
  
	parseSpace(str, parsed); 
  
    if (handler(parsed)) 
        return 0; 
    else
        return 1; 
}
  
int main() {
    char inputString[1000], *parsedArgs[MAX];
    int execFlag = 0;
    init_shell();
  	
    while (1) {
        // recebe entrada
        if (takeInput(inputString)) 
            continue;
        // processa
        execFlag = processString(inputString, parsedArgs);
        // execflag retorna zero se não existe comando ou se é um comando implementado internamente
  
        // executa
        if (execFlag)  
            execArgs(parsedArgs);
    } 
    return 0;
}

