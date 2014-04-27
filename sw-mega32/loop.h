#ifndef LOOP_H
#define LOOP_H

#include <inttypes.h>
#include <stdlib.h>


typedef enum {
    Cmd_Nop,
    Cmd_ButtonPress,
    Cmd_SayTime,            //2
    Cmd_PlayFileName,       //3
    Cmd_PlayFileHandle,     //4
    Cmd_FinishedPlay,       //5
    Cmd_ButtonRelease,
    Cmd_ClockTick,
} command_t;


typedef struct {
    command_t cmd;
    union {
        uint16_t arg1i;
        void* arg1p;
    };
    // uint16_t arg2;
} message_t;


#define MAX_MSG_BUF_LEN         4


typedef struct {
    message_t       msg_buf[MAX_MSG_BUF_LEN];
    int8_t          wrpos;
    int8_t          rdpos;
} message_buffer_t;

extern message_buffer_t g_message_buffer;



void loop_init(void);

message_t *loop_put_msg_begin(void);

void loop_put_msg_end(message_t *msg);

void loop_loop(int8_t once);

#endif
