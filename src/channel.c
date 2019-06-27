#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mqueue.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "types.h"


t_channel *create_channel();
void join_channel(t_channel *channel, char *user);
void leave_channel(t_channel *channel, char *user);

t_channel *create_channel(char *username) {
  char queue_name[CHANNEL_FILE_LEN];
  char buffer[MAX_USERNAME_LEN];
  printf("Qual o nome do canal?\n");
  fgets(buffer, MAX_USERNAME_LEN, stdin);

  mqd_t queue;
  strcpy(queue_name, CHAT_PREFIX);
  strcat(queue_name, queue_name);

  struct mq_attr attr;
  attr.mq_maxmsg = MAX_MSG;
  attr.mq_msgsize = MAX_MSG_LEN * sizeof(char);

  __mode_t old_umask = umask(0155);
  queue = mq_open(queue_name, O_RDWR|O_CREAT, 0622, &attr);
  umask(old_umask);

  if (queue < 0) {
    perror("mq_open");
    exit(1);
  }

  t_channel *channel = malloc(sizeof(t_channel));
  strcpy(channel->channel_name, queue_name);
  strcpy(channel->channel_users[0], username);
  channel->channel_queue = queue;
  channel->current_users = 1;

  return channel;
}

void join_channel(t_channel *channel, char *user) {
  if (channel->current_users < MAX_USERS_PER_CHANNEL) {
    strcpy(channel->channel_users[channel->current_users], user);
    channel->current_users += 1;
  }
}

void leave_channel(t_channel *channel, char *user) {
  if (channel->current_users == 1 && !strcmp(user, channel->channel_users[0])) {
    strcpy(channel->channel_users[0], "\0");
    channel->current_users = 0;
    return;
  }

  for (int i = 0; i < channel->current_users; i++) {
    if (!strcmp(user, channel->channel_users[i])) {
      strcpy(channel->channel_users[i], channel->channel_users[channel->current_users - 1]);
      strcpy(channel->channel_users[channel->current_users - 1], "\0");
      channel->current_users -= 1;
      return;
    }
  }
}

void send_message_to_members(t_channel *channel, char *message) {
  for (int i = 0; i < channel->current_users; i++) {
    char message_to_member[MAX_MSG_LEN];
    strcpy(message_to_member, "#");
    strcat(message_to_member, channel->channel_name);
    strcat(message_to_member, ":");
    strcat(message_to_member, channel->channel_users[i]);
    strcat(message_to_member, ":");
    strcat(message_to_member, message);
  }
}