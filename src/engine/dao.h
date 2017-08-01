#ifndef FS_DAO_H
#define FS_DAO_H

#include "core.h"
#include <sqlite3.h>

struct FSDao {
    sqlite3 *db;
    sqlite3_stmt *hiscore_stmt;
    sqlite3_stmt *replay_overview_stmt;
    sqlite3_stmt *replay_overview_select_stmt;
    sqlite3_stmt *replay_overview_complete_stmt;
    sqlite3_stmt *replay_input_stmt;
    sqlite3_stmt *replay_output_stmt;

    // NOTE: We can merge the following since one is used for input, the
    // other used during output.
    u32 replay_overview_row_id;
    u32 last_input_keystate;

    u32 output_replay_id;
    u32 last_output_keystate;
};

const char* daoGetDatabasePath(void);
void daoInit(FSDao *dao);
void daoSaveHiscore(FSDao *dao, const FSEngine *f);
void daoInsertReplayOverview(FSDao *dao, const FSEngine *f);
void daoInsertReplayInput(FSDao *dao, u32 ticks, u32 keystate);
void daoMarkReplayComplete(FSDao *dao);

void daoLoadReplay(FSDao *dao, FSEngine *f, u32 replay_id);
u32 daoGetReplayInput(FSDao *dao, u32 tick);

#endif
