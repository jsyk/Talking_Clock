#ifndef VOICE_H
#define VOICE_H

#include "loop.h"

#define MAX_SENTENCE_WORDS      16

/* Enum of Word types */
typedef enum {
    Word_NONE,
    Word_NUMBER,
    Word_Hour,
    Word_Hours,
    Word_ItIs,
    Word_Minute,
    Word_Minutes,
} voiceword_t;


/* Sentence buffer type */
typedef struct {
    voiceword_t     words_buf[MAX_SENTENCE_WORDS];
    int16_t         words_arg_buf[MAX_SENTENCE_WORDS];
    int8_t          wrpos;
    int8_t          rdpos;
    int8_t          is_speaking;
} sentence_buffer_t;


/* Global Sentence buffer */
extern sentence_buffer_t g_sentence_buf;


void voice_init(void);

int8_t voice_process(const message_t *msg);

#endif
