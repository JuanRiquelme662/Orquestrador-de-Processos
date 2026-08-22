#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>     
#include <sys/types.h>   
#include <sys/wait.h>    

typedef struct task{
    char nome[100];
    char *args[100];
    int qnt_tokens;
}task;

void tokenizacao(int *qunt_tokens, char *tokens_sep[], char *entrada) {
    char *token = strtok(entrada, " ");
    *qunt_tokens = 0;

    while(token != NULL){
        tokens_sep[*qunt_tokens] = token;
        (*qunt_tokens)++;
        token = strtok(NULL, " ");
    }

}

void cadastro_task(int qnt_tokens, char *tokens_sep[], task tasks_cadastradas[], int *qnt_tasks) {
    if (qnt_tokens < 3) {
        printf("Erro: uso correto é 'task <nome> <programa> [args...]'\n");
        return;
    }
    task *nova_task  = &tasks_cadastradas[*qnt_tasks];
    //uso strcpy porque nao tem como fazer nome apontar para tokens_sep porque nome nao e um ponteiro 
    strcpy(nova_task->nome, tokens_sep[1]);
    int j = 0;
    for(int i = 2; i < qnt_tokens; i++){
        //uso str dup porque so fazer args apontar para o mesmo endereco de tokens_sep vai dar problema quando tokens_sep for sobrescrito
        nova_task->args[j] = strdup(tokens_sep[i]);
        j++;
    }
    nova_task->args[j] = NULL;
    nova_task->qnt_tokens = j;
    (*qnt_tasks)++;

    printf("Task '%s' cadastrada.\n", nova_task->nome);
}

void run(char *tokens_sep[], task tasks_cadastradas[], int qnt_tasks) {
    int encontrou = 0;
    for (int i = 0; i < qnt_tasks; i++) {
        if(strcmp(tokens_sep[1], tasks_cadastradas[i].nome) == 0){
            encontrou =1;
            pid_t pid = fork();
            
            if (pid == 0) {
                //processo filho
                //execvp pega o primeiro argumento com o nome do programa a ser executado e o segundo, o argumento completo
                execvp(tasks_cadastradas[i].args[0], tasks_cadastradas[i].args);
                //essas linhas so sao rodades caso a linha anterior nao consiga ser executada, porque execvp ja "mata" o processo filho 
                printf("Erro ao executar task '%s'\n", tasks_cadastradas[i].nome);
                exit(1);
            }else if(pid > 0){
                //processo pai
                wait(NULL);
                
            }else{
                printf("Erro ao criar processo filho\n");
            }

            break;
        }
    }
    if (encontrou == 0) {
        printf("Erro: task '%s' não encontrada\n", tokens_sep[1]);
    }
}
int main(int argc, char *argv[]) {
    task tasks_cadastradas[100];
    int qnt_tasks = 0;
    char entrada[100];
    char *tokens_sep[100];
    int qnt_tokens = 0;

    while(1){
        printf("process_flow>");
        if (fgets(entrada, sizeof(entrada), stdin) == NULL) {
            printf("programa encerrado>");
            break;
        }

        entrada[strcspn(entrada, "\n")] = '\0';
        
        if (strcmp(entrada, "exit") == 0) {
            printf("programa encerrado>");
            break;
        }
        if (strlen(entrada) == 0) {
            continue;
        }

        tokenizacao(&qnt_tokens, tokens_sep, entrada);

        //identificacao do comando digitado pelo usuario
        if(strcmp(tokens_sep[0], "task") == 0){
            cadastro_task(qnt_tokens, tokens_sep, tasks_cadastradas, &qnt_tasks);
        }else if(strcmp(tokens_sep[0], "run") == 0){
            run(tokens_sep, tasks_cadastradas, qnt_tasks);
        }else{
            printf("Erro: comando '%s' não reconhecido\n", tokens_sep[0]);
        }

    }
    return 0;
}