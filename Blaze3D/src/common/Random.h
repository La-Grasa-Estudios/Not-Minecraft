#pragma once

#include <cstdint>

#ifndef RANDOM_H
#define RANDOM_H

class Random
{
private:
	uint64_t seed;
	bool haveNextNextGaussian = false;
	double nextNextGaussian = 0.0;
	static const long multiplier = (const long)0x5DEECE66DL;
	static const long addend = 0xBL;
	static const long mask = (1L << 30L) - 1L;
	int next(int bits);
public:
	Random(uint64_t seed);
	Random();
	int NextInt(int max);
	int NextInt();
	int64_t NextLong();
	float NextFloat();
	double NextDouble();
	double NextGaussian();
	bool NextBoolean();
	void SetSeed(uint64_t seed);
	static Random GlobalRand;
};


#endif

