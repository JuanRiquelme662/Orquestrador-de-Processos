#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>     
#include <sys/types.h>   
#include <sys/wait.h> 
#include <fcntl.h>
#include "funcoes.h"
 
 
void tokenizacao(int *qnt_tokens, char *tokens_sep[], char *entrada) {
    char *token = strtok(entrada, " ");
    *qnt_tokens = 0;
 
    while(token != NULL){
        tokens_sep[*qnt_tokens] = token;
        (*qnt_tokens)++;
        //eu passo NULL para que ela continue a partir do ponto onde parou na string original
        token = strtok(NULL, " ");
    }
 
}
 
void cadastro_task(int qnt_tokens, char *tokens_sep[], task tasks_cadastradas[], int *qnt_tasks, int modo_interativo) {
    if (qnt_tokens < 3) {
        fprintf(stderr, "Erro: uso correto é 'task <nome> <programa> [args...]'\n");
        return;
    }
    if (buscar_task(tokens_sep[1], tasks_cadastradas, *qnt_tasks) != NULL) {
        fprintf(stderr, "Erro: já existe uma task com o nome '%s'\n", tokens_sep[1]);
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
    //preenche os campos de input output e modo_append com valores padrao para evitar lixo de memoria
    nova_task->arquivo_input[0] = '\0';
    nova_task->arquivo_output[0] = '\0';
    nova_task->modo_append = 0;
    (*qnt_tasks)++;

    //so mostra a confirmacao no modo interativo (stdin) em modo arquivo o gabarito nao espera essa linha
    if (modo_interativo) {
        fprintf(stderr, "Task '%s' cadastrada.\n", nova_task->nome);
    }
}
 
task* buscar_task(char *nome, task tasks_cadastradas[], int qnt_tasks) {
    for (int i = 0; i < qnt_tasks; i++) {
        if (strcmp(nome, tasks_cadastradas[i].nome) == 0) {
            return &tasks_cadastradas[i];
        }
    }
    return NULL;
}
 
//apenas inicia a task nao faz o pai esperar pelo filho
pid_t iniciar_task(task *t) {
    fflush(stdout);
    pid_t pid = fork();
    int arq_output;
    int arq_input;
 
    if (pid == 0) {
        //processo filho
        if(strlen(t->arquivo_input) > 0) {
            arq_input = open(t->arquivo_input, O_RDONLY);
            if(arq_input < 0){
                fprintf(stderr, "Erro: arquivo de entrada '%s' não encontrado\n", t->arquivo_input);
                exit(1);
            }
            dup2(arq_input, 0);
            close(arq_input);
        }
        if(strlen(t->arquivo_output) > 0) {
            if(t->modo_append) {
                 //o 0664 indica que o dono do arquivo pode ler e escrever nele, todo mundo mais só pode ler, ninguém mais pode modificar
                arq_output = open(t->arquivo_output, O_WRONLY | O_CREAT | O_APPEND, 0644);
            }else{
                arq_output = open(t->arquivo_output, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            }
            if(arq_output < 0){
                fprintf(stderr, "Erro: não foi possível abrir o arquivo de '%s' de saída \n", t->arquivo_output);
                exit(1);
            }
            dup2(arq_output, 1);
            close(arq_output);
            
        }
        //execvp pega o primeiro argumento com o nome do programa a ser executado e o segundo, o argumento completo
        execvp(t->args[0], t->args);
        //essas linhas so sao rodades caso a linha anterior nao consiga ser executada, porque execvp ja "mata" o processo filho 
        fprintf(stderr, "Erro ao executar task '%s'\n", t->nome);
        exit(1);
    } else if(pid < 0) {
        //pid negativo significa que houve erro na criacao do processo filho
        fprintf(stderr, "Erro ao criar processo filho\n");
    }
    return pid;
}
 
//executa a task mas faz o pai esperar pelo filho
void aguardar_task(task *t) {
    pid_t pid = iniciar_task(t);
    if (pid > 0) {
        //campo para o processo pai esperar o processo filho 
        wait(NULL);
    }
}
 
void run(char *tokens_sep[], task tasks_cadastradas[], int qnt_tasks) {
    //chama a fucnao buscar_task e retorna a task encontrada para um ponteiro so tipo task
    task *t = buscar_task(tokens_sep[1], tasks_cadastradas, qnt_tasks);
 
    if (t == NULL) {
        fprintf(stderr, "Erro: task '%s' não encontrada\n", tokens_sep[1]);
        return;
    }
    //executa a task encontrada atraves do endereco encontrado por buscar_task
    aguardar_task(t);
}
 
void run_sequencial(char *tokens_sep[], task tasks_cadastradas[], int qnt_tokens, int qnt_tasks) {
    for (int i = 2; i < qnt_tokens; i++) {
        task *t = buscar_task(tokens_sep[i], tasks_cadastradas, qnt_tasks);
        if (t == NULL) {
            fprintf(stderr, "Erro: task '%s' não encontrada\n", tokens_sep[i]);
            continue;
        }
        aguardar_task(t);
    }
}
 
void run_paralelo(char *tokens_sep[], task tasks_cadastradas[], int qnt_tokens, int qnt_tasks){
    //guarda os pids dos processos iniciados
    pid_t pids[50];
    //numero de processos iniciados
    int num_pids = 0;
 
    for (int i = 2; i < qnt_tokens; i++) {
        task *t = buscar_task(tokens_sep[i], tasks_cadastradas, qnt_tasks);
        if (t == NULL) {
            fprintf(stderr, "Erro: task '%s' não encontrada\n", tokens_sep[i]);
            continue;
        }
        //executa direto ao inves de chamar a funcao aguardar_task, e guarda o pid do processo filho
        pids[num_pids] = iniciar_task(t);
        num_pids++;
    }
    //espera todos os processos filhos terminarem
    for (int i = 0; i < num_pids; i++){
        //pode receber um PID negativo, e um comportamento raro, mas vale registrar
        waitpid(pids[i], NULL, 0);
    }
 
}
 
void run_pipe(char *tokens_sep[], task tasks_cadastradas[], int qnt_tokens, int qnt_tasks) {
    int qnt_pipes = qnt_tokens - 3;// -3 porque a quantidade de taks e tokens-2 por conta do comando e do pipe, e a quantidade de pipes e igual a qnt_tasks-1
    int pipes[50][2]; //armazena os pipes criados
    int tasks_pipe = qnt_pipes + 1;
    pid_t pids[50];
    int num_pids = 0;
 
    //verifica se as operacoes presentes no pipe existem
    for (int i = 0; i < tasks_pipe; i++) {
        char *nome_task = tokens_sep[i + 2];
        if (buscar_task(nome_task, tasks_cadastradas, qnt_tasks) == NULL) {
            fprintf(stderr, "Erro: task '%s' não encontrada. Pipe cancelado.\n", nome_task);
            return;  // ccancela a operacao
        }
    }
 
    for(int i = 0; i < qnt_pipes; i++){
        if(pipe(pipes[i])== -1){
            fprintf(stderr, "Erro ao criar pipe\n");
            return;
        }
    }
 
    //o for vai:
    // 1. buscar a task pelo nome
    // 2. checar se a task foi encontrada e criar um processo filho
    // 3. redirecionar a entrada e saida do processo filho para os pipes corretos
    for(int i = 0; i < tasks_pipe; i++){
        char *nome_task = tokens_sep[i + 2];
        task *t = buscar_task(nome_task, tasks_cadastradas, qnt_tasks); 
        if (t == NULL) {
            fprintf(stderr, "Erro: task '%s' não encontrada\n", nome_task);
            continue;
        }
        fflush(stdout);
        pid_t pid = fork();
        if(pid == 0){
            //onde o proceso filho entra
            if(i > 0) {
                //checa se nao e a primeira
                //le da pipe anterior
                dup2(pipes[i - 1][0], 0);
            }
            if(i < qnt_pipes) {
                //checa se nao e a ultima
                //escreve na pipe atual
                dup2(pipes[i][1], 1);
            }
            //fechar todos pipes
            for(int j = 0; j < qnt_pipes; j++){
                close(pipes[j][0]);
                close(pipes[j][1]);
            }
            execvp(t->args[0], t->args);
            fprintf(stderr, "Erro ao executar task '%s'\n", t->nome);
            exit(1);
        }
        //crio os pids para depois conseguir esperar por eles
        pids[num_pids] = pid;
        num_pids++;
    }
    for(int j = 0; j < qnt_pipes; j++){
        close(pipes[j][0]);
        close(pipes[j][1]);
    }
     for(int k = 0; k < num_pids; k++){
        waitpid(pids[k], NULL, 0);
    }
}
 
//caminho vai pegar o segundo argumento da entrada ou seja tokens_sep[1]
void mudar_workdir(char *caminho) {
    int resultado = chdir(caminho);
    //se o chdir der certo retorna 0 e tudo roda certo, mas se der erro retorna -1 e entra no if
    if (resultado == -1) {
        fprintf(stderr, "Erro: diretório '%s' não existe ou não pode ser acessado\n", caminho);
    }
}
 
void definir_input(char *nome_task, char *arquivo, task tasks_cadastradas[], int qnt_tasks) {
    task *t = buscar_task(nome_task, tasks_cadastradas, qnt_tasks);
    if (t == NULL) {
        fprintf(stderr, "Erro: task '%s' não encontrada\n", nome_task);
        return;
    }
    strcpy(t->arquivo_input, arquivo);
}
 
void definir_output(char *nome_task, char *arquivo, task tasks_cadastradas[], int qnt_tasks){
    task *t = buscar_task(nome_task, tasks_cadastradas, qnt_tasks);
    if (t == NULL) {
        fprintf(stderr, "Erro: task '%s' não encontrada\n", nome_task);
        return;
    }
    strcpy(t->arquivo_output, arquivo);
    t->modo_append = 0;
}
 
void definir_output_append(char *nome_task, char *arquivo, task tasks_cadastradas[], int qnt_tasks){
    task *t = buscar_task(nome_task, tasks_cadastradas, qnt_tasks);
    if (t == NULL) {
        fprintf(stderr, "Erro: task '%s' não encontrada\n", nome_task);
        return;
    }
    strcpy(t->arquivo_output, arquivo);
    t->modo_append = 1;
}
 
//qnt_jobs e equivalente a job_id
void esperar_job(int id, job jobs_ativos[], int qnt_jobs) {
    for (int i = 0; i < qnt_jobs; i++) {
        if (jobs_ativos[i].job_id == id) {
            waitpid(jobs_ativos[i].pid, NULL, 0);
            return;
        }
    }
    fprintf(stderr, "Erro: job com ID %d não encontrado\n", id);
    
}