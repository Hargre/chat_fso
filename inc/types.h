#ifndef __CHAT_TYPES_H__
#define __CHAT_TYPES_H__

#define MAX_USERNAME_LEN 10
#define CHAT_FILE_LEN 17 // 6 from chat- prefix + 10 from username len + extra.
#define CHANNEL_FILE_LEN 18 // 7 from canal- prefix + 10 from username len + extra.
#define MAX_TEXT_LEN 500
#define MAX_MSG_LEN 522
#define MAX_MSG 10
#define MAX_USERS_PER_CHANNEL 10
#define CHAT_PREFIX "/chat-"
#define CHANNEL_PREFIX "/canal-"

#define COLOR_RESET  "\033[0m"
#define BOLD         "\033[1m"
#define BLACK_TEXT   "\033[30;1m"
#define RED_TEXT     "\033[31;1m"
#define GREEN_TEXT   "\033[32;1m"
#define YELLOW_TEXT  "\033[33;1m"
#define BLUE_TEXT    "\033[34;1m"
#define MAGENTA_TEXT "\033[35;1m"
#define CYAN_TEXT    "\033[36;1m"
#define WHITE_TEXT   "\033[37;1m"

#endif