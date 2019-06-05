# Chat FSO - Felipe Hargreaves - 15/0009313

## Trabalho final da disciplina de Fundamentos de Sistemas Operacionais, 2019/1, Universidade de Brasília.

### Compilando e Rodando o Projeto
Para dar a build do projeto, basta digitar `make` na pasta raiz. Em seguida, para rodar o chat, `make run`.

### Funcionalidades Desenvolvidas

Até o presente momento, foram desenvolvidas as seguintes funcionalidades do chat, de acordo com o protocolo:

- Criar fila
  - Configurar permissões (com umask)
  - Validações - não permitir criar a fila se já existir
  - Validações - não permitir criar fila com nome "all"

- Enviar mensagem para um usuário
  - Enviar mensagem no formato DE:PARA:MENSAGEM
  - Usar fila em modo não blockante (O_NONBLOCK)
  - Usar thread separada para envio de mensagem
  - Informar erro no formato UNKNOWNUSER PARA caso o usuário não exista
  - Tentar reenviar a mensagem até três vezes caso algum erro aconteça

- Enviar mensagem broadcast
  - Iterar por todos os usuários ativos e tentar enviar a mensagem para cada um
  - Usar thread separada para envio de broadcast
  - Mesmo mecanismo de reenvio da mensagem para apenas um usuário

- Receber mensagens
  - Usar thread separada apenas para receber mensagens
  - Formatar mensagens para apresentação

- Listar usuários ativos - comando list

- Sair do programa
  - Usar sinal para bloquear saída via Ctrl-C
  - Sair através do comando "sair"

### Problemas conhecidos
 - O sistema não cria threads novas para o reenvio. No caso do broadcast isso pode gerar atraso para enviar a mensagem para todos os usuários.