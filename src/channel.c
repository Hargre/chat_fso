#include "channel.h"
#include "types.h"
#include <stdbool.h>
#include <mqueue.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

int create_channel(char *channel_name, char *channel_owner) {
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
    return false;
  }

  char channel_users_path[100];
  strcpy(channel_users_path, "/tmp");
  strcat(channel_users_path, channel_filename);

  FILE *channel_users = fopen(channel_users_path, "wb");

  if (channel_users != NULL) {
    fwrite(channel_owner, sizeof(char), MAX_USERNAME_LEN, channel_users);
  } else {
    mq_unlink(channel_filename);
    return false;
  }

  mq_close(queue);
  fclose(channel_users);

  return true;
}