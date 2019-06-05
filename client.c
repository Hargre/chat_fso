#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <mqueue.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "types.h"

char username[MAX_USERNAME_LEN];

mqd_t register_user(char *queue_name, char *username) {
  printf("Bem-vindo ao chat! Qual seu nome? (max 10 caracteres)\n");
  scanf("%s", username);

  if (!strcmp(username, "all")) {
    printf("Você não pode criar um usuário com esse nome!\n");
    exit(1);
  }

  struct mq_attr attr;
  attr.mq_maxmsg = MAX_MSG;
  attr.mq_msgsize = MAX_MSG_LEN * sizeof(char);

  mqd_t queue;
  strcpy(queue_name, CHAT_PREFIX);
  strcat(queue_name, username);


  DIR *queues_dir;
  struct dirent *ptr_dir;
  queues_dir = opendir("/dev/mqueue");

  if (queues_dir) {
    while ((ptr_dir = readdir(queues_dir)) != NULL) {
      char filename[CHAT_FILE_LEN];

      strcpy(filename, "/");
      strcat(filename, ptr_dir->d_name);

      if (!strcmp(filename, queue_name)) {
          printf("Já existe outro usuário com esse nome!\n");
          exit(1);
      }
    }
  }

  __mode_t old_umask = umask(0155);
  queue = mq_open(queue_name, O_RDWR|O_CREAT, 0622, &attr);
  umask(old_umask);

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
    char sender[MAX_USERNAME_LEN];
    strcpy(sender, token);

    token = strtok(NULL, ":");
    char receiver[MAX_USERNAME_LEN];
    strcpy(receiver, token);

    token = strtok(NULL, ":");
    char message_content[MAX_TEXT_LEN];
    strcpy(message_content, token);

    if (!strcmp(receiver, "all")) {
      strcat(formatted_message, "Broadcast de ");
    }

    strcat(formatted_message, sender);
    strcat(formatted_message, ": ");
    strcat(formatted_message, message_content);

    printf(CYAN_TEXT "%s\n" COLOR_RESET, formatted_message);
    memset(message, 0, MAX_MSG_LEN);
    memset(formatted_message, 0, MAX_MSG_LEN);
  }

}

void retry(mqd_t queue, char *message_body) {
  int retries = 0;

  while (retries < 3) {
    int status = mq_send(queue, (const char *) message_body, MAX_MSG_LEN * sizeof(char), 0);
    if (status < 0) {
      retries += 1;
      sleep(2);
    } else {
      return;
    }
  }
  printf("ERRO %s\n", message_body);
}

void *run_thread_broadcast(void *message_body) {
  struct mq_attr attr;
  attr.mq_maxmsg = MAX_MSG;
  attr.mq_msgsize = MAX_MSG_LEN * sizeof(char);

  char message[MAX_MSG_LEN];
  strcpy(message, (char *) message_body);

  DIR *queues_dir;
  struct dirent *ptr_dir;

  char receiver_queue[CHAT_FILE_LEN];

  queues_dir = opendir("/dev/mqueue");

  if (queues_dir) {
    while ((ptr_dir = readdir(queues_dir)) != NULL) {
      memset(receiver_queue, 0, CHAT_FILE_LEN);
      if (!strncmp(ptr_dir->d_name, "chat-", 5)) {
        strcpy(receiver_queue, "/");
        strcat(receiver_queue, ptr_dir->d_name);

        __mode_t old_umask = umask(0155);
        mqd_t oq = mq_open(receiver_queue, O_WRONLY|O_NONBLOCK, 0622, &attr);
        umask(old_umask);

        if (oq < 0) {
          continue;
        }

        int status = mq_send(oq, (const char *) message_body, MAX_MSG_LEN * sizeof(char), 0);
        int retries = 0;

        if (status < 0) {
          retry(oq, message_body);
        }
      }
    }
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

  if (!strcmp(receiver_name, "all")) {
    pthread_t broadcast;
    pthread_create(&broadcast, NULL, run_thread_broadcast, message_body);
  } else {
    strcpy(receiver_queue, "/chat-");
    strcat(receiver_queue, receiver_name);

    __mode_t old_umask = umask(0155);
    mqd_t oq = mq_open(receiver_queue, O_WRONLY|O_NONBLOCK, 0622, &attr);
    umask(old_umask);

    if (oq < 0) {
      if (errno == ENOENT) {
        printf(RED_TEXT "UNKNOWNUSER %s\n" COLOR_RESET, receiver_name);
      }
      pthread_exit((void *) 1);
    }

    int status = mq_send(oq, (const char *) message_body, MAX_MSG_LEN * sizeof(char), 0);
    int retries = 0;

    if (status < 0) {
      retry(oq, message_body);
    }
  }
}

void list_users() {
  DIR *queues_dir;
  struct dirent *ptr_dir;

  queues_dir = opendir("/dev/mqueue");

  if (queues_dir) {
    printf("Usuários disponíveis:\n");
    while ((ptr_dir = readdir(queues_dir)) != NULL) {
      if (!strncmp(ptr_dir->d_name, "chat-", 5)) {
        char queue_name[CHAT_FILE_LEN];
        strcpy(queue_name, ptr_dir->d_name);

        char *token = strtok(queue_name, "-");
        token = strtok(NULL, "-");

        printf(GREEN_TEXT "- %s\n" COLOR_RESET, token);
      }
    }
  }
}

void exit_handler(int signum) {
  printf("Para sair do programa, digite 'sair'\n");
}

int main(int argc, char const *argv[]) {
  signal(SIGINT, exit_handler);

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

    if (strcmp(message_body, "list\n") == 0) {
      list_users();
      continue;
    } else if (strcmp(message_body, "sair\n") == 0) {
      mq_unlink(queue_name);
      return 0;
    } else if (strncmp(message_body, username, strlen(username)) == 0) {
      char message_body_thread[MAX_MSG_LEN];
      strcpy(message_body_thread, message_body);

      pthread_t sender;
      pthread_create(&sender, NULL, run_thread_send, message_body_thread);
    } else {
      printf("Comando inválido\n");
    }

  }

  return 0;
}
