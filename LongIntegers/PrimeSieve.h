#pragma once

#ifndef _PRIMESIEVE_H_
#define _PRIMESIEVE_H_

#ifndef _UTILITYFUNCTIONS_H_
#include "UtilityFunctions.h"
#endif
#ifdef _WIN32
#include "stdafx.h"
#endif
#include "LongInteger.h"

// Based on the code of Roman Pasechnik. Adapted to work with my LongInteger class

// Will need to replace the vector class with a user defined type as a vector has a max size determined by size_t (which
// is usually the same as int
// Will also need to change quite a few of the variables from ints to LongIntegers

class PrimeSieve
{
public:
	/** Constructor.
	Constructs a prime sieve for the integer range [1,n]. */
	PrimeSieve(int i);
	PrimeSieve(LongInteger li);


	/** Get prime by index */
	LongInteger GetPrime(int index) const { return m_LIPrimes[index]; }
	
	/** Returns prime index in collection */
	int GetPrimeIndex(int number, int lowerBound, int upperBound) const;
	int GetPrimeIndex(LongInteger number, int lowerBound, int upperBound) const;
	
	/** Get number of prime numbers in sieve */
	const LongInteger GetNumberOfPrimes() const { return m_NumberOfPrimes; }
	const int GetNumberOfPrimes() const { return m_NumberOfPrimes; }

	/** Gives the product of primes in range [_low, _high] */
	LongInteger Primorial(int low, int high);
	LongInteger Primorial(LongInteger low, LongInteger high);



private:

		/** Get a high bound for pi(n), the number of primes less or equal n. */
	static int GetPiHighBound(double n);


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
	void MakePrimeList(int n);
	void MakePrimeList(LongInteger li);


	//////////////////////////////////////////////////////////////////////////


	/** Prime numbers */
	LongIntVec m_LIPrimes;


	/** Number of primes in collection */
	int m_NumberOfPrimes;

};


#endif
