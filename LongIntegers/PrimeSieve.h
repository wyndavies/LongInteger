#pragma once

#ifndef _PRIMESIEVE_H_
#define _PRIMESIEVE_H_

#include "UtilityFunctions.h"
#ifdef _WIN32
#include "stdafx.h"
#endif
#include "LongInteger.h"

// Based on the code of Roman Pasechnik. The logic has had no changes. I have simply adapted it to work with my LongInteger class

// Will need to replace the vector class with a user defined type as a vector has a max size determined by size_t (which
// is usually the same as int
// Will also need to change quite a few of the variables from ints to LongIntegers

class PrimeSieve
{
public:
	/** Constructor.
	Constructs a prime sieve for the integer range [1,n]. */
	PrimeSieve(const LongInteger& li);

	/** Get prime by index */
	LongInteger GetPrime(LongInteger index) { return m_LIMPrimes[index]; }
	
	/** Returns prime index in collection */
	LongInteger GetPrimeIndex(const LongInteger& number, LongInteger lowerBound, LongInteger upperBound);

	/** Get number of prime numbers in sieve */
	LongInteger GetNumberOfPrimes() { return m_NumberOfPrimes; }

	/** Gives the product of primes in range [_low, _high] */
	LongInteger Primorial(const LongInteger& low, const LongInteger& high);



private:

	//////////////////////////////////////////////////////////////////////////


	/** Prime number sieve, Eratosthenes (276-194 b. t.) This implementation
	considers only multiples of primes greater than 3, so the smallest value
	has to be mapped to 5.

	Note: There is no multiplication operation in this function.

	After execution of the function this boolean array includes
	all composite numbers in [5,n] disregarding multiples of 2 and 3. */
	static void SieveOfEratosthenes(BoolVec& composite);


	//////////////////////////////////////////////////////////////////////////


	/** Transforms the sieve of Eratosthenes
	into the sequence of prime numbers. */
	void MakePrimeList(const LongInteger& li);


	//////////////////////////////////////////////////////////////////////////


	/** Prime numbers */
	LongIntMap m_LIMPrimes;

	/** Number of primes in collection */
	LongInteger m_NumberOfPrimes;

};


#endif
