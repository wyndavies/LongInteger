#ifdef _WIN32
#include "stdafx.h"
#endif
#include "PrimeSieve.h"

#include <cmath>

// Based on the code of Roman Pasechnik. The logic has had no changes. I have simply adapted it to work with my LongInteger class

// This is the code that calculates the prime factors.
// My maths isn't great and Pasechnik didn't put much in the way of comments in the code, so I couldn't explain how the logic works
// The original comments have been left in the code


PrimeSieve::PrimeSieve(const LongInteger& lin)
{
	MakePrimeList(lin);
}

//////////////////////////////////////////////////////////////////////////


void PrimeSieve::SieveOfEratosthenes(BoolVec& composite)
{
	int d1 = 8;
	int d2 = 8;
	int p1 = 3;
	int p2 = 7;
	int s1 = 7;
	int s2 = 3;
	int n = 0;
	int len = (int)composite.size();
	bool toggle = false;

	while (s1 < len) // -- scan sieve
	{
		if (!composite[n++]) // -- if a prime is found
		{ // -- cancel its multiples
			int inc = p1 + p2;

			for (int k = s1; k < len; k += inc)
				composite[k] = true;

			for (int k = s1 + s2; k < len; k += inc)
				composite[k] = true;
		}

		if (toggle = !toggle) // Never mind, it's ok.
		{
			s1 += d2;
			d1 += 16;
			p1 += 2;
			p2 += 2;
			s2 = p2;
		}
		else
		{
			s1 += d1;
			d2 += 8;
			p1 += 2;
			p2 += 6;
			s2 = p1;
		}
	}
}


void PrimeSieve::MakePrimeList(const LongInteger& lin)
{
	// Fetch two first primes manually
	m_LIMPrimes[LongInteger(0)] = 2;
	m_LIMPrimes[LongInteger(1)] = 3;

	// Fetch vector representing composite numbers
	// [index] -- number
	// [value] -- is number composite or prime
	LongInteger loopMax = lin / 3;
	BoolVec composite;
	bool b = false;
	for (LongInteger i = 0; i < loopMax; ++i)
	{
		composite.push_back(b);
	}
	SieveOfEratosthenes(composite);
	bool toggle = false;

	// Fetch prime numbers
	LongInteger p = 5;
	LongInteger index = 2;
	UINT i = 0;
	while (p <= lin)
	{
		if (!composite[i++])
		{
			m_LIMPrimes[index] = p;
			++index;
		}

		// -- never mind, it's ok.
		p += (toggle = !toggle) ? 2 : 4;
	}

	// Number of primes
	m_NumberOfPrimes = index;

	// WD - Add a blank entry at the end. Needed as the algorithms continue until they go 1 past the last prime.
	m_LIMPrimes[index + 1] = LongInteger(0);
}


LongInteger PrimeSieve::Primorial(const LongInteger& low, const LongInteger& high)
{
	// WD - changed quite a bit in terms of actual code, but the logic is the same as the original algorithm
	//      It is finding the start and end markers to specify the range where all the primes between 'low' and 'high' are
	//      and then calling SequenceProduct to multiply them together.
	LongInteger liMinIdx, liMaxIdx;
	liMinIdx = GetPrimeIndex(low, LongInteger(3), LongInteger(m_NumberOfPrimes));
	liMaxIdx = GetPrimeIndex(high, LongInteger(liMinIdx), LongInteger(m_NumberOfPrimes));

	auto beginning = m_LIMPrimes.find(liMinIdx);
	auto ending = m_LIMPrimes.find(liMaxIdx);

	return UtilityFunctions::SequenceProduct2(beginning, ending);
}


LongInteger PrimeSieve::GetPrimeIndex(const LongInteger& number, LongInteger lowerBound, LongInteger upperBound)
{
	// Binary search
	while (lowerBound < upperBound)
	{
		LongInteger mid = (lowerBound + upperBound) >> 1;
		if (m_LIMPrimes[mid] < number)
			lowerBound = mid + 1;
		else
			upperBound = mid;
	}

	if (lowerBound >= m_NumberOfPrimes)
		return lowerBound;

	if (m_LIMPrimes[lowerBound] == number)
		lowerBound++;

	return lowerBound;
}

