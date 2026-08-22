#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
        
        for (int i = 0; i < qnt_tokens; i++) {
            printf("Token %d: %s\n", i, tokens_sep[i]);
        }

    }
    return 0;
}