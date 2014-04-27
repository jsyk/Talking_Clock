#ifndef VFS_H
#define VFS_H

#include <avr/io.h>
#include "tff.h"
#include "loop.h"

/** SD Filesystem */
extern FATFS g_fatfs;        // 544B
extern DIR g_cwd;        // 14B, maybe not needed
extern FIL g_file;       // 22B



void vfs_init(void);

void sdcard_slow_mode(void);
void sdcard_fast_mode(void);

int8_t vfs_process(const message_t *msg);

#endif
