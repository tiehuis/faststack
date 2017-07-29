#ifndef FS_DAO_H
#define FS_DAO_H

#include "core.h"
#include <sqlite3.h>

struct FSDao {
    sqlite3 *db;
    sqlite3_stmt *hiscore_stmt;
    sqlite3_stmt *replay_overview_stmt;
    sqlite3_stmt *replay_overview_complete_stmt;
    sqlite3_stmt *replay_input_stmt;
    u32 replay_overview_row_id;
    u32 last_input_keystate;
};

void daoInit(FSDao *dao);
void daoSaveHiscore(FSDao *dao, const FSEngine *f);
void daoInsertReplayOverview(FSDao *dao, const FSEngine *f);
void daoInsertReplayInput(FSDao *dao, u32 ticks, u32 keystate);
void daoMarkReplayComplete(FSDao *dao);

#endif
