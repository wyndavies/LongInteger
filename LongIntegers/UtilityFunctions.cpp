#include "stdafx.h"
#include "UtilityFunctions.h"
#include "LongInteger.h"

// Based on code by Roman Pasechnik for his PrimeSwing algorithm for calculating factorials
// The code has been changed somewhat as it was originally meant for numbers held as 2's compliment.
// My LongInteger class uses a separate flag for positive/negative and the number is stored in the same
// way regardless of sign. Hence I modified the code. I used an array instead of calculating the number
// of bytes each time.

const int UtilityFunctions::bitCount[] = { 0,1,1,2,1,2,2,3,1,2,2,3,2,3,3,4,
1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,
1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,
2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,
2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,
2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
4,5,5,6,5,6,6,7,5,6,6,7,6,7,7,8
};


LongInteger UtilityFunctions::BitCount(const LongInteger &lin)
{
	// Population count of set bits in a LongInteger
	// As the digit[] array is the same for positive and negative we don't need to worry about 2's compliment notation
	LongInteger popCount = 0;

	for (UINT i = 0; i < lin.getSize(); i++)
	{
		popCount += (bitCount[lin.getDigit(i)]);
	}

	return popCount;
}


LongInteger UtilityFunctions::SequenceProduct2(const LongIntMap::iterator& _begin,
	const LongIntMap::iterator& _end)
{
	// First copy collection to sorted container
	LongIntSet sorted;
	for (auto it = _begin; it != _end; ++it) {
		if (it->second.equalsZero())
			continue;
		sorted.insert(it->second);
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
}
