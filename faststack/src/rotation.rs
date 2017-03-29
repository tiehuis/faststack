//! Defines various rotation systems.
//!
//! Rotation systems consist of rotation rules and wallkick rules. All rotation
//! systems are based off the SRS base rotations by default with differences
//! in rotation encoded within their wallkick tables.
//
// TODO: Determine whether returning an iterator instead of static slices is
// a more ergonomic solution. One thing this would make easier is performing
// modifications on top of the default rotations/wallkicks for example.

use primitives::{PieceType, Theta};
use primitives::PieceType::*;
use primitives::Theta::*;

/// Count the number of args in a macro parameter pack.
macro_rules! count {
    () => (0usize);
    ($x:tt $($xs:tt)*) => (1usize + count!($($xs)*))
}

/// Construct a (S)tatic (A)rray (REF)erence as an expression inline.
macro_rules! sa_ref {
    ($ty:ty; $($x:expr),*) => {
        {
            static A: &'static [$ty; count!($($x)*)] = &[$($x),*];
            A
        }
    }
}

/// Specialized types
macro_rules! i8_sa_ref { ($($x:expr),*) => (sa_ref![(i8, i8); $($x),*]) }
macro_rules! u8_sa_ref { ($($x:expr),*) => (sa_ref![(u8, u8); $($x),*]) }

/// Types of rotation systems.
#[derive(Clone, Copy, Debug)]
pub enum RotationSystemType {
    SRS
}

impl RotationSystemType {
    /// Polymorphic function for retriving kicks from a RotationSystem
    /// implementer.
    ///
    /// We prefer this to a trait object since it allows the implementors of
    /// `RotationTable` to be different sizes and store different data.
    pub fn kicks(&self, p: PieceType, c: Theta, r: Theta) -> &'static [(i8, i8)] {
        match *self {
            RotationSystemType::SRS => RotationSystemSRS.kicks(p, c, r)
        }
    }
}

/// Return the offsets of the specified piece and rotation.
///
/// These are shared across every type of rotation system.
fn piece_offsets(p: PieceType, r: Theta) -> &'static [(u8, u8)]
{
    match (p, r) {
        (I, R0)   => u8_sa_ref![(0, 1), (1, 1), (2, 1), (3, 1)],
        (I, R90)  => u8_sa_ref![(2, 0), (2, 1), (2, 2), (2, 3)],
        (I, R180) => u8_sa_ref![(0, 2), (1, 2), (2, 2), (3, 2)],
        (I, R270) => u8_sa_ref![(1, 0), (1, 1), (1, 2), (1, 3)],

        (J, R0)   => u8_sa_ref![(0, 0), (0, 1), (1, 1), (2, 1)],
        (J, R90)  => u8_sa_ref![(1, 0), (1, 1), (1, 2), (2, 0)],
        (J, R180) => u8_sa_ref![(0, 1), (1, 1), (2, 1), (2, 2)],
        (J, R270) => u8_sa_ref![(0, 2), (1, 0), (1, 1), (1, 2)],

        (L, R0)   => u8_sa_ref![(0, 1), (1, 1), (2, 0), (2, 1)],
        (L, R90)  => u8_sa_ref![(1, 0), (1, 1), (1, 2), (2, 2)],
        (L, R180) => u8_sa_ref![(0, 1), (0, 2), (1, 1), (2, 1)],
        (L, R270) => u8_sa_ref![(0, 0), (1, 0), (1, 1), (1, 2)],

        (O, R0)   => u8_sa_ref![(1, 0), (1, 1), (2, 0), (2, 1)],
        (O, R90)  => u8_sa_ref![(1, 0), (1, 1), (2, 0), (2, 1)],
        (O, R180) => u8_sa_ref![(1, 0), (1, 1), (2, 0), (2, 1)],
        (O, R270) => u8_sa_ref![(1, 0), (1, 1), (2, 0), (2, 1)],

        (S, R0)   => u8_sa_ref![(0, 1), (1, 0), (1, 1), (2, 0)],
        (S, R90)  => u8_sa_ref![(1, 0), (1, 1), (2, 1), (2, 2)],
        (S, R180) => u8_sa_ref![(0, 2), (1, 1), (1, 2), (2, 1)],
        (S, R270) => u8_sa_ref![(0, 0), (0, 1), (1, 1), (1, 2)],

        (T, R0)   => u8_sa_ref![(0, 1), (1, 0), (1, 1), (2, 1)],
        (T, R90)  => u8_sa_ref![(1, 0), (1, 1), (1, 2), (2, 1)],
        (T, R180) => u8_sa_ref![(0, 1), (1, 1), (1, 2), (2, 1)],
        (T, R270) => u8_sa_ref![(0, 1), (1, 0), (1, 1), (1, 2)],

        (Z, R0)   => u8_sa_ref![(0, 0), (1, 0), (1, 1), (2, 1)],
        (Z, R90)  => u8_sa_ref![(1, 1), (1, 2), (2, 0), (2, 1)],
        (Z, R180) => u8_sa_ref![(0, 1), (1, 1), (1, 2), (2, 2)],
        (Z, R270) => u8_sa_ref![(0, 1), (0, 2), (1, 0), (1, 2)],
    }
}

/// Defines a trait abstracting over rotation implementations.
pub trait RotationTable {
    /// Returns an iterator over successive wallkick offsets to test.
    fn kicks(&self, p: PieceType, c: Theta, r: Theta) -> &'static [(i8, i8)];
}

pub struct RotationSystemSRS;

impl RotationTable for RotationSystemSRS {
    fn kicks(&self, p: PieceType, c: Theta, r: Theta) -> &'static [(i8, i8)] {
        match (p, r, c) {
            // I clockwise
            (I, R90, R0)   =>
                i8_sa_ref![( 0, 0), (-1, 0), (-1,-1), ( 0, 2), (-1, 2)],
            (I, R90, R90)  =>
                i8_sa_ref![( 0, 0), ( 1, 0), ( 1, 1), ( 0,-2), ( 1,-2)],
            (I, R90, R180) =>
                i8_sa_ref![( 0, 0), ( 1, 0), ( 1,-1), ( 0, 2), ( 1, 2)],
            (I, R90, R270) =>
                i8_sa_ref![( 0, 0), (-1, 0), (-1, 1), ( 0,-2), (-1,-2)],

            // I anti-clockwise
            (I, R270, R0)   =>
                i8_sa_ref![( 0, 0), (-2, 0), ( 1, 0), (-2, 1), ( 1,-2)],
            (I, R270, R90)  =>
                i8_sa_ref![( 0, 0), (-1, 0), ( 2, 0), (-1,-2), ( 2, 1)],
            (I, R270, R180) =>
                i8_sa_ref![( 0, 0), ( 2, 0), (-1, 0), ( 2,-1), (-1, 2)],
            (I, R270, R270) =>
                i8_sa_ref![( 0, 0), ( 1, 0), (-2, 0), ( 1, 2), (-2,-1)],

            // J, L, S, T, Z clockwise
            (_, R90, R0)   =>
                i8_sa_ref![( 0, 0), (-1, 0), (-1,-1), ( 0, 2), (-1, 2)],
            (_, R90, R90)  =>
                i8_sa_ref![( 0, 0), ( 1, 0), ( 1, 1), ( 0,-2), ( 1,-1)],
            (_, R90, R180) =>
                i8_sa_ref![( 0, 0), ( 1, 0), ( 1,-1), ( 0, 2), ( 1, 2)],
            (_, R90, R270) =>
                i8_sa_ref![( 0, 0), (-1, 0), (-1, 1), ( 0,-2), (-1,-2)],

            // J, L, S, T, Z anti-clockwise
            (_, R270, R0)   =>
                i8_sa_ref![( 0, 0), ( 1, 0), ( 1,-1), ( 0, 2), ( 1, 2)],
            (_, R270, R90)  =>
                i8_sa_ref![( 0, 0), ( 1, 0), ( 1, 1), ( 0,-2), ( 1,-2)],
            (_, R270, R180) =>
                i8_sa_ref![( 0, 0), (-1, 0), (-1,-1), ( 0, 2), (-1, 2)],
            (_, R270, R270) =>
                i8_sa_ref![( 0, 0), (-1, 0), (-1, 1), ( 0,-2), (-1,-2)],

            // Every other rotation (i.e. 180 spin)
            _ => i8_sa_ref![]
        }
    }
}
