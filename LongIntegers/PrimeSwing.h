#pragma once

// Based on the class created by Roman Pasechnik. Modified to use my LongInteger class instead of ints


#ifndef _PRIMESWING_H_
#define _PRIMESWING_H_

#ifndef _PRIMESIEVE_H_
#include "PrimeSieve.h"
#endif

class PrimeSwing
{
public:
	PrimeSwing();
	~PrimeSwing();


	/** Computes factorial of integer number */
//	LongInteger Factorial(int number);
	LongInteger Factorial(const LongInteger& number);
protected:
//	LongInteger Swing(int number, PrimeSieve& sieve);
	LongInteger Swing(const LongInteger& number, PrimeSieve& sieve);

private:
	/** Computes factorial of integer number recursively */
//	virtual void RecFactorial(LongInteger& result, int number, PrimeSieve& sieve);
	virtual void RecFactorial(LongInteger& result, const LongInteger& number, PrimeSieve& sieve);


	/** Gets multiplies for Swing function. */
//	IntVec GetMultiplies(int number, PrimeSieve& sieve);
	LongIntVec GetMultiplies(const LongInteger& number, PrimeSieve& sieve);

	static const int smallOddSwing[];
};

#endif