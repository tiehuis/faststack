//! Defines all compile-time constants used in various locations.

/// Number of different `PieceType`'s
pub const PIECE_TYPE_COUNT: usize = 7;

/// Number of different `RotationSystem`'s
pub const ROTATION_SYSTEM_COUNT: usize = 7;

/// Number of different `Piece` rotations.
pub const PIECE_ROTATION_COUNT: usize = 4;

/// Number of blocks per `Piece`.
pub const PIECE_BLOCK_COUNT: usize = 4;

/// Maximum width a field grid.
pub const MAX_GRID_WIDTH: usize = 20;

/// Maximum height of a field grid.
pub const MAX_GRID_HEIGHT: usize = 25;

/// Maximum number of preview pieces that can be displayed.
pub const MAX_PREVIEW_COUNT: usize = 5;
