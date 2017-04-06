#pragma once


#ifndef _UTILITYFUNCTIONS_H_
#define _UTILITYFUNCTIONS_H_

//////////////////////////////////////////////////////////////////////////

#ifndef _UTILITYTYPES_H_
#include "UtilityTypes.h"

#endif

//////////////////////////////////////////////////////////////////////////

// Based on the code of Roman Pasechnik. The logic has had no changes. I have simply adapted it to work with my LongInteger class
//
// Using a set of LongIntegers instead of a set of ints seems to have worked just fine without any further changes.
//
// The original comments have been left in the code

class UtilityFunctions
{
public:
	// WD - not sure why this function is here. It didn't seem to get called in the original code so dunno.
	template<typename T>
	static T log2(T _n)
	{
		return log(_n) * 1.4426950408889634;
	}



	// The bitcount for ints is not called anymore, but left in for reference. Might delete it later. Not decided.

	/* Returns the number of one-bits in the two's complement binary
	* representation of the specified value.  This function is
	* sometimes referred to as the population count. */
	static int BitCount(int _n)
	{
		_n = _n - (unsigned(_n >> 1) & 0x55555555);
		_n = (_n & 0x33333333) + (unsigned(_n >> 2) & 0x33333333);
		_n = (_n + unsigned(_n >> 4)) & 0x0f0f0f0f;
		_n = _n + unsigned(_n >> 8);
		_n = _n + unsigned(_n >> 16);
		return _n & 0x3f;
	}

	static const int bitCount[];

	static LongInteger BitCount(const LongInteger &lin);

	/* Sorting numbers keeps the resulting products more nearly equal in size
	* than would the simpler approach.
	* Nearly equal products are more efficient for the current multiply implementation.
	* Such sorting is more efficient than repeated N?1 multiplies since it forms big multiplies,
	* allowing Karatsuba and higher algorithms to be used. And even below the Karatsuba threshold
	* a big block of work can be more efficient for the base case algorithm. */
	// Above comment is from Roman Pasachnik. I've implemented Karatsuba and ToomCook3 and multithreading for
	// the multiplication. Using multiset seems to give quite good performance, at least relative to the time
	// taken for multiplication. Once the set gets big enough for the updates to be noticeable the set is so
	// big that the multiplication - even with optimisations - is substantial. Nonetheless I'll look at quicker
	// alternatives. Maybe a FIFO queue?
	template<typename _InputIterator>
	static LongInteger SequenceProduct(const _InputIterator& _begin,
		const _InputIterator& _end)
	{
		// First copy collection to sorted container
		LongIntSet sorted;
		for (_InputIterator it = _begin; it != _end; ++it) {
			sorted.insert(*it);
		}

		// Start from smaller numbers
		// Get first and second smallest numbers
		LongIntSet::iterator itFirst = sorted.begin();
		if (itFirst == sorted.end())
			return 0;

		LongIntSet::iterator itSecond = itFirst;
		++itSecond;

		while (itSecond != sorted.end())
		{
			// Multiply them
			LongInteger prod = (*itFirst) * (*itSecond);

			// Remove them from container
			sorted.erase(itFirst);
			sorted.erase(itSecond);

			// Put their product in container
			sorted.insert(prod);

			// And repeat all this stuff
			itFirst = sorted.begin();
			itSecond = itFirst;
			++itSecond;
		}

		// After all of this we have single element in sorted
		// container -- product of all elements of sequence.
		return *itFirst;


		// This implementation may be optimized further, I guess.
		// Maybe using of std::set is not a best choice.
	}


//	template<typename _InputIterator>
	static LongInteger SequenceProduct2(const LongIntMap::iterator& _begin,
		const LongIntMap::iterator& _end);

};

#endif

