///
// hiscore.c
// =========
//
// Functions for writing and reading to the hiscore file.
//
// The hiscore file is a simple csv-like file with space-delimiters.
//
// A sample hiscore file may look like the following:
//
// ```
// time,blocks,tps,kpt,date
// 41.230,107,2.31244,3.12312,2016-07-02 18:15:02
// 47.439,127,2.13313,3.12312,2016-07-02 18:18:43
// ```
///

#include "core.h"
#include "config.h"
#include "engine.h"
#include "log.h"

#include <stdio.h>
#include <time.h>

static void hsWriteHeader(FILE *fd)
{
    fprintf(fd, "time,blocks,tps,kpt,goal,date\n");
}

static char* hsCurrentDate(void)
{
    static char buffer[26];
    time_t timer;
    struct tm *tmInfo;

    time(&timer);
    tmInfo = localtime(&timer);
    strftime(buffer, sizeof(buffer), "%F %H:%M:%S", tmInfo);
    return buffer;
}

// Get a handle to the hiscore file. If it does not exist, then create it
// with the default header values.
static FILE* hsGetFileHandle(void)
{
    FILE *fd = fopen(FS_HISCORE_FILENAME, "a+");
    if (fd == NULL) {
        fsLogWarning("failed to open hiscore file");
        return NULL;
    }

    // It is implementation defined according to the C11 standard whether
    // we start at the end of a file when opening in append. Seek to the
    // end to ensure where we are.
    fseek(fd, 0, SEEK_END);

    switch (ftell(fd)) {
        case 0:
            hsWriteHeader(fd);
            break;
        case -1:
            fsLogWarning("error reading file");
            return NULL;
        default:
            break;
    }

    return fd;
}

void fsHiscoreInsert(const FSEngine *f)
{
    FILE *fd = hsGetFileHandle();
    if (fd == NULL) {
        return;
    }

    const int msElapsed = f->msPerTick * f->totalTicks;
    fprintf(fd, "%.3f,%d,%.5f,%.5f,%d,%s\n",
            (float) msElapsed / 1000,
            f->blocksPlaced,
            (float) f->blocksPlaced / ((float) msElapsed / 1000),
            (float) f->totalKeysPressed / f->blocksPlaced,
            f->goal,
            hsCurrentDate()
    );

    fclose(fd);
}
