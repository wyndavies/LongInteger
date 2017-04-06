#ifndef _PRIMESIEVE_H_
#include "PrimeSieve.h"
#endif

#include <cmath>

// Based on the code of Roman Pasechnik. The logic has had no changes. I have simply adapted it to work with my LongInteger class

// This is the code that calculates the prime factors.
// My maths isn't great and Pasechnik didn't put much in the way of comments in the code, so I couldn't explain how the logic works
// The original comments have been left in the code


PrimeSieve::PrimeSieve(const LongInteger& lin)
//	: m_LIPrimes((int)GetPiHighBound(lin))
{
	// The estimate for the size is required otherwise I get out of bounds errors.
	MakePrimeList(lin);
}


int PrimeSieve::GetPiHighBound(double n)
{
	if (n < 17)
		return 6;
	return static_cast<int>(n / (log(n) - 1.5));
}

LongInteger PrimeSieve::GetPiHighBound(const LongInteger& lin)
{
	if (lin < 17)
	{
		return LongInteger(6);
	}

	// Let us try an approximation of the original function
	// This gives a slightly larger value than the original function, but not enough that I'm concerned about it
	LongInteger returnValue = LongInteger::ln(lin);
	returnValue *= 2;
	returnValue -= 3;
	returnValue = (lin * 2) / returnValue;

	return returnValue;
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


void PrimeSieve::MakePrimeList(const LongInteger& lin)
{
	// WD - 05/04/2017 - Started adding shadow code that populates the map

	// Fetch two first primes manually
	m_LIPrimes.push_back(2);
	m_LIPrimes.push_back(3);

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
	UINT i = 0, j = 2;
	while (p <= lin)
	{
		if (!composite[i++])
		{
			m_LIPrimes.push_back(p);
			++j;

			m_LIMPrimes[index] = p;
			index++;
		}

		// -- never mind, it's ok.
		p += (toggle = !toggle) ? 2 : 4;
	}

	// Number of primes
	m_NumberOfPrimes = j;

	// Add a blank entry at the end
	m_LIPrimes.push_back(LongInteger(0));
	m_LIMPrimes[index + 1] = LongInteger(0);
	int iTest = m_LIPrimes.size();
	int iTest2 = m_LIMPrimes.size();
}


LongInteger PrimeSieve::Primorial(const LongInteger& low, const LongInteger& high)
{
	int minIdx = GetPrimeIndex(low, 3, m_NumberOfPrimes);
	int maxIdx = GetPrimeIndex(high, minIdx, m_NumberOfPrimes);

	auto returnthingy = UtilityFunctions::SequenceProduct(&m_LIPrimes[minIdx],
		&m_LIPrimes[maxIdx]);

	// Shadow code to test using std::map
	LongInteger liMinIdx, liMaxIdx;
	liMinIdx = GetPrimeIndex(low, LongInteger(3), LongInteger(m_NumberOfPrimes));
	liMaxIdx = GetPrimeIndex(high, LongInteger(liMinIdx), LongInteger(m_NumberOfPrimes));

	auto beginning = m_LIMPrimes.find(liMinIdx);
	auto ending = m_LIMPrimes.find(liMaxIdx);

	auto mapreturnthing = UtilityFunctions::SequenceProduct2(beginning, ending);

	if (returnthingy != mapreturnthing)
	{
		CString arsebiscuits = L"No";
	}

	return returnthingy;
}


int PrimeSieve::GetPrimeIndex(const LongInteger& number, int lowerBound, int upperBound) const
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


LongInteger PrimeSieve::GetPrimeIndex(const LongInteger& number, LongInteger& lowerBound, LongInteger& upperBound)
{
	// Binary search
	CString strLower = lowerBound.toDecimal();
	CString strUpper = upperBound.toDecimal();
	CString strNumber = number.toDecimal();
	while (lowerBound < upperBound)
	{
		LongInteger mid = (lowerBound + upperBound) >> 1;
		CString strMid = mid.toDecimal();
		CString strValue = m_LIMPrimes[mid].toDecimal();
		if (m_LIMPrimes[mid] < number)
			lowerBound = mid + 1;
		else
			upperBound = mid;
		strLower = lowerBound.toDecimal();
		strUpper = upperBound.toDecimal();
	}

	if (lowerBound >= m_NumberOfPrimes)
		return lowerBound;

	if (m_LIMPrimes[lowerBound] == number)
		lowerBound++;

	strLower = lowerBound.toDecimal();
	return lowerBound;
}

