#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mqueue.h>
#include <pthread.h>
#include <unistd.h>
#include "types.h"

char username[MAX_USERNAME_LEN];

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

void *run_thread_receive(void *queue) {
  char message[MAX_MSG_LEN];

  while (1) {
    int status = mq_receive((mqd_t) queue, (char *) message, sizeof(message), 0);

    if (status < 0) {
      perror("error receiving");
      exit(1);
    }

    char formatted_message[MAX_MSG_LEN];
    char *token = strtok(message, ":");

    strcat(formatted_message, token);
    strcat(formatted_message, ": ");

    for (int i = 0; i < 2; i++) {
      token = strtok(NULL, ":");
    }

    strcat(formatted_message, token);

    printf("%s\n", formatted_message);
    memset(message, 0, MAX_MSG_LEN);
    memset(formatted_message, 0, MAX_MSG_LEN);
  }

}

void *run_thread_send(void *message_body) {
  struct mq_attr attr;
  attr.mq_maxmsg = MAX_MSG;
  attr.mq_msgsize = MAX_MSG_LEN * sizeof(char);

  char message[MAX_MSG_LEN];
  strcpy(message, (char *) message_body);

  char receiver_queue[CHAT_FILE_LEN];
  char receiver_name[MAX_USERNAME_LEN];

  int j = 0;

  for (int i = strlen(username) + 1; i < strlen(message); i++) {
    if (message[i] == ':') break;

    receiver_name[j] = message[i];
    j++;
  }

  strcpy(receiver_queue, "/chat-");
  strcat(receiver_queue, receiver_name);

  mqd_t oq = mq_open(receiver_queue, O_WRONLY, 0622, &attr);

  if (oq < 0) {
    perror("open");
    exit(1);
  }

  int status = mq_send(oq, (const char *) message_body, MAX_MSG_LEN * sizeof(char), 0);

  if (status < 0) {
    perror("send");
    exit(1);
  }
}

int main(int argc, char const *argv[]) {
  char queue_name[CHAT_FILE_LEN];

  char message_body[MAX_MSG_LEN];
  mqd_t queue = register_user(queue_name, username);
  getchar();

  printf("Fila %s criada para user %s\n", queue_name, username);

  pthread_t receiver;
  pthread_create(&receiver, NULL, run_thread_receive, (void *)queue);

  printf("Envie mensagens no formato de:para:mensagem\n");

  while (1) {
    memset(message_body, 0, MAX_MSG_LEN);
    fgets(message_body, MAX_MSG_LEN, stdin);
    
    char message_body_thread[MAX_MSG_LEN];
    strcpy(message_body_thread, message_body);

    pthread_t sender;
    pthread_create(&sender, NULL, run_thread_send, message_body_thread);
  }

  pthread_join(receiver, NULL);
  mq_unlink(queue_name);
  return 0;
}
