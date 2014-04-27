#ifndef IR_H
#define IR_H

#include "loop.h"

typedef struct {
    uint32_t cmdcode;
    uint8_t cmdlen;
    //
    message_t genmsg;
} ir_code_t;


void ir_init(void);

int8_t ir_process(const message_t *msg);


#endif
