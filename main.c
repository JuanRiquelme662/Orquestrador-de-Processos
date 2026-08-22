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

    if (pid == 0) {
        //processo filho
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
        printf("Erro ao mudar diretório de trabalho\n");
    }
}


//main-----------------------------------------------------------------------------------------------------------------------------------------------
int main(int argc, char *argv[]) {
    //lembrar de aumentar o tamanho das entradas, principalmente quando comecar a mexer com pipe!!!!
    task tasks_cadastradas[100];
    int qnt_tasks = 0;
    char entrada[100];
    char *tokens_sep[100];
    int qnt_tokens = 0;

    while(1){
        printf("processflow>");
        fflush(stdout);
        if (fgets(entrada, sizeof(entrada), stdin) == NULL) {
            printf("programa encerrado>\n");
            break;
        }

        entrada[strcspn(entrada, "\n")] = '\0';
        
        if (strcmp(entrada, "exit") == 0) {
            printf("programa encerrado>\n");
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
        }else if(strcmp(tokens_sep[1], "workdir") == 0){
            if(qnt_tokens < 2) {
                printf("Erro: comando 'workdir' precisa de um argumento\n");
                continue;

            }
            mudar_workdir(tokens_sep[1]);
        }else{
            printf("Erro: comando '%s' não reconhecido\n", tokens_sep[0]);

        }

    }
    return 0;
}