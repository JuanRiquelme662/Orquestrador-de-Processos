#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>     
#include <sys/types.h>   
#include <sys/wait.h> 
#include <fcntl.h>

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

//funcoes-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
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
    //preenche os campos de input output e modo_append com valores padrao para evitar lixo de memoria
    nova_task->arquivo_input[0] = '\0';
    nova_task->arquivo_output[0] = '\0';
    nova_task->modo_append = 0;
    (*qnt_tasks)++;

    printf("Task '%s' cadastrada.\n", nova_task->nome);
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
    pid_t pid = fork();
    int arq_output;
    int arq_input;

    if (pid == 0) {
        //processo filho
        if(strlen(t->arquivo_input) > 0) {
            arq_input = open(t->arquivo_input, O_RDONLY);
            if(arq_input < 0){
                printf("Erro: arquivo de entrada '%s' não encontrado\n", t->arquivo_input);
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
                printf("Erro: saida de entrada '%s' não encontrado\n", t->arquivo_output);
                exit(1);
            }
            dup2(arq_output, 1);
            close(arq_output);
            
        }
        //execvp pega o primeiro argumento com o nome do programa a ser executado e o segundo, o argumento completo
        execvp(t->args[0], t->args);
        //essas linhas so sao rodades caso a linha anterior nao consiga ser executada, porque execvp ja "mata" o processo filho 
        printf("Erro ao executar task '%s'\n", t->nome);
        exit(1);
    } else if(pid < 0) {
        //pid negativo significa que houve erro na criacao do processo filho
        printf("Erro ao criar processo filho\n");
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
        printf("Erro: task '%s' não encontrada\n", tokens_sep[1]);
        return;
    }
    //executa a task encontrada atraves do endereco encontrado por buscar_task
    aguardar_task(t);
}

void run_sequencial(char *tokens_sep[], task tasks_cadastradas[], int qnt_tokens, int qnt_tasks) {
    for (int i = 2; i < qnt_tokens; i++) {
        task *t = buscar_task(tokens_sep[i], tasks_cadastradas, qnt_tasks);
        if (t == NULL) {
            printf("Erro: task '%s' não encontrada\n", tokens_sep[i]);
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
            printf("Erro: task '%s' não encontrada\n", tokens_sep[i]);
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
//caminho vai pegar o segundo argumento da entrada ou seja tokens_sep[1]
void mudar_workdir(char *caminho) {
    int resultado = chdir(caminho);
    //se o chdir der certo retorna 0 e tudo roda certo, mas se der erro retorna -1 e entra no if
    if (resultado == -1) {
        printf("Erro: diretório '%s' não existe ou não pode ser acessado\n", caminho);
    }
}

void definir_input(char *nome_task, char *arquivo, task tasks_cadastradas[], int qnt_tasks) {
    task *t = buscar_task(nome_task, tasks_cadastradas, qnt_tasks);
    if (t == NULL) {
        printf("Erro: task '%s' não encontrada\n", nome_task);
        return;
    }
    strcpy(t->arquivo_input, arquivo);
}

void definir_output(char *nome_task, char *arquivo, task tasks_cadastradas[], int qnt_tasks){
    task *t = buscar_task(nome_task, tasks_cadastradas, qnt_tasks);
    if (t == NULL) {
        printf("Erro: task '%s' não encontrada\n", nome_task);
        return;
    }
    strcpy(t->arquivo_output, arquivo);
    t->modo_append = 0;
}

void definir_output_append(char *nome_task, char *arquivo, task tasks_cadastradas[], int qnt_tasks){
    task *t = buscar_task(nome_task, tasks_cadastradas, qnt_tasks);
    if (t == NULL) {
        printf("Erro: task '%s' não encontrada\n", nome_task);
        return;
    }
    strcpy(t->arquivo_output, arquivo);
    t->modo_append = 1;
}
//main-----------------------------------------------------------------------------------------------------------------------------------------------
int main(int argc, char *argv[]) {
    //lembrar de aumentar o tamanho das entradas, principalmente quando comecar a mexer com pipe!!!!
    task tasks_cadastradas[100];
    int qnt_tasks = 0;
    char entrada[100];
    char *tokens_sep[100];
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
        } else {
            printf("Erro: comando '%s' não reconhecido\n", tokens_sep[0]);

        }

    }

    if (entrada_padrao != stdin) {
        fclose(entrada_padrao);
    }
    return 0;
}