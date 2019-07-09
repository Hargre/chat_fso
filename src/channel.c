#include "channel.h"
#include "types.h"
#include <stdbool.h>
#include <mqueue.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

t_channel *create_channel(char *channel_name, char *channel_owner) {
  char channel_filename[CHANNEL_FILE_LEN];

  struct mq_attr attr;
  attr.mq_maxmsg = MAX_MSG;
  attr.mq_msgsize = MAX_MSG_LEN * sizeof(char);

  mqd_t queue;
  strcpy(channel_filename, CHANNEL_PREFIX);
  strcat(channel_filename, channel_name);

  __mode_t old_umask = umask(0155);
  queue = mq_open(channel_filename, O_RDWR|O_CREAT, 0622, &attr);
  umask(old_umask);

  if (queue < 0) {
    perror("mq_open");
    return NULL;
  }

  t_channel *channel = malloc(sizeof(t_channel));
  strcpy(channel->channel_name, channel_name);
  strcpy(channel->channel_users[0], channel_owner);
  channel->current_users = 1;

  return channel;
}

void *run_thread_channel_receive(void *channel) {
  char channel_filename[CHANNEL_FILE_LEN];

  struct mq_attr attr;
  attr.mq_maxmsg = MAX_MSG;
  attr.mq_msgsize = MAX_MSG_LEN * sizeof(char);

  mqd_t queue;
  strcpy(channel_filename, CHANNEL_PREFIX);
  strcat(channel_filename, ((t_channel *)channel)->channel_name);

  __mode_t old_umask = umask(0155);
  queue = mq_open(channel_filename, O_RDWR|O_CREAT, 0622, &attr);
  umask(old_umask);

  if (queue < 0) {
    perror("mq_open");
    return false;
  }

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

    if (!strcmp(receiver, "JOIN")) {
      join_channel((t_channel *)channel, sender);
    } else {

      if (!is_user_in_channel((t_channel *)channel, sender)) {
        inform_permission_denied((t_channel *)channel, sender);
        continue;
      }

      token = strtok(NULL, ":");
      char message_body[MAX_MSG_LEN];
      memset(message_body, 0, MAX_MSG_LEN);
      strcat(message_body, "<");
      strcat(message_body, sender);
      strcat(message_body, "> ");
      strcat(message_body, token);

      send_message_to_channel_users(channel, message_body);
    }
  }
}

int is_user_in_channel(t_channel *channel, char *user) {
  for (int i = 0; i < channel->current_users; i++) {
    if (!strcmp(channel->channel_users[i], user)) {
      return true;
    }
  }
  return false;
}

void inform_permission_denied(t_channel *channel, char *user) {
  char receiver_queue[CHAT_FILE_LEN];
  strcpy(receiver_queue, CHAT_PREFIX);
  strcat(receiver_queue, user);

  char message[MAX_MSG_LEN];
  strcpy(message, "#");
  strcat(message, channel->channel_name);
  strcat(message, ":");
  strcat(message, user);
  strcat(message, ":");
  strcat(message, "NOT A MEMBER");

  struct mq_attr attr;
  attr.mq_maxmsg = MAX_MSG;
  attr.mq_msgsize = MAX_MSG_LEN * sizeof(char);

  __mode_t old_umask = umask(0155);
  mqd_t oq = mq_open(receiver_queue, O_WRONLY|O_NONBLOCK, 0622, &attr);
  umask(old_umask);

  int status = mq_send(oq, (const char *) message, MAX_MSG_LEN * sizeof(char), 0);
}

void send_message_to_channel_users(t_channel *channel, char *message) {
  for (int i = 0; i < channel->current_users; i++) {
    char receiver_queue[CHAT_FILE_LEN];
    strcpy(receiver_queue, CHAT_PREFIX);
    strcat(receiver_queue, channel->channel_users[i]);

    char full_message[MAX_MSG_LEN];
    strcpy(full_message, "#");
    strcat(full_message, channel->channel_name);
    strcat(full_message, ":");
    strcat(full_message, channel->channel_users[i]);
    strcat(full_message, ":");
    strcat(full_message, message);

    struct mq_attr attr;
    attr.mq_maxmsg = MAX_MSG;
    attr.mq_msgsize = MAX_MSG_LEN * sizeof(char);

    __mode_t old_umask = umask(0155);
    mqd_t oq = mq_open(receiver_queue, O_WRONLY|O_NONBLOCK, 0622, &attr);
    umask(old_umask);

    if (oq < 0) {
      continue;
    }

    int status = mq_send(oq, (const char *) full_message, MAX_MSG_LEN * sizeof(char), 0);
  }
}

void join_channel(t_channel *channel, char *username) {
  strcpy(channel->channel_users[channel->current_users], username);
  channel->current_users += 1;
}
