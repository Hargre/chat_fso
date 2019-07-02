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
  strcpy(channel->channel_name, channel_filename);
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
  strcpy(channel_filename, ((t_channel *)channel)->channel_name);

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
    }
  }
}

void join_channel(t_channel *channel, char *username) {
  strcpy(channel->channel_users[channel->current_users], username);
  channel->current_users += 1;
}
