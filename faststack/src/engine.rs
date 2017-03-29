//! Core Engine code.

use std::default::Default;

/// Represents all possible states the engine may be in.
#[derive(Clone, Copy, Debug)]
pub enum GameState {
    /// Pre-game countdown
    Ready,

    /// Pre-game countdown
    Go,

    /// A block is falling and is in mid-air
    Falling,

    /// A block is current immediately above the field/floor
    Landed,

    /// A new piece is being generated
    NewPiece,

    /// Lines are currently being cleared
    Lines,

    /// Quit request in engine
    Quit,

    /// Game end condition has been encountered
    GameOver,

    /// Restart request in engine
    Restart
}

/// Tracks overall game statistics.
#[derive(Clone, Debug, Default)]
pub struct Statistics {
    /// How many keys were pressed
    total_keys_pressed: u32,

    /// How many blocks were placed
    blocks_placed: u32,

    /// Number of lines cleared
    lines_cleared: u32,

    /// Current finesse
    finesse: u32,
}

/// Configurable options which are passed through to an `Engine`.
#[derive(Clone, Debug)]
pub struct Options {
    /// Target number of lines to clear until complete
    goal: u32,

    /// Width of the field
    field_width: u8,

    /// Height of the field
    field_height: u8,

    /// Height of the hidden portion of the field
    field_hidden: u8,

    /// The type of `RotationSystem` used
    rotation_system: RotationSystem,

    /// How many ms must elapse before a piece moves
    das_speed: u16,

    /// How many ms a key must be held before repeated movement
    das_delay: u16,

    /// Number of ms which elapse per tick
    ms_per_tick: u8,

    /// How many game ticks should occur before a draw
    ticks_per_draw: u8,

    /// Maximum number of floorkicks that can occur for a single piece
    floorkick_limit: u8,

    /// How many ms before a piece automatically locks
    lock_delay: u32,

    /// How many ms before a piece drops a square with user input
    soft_drop_gravity: u32,

    /// How many ms before a piece drops a square without user input
    gravity: u32,

    /// Number of preview pieces to display
    preview_piece_count: u8,
}

impl Default for Options {
    fn default() -> Options {
        Options {
            goal: 40,
            field_width: 10,
            field_height: 22,
            field_hidden: 3,
            rotation_system: RotationSystem::SRS,
            das_speed: 16,
            das_delay: 150,
            ms_per_tick: 16,
            ticks_per_draw: 1,
            floorkick_limit: 0,
            lock_delay: 500,
            soft_drop_gravity: 0,
            gravity: 2000,
            preview_piece_count: 2
        }
    }
}

/// Internal configuration values that must be stored for replication.
#[derive(Clone, Debug)]
pub struct Internal {
    /// The seed used for all events in this game.
    seed: u64,
}

/// Main `Engine` which manages the overall game state.
#[derive(Debug)]
pub struct Engine {
    /// Current state of the game.
    state: GameState,

    /// Field which holds the set pieces.
    grid: [[Option<PieceType>; constants::FIELD_MAX_WIDTH]; constants::FIELD_MAX_HEIGHT],

    /// Configurable option values
    options: Options,

    /// Internal option values
    internal: Internal,

    /// Currently active piece
    piece: Piece,

    /// Current hold piece
    hold_piece: Option<PieceType>,

    /// Buffer of all the pending next pieces
    next_pieces: [PieceType: constants::MAX_PREVIEW_COUNT],

    /// Random context used for generating new pieces
    random_context: RandomContext,

    /// The number of game updates that have occurred since the game started
    ///
    /// This excludes the `Ready`, `Go` states.
    total_ticks: u64,

    /// The number of ticks since game starting (including `Ready, Go`).
    total_ticks_all: u64,
}

impl Engine {
    /// Construct a new `Engine`.
    pub fn new() -> Engine {
        unimplemented!()
    }

    /// Reset an `Engine` back into a start state using its current `Options`.
    pub fn reset(&mut self) {
        unimplemented!()
    }

    /// Perform a single game-tick.
    ///
    /// It is expected that the caller manages the time and calls this function
    /// again in precisely `Engine::options.ms_per_tick` milliseconds.
    pub fn tick(&mut self) {
        unimplemented!()
    }

    /// Returns true if the specified coordinates in the grid are occupied by
    /// a block.
    fn is_occupied(&self, (x, y): (u8, u8)) -> bool {
        unimplemented!()
    }

    /// Returns true if the specified piece would collide with the current
    /// field.
    fn is_collision(&self, p: &Piece) -> bool {
        unimplemented!()
    }

    /// Lock the current piece onto the field.
    ///
    /// This assumes that the piece currently does not collide with anything on
    /// the field, and should be preceeded by an `is_collision` call somewhere.
    fn lock_piece(&mut self) {
        unimplemented!()
    }

    /// Generate a new piece and update the preview sequence.
    fn next_piece(&mut self) {
        unimplemented!()
    }

    /// Try and rotate the current piece in place, returning whether the
    /// rotation suceeded.
    ///
    /// This will apply the wallkick rules of the current rotation system.
    fn rotate(&mut self, theta: Theta) -> bool {
        unimplemented!()
    }

    /// Apply gravity for the current tick.
    ///
    /// This is fractional and may not result in a visible drop.
    fn gravity(&mut self) {
        unimplemented!()
    }

    /// Clear any complete lines in the grid and update associated variables.
    fn clear_lines(&mut self) {
        unimplemented!()
    }

    /// Try and perform a hold, returning whether the hold succeeded.
    ///
    /// This will make the next piece in the preview buffer the current piece
    /// if the holdwas successful.
    fn hold(&mut self) -> bool {
        unimplemented!()
    }
}
