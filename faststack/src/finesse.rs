//! Calculates optimal piece placements for a standard 10-wide playfield.

use primitives::{PieceType, Theta};
use primitives::PieceType::*;
use primitives::Theta::*;

fn position_delta(ty: PieceType, theta: Theta) -> u8 {
    let a = match ty {
        I | O             => [0, 2, 0, 1],
        J | L | S | T | Z => [0, 1, 0, 0]
    };

    a[theta as usize]
}

#[allow(unused_variables)]
fn rotation_delta(ty: PieceType, theta: Theta, x: u8) -> u8 {
    match (ty, theta) {
        (_, R0) | (I, R180) | (S, R180) | (Z, R180) | (O, _) => 0,
        (_, R90) | (_, R270) => 1,
        _ => 2
    }
}

fn movement_delta(ty: PieceType, theta: Theta, x: u8) -> u8 {
    assert!(x < 10);

    let a = match (ty, theta) {
        (I, R0) | (I, R180) | (J, R180) | (L, R180) =>
            [1, 2, 1, 0, 1, 2, 1, 0, 0, 0],
        (J, R0) | (L, R0) | (S, R0) | (S, R180) |
        (T, R0) | (T, R180) | (Z, R0) | (Z, R180) =>
            [1, 2, 1, 0, 1, 2, 2, 1, 0, 0],
        (J, R90) | (L, R90) | (T, R90) =>
            [1, 1, 2, 1, 0, 1, 2, 2, 1, 0],
        (J, R270) | (L, R270) | (T, R270) =>
            [1, 2, 1, 0, 1, 2, 2, 1, 1, 0],
        (S, R90) | (S, R270) | (Z, R90) | (Z, R270) =>
            [1, 1, 1, 0, 0, 1, 2, 1, 1, 0],
        (I, _) =>
            [1, 1, 1, 1, 0, 0, 1, 1, 1, 1],
        (O, _) =>
            [1, 2, 2, 1, 0, 1, 2, 2, 1, 0],
    };

    a[x as usize]
}

/// Returns the minimal number of rotation and movements to reach the
/// specified destination from the default entry-point.
//
// It would be great if this could be improved to calculate finesse
// dynamically for arbitrary rotation systems and field widths.
//
// Overhang finesse would be exceptionally cool.
pub fn minimal_moves_required(ty: PieceType, theta: Theta, x: u8) -> (u8, u8) {
    let delta = position_delta(ty, theta);

    (
        rotation_delta(ty, theta, delta + x),
        movement_delta(ty, theta, delta + x)
    )
}
