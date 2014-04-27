#include "voice.h"
#include "setting.h"
#include "serial.h"
#include "clock.h"
#include <avr/pgmspace.h>
#include <string.h>

/* Global Sentence buffer */
/*extern*/ sentence_buffer_t g_sentence_buf;

static char g_voice_fname_str[25];

static inline int8_t iwrap(int8_t x)
{
    if (x >= MAX_SENTENCE_WORDS) {
        x -= MAX_SENTENCE_WORDS;
    }
    return x;
}

static int voice_add_word(sentence_buffer_t *sb, voiceword_t wrd, int16_t arg1)
{
    int8_t wp = sb->wrpos;
    int8_t next_wp = iwrap(wp+1);

    if (next_wp == sb->rdpos) {
        /* Buffer full! */
        usart_sendstr_P(PSTR("[voice_add_word FULL]\n"));
        return 1;
    }

    sb->words_buf[wp] = wrd;
    sb->words_arg_buf[wp] = arg1;

    sb->wrpos = next_wp;

    return 0;
}


static int voice_word_to_fname(char *fname, voiceword_t wrd, int16_t arg1)
{
#if (SOUND_SAMPLERATE_KHZ == 22) && (SOUND_BITS_PER_SAMPLE == 8)
    strcpy_P(fname, PSTR("22k8u.en1/"));
#endif
#if (SOUND_SAMPLERATE_KHZ == 8) && (SOUND_BITS_PER_SAMPLE == 16)
    strcpy_P(fname, PSTR("8k16u.en1/"));
#endif

    switch (wrd) {
        case Word_NONE: {
            fname[0] = 0;
            break;
        }
        case Word_NUMBER: {
            strcat_P(fname, PSTR("n"));
            itoa(arg1, &fname[strlen(fname)], 10);
            break;
        }
        case Word_Hour: {
            strcat_P(fname, PSTR("hour"));
            break;
        }
        case Word_Hours: {
            strcat_P(fname, PSTR("hours"));
            break;
        }
        case Word_ItIs: {
            strcat_P(fname, PSTR("it-is"));
            break;
        }
        case Word_Minute: {
            strcat_P(fname, PSTR("minute"));
            break;
        }
        case Word_Minutes: {
            strcat_P(fname, PSTR("minutes"));
            break;
        }
    }
    strcat_P(fname, PSTR(".raw"));
    return 0;
}



void voice_init(void)
{
    g_sentence_buf.rdpos = 0;
    g_sentence_buf.wrpos = 0;
    g_sentence_buf.is_speaking = 0;

    g_voice_fname_str[0] = 0;
}


int8_t voice_process(const message_t *msg)
{
    int8_t msg_recognized = 1;

    switch (msg->cmd) {
        case Cmd_SayTime: {
            clockinfo_t *clk = msg->arg1p;
            voice_add_word(&g_sentence_buf, Word_ItIs, 0);
            voice_add_word(&g_sentence_buf, Word_NUMBER, clk->hour);
            if (clk->hour == 1)
                voice_add_word(&g_sentence_buf, Word_Hour, 0);
            else
                voice_add_word(&g_sentence_buf, Word_Hours, 0);
            voice_add_word(&g_sentence_buf, Word_NUMBER, clk->min);
            if (clk->min == 1)
                voice_add_word(&g_sentence_buf, Word_Minute, 0);
            else
                voice_add_word(&g_sentence_buf, Word_Minutes, 0);
            break;
        }

        case Cmd_FinishedPlay: {
            if (g_sentence_buf.is_speaking) {
                /* word speach finished */
                g_sentence_buf.rdpos = iwrap(g_sentence_buf.rdpos + 1);
                g_sentence_buf.is_speaking = 0;
            }
            break;
        }

        default:
            msg_recognized = 0;
            break;
    }

    if (g_sentence_buf.is_speaking) {
        /* currently speaking */
    } else {
        /* currently not speaking */
        /* Are there words to tell? */
        if (g_sentence_buf.wrpos != g_sentence_buf.rdpos) {
            /* start speaking */
            g_sentence_buf.is_speaking = 1;
            voice_word_to_fname(g_voice_fname_str,
                g_sentence_buf.words_buf[g_sentence_buf.rdpos], 
                g_sentence_buf.words_arg_buf[g_sentence_buf.rdpos]);

            
            message_t *nmsg = loop_put_msg_begin();
            nmsg->cmd = Cmd_PlayFileName;
            nmsg->arg1p = g_voice_fname_str;
            loop_put_msg_end(nmsg);
        }

    }

    return msg_recognized;
}
