#ifndef __CHANNEL_H__
#define __CHANNEL_H_

#include "types.h"
#include <stdio.h>
#include <mqueue.h>

typedef struct {
  char channel_name[CHANNEL_FILE_LEN];
  char channel_users[MAX_USERS_PER_CHANNEL][MAX_USERNAME_LEN];
  mqd_t channel_queue;
  int current_users;
} t_channel;

t_channel *create_channel(char *channel_name, char *channel_owner);
void join_channel(t_channel *channel, char *username);
void leave_channel(char *channel_name, char *username);
void send_message_to_channel_users(t_channel *channel, char *message);
FILE *open_channel_data(char *channel_name, char *mode);
void *run_thread_channel_receive(void *channel_name);
int is_user_in_channel(t_channel *channel, char *user);
void inform_permission_denied(t_channel *channel, char *user);
#endif