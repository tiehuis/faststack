// test_randomizer.c
// =================
//
// Tests the implemented randomizer against their theoretical probablities
// for generating each piece. All output is dumped every run which can help
// when comparing the behaviour of different randomizers.
//
// See this thread for some useful reading:
//  https://tetrisconcept.net/threads/randomizer-theory.512/
//
// Variances
// ---------
//
// Memoryless: 42
// 63-bag: 34 + 82/275 = 34.29818181...
// NES (ideal): 32 + 2/3 = 32.66666666...
// Cultris II (sample): ~27.65
// 28-bag: 27 + 13/25 = 27.52
// 4 history 2 roll: ~20.56697
// 14-bag: 19
// 5-bag: 18
// 10-bag: 16 + 32/35 = 16.91428571...
// 7 from 8: 15 + 82/175 = 15.46857142...
// 1+7: 12 + 13/14 = 12.92857142...
// 6-bag: 12 + 5/6 = 12.83333333...
// 3 history strict: 12
// 8-bag: 11 + 43/56 = 11.76785714...
// 4 history 4 roll (TGM1): 10.13757557...
// 7-bag: 8
// 7-bag with seam match check: 7.5
// 4 history 6 roll (TGM2): 7.34494156...
// 4 history strict: 6
// TGM3 (sample): ~5.31

#define VARIANCE_MEMORYLESS 42
#define VARIANCE_6_BAG 12.8333f
#define VARIANCE_7_BAG 8
#define VARIANCE_7_BAG_SEAM_CHECK 7.5f
#define VARIANCE_14_BAG 19
#define VARIANCE_28_BAG 27.52f
#define VARIANCE_63_BAG 34.29818f
#define VARIANCE_TGM1 10.13757557f
#define VARIANCE_TGM2 7.34494156f
#define VARIANCE_TGM3 5.31f

#define VARIANCE_ALLOWANCE 0.1f

#include "framework.h"
#include "faststack.h"
#include <stdlib.h>
#include <stdio.h>

static FSEngine engine;

static const char *pieceTypeNames[] = {
    "I",
    "J",
    "L",
    "O",
    "S",
    "T",
    "Z",
    "None"
};

// Compute the distribution across a number of samples with the current
// randomizer.
static void test_distribution(int randomizerType, double targetVariance)
{
    engine.randomizer = randomizerType;

    const uint64_t limit = 10000000;   // 10 Million

    // How many times a specific piece has been seen
    uint64_t seen[FS_NPT] = {0};

    // The last index at which the specified piece was seen. For variance.
    uint64_t lastSeen[FS_NPT] = {0};

    // Variance Sum and Sum of Squares.
    //
    // Will this overflow? The worst case would computing the square of the
    // distance between 0 and the limit (100 million). This is < 2^64 so we
    // are ok.
    uint64_t varSum = 0;
    uint64_t varSumSq = 0;

    for (uint64_t i = 0; i < limit; ++i) {
        const FSBlock ty = fsNextRandomPiece(&engine);
        const uint64_t x = i - lastSeen[ty];

        varSum += x;
        varSumSq += x * x;

        seen[ty] += 1;
        lastSeen[ty] = i;
    }

    printf(" = Distribution\n");
    for (int i = 0; i < FS_NPT; ++i) {
        const double weight = ((double) seen[i] * 100) / limit;
        printf("    %s - %2.3f%%\n", pieceTypeNames[i], weight);
    }

    printf(" = Variance\n");
    const double variance = (double) (varSumSq - (varSum * varSum) / limit)
                                        / (limit - 1);
    printf("    target = %2.3f\n", targetVariance);
    printf("    actual = %2.3f\n", variance);
}

static void test_simple(void)
{
    printf("\nSimple Randomizer\n");
    test_distribution(FST_RAND_SIMPLE, VARIANCE_MEMORYLESS);
}

static void test_bag7(void)
{
    printf("\nBag7 Randomizer\n");
    test_distribution(FST_RAND_BAG7, VARIANCE_7_BAG);
}

static void test_bag7_seam_check(void)
{
    printf("\nBag7 Seam Check Randomizer\n");
    test_distribution(FST_RAND_BAG7_SEAM_CHECK, VARIANCE_7_BAG_SEAM_CHECK);
}

static void test_bag6(void)
{
    printf("\nBag6 Randomizer\n");
    test_distribution(FST_RAND_BAG6, VARIANCE_6_BAG);
}

static void test_multi_bag2(void)
{
    printf("\nBag14 Randomizer\n");
    test_distribution(FST_RAND_MULTI_BAG2, VARIANCE_14_BAG);
}

static void test_multi_bag4(void)
{
    printf("\nBag28 Randomizer\n");
    test_distribution(FST_RAND_MULTI_BAG4, VARIANCE_28_BAG);
}

static void test_multi_bag9(void)
{
    printf("\nBag63 Randomizer\n");
    test_distribution(FST_RAND_MULTI_BAG9, VARIANCE_63_BAG);
}

static void test_tgm1(void)
{
    printf("\nTGM1 Randomizer\n");
    test_distribution(FST_RAND_TGM1, VARIANCE_TGM1);
}

static void test_tgm2(void)
{
    printf("\nTGM2 Randomizer\n");
    test_distribution(FST_RAND_TGM1, VARIANCE_TGM2);
}

static void test_tgm3(void)
{
    printf("\nTGM3 Randomizer\n");
    test_distribution(FST_RAND_TGM3, VARIANCE_TGM3);
}

int main(void)
{
    engine.lastRandomizer = FST_RAND_UNDEFINED;
    fsRandSeed(&engine.randomContext, fsGetRoughSeed());

    test_simple();
    test_bag7();
    test_bag7_seam_check();
    test_bag6();
    test_multi_bag2();
    test_multi_bag4();
    test_multi_bag9();
    test_tgm1();
    test_tgm2();
    test_tgm3();
}
