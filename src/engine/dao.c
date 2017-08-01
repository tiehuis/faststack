// ```
// FSFSDao dao;
// daoInit(&dao);
//
// daoInsertReplayOverview(&dao);
// daoInsertReplayInput(&dao, 1, 0x45);
// daoMarkReplayComplete(&dao);
// ```

#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>

#include "core.h"
#include "dao.h"
#include "engine.h"
#include "log.h"
#include "option.h" // for FileExists

// For mkdir
#ifdef __linux__
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif

#define DAO_FILENAME "fs.db"

static void setupHiscoreTable(FSDao *dao);
static void setupReplayOverviewTable(FSDao *dao);
static void setupReplayInputTable(FSDao *dao);

// Resolves the db file to load.
//
// Load priority is as follows:
//  - fs.db (only if already exists)
//  - $XDG_DATA_HOME/faststack/database.db (linux, created if doesn't exist)
//  - fs.db (created if doesn't exist)
const char* daoGetDatabasePath(void)
{
    if (fileExists(DAO_FILENAME)) {
        return DAO_FILENAME;
    }

#ifdef __linux__
    static char dbPath[128];

    const char *fsDatabase = "faststack";
    char *dataHome = getenv("XDG_DATA_HOME");
    if (dataHome) {
        snprintf(dbPath, sizeof(dbPath), "%s/%s", dataHome, fsDatabase);
    } else {
        char *homePath = getenv("HOME");
        if (!homePath) {
            homePath = "~";
        }

        snprintf(dbPath, sizeof(dbPath), "%s/%s/%s",
                 homePath, ".local/share", fsDatabase);
    }

    // sqlite3 will create the database for us, but it won't create any
    // leading directories, so perform that ourselves.
    //
    // Assumes you have a .local/share created and does not exist as a
    // normal file.
    errno = 0;
    if (mkdir(dbPath, 0777) != 0) {
        if (errno != EEXIST) {
            fsLogFatal("mkdir '%s' returned %s", dbPath, strerror(errno));
            exit(1);
        }
    } else {
        fsLogInfo("created new database directory %s", dbPath);
    }

    strncat(dbPath, "/database.db", sizeof(dbPath) - strlen(dbPath) - 1);
    return dbPath;
#else
    return DAO_FILENAME;
#endif
}

// Need to keep track of the replay foreign key.
void daoInit(FSDao *dao)
{
    const char *path = daoGetDatabasePath();
    fsLogInfo("using database at %s", path);

    if (sqlite3_open(path, &dao->db) != SQLITE_OK) {
        fsLogFatal("%s", sqlite3_errmsg(dao->db));
        exit(1);
    }

    setupHiscoreTable(dao);
    setupReplayOverviewTable(dao);
    setupReplayInputTable(dao);
}

void daoDeinit(FSDao *dao)
{
    if (sqlite3_finalize(dao->hiscore_stmt) != SQLITE_OK) {
        fsLogWarning("%s", sqlite3_errmsg(dao->db));
    }

    if (sqlite3_finalize(dao->replay_overview_stmt) != SQLITE_OK) {
        fsLogWarning("%s", sqlite3_errmsg(dao->db));
    }

    if (sqlite3_finalize(dao->replay_overview_complete_stmt) != SQLITE_OK) {
        fsLogWarning("%s", sqlite3_errmsg(dao->db));
    }

    if (sqlite3_finalize(dao->replay_input_stmt) != SQLITE_OK) {
        fsLogWarning("%s", sqlite3_errmsg(dao->db));
    }

    if (sqlite3_close(dao->db) != SQLITE_OK) {
        fsLogWarning("%s", sqlite3_errmsg(dao->db));
    }
}

// If new hiscore fields are added in the future, previously unknown fields
// will simply have NULL values and will not have any associated data. This
// is handled on the date retrieval side.
static void setupHiscoreTable(FSDao *dao)
{
    const char create_stmt[] =
        "create table if not exists hiscore"
        "("
            "id INTEGER PRIMARY KEY,"
            "replay_id INTEGER REFERENCES replay_overview(id),"
            "time FLOAT,"
            "tps FLOAT,"
            "kpt FLOAT,"
            "goal INTEGER,"
            "date DATETIME"
        ");";

    if (sqlite3_exec(dao->db, create_stmt, NULL, NULL, NULL) != SQLITE_OK) {
        fsLogFatal("%s", sqlite3_errmsg(dao->db));
        exit(1);
    }

    const char insert_stmt[] =
        "insert into hiscore (replay_id, time, tps, kpt, goal, date) "
        "values (?, ?, ?, ?, ?, datetime(\"now\"));";

    if (sqlite3_prepare_v2(
            dao->db,
            insert_stmt,
            sizeof(insert_stmt),
            &dao->hiscore_stmt,
            NULL
        ) != SQLITE_OK)
    {
        fsLogFatal("%s", sqlite3_errmsg(dao->db));
        exit(1);
    }
}

void daoSaveHiscore(FSDao *dao, const FSEngine *f)
{
    sqlite3_stmt *s = dao->hiscore_stmt;
    const int msElapsed = f->msPerTick * f->totalTicks;

    sqlite3_bind_int(s, 1, dao->replay_overview_row_id);
    sqlite3_bind_double(s, 2, (double) msElapsed / 1000);
    sqlite3_bind_double(s, 3, (double) f->blocksPlaced / ((double) msElapsed / 1000));
    sqlite3_bind_double(s, 4, (double) f->totalKeysPressed / f->blocksPlaced);
    sqlite3_bind_int(s, 5, f->goal);

    sqlite3_step(s);
    sqlite3_clear_bindings(s);
    sqlite3_reset(s);
}

static void setupReplayOverviewTable(FSDao *dao)
{
    const char create_stmt[] =
        "create table if not exists replay_overview"
        "("
            "id INTEGER PRIMARY KEY,"
            "version INTEGER,"
            "date DATETIME,"
            "complete INT2,"
            "seed INTEGER,"
            "goal INTEGER,"
            "field_width INTEGER,"
            "field_height INTEGER,"
            "field_hidden INTEGER,"
            "initial_action_style INTEGER,"
            "das_speed INTEGER,"
            "das_delay INTEGER,"
            "ms_per_tick INTEGER,"
            "ticks_per_draw INTEGER,"
            "are_delay INTEGER,"
            "are_cancellable INTEGER,"
            "lock_style INTEGER,"
            "lock_delay INTEGER,"
            "floorkick_limit INTEGER,"
            "one_shot_soft_drop INTEGER,"
            "rotation_system INTEGER,"
            "gravity INTEGER,"
            "soft_drop_gravity INTEGER,"
            "randomizer INTEGER,"
            "ready_phase_length INTEGER,"
            "go_phase_length INTEGER,"
            "infinite_ready_go_hold INTEGER,"
            "next_piece_count INTEGER"
        ");";

    if (sqlite3_exec(dao->db, create_stmt, NULL, NULL, NULL) != SQLITE_OK) {
        fsLogFatal("%s", sqlite3_errmsg(dao->db));
        exit(1);
    }

    const char insert_stmt[] =
        "insert into replay_overview"
        "("
            "date,"
            "version,"
            "complete,"
            "seed,"
            "goal,"
            "field_width,"
            "field_height,"
            "field_hidden,"
            "initial_action_style,"
            "das_speed,"
            "das_delay,"
            "ms_per_tick,"
            "ticks_per_draw,"
            "are_delay,"
            "are_cancellable,"
            "lock_style,"
            "lock_delay,"
            "floorkick_limit,"
            "one_shot_soft_drop,"
            "rotation_system,"
            "gravity,"
            "soft_drop_gravity,"
            "randomizer,"
            "ready_phase_length,"
            "go_phase_length,"
            "infinite_ready_go_hold,"
            "next_piece_count"
        ") "
        "values"
        "("
            "datetime(\"now\"),"
            "1,"
            "0,"
            "?,"
            "?,"
            "?,"
            "?,"
            "?,"
            "?,"
            "?,"
            "?,"
            "?,"
            "?,"
            "?,"
            "?,"
            "?,"
            "?,"
            "?,"
            "?,"
            "?,"
            "?,"
            "?,"
            "?,"
            "?,"
            "?,"
            "?,"
            "?"
        ");";

    if (sqlite3_prepare_v2(
            dao->db,
            insert_stmt,
            sizeof(insert_stmt),
            &dao->replay_overview_stmt,
            NULL
        ) != SQLITE_OK)
    {
        fsLogFatal("%s", sqlite3_errmsg(dao->db));
        exit(1);
    }

    const char select_stmt[] =
        "select * from replay_overview where id = ?;";

    if (sqlite3_prepare_v2(
            dao->db,
            select_stmt,
            sizeof(select_stmt),
            &dao->replay_overview_select_stmt,
            NULL
        ) != SQLITE_OK)
    {
        fsLogFatal("%s", sqlite3_errmsg(dao->db));
        exit(1);
    }

    const char update_stmt[] =
        "update replay_overview set complete=1 where id = ?;";

    if (sqlite3_prepare_v2(
            dao->db,
            update_stmt,
            sizeof(update_stmt),
            &dao->replay_overview_complete_stmt,
            NULL
        ) != SQLITE_OK)
    {
        fsLogFatal("%s", sqlite3_errmsg(dao->db));
        exit(1);
    }
}

// NOTE: Current storage means reads every frame to check if the tick
// is accessed.
//
// These could be buffered however to perform only every second or so easily
// so don't worry about changing format right now.
static void setupReplayInputTable(FSDao *dao)
{
    const char create_stmt[] =
        "create table if not exists replay_input"
        "("
            "id INTEGER PRIMARY KEY,"
            "replay_id INTEGER REFERENCES replay_overview(id),"
            "tick INTEGER,"
            "keystate INTEGER"
        ");";

    if (sqlite3_exec(dao->db, create_stmt, NULL, NULL, NULL) != SQLITE_OK) {
        fsLogFatal("%s", sqlite3_errmsg(dao->db));
        exit(1);
    }

    const char insert_stmt[] =
        "insert into replay_input"
        "("
            "replay_id,"
            "tick,"
            "keystate"
        ")"
        "values"
        "("
            "?,"
            "?,"
            "?"
        ");";

    if (sqlite3_prepare_v2(
            dao->db,
            insert_stmt,
            sizeof(insert_stmt),
            &dao->replay_input_stmt,
            NULL
        ) != SQLITE_OK)
    {
        fsLogFatal("%s", sqlite3_errmsg(dao->db));
        exit(1);
    }

    const char get_stmt[] =
        "select keystate from replay_input "
        "where replay_id = ? and tick = ?;";

    if (sqlite3_prepare_v2(
            dao->db,
            get_stmt,
            sizeof(get_stmt),
            &dao->replay_output_stmt,
            NULL
        ) != SQLITE_OK)
    {
        fsLogFatal("%s", sqlite3_errmsg(dao->db));
        exit(1);
    }

    dao->last_input_keystate = 0;
    dao->last_output_keystate = 0;
}

void daoInsertReplayOverview(FSDao *dao, const FSEngine *f)
{
    sqlite3_stmt *s = dao->replay_overview_stmt;

    sqlite3_bind_int(s,  1, f->seed);
    sqlite3_bind_int(s,  2, f->goal);
    sqlite3_bind_int(s,  3, f->fieldWidth);
    sqlite3_bind_int(s,  4, f->fieldHeight);
    sqlite3_bind_int(s,  5, f->fieldHidden);
    sqlite3_bind_int(s,  6, f->initialActionStyle);
    sqlite3_bind_int(s,  7, f->dasSpeed);
    sqlite3_bind_int(s,  8, f->dasDelay);
    sqlite3_bind_int(s,  9, f->msPerTick);
    sqlite3_bind_int(s, 10, f->ticksPerDraw);
    sqlite3_bind_int(s, 11, f->areDelay);
    sqlite3_bind_int(s, 12, f->areCancellable);
    sqlite3_bind_int(s, 13, f->lockStyle);
    sqlite3_bind_int(s, 14, f->lockDelay);
    sqlite3_bind_int(s, 15, f->floorkickLimit);
    sqlite3_bind_int(s, 16, f->oneShotSoftDrop);
    sqlite3_bind_int(s, 17, f->rotationSystem);
    sqlite3_bind_int(s, 18, f->gravity);
    sqlite3_bind_int(s, 19, f->softDropGravity);
    sqlite3_bind_int(s, 20, f->randomizer);
    sqlite3_bind_int(s, 21, f->readyPhaseLength);
    sqlite3_bind_int(s, 22, f->goPhaseLength);
    sqlite3_bind_int(s, 23, f->infiniteReadyGoHold);
    sqlite3_bind_int(s, 24, f->nextPieceCount);

    sqlite3_step(s);
    sqlite3_clear_bindings(s);
    sqlite3_reset(s);

    dao->replay_overview_row_id = sqlite3_last_insert_rowid(dao->db);
}

void daoInsertReplayInput(FSDao *dao, u32 ticks, u32 keystate)
{
    // Only store deltas and not each state.
    if (dao->last_input_keystate == keystate) {
        return;
    }

    sqlite3_stmt *s = dao->replay_input_stmt;

    sqlite3_bind_int(s, 1, dao->replay_overview_row_id);
    sqlite3_bind_int(s, 2, ticks);
    sqlite3_bind_int(s, 3, keystate);

    sqlite3_step(s);
    sqlite3_clear_bindings(s);
    sqlite3_reset(s);

    dao->last_input_keystate = keystate;
}

static void daoLoadReplayOverview(FSDao *dao, FSEngine *f, u32 replay_id)
{
    sqlite3_stmt *s = dao->replay_overview_select_stmt;

    sqlite3_bind_int(s, 1, replay_id);
    if (sqlite3_step(s) != SQLITE_ROW) {
        fsLogFatal("no replay found with id: %d", replay_id);
        exit(1);
    }

    if (sqlite3_column_int(s, 3) == 0) {
        fsLogWarning("incomplete replay being played!");
    }

    // Skip id, version, date and complete
    f->seed = sqlite3_column_int(s, 4);
    f->goal = sqlite3_column_int(s, 5);
    f->fieldWidth = sqlite3_column_int(s, 6);
    f->fieldHeight = sqlite3_column_int(s, 7);
    f->fieldHidden = sqlite3_column_int(s, 8);
    f->initialActionStyle = sqlite3_column_int(s, 9);
    f->dasSpeed = sqlite3_column_int(s, 10);
    f->dasDelay = sqlite3_column_int(s, 11);
    f->msPerTick = sqlite3_column_int(s, 12);
    f->ticksPerDraw = sqlite3_column_int(s, 13);
    f->areDelay = sqlite3_column_int(s, 14);
    f->areCancellable = sqlite3_column_int(s, 15);
    f->lockStyle = sqlite3_column_int(s, 16);
    f->lockDelay = sqlite3_column_int(s, 17);
    f->floorkickLimit = sqlite3_column_int(s, 18);
    f->oneShotSoftDrop = sqlite3_column_int(s, 19);
    f->rotationSystem = sqlite3_column_int(s, 20);
    f->gravity = sqlite3_column_int(s, 21);
    f->softDropGravity = sqlite3_column_int(s, 22);
    f->randomizer = sqlite3_column_int(s, 23);
    f->readyPhaseLength = sqlite3_column_int(s, 24);
    f->goPhaseLength = sqlite3_column_int(s, 25);
    f->infiniteReadyGoHold = sqlite3_column_int(s, 26);
    f->nextPieceCount = sqlite3_column_int(s, 27);

    sqlite3_clear_bindings(s);
    sqlite3_reset(s);
}

void daoLoadReplay(FSDao *dao, FSEngine *f, u32 replay_id)
{
    daoLoadReplayOverview(dao, f, replay_id);

    dao->output_replay_id = replay_id;
    dao->last_output_keystate = 0;
}

u32 daoGetReplayInput(FSDao *dao, u32 tick)
{
    sqlite3_stmt *s = dao->replay_output_stmt;

    sqlite3_bind_int(s, 1, dao->output_replay_id);
    sqlite3_bind_int(s, 2, tick);

    if (sqlite3_step(s) == SQLITE_ROW) {
        dao->last_output_keystate = sqlite3_column_int(s, 0);
    }

    sqlite3_step(s);
    sqlite3_clear_bindings(s);
    sqlite3_reset(s);

    return dao->last_output_keystate;
}

void daoMarkReplayComplete(FSDao *dao)
{
    sqlite3_stmt *s = dao->replay_overview_complete_stmt;

    sqlite3_bind_int(s, 1, dao->replay_overview_row_id);

    sqlite3_step(s);
    sqlite3_clear_bindings(s);
    sqlite3_reset(s);
}
