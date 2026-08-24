#ifndef FUNCOES_H
#define FUNCOES_H

#include <sys/types.h>
 
typedef struct task{
    char nome[100];
    char *args[100];
    int qnt_tokens;
    //guarda o aquivo que vai ser lido na entrada
    char arquivo_input[100];
    //guarda o arquivo que vai ser escrito na saida
    char arquivo_output[100];
    int modo_append;
}task;
 
typedef struct job {
    int job_id;
    pid_t pid;
    char nome_task[100];
} job;
 
void tokenizacao(int *qnt_tokens, char *tokens_sep[], char *entrada);
void cadastro_task(int qnt_tokens, char *tokens_sep[], task tasks_cadastradas[], int *qnt_tasks);
task* buscar_task(char *nome, task tasks_cadastradas[], int qnt_tasks);
pid_t iniciar_task(task *t);
void aguardar_task(task *t);
void run(char *tokens_sep[], task tasks_cadastradas[], int qnt_tasks);
void run_sequencial(char *tokens_sep[], task tasks_cadastradas[], int qnt_tokens, int qnt_tasks);
void run_paralelo(char *tokens_sep[], task tasks_cadastradas[], int qnt_tokens, int qnt_tasks);
void run_pipe(char *tokens_sep[], task tasks_cadastradas[], int qnt_tokens, int qnt_tasks);
void mudar_workdir(char *caminho);
void definir_input(char *nome_task, char *arquivo, task tasks_cadastradas[], int qnt_tasks);
void definir_output(char *nome_task, char *arquivo, task tasks_cadastradas[], int qnt_tasks);
void definir_output_append(char *nome_task, char *arquivo, task tasks_cadastradas[], int qnt_tasks);
void esperar_job(int id, job jobs_ativos[], int qnt_jobs);
 
#endif
 