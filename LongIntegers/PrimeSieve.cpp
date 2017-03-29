#ifndef _PRIMESIEVE_H_
#include "PrimeSieve.h"
#endif

#include <cmath>

PrimeSieve::PrimeSieve(int n)
	: m_LIPrimes(GetPiHighBound(n))
{
	MakePrimeList(n);
}


PrimeSieve::PrimeSieve(LongInteger lin)
{
	MakePrimeList(lin);
}


int PrimeSieve::GetPiHighBound(double n)
{
	if (n < 17)
		return 6;
	return static_cast<int>(n / (log(n) - 1.5));
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
	int len = composite.size();
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


void PrimeSieve::MakePrimeList(int n)
{
	// Fetch two first primes manually
	m_LIPrimes[0] = 2;
	m_LIPrimes[1] = 3;

	// Fetch vector representing composite numbers
	// [index] -- number
	// [value] -- is number composite or prime
	BoolVec composite(n / 3);
	SieveOfEratosthenes(composite);
	bool toggle = false;

	// Fetch prime numbers
	int p = 5, i = 0, j = 2;
	while (p <= n)
	{
		if (!composite[i++])
		{
			m_LIPrimes[j] = p;
			++j;
		}

		// -- never mind, it's ok.
		p += (toggle = !toggle) ? 2 : 4;
	}

	// Number of primes
	m_NumberOfPrimes = j;
}


void PrimeSieve::MakePrimeList(LongInteger lin)
{
	// Fetch two first primes manually
	m_LIPrimes.push_back(2);
	m_LIPrimes.push_back(3);

	// Fetch vector representing composite numbers
	// [index] -- number
	// [value] -- is number composite or prime
//	BoolVec composite((int)(lin / 3)); // This won't work - what if lin is greater than maxint
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
	UINT i = 0, j = 2;
	while (p <= lin)
	{
		if (!composite[i++])
		{
			m_LIPrimes.push_back(p);
			++j;
		}

		// -- never mind, it's ok.
		p += (toggle = !toggle) ? 2 : 4;
	}

	// Number of primes
	m_NumberOfPrimes = j;
}


LongInteger PrimeSieve::Primorial(int low, int high)
{
	int minIdx = GetPrimeIndex(low, 3, m_NumberOfPrimes);
	int maxIdx = GetPrimeIndex(high, minIdx, m_NumberOfPrimes);

	return UtilityFunctions::SequenceProduct(&m_LIPrimes[minIdx],
		&m_LIPrimes[maxIdx]);
}


LongInteger PrimeSieve::Primorial(LongInteger low, LongInteger high)
{
	int minIdx = GetPrimeIndex(low, 3, m_NumberOfPrimes);
	int maxIdx = GetPrimeIndex(high, minIdx, m_NumberOfPrimes);

	return UtilityFunctions::SequenceProduct(&m_LIPrimes[minIdx],
		&m_LIPrimes[maxIdx]);
}


int PrimeSieve::GetPrimeIndex(int number, int lowerBound, int upperBound) const
{
	// Binary search

	while (lowerBound < upperBound)
	{
		int mid = (static_cast<unsigned int>(lowerBound + upperBound)) >> 1;

		if (m_LIPrimes[mid] < number)
			lowerBound = mid + 1;
		else
			upperBound = mid;
	}

	if (lowerBound >= static_cast<int>(m_LIPrimes.size()))
		return lowerBound;

	if (m_LIPrimes[lowerBound] == number)
		lowerBound++;

	return lowerBound;
}

int PrimeSieve::GetPrimeIndex(LongInteger number, int lowerBound, int upperBound) const
{
	// Binary search

	while (lowerBound < upperBound)
	{
		int mid = (static_cast<unsigned int>(lowerBound + upperBound)) >> 1;

		if (m_LIPrimes[mid] < number)
			lowerBound = mid + 1;
		else
			upperBound = mid;
	}

	if (lowerBound >= static_cast<int>(m_LIPrimes.size()))
		return lowerBound;

	if (m_LIPrimes[lowerBound] == number)
		lowerBound++;

	return lowerBound;
}

