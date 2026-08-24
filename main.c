#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>     
#include <sys/types.h>   
#include <sys/wait.h> 
#include <fcntl.h>
#include "funcoes.h"

//main-----------------------------------------------------------------------------------------------------------------------------------------------
int main(int argc, char *argv[]) {
    task tasks_cadastradas[100];
    int qnt_tasks = 0;
    char entrada[1024];
    char *tokens_sep[1024];
    int qnt_tokens = 0;
    FILE *entrada_padrao = stdin;
    int job_id = 0;
    job jobs_ativos[100];

    if(argc > 1){
        //usa argv[1] porque o arg[0] e o nome do programa, e argv[1] e o primeiro argumento passado pelo usuario
        entrada_padrao = fopen(argv[1],"r");
        if(entrada_padrao == NULL){
            printf("Erro fatal!: arquivo '%s' não encontrado\n", argv[1]);
            exit(1);
        }
    }
    if (argc > 2){
        printf("Erro fatal!: numero de argumentos invalido\n");
        exit(1);
    }

    while(1){
        if(entrada_padrao == stdin){
            printf("processflow>");
            //faz o prompt ser exibido antes da entrada do usuario
            fflush(stdout);
        }
            
        if (fgets(entrada, sizeof(entrada), entrada_padrao) == NULL) {
            printf("programa encerrado>\n");
            break;
        }

        entrada[strcspn(entrada, "\n")] = '\0';
        if (strchr(entrada, '"') != NULL || strchr(entrada, '\'') != NULL) {
            printf("Aviso: aspas não são suportadas; os argumentos serão separados apenas por espaço\n");
        }

        if (entrada_padrao != stdin) {
            printf("%s\n", entrada);
        }

        if (strcmp(entrada, "exit") == 0) {
            printf("programa encerrado>\n");
            break;
        }
        //ve se a entrada esta vazia via strlen
        if (strlen(entrada) == 0) {
            continue;
        }

        tokenizacao(&qnt_tokens, tokens_sep, entrada);

        //identificacao do comando digitado pelo usuario
        if(strcmp(tokens_sep[0], "task") == 0){
            cadastro_task(qnt_tokens, tokens_sep, tasks_cadastradas, &qnt_tasks);

        }else if(strcmp(tokens_sep[0], "run") == 0){
            if(qnt_tokens < 2) {
                printf("Erro: comando 'run' precisa de um argumento\n");
                //volta para o loop mais proximo caso a pessoa nao digite o comando corretamente
                continue;

            }
            if(strcmp(tokens_sep[1], "sequential") == 0){
                run_sequencial(tokens_sep, tasks_cadastradas, qnt_tokens, qnt_tasks);

            }else if(strcmp(tokens_sep[1], "parallel") == 0){
                run_paralelo(tokens_sep, tasks_cadastradas, qnt_tokens, qnt_tasks);

            }else if(strcmp(tokens_sep[1], "pipe") == 0){
                run_pipe(tokens_sep, tasks_cadastradas, qnt_tokens, qnt_tasks);
            }else{
                run(tokens_sep, tasks_cadastradas, qnt_tasks);

            }
        }else if(strcmp(tokens_sep[0], "workdir") == 0){
            if(qnt_tokens < 2) {
                printf("Erro: comando 'workdir' precisa de um argumento\n");
                continue;
            }
            mudar_workdir(tokens_sep[1]);

        } else if (strcmp(tokens_sep[0], "input") == 0) {
            if (qnt_tokens < 3) {
                printf("Erro: uso correto é 'input <task> <arquivo>'\n");
                continue;
            }
            definir_input(tokens_sep[1], tokens_sep[2], tasks_cadastradas, qnt_tasks);

        } else if (strcmp(tokens_sep[0], "output") == 0) {
            if (qnt_tokens < 3) {
                printf("Erro: uso correto é 'output <task> <arquivo>'\n");
                continue;
            }
            definir_output(tokens_sep[1], tokens_sep[2], tasks_cadastradas, qnt_tasks);

        } else if (strcmp(tokens_sep[0], "append") == 0) {
            if (qnt_tokens < 3) {
                printf("Erro: uso correto é 'append <task> <arquivo>'\n");
                continue;
            }
            definir_output_append(tokens_sep[1], tokens_sep[2], tasks_cadastradas, qnt_tasks);

        } else if(strcmp(tokens_sep[0], "start") == 0){
            if(qnt_tokens < 2) {
                printf("Erro: uso correto é 'start <task>'\n");
                continue;
            }
                task *t = buscar_task(tokens_sep[1], tasks_cadastradas, qnt_tasks);
                if (t == NULL) {
                    printf("Erro: task '%s' não encontrada\n", tokens_sep[1]);
                    continue;
                }
                jobs_ativos[job_id].job_id = job_id + 1;
                jobs_ativos[job_id].pid = iniciar_task(t);
                strcpy(jobs_ativos[job_id].nome_task, tokens_sep[1]);
                printf("[%d] %d\n", job_id + 1, jobs_ativos[job_id].pid);
                job_id++;
            
        } else if(strcmp(tokens_sep[0], "jobs") == 0){
             for (int i = 0; i < job_id; i++) {
                printf("[%d] %d %s\n", jobs_ativos[i].job_id, jobs_ativos[i].pid, jobs_ativos[i].nome_task);
            }
        }  else if (strcmp(tokens_sep[0], "wait") == 0) {
            if (qnt_tokens < 2) {
                printf("Erro: uso correto é 'wait <jobId>'\n");
                continue;
            }
            int id_procurado = atoi(tokens_sep[1]);
            esperar_job(id_procurado, jobs_ativos, job_id);
        } else {
            printf("Erro: comando '%s' não reconhecido\n", tokens_sep[0]);

        }
    }
    for (int i = 0; i < job_id; i++) {
        waitpid(jobs_ativos[i].pid, NULL, 0);
    }
    if (entrada_padrao != stdin) {
        fclose(entrada_padrao);
    }
    return 0;
}