#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void tokenizacao(int *qunt_tokens, char *tokens_sep[], char *entrada) {
    char *token = strtok(entrada, " ");
    *qunt_tokens = 0;

    while(token != NULL){
        tokens_sep[*qunt_tokens] = token;
        (*qunt_tokens)++;
        token = strtok(NULL, " ");
    }

}

void cadastro_taks(){
    if (num_tokens < 3) {
        printf("Erro: uso correto é 'task <nome> <programa> [args...]'\n");
        return;
    }
}
int main(int argc, char *argv[]) {
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