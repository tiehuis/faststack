use std::default::Default;
use constants;

/// Represents a relative or absolute rotation in one of four directions.
#[derive(Clone, Copy, Debug)]
#[repr(u8)]
pub enum Theta {
    R0, R90, R180, R270
}

/// Represents a single type of piece.
#[derive(Clone, Copy, Debug, Eq, PartialEq, Ord, PartialOrd)]
#[repr(u8)]
pub enum PieceType {
    I, J, L, O, S, T, Z,
}

macro_rules! impl_from_piece_type {
    ($ty:ty) => {
        impl From<$ty> for PieceType {
            /// Convert a raw unsigned integer into a `PieceType`.
            ///
            /// This invariant must be managed by the caller and will panic on
            /// bad input. If the input value is in the range
            /// `[0, PIECE_TYPE_COUNT]` then it will never panic.
            fn from(input: $ty) -> PieceType {
                if input >= constants::PIECE_TYPE_COUNT as $ty {
                    panic!("integer is too large to construct a PieceType");
                }

                unsafe { ::std::mem::transmute(input as u8) }
            }
        }
    }
}

impl_from_piece_type!(u8);
impl_from_piece_type!(u16);
impl_from_piece_type!(u32);
impl_from_piece_type!(u64);
impl_from_piece_type!(usize);

/// Represents a single piece and associated information.
///
/// A piece typically is closesly tied to a field. Many functions that act on
/// a piece are found in the `Engine` struct.
#[derive(Clone, Debug)]
pub struct Piece {
    /// The type of piece
    pub ty: PieceType,

    /// X position in grid
    pub x: u8,

    /// Y position in grid
    pub y: u8,

    /// Current absolute rotation of piece from entry
    pub theta: Theta,

    /// Current lock timer elapsed
    pub lock_timer: u32,

    /// Number of rotations the piece has performed
    pub rotation_count: u32,

    /// Number of movements the piece has performed
    pub movement_count: u32,

    /// Number of floorkicks the piece has performed
    pub floorkick_count: u32,

    /// Can this piece be held?
    pub can_hold: bool
}

impl Default for Piece {
    /// This is typically only used internally since the default `PieceType`
    /// and `x`, `y` values are not reliable defaults.
    fn default() -> Self {
        Piece {
            ty: PieceType::I,
            x: 0,
            y: 0,
            theta: Theta::R0,
            lock_timer: 0,
            rotation_count: 0,
            movement_count: 0,
            floorkick_count: 0,
            can_hold: true
        }
    }
}

impl Piece {
    /// Returns a new `Piece` of the specified type.
    pub fn new(ty: PieceType) -> Self {
        Piece { ty, ..Default::default() }
    }

    /// Returns an iterator of current grid positions of the `Piece`.
    pub fn blocks(&self) -> [(u8, u8); constants::PIECE_BLOCK_COUNT] {
        [(0, 0), (1, 1), (2, 2), (3, 3)]
    }
}
