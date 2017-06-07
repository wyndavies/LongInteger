#ifdef _WIN32
#include "stdafx.h"
#endif
#include "PrimeSwing.h"

#include <cmath>

// Based on the code of Roman Pasechnik. The logic has had no changes. I have simply adapted it to work with my LongInteger class

// This is the code that calculates the factorials. If n < 20 it uses the normal method, otherwise it uses Roman Pasechnik implementation.
// My maths isn't great and Pasechnik didn't put much in the way of comments in the code, so I couldn't explain how the logic works
// The original comments have been left in the code

PrimeSwing::PrimeSwing()
{
}


PrimeSwing::~PrimeSwing()
{
}


const int PrimeSwing::smallOddSwing[] = {
	1, 1, 1, 3, 3, 15, 5, 35, 35, 315, 63, 693, 231, 3003, 429,
	6435, 6435, 109395, 12155, 230945, 46189, 969969, 88179,
	2028117, 676039, 16900975, 1300075, 35102025, 5014575,
	145422675, 9694845, 300540195, 300540195 };


LongInteger PrimeSwing::Factorial(const LongInteger& number)
{
	LongInteger result;

	// For very small n the 'NaiveFactorial' is OK.
	if (number < 20)
	{
		result = 1;
		for (int i = 2; i <= number; i++)
		{
			result *= i;
		}
		return result;
	}

	PrimeSieve sieve(number);
	RecFactorial(result, number, sieve);
	result <<= (number - UtilityFunctions::BitCount(number));

	return result;
}


void PrimeSwing::RecFactorial(LongInteger& result, const LongInteger& number, PrimeSieve& sieve)
{
	// Stop recursion here
	if (number < 2)
	{
		result = 1;
		return;
	}

	// Continue calculating recursively
	RecFactorial(result, number / 2, sieve);
	result *= result;
	result *= Swing(number, sieve);
}



LongInteger PrimeSwing::Swing(const LongInteger& number, PrimeSieve& sieve)
{
	// Small precalculated values
	if (number < 33) {
		return smallOddSwing[(int)number];
	}

	// Fetch multiplies
	LongIntVec multiplies = GetMultiplies(number, sieve);

	// Return multiplies of primorials
	return sieve.Primorial(number / 2, number) *
		UtilityFunctions::SequenceProduct(multiplies.begin(), multiplies.end());
}


LongIntVec PrimeSwing::GetMultiplies(const LongInteger& number, PrimeSieve& sieve)
{
	// This will need to be modified once I have a vector-equivalent class that can be indexed by LongIntegers

	LongIntVec multiplies;
	// Ah-ha - need to get a square root function for LongInteger
	LongInteger sqrtN = LongInteger::sqrt(number);

	LongInteger maxIdx = sieve.GetPrimeIndex(sqrtN, LongInteger(2), sieve.GetNumberOfPrimes());
	LongInteger prime,q,p;
	for (LongInteger i = 1; i < maxIdx; ++i)
	{
		prime = sieve.GetPrime(i);

		q = number;
		p = 1;

		while ((q /= prime) > 0)
			if ((q.getDigit(0) & 1) == 1)
				p *= prime;

		if (p > 1)
			multiplies.push_back(p);
	}

	LongInteger minIdx = maxIdx;
	maxIdx = sieve.GetPrimeIndex(number / 3, minIdx, sieve.GetNumberOfPrimes());

	for (LongInteger i = minIdx; i < maxIdx; ++i)
	{
		prime = sieve.GetPrime(i);

		if (((number / prime) & 1) == 1)
			multiplies.push_back(prime);
	}

	return multiplies;
}
