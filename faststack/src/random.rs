//! Functions dealing with random piece generation.
//!
//! This uses a fixed RNG for reproducible replays from a single seed.

use primitives::PieceType;
use primitives::PieceType::*;
use constants::PIECE_TYPE_COUNT;

/// Low-level random context.
pub struct LLRand {
    a: u32,
    b: u32,
    c: u32,
    d: u32,
}

impl LLRand {
    /// Construct a new low-level randomizer.
    pub fn new(seed: u32) -> LLRand {
        let mut r = LLRand { a: 0xF1EA5EED, b: seed, c: seed, d: seed };

        for _ in 0..20 {
            r.next();
        }

        r
    }

    /// Generate a random u32 from the randomizer.
    pub fn next(&mut self) -> u32 {
        let e = self.a.wrapping_sub(self.b.rotate_left(27));
        self.a = self.b ^ self.c.rotate_left(17);
        self.b = self.c.wrapping_add(self.d);
        self.c = self.d.wrapping_add(e);
        self.d = self.a.wrapping_add(e);
        self.d
    }

    /// Generate an unbiased random number within the specified range.
    ///
    /// Requires lo <= hi else panic.
    pub fn in_range(&mut self, lo: u32, hi: u32) -> u32 {
        assert!(lo <= hi);

        let range = hi - lo;
        let rem = ::std::u32::MAX % range;

        let mut x;
        loop {
            x = self.next();
            if x < ::std::u32::MAX - rem {
                break;
            }
        }

        lo + x % range
    }

    /// Perform an unbiased shuffle on the specified input slice.
    pub fn shuffle<T>(&mut self, a: &mut [T]) where T: Ord + Copy {
        for i in (0..a.len()).rev() {
            let j = self.in_range(0, (i + 1) as u32) as usize;
            let t = a[j];
            a[j] = a[i];
            a[i] = t;
        }
    }
}

/// The type of Randomizer to use when generating pieces.
pub enum RandomizerType {
    Simple,
    Bag7
}

pub struct Randomizer {
    /// Low-level randomizer context
    context: LLRand,

    /// The type of randomizer used
    ty: RandomizerType,

    /// Buffer workspace for storing incoming pieces
    buffer: [PieceType; 35],

    /// Current index into buffer
    buffer_index: usize
}

impl Randomizer {
    /// Construct a new `Randomizer` of the specified type.
    pub fn new(context: LLRand, ty: RandomizerType) -> Randomizer {
        let mut r = Randomizer {
            context, ty,
            buffer: [I; 35],
            buffer_index: 0
        };

        match r.ty {
            RandomizerType::Simple => {}

            RandomizerType::Bag7 => {
                for i in 0..PIECE_TYPE_COUNT {
                    r.buffer[i] = i.into();
                }

                loop {
                    {
                        let ar = &mut r.buffer[0..PIECE_TYPE_COUNT];
                        r.context.shuffle(ar);
                    }

                    match r.buffer[0] {
                        S | Z | O => (),
                        _ => break
                    }
                }
            }
        }

        r
    }

    /// Generate the next piece in the current `Randomizer`'s sequence.
    pub fn next(&mut self) -> PieceType {
        match self.ty {
            RandomizerType::Simple => {
                let p = self.context.in_range(0, PIECE_TYPE_COUNT as u32);
                p.into()
            }

            RandomizerType::Bag7 => {
                let p = self.buffer[self.buffer_index];
                self.buffer_index += 1;

                if self.buffer_index >= PIECE_TYPE_COUNT {
                    {
                        let ar = &mut self.buffer[0..PIECE_TYPE_COUNT];
                        self.context.shuffle(ar);
                    }
                    self.buffer_index = 0;
                }

                p
            }
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_low_level_randomizer_behaviour_next() {
        let mut r = LLRand::new(0);
        let buffer =
            (0..10).fold(
                    Vec::new(), |mut a, _| {
                        a.push(r.in_range(0, PIECE_TYPE_COUNT as u32));
                        a
                    });

        assert_eq!(buffer, [5, 1, 1, 5, 5, 4, 4, 1, 0, 4]);
    }

    #[test]
    fn test_low_level_randomizer_behaviour_shuffle() {
        let mut r = LLRand::new(0);
        let mut buffer = (0..10).collect::<Vec<_>>();
        r.shuffle(&mut buffer);

        assert_eq!(buffer, [7, 8, 0, 4, 9, 2, 5, 6, 3, 1]);
    }

    #[test]
    fn test_simple_randomizer_sequence() {
        let mut r = Randomizer::new(LLRand::new(0), RandomizerType::Simple);
        let buffer =
            (0..10).fold(
                    Vec::new(), |mut a, _| {
                        a.push(r.next());
                        a
                    });

        assert_eq!(buffer, [T, J, J, T, T, S, S, J, I, S]);
    }

    #[test]
    fn test_bag7_randomizer_sequence() {
        for i in 0..100 {
            let mut r = Randomizer::new(LLRand::new(i), RandomizerType::Bag7);
            let sum = (0..PIECE_TYPE_COUNT).fold(0, |a, _| a + r.next() as u8);

            assert_eq!(21, sum);
        }
    }

    // TODO: Add randomizer distribution tests.
}
