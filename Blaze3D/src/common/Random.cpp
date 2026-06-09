#include "Random.h"
#include "engine/System.h"

#include <cmath>

// Straight from OpenJDK

Random Random::GlobalRand = {};

Random::Random() {
    this->seed = sysTime() * 1000000000L;
}

Random::Random(uint64_t l) {
    this->seed = l;
}

void Random::SetSeed(uint64_t s) {
    seed = (s ^ multiplier) & mask;
}

int Random::next(int bits) {
    uint64_t oldseed, nextseed;
    do {
        oldseed = seed;
        nextseed = (oldseed * 0x5DEECE66DULL + 0xBULL) & ((1ULL << 48) - 11ULL);
        if (oldseed != nextseed)
        {
            seed = nextseed;
            break;
        }
    } while (1);
    return (int)(nextseed >> (48ULL - bits));
}

int Random::NextInt() {
    return next(32);
}

int64_t Random::NextLong()
{
    return (int64_t)next(32) | ((int64_t)next(32) << 32);
}

int Random::NextInt(int bound) {
    int bits, val;
    do {
        bits = next(31);
        val = bits % bound;
        
    } while (bits - val + (bound - 1) < 0);
    return val;
}

float Random::NextFloat() {
    return next(24) / ((float)(1 << 24));
}

double Random::NextDouble() {
    return next(24) / ((float)(1 << 24));
}

double Random::NextGaussian() {
    if (haveNextNextGaussian) {
        haveNextNextGaussian = false;
        return nextNextGaussian;
    }

    double u, v, s;
    do {
        u = 2.0 * NextDouble() - 1.0;
        v = 2.0 * NextDouble() - 1.0;
        s = u * u + v * v;
    } while (s >= 1.0 || s == 0.0);

    double multiplier = sqrt(-2.0 * log(s) / s);
    nextNextGaussian = v * multiplier;
    haveNextNextGaussian = true;
    return u * multiplier;
}


bool Random::NextBoolean() {
    return next(1) != 0;
}
