ProcessFlow

Orquestrador de processos em C. Cadastra tarefas (programas do sistema) e as executa como processos filhos, com suporte a execução sequencial, paralela, em pipe, redirecionamento de entrada/saída, diretório de trabalho e execução em background com controle de jobs.

Arquivos do projeto
Arquivo	Responsabilidade
main.c	Loop principal: modo interativo (processflow>) e modo workflow (arquivo .pf), leitura e roteamento de comandos
funcoes.c	Implementação de todas as funções: tokenização, cadastro de tasks, execução (simples/sequencial/paralela/pipe), redirecionamento de entrada/saída, workdir, jobs em background
funcoes.h	Declaração das structs task e job, e protótipos de todas as funções usadas em main.c e funcoes.c
Makefile	Compilação, limpeza e execução do projeto
Sistema operacional

Desenvolvido e testado em Windows via WSL (Windows Subsystem for Linux), com Ubuntu, usando gcc.

Como compilar
bash
make

Isso gera o executável processflow a partir de main.c e funcoes.c.

Se preferir compilar manualmente, sem o Makefile:

bash
gcc -Wall -Wextra -o processflow main.c funcoes.c

Atenção: os dois arquivos .c precisam entrar no mesmo comando de compilação — compilar só main.c gera erro de undefined reference, pois as funções chamadas no main.c estão implementadas em funcoes.c.

Como executar

Modo interativo (apresenta o prompt processflow>):

bash
./processflow

Modo workflow (lê e executa comandos de um arquivo .pf, imprimindo cada linha antes de processá-la):

bash
./processflow arquivo.pf

Em ambos os modos, o programa é encerrado com o comando exit ou com CTRL-D.

Comandos suportados
Comando	Descrição
task <nome> <programa> [args...]	Cadastra uma tarefa
run <nome>	Executa uma tarefa cadastrada, aguardando seu término
run sequential <t1> <t2> ...	Executa várias tarefas em sequência, uma após o término da anterior
run parallel <t1> <t2> ...	Inicia várias tarefas ao mesmo tempo e aguarda todas terminarem
run pipe <t1> <t2> ...	Encadeia a saída de uma tarefa como entrada da próxima, cada uma em um processo
input <task> <arquivo>	Define um arquivo como entrada padrão da tarefa
output <task> <arquivo>	Define um arquivo como saída padrão da tarefa (sobrescreve o arquivo)
append <task> <arquivo>	Define um arquivo como saída padrão da tarefa (acrescenta ao final)
workdir <diretório>	Altera o diretório de trabalho usado pelas tarefas seguintes
start <task>	Inicia uma tarefa em background, retorna o prompt imediatamente e informa [jobId] pid
jobs	Lista os jobs iniciados em background
wait <jobId>	Aguarda o término de um job específico
exit	Encerra o programa
Como testar

Compile o projeto e crie um arquivo de workflow, por exemplo:

bash
cat > teste.pf << 'EOF'
task listar /bin/ls -l
task ordenar /usr/bin/sort
task contar /usr/bin/wc -l
run pipe listar ordenar contar
exit
EOF
./processflow teste.pf

Ou use o alvo do Makefile (ajuste o nome do arquivo conforme necessário):

bash
make test
Limpeza
bash
make clean

Remove o executável e os arquivos .txt gerados por testes anteriores.