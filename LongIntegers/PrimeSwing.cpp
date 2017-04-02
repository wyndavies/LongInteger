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

LongInteger PrimeSwing::Factorial(int number)
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
	RecFactorial(result, (int)number, sieve);
	result <<= (number - UtilityFunctions::BitCount(number));

	return result;
}


void PrimeSwing::RecFactorial(LongInteger& result, int number, PrimeSieve& sieve)
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

void PrimeSwing::RecFactorial(LongInteger& result, LongInteger& number, PrimeSieve& sieve)
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


LongInteger PrimeSwing::Swing(int number, PrimeSieve& sieve)
{
	// Small precalculated values
	if (number < 33) {
		return smallOddSwing[number];
	}

	// Fetch multiplies
	IntVec multiplies = GetMultiplies(number, sieve);

	// Return multiplies of primorials
	return sieve.Primorial(number / 2, number) *
		UtilityFunctions::SequenceProduct(multiplies.begin(), multiplies.end());
}

LongInteger PrimeSwing::Swing(LongInteger& number, PrimeSieve& sieve)
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


IntVec PrimeSwing::GetMultiplies(int number, PrimeSieve& sieve)
{
	IntVec multiplies;
	int sqrtN = static_cast<int>(sqrt(static_cast<double>(number)));

	LongInteger limaxIdx = sieve.GetPrimeIndex(sqrtN, 2, (int)sieve.GetNumberOfPrimes());
	int maxIdx = (int)limaxIdx;

	for (int i = 1; i < maxIdx; ++i)
	{
		int prime = (int)sieve.GetPrime(i);

		int q = number, p = 1;

		while ((q /= prime) > 0)
			if ((q & 1) == 1)
				p *= prime;

		if (p > 1)
			multiplies.push_back(p);
	}

	int minIdx = maxIdx;
	maxIdx = sieve.GetPrimeIndex(number / 3, minIdx, (int)(sieve.GetNumberOfPrimes()));

	for (int i = minIdx; i < maxIdx; ++i)
	{
		int prime = (int)sieve.GetPrime(i);

		if (((number / prime) & 1) == 1)
			multiplies.push_back(prime);
	}

	return multiplies;
}


LongIntVec PrimeSwing::GetMultiplies(LongInteger& number, PrimeSieve& sieve)
{
	// This will need to be modified once I have a vector-equivalent class that can be indexed by LongIntegers

	LongIntVec multiplies;
	// Ah-ha - need to get a square root function for LongInteger
	LongInteger sqrtN = LongInteger::sqrt(number);

	LongInteger maxIdx = sieve.GetPrimeIndex(sqrtN, 2, (int)sieve.GetNumberOfPrimes());
	LongInteger* prime = new LongInteger();
	LongInteger* q = new LongInteger();
	LongInteger* p = new LongInteger();
	for (LongInteger i = 1; i < maxIdx; ++i)
	{
		*prime = sieve.GetPrime((int)i);

		*q = number;
		*p = 1;

		while ((*q != prime) > 0)
			if ((q->getDigit(0) & 1) == 1)
				*p *= prime;

		if (*p > 1)
			multiplies.push_back(*p);
	}

	LongInteger minIdx = maxIdx;
	maxIdx = sieve.GetPrimeIndex(number / 3, (int)minIdx, (int)sieve.GetNumberOfPrimes());

	for (LongInteger i = minIdx; i < maxIdx; ++i)
	{
		LongInteger prime = sieve.GetPrime((int)i);

		if (((number / prime) & 1) == 1)
			multiplies.push_back(prime);
	}

	delete prime;
	delete p;
	delete q;

	return multiplies;
}
