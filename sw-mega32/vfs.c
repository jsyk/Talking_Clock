#include "vfs.h"
#include "serial.h"
#include "diskio.h"
#include <avr/pgmspace.h>

/** SD Filesystem */
FATFS g_fatfs;        // 544B
DIR g_cwd;        // 14B, maybe not needed
FIL g_file;       // 22B



void vfs_init(void)
{
    /* invalidate FS object */
    g_fatfs.fs_type = 0;
}

void sdcard_slow_mode(void)
{
    SPCR = (1<<SPE) | (1<<MSTR) | (2 << SPR0);  /* init SPI, fosc / 64 */
}

void sdcard_fast_mode(void)
{
    SPCR = (1<<SPE) | (1<<MSTR);        /* SPI: fosc/4 */
    SPSR = (1 << SPI2X);                /* 2x */
}


int8_t vfs_process(const message_t *msg)
{
    int8_t msg_recognized = 1;
    FRESULT fres;

    switch (msg->cmd) {
        case Cmd_PlayFileName: {
            if (g_fatfs.fs_type == 0) {
                /* mount FS */
                sdcard_slow_mode();
                usart_sendstr_P(PSTR(" / disk init...\n"));
                unsigned char res = disk_initialize(0);
                usart_sendstr_P(PSTR("result of disk init = 0x"));
                usart_sendhexb(res);
                usart_send_nl();

                if (res == 0) {
                    /* fast mode is needed to sustain the demands of sound-play */
                    sdcard_fast_mode();
                    usart_sendstr_P(PSTR("SPI fast mode set.\n"));
                } else {
                    usart_sendstr_P(PSTR("WARNING: SPI fast mode NOT set.\n"));
                }

                usart_sendstr_P(PSTR(" / fat mount...\n"));
                fres = f_mount(0, &g_fatfs);
                usart_sendstr_P(PSTR("result fat mount = 0x"));
                usart_sendhexb((unsigned char)fres);
                usart_send_nl();

                if (fres != FR_OK) {
                    g_fatfs.fs_type = 0;
                    goto out;
                }
            }

            /* disk/vfs mount ok. */
            /* open file, play */
            usart_sendstr_P(PSTR("File open: ")); usart_sendstr(msg->arg1p); usart_send_nl();
            fres = f_open(&g_file, msg->arg1p, FA_READ|FA_OPEN_EXISTING);
            usart_sendstr_P(PSTR("Result of f_open: 0x"));
            usart_sendhexb((unsigned char)fres);
            usart_send_nl();

            if (fres == FR_OK) {
                /* file open OK */
                message_t *nmsg = loop_put_msg_begin();
                nmsg->cmd = Cmd_PlayFileHandle;
                nmsg->arg1p = &g_file;
                loop_put_msg_end(nmsg);
            } else {
                /* file open FAIL */
                g_fatfs.fs_type = 0;
                message_t *nmsg = loop_put_msg_begin();
                nmsg->cmd = Cmd_PlayFileName;
                nmsg->arg1p = msg->arg1p;
                loop_put_msg_end(nmsg);
            }

            break;
        }

        default:
            msg_recognized = 0;
            break;
    }

out:
    return msg_recognized;
}
