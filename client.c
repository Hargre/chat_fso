#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mqueue.h>
#include <unistd.h>
#include "types.h"

mqd_t register_user(char *queue_name, char *username) {
  printf("Bem-vindo ao chat! Qual seu nome? (max 10 caracteres)\n");
  scanf("%s", username);
  
  struct mq_attr attr;
  attr.mq_maxmsg = MAX_MSG;
  attr.mq_msgsize = MAX_MSG_LEN * sizeof(char);

  mqd_t queue;
  strcpy(queue_name, CHAT_PREFIX);
  strcat(queue_name, username);

  queue = mq_open(queue_name, O_RDWR|O_CREAT, 0622, &attr);

  if (queue < 0) {
    perror("mq_open");
    exit(1);
  }

  return queue;
}

int main(int argc, char const *argv[]) {
  char queue_name[CHAT_FILE_LEN];
  char username[MAX_USERNAME_LEN];
  mqd_t queue = register_user(queue_name, username);

  printf("Fila %s criada para user %s\n", queue_name, username);

  mq_unlink(queue_name);
  return 0;
}
