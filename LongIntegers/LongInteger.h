/*
* LongInteger.h
*
* Header for LongInteger class. Can handle positive integers up to 4 billion digits in length.
* It holds the number as a byte array and indexes it using an uint (hence the size limit)
* Addition, subtraction, multiplication and division and powers are handled.
* Standard operators - +,-,*,/,+=,-=,/=,*=,<,>,<=,>=,!=,== are handled
* The = operator is overriden to provide copy by value instead of the default copy by reference
* It is also overloaded, so that strings and ints can be assigned to a LongInteger
*
* The karatsuba algorithm is used to multiply large numbers. It is more complex than long multiplication,
* but the number of steps needed doesn't increase as rapidly as the number length increases.
* The karatsuba algorithm is only accessed via the multiplyNumber method
*
* ToomCook3 algorithm has been added for multiplying even larger numbers. The implementation looks rather complex, so
* there may well be inefficiencies. Performance is odd, which convinces me all is not correct
*
* Burnikel-Ziegler algorithm for division has been implemented. It ended up with much more code than expected
* and also handles scenarios that aren't mentioned at all in Burnikel and Ziegler's paper, so that isn't a good sign.
* It works and divides numbers faster than the division algorithm I implemented for large numbers, where the divisor
* is at least half the size of the value being divided.
*
*
* The compiler needs to support C++14 to get this code to work.
* Visual Studio 2015 Community Edition supports this without any amendments needed.
*
* I got NetBeans to compile this code by going into
* Project Properties - C++ Compiler - Additional options - and added '-std=c++14 -pthread'. The -pthread is to
* add multithreading support otherwise NetBeans gets very confused.
* 
* Managed to get Eclipse to compile the code as well, although I cannot get the IDE to recognise C++14 code
* It insists there are errors, but hit compile and it builds fine. It will also debug and step through the code
* without issues, but still with warnings about unrecognised code all over the place.
*
* Unique Pointers are used because the memory management was getting out of hand and was becoming a real mess.
* Where it is manageable I've kept using simple points to reduce overheads.
*
*/

#pragma once
#include <vector>
#include <memory>
#include "QueueOfThreads.h"
#include "GeneralIntWrapper.h"

#ifndef _WIN32
/* Linux includes. Should cover all non-Windows platforms (and Windows, but Windows code was built using Microsoft-specific
   classes, such as CString), but only tested on Ubuntu 16 so far.
   For my reference - the preprocessor definitions for different platforms
   __linux__       Defined on Linux
   __sun           Defined on Solaris
   __FreeBSD__     Defined on FreeBSD
   __NetBSD__      Defined on NetBSD
   __OpenBSD__     Defined on OpenBSD
   __APPLE__       Defined on Mac OS X
   __hpux          Defined on HP-UX
   __osf__         Defined on Tru64 UNIX (formerly DEC OSF1)
   __sgi           Defined on Irix
   _AIX            Defined on AIX
   _WIN32          Defined on Windows

   I believe that __linux and linux work on the Linux platform as well.
*/
#include <string>
#include <cstring>
#include <sstream>
#include <algorithm>
typedef unsigned char byte; // Didn't realise byte and UINT were Microsoft specific definitions. Added them here for Linux
typedef unsigned int UINT;

using std::stringstream;
using std::string;
#endif

// This definition is to add compatibility with C++11. The code is intended for C++14, but it turns out
// the template below is all that is needed to run under the C++11 standard
#if __cplusplus == 201103L
template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args) {
	return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}
#endif

using std::vector;
using std::unique_ptr;
#if __cplusplus != 201103L
using std::make_unique;
#endif
using std::move;

// The class is named below with no implementation so as to avoid a Catch-22 situation with the typedefs
// The typedefs need to be defined using class LongInteger, but class LongInteger needs the typedefs
// So this definition says "I'm going to use a class called LongInteger and I'll fill in the details later on"
class LongInteger;
typedef GeneralIntWrapper<LongInteger> LongIntWrapper; // A shorthand to make it easier to type
typedef unique_ptr<LongInteger> LongIntegerUP; // Ditto
// This for passing a pointer to a function that takes 2 const LongInteger parameters and a bool and returns a pointer to a LongInteger
// The function pointer is referred to as 'Lifunction' in the code
typedef LongInteger* (*LIfunction)(const LongInteger&, const LongInteger&, bool); 

class LongInteger {
public:
	// Have these listed here (public and at the top of the class) whilst still working on them

	// Still in testing. Performance is weird in release build (improves dramatically at multiples of 350 and then drops 
	// rapidly until the next multiple is hit), but works as expected in debug build. Haven't worked out why.
	static LongInteger* ToomCook3(const LongInteger&, const LongInteger&, bool bBackgroundThread = false);

	// This is under development. The code is rather messy as I'm struggling to follow the paper due to the heavy usage of maths
	// terminology I'm unfamiliar with.
	// So far it shows that it is a lot quicker than long division if the divisor is at least half the size (number of digits)
	// of the value. Otherwise it is a lot slower. So working on a framework to adjust the size of the divisor and then
	// adjust the results. Despite being a lot more work it is still quicker. Needs quite a bit of testing
	// to work out the optimal values.
	static void BurnikelZiegler(const LongInteger&, const LongInteger&, LongIntegerUP&, LongIntegerUP&);
	// This is the framework that does the adjustments prior to calling the BurnikelZiegler method
	static bool DivAndMod(const LongInteger&, const LongInteger&, LongIntegerUP&, LongIntegerUP&);

private:
	// This is the front end to the Karatsuba algorithm which also tidies up any memory allocated
	static LongInteger multiplyChooser(const LongInteger&, const LongInteger&);
    // Restoring division seems to work fine. Some more testing needed to see what the performance is
	static void RestoringDivision(LongInteger&, LongInteger&, LongInteger*, LongInteger*);

public: // Make this public during development
		// This is the recursive part of the karatsuba algorithm
	static LongInteger* karatsuba(const LongInteger&, const LongInteger&, bool bBackgroundThread = false);

	// Constants
public:
	static const UINT BASEVAL = 256; // The effective base of the numbers (each digit is 1 byte)
	static const UINT BASEVALBITS = 8; // Size of each digit in bits
	static const UINT BASEVALBITSSHIFT = 3; // If dividing/multiplying by basevalbits, using shifts
	static const byte BASEMAX = 255; // Highest value that can be stored in each digit
	static const UINT ABSMAXSIZE = 4294967295; // Largest number of values that can be stored (index is by uint, so 2^32-1)
	static const UINT SIZESTEP = 10000; // Internal byte array is increased or decreased in block of this size
										//	static const UINT KARATSUBACUTOFF = 50; // Karatsuba cutoff size. Numbers below this size will use long multiplication
										//	static const UINT TOOMCOOK3CUTOFF = 150; // Testing shows this is the optimal value. Update - 150 is the best in debug builds. In release builds it is not. In release it is all over the place.
										//	static const UINT BURKINELZIEGLERCUTOFF = 50; // 50 seems to work well for the recursion point

										// Making these non-const so I can test different values
	static UINT KARATSUBACUTOFF;
	static UINT KARATSUBATHREADING;
	static UINT TOOMCOOK3CUTOFF;
	static UINT TOOMCOOK3THREADING;
	static UINT BURKINELZIEGLERCUTOFF;

public:
	// For testing only
	void assignByteArray(byte*, UINT);

	// Constructors
	LongInteger();
	LongInteger(const int);
	LongInteger(const UINT);
	LongInteger(const LongInteger&);
	LongInteger(LongInteger*);
#ifdef _WIN32
	LongInteger(CString&);
#else
	// Add in wstring option as well once this is compiling
	LongInteger(const string&);
#endif
	~LongInteger();

	// Useful functions
	const UINT getSize() const;

	bool assignNumber(int);
	bool assignNumber(UINT);
#ifdef _WIN32
	bool assignNumber(CString&);
#else
	bool assignNumber(const string&);
#endif

	byte getDigit(UINT) const;
	bool setDigit(UINT, byte);
	// Split the current LongInt. Send in 2 LongInts to hold the result. Returns false if it fails
	bool split(LongInteger*, LongInteger*, UINT) const;
	// Split 'this' into 3 parts of UINT size and return as an array of LongIntegers
	UINT toomsplit(LongInteger**, UINT) const;


	// Conversion methods
#ifdef _WIN32
	CString toDecimal();
	CString toHexString() const; // Useful for testing
	CString toArrayNumbers(); // Also useful for testing
#else
	string toDecimal();
	string toHexString() const; // Useful for testing
	string toArrayNumbers(); // Also useful for testing
#endif
	explicit operator int(); // Convert to int. Make it explicit to avoid confusion in other methods
	explicit operator UINT();

	// Arithmetic functions
	bool addNumber(int);
	bool addNumber(const LongInteger&);
	bool subtractNumber(int);
	bool subtractNumber(const LongInteger&);
	bool multiplyNumber(int);
	bool multiplyNumber(const LongInteger&);
	bool divideNumber(int);
	bool divideNumber(const LongInteger&);
	bool powerCalc(LongInteger&, const LongInteger&);
	bool powerCalc(const LongInteger&);
	bool powerCalc(int);
	bool arrowCalc(UINT, int);
	bool modulus(const LongInteger&);

	// Arithmetic operators
	LongInteger operator+(const LongInteger&) const; // Addition
	LongInteger operator+() const; // Unary plus
	LongInteger operator-(const LongInteger&) const;
	LongInteger operator-() const; // Unary negate
	LongInteger operator*(const LongInteger&) const;
	LongInteger operator/(const LongInteger&) const;
	LongInteger operator%(const LongInteger&) const;

	LongInteger operator+(int) const; // Addition
	LongInteger operator-(int) const;
	LongInteger operator*(int) const;
	LongInteger operator/(int) const;
	LongInteger operator%(int) const;

	LongInteger& operator=(const LongInteger&);
	LongInteger& operator++(); // Prefix increment
	LongInteger operator++(int); // Postfix increment (the int is unused - a placeholder in the syntax)
	LongInteger& operator+=(const LongInteger&); // Adding a LongInteger
	LongInteger& operator+=(int); // Adding an int
	LongInteger& operator--(); // Prefix decrement
	LongInteger operator--(int); // Postfix decrement
	LongInteger& operator-=(const LongInteger&);
	LongInteger& operator-=(int);
	LongInteger& operator*=(const LongInteger&);
	LongInteger& operator*=(int);
	LongInteger& operator/=(const LongInteger&);
	LongInteger& operator/=(int);
	LongInteger& operator%=(const LongInteger&);
	LongInteger& operator%=(int);

	// Bitwise operators
	bool bitwiseand(const LongInteger&);
	LongInteger operator&(const LongInteger&);
	LongInteger operator&(int);
	LongInteger& operator&=(const LongInteger&);
	LongInteger& operator&=(int);

	bool bitwiseor(const LongInteger&);
	LongInteger operator|(const LongInteger&);
	LongInteger operator|(int);
	LongInteger& operator|=(const LongInteger&);
	LongInteger& operator|=(int);

	bool bitwisenot();
	LongInteger operator~() const;

	bool bitwisexor(const LongInteger&);
	LongInteger operator^(const LongInteger&);
	LongInteger operator^(int);
	LongInteger& operator^=(const LongInteger&);
	LongInteger& operator^=(int);

	bool bitshiftright(const LongInteger&);
	bool bitshiftleft(const LongInteger&);
	bool bitshiftright(UINT);
	bool bitshiftleft(UINT);
	LongInteger operator >> (const LongInteger&);
	LongInteger operator >> (UINT);
	LongInteger& operator>>=(const LongInteger&);
	LongInteger& operator>>=(UINT);
	LongInteger operator<<(const LongInteger&);
	LongInteger operator<<(UINT);
	LongInteger& operator<<=(const LongInteger&);
	LongInteger& operator<<=(UINT);

	LongInteger abs(const LongInteger&);

	bool equalsZero() const;
	bool isProcessing();
	// Commenting these 2 calls out as I need a better way of waiting on completion
	// This was a hack to get the program working properly in Windows
	void setProcessing(bool);
	//    void setProcessing(bool, CWinThread* = nullptr);
	//    HANDLE getProcessingHandle();
	bool isShuttingDown();
	void setShuttingDown(bool);

	inline bool overflow() const {
		return bOverflow;
	}

private:
	byte* digits = nullptr;
	bool bPositive = true;
	UINT maxSize = 0;
	UINT size = 0;
	bool bOverflow = false;
	bool static bShuttingDown;
	bool bProcessing;
	// Commented out at the moment. Not just because it is Windows specific, but
	// because I need a more secure way of waiting on completion
	//    CWinThread *thrProcessingRef = nullptr;

	void init();
	void reset(); // Reset the internal array
	void copy(const LongInteger&);
	void addInternal(byte*, UINT, UINT, UINT);
	void checkSize();
	void recalcMaxSize();
	bool increaseSize();
	bool decreaseSize();
	bool increment(); // Helper function for ++
	bool decrement(); // Helper function for --

					  // Helper methods
	bool arrowCalc(const LongInteger&, UINT, LongInteger&);
	bool powerCalcHelper(const LongInteger&, LongInteger&);
	bool divHelper(int);
	bool divByByte(byte);
	bool multHelper(int);
	bool multHelper2(UINT);
	UINT UINTpower(UINT, UINT);

	// Helper methods for Burnikel-Ziegler division
	static LongIntegerUP merge(vector<LongIntegerUP>& vList, UINT uNumParts, UINT uSizeParts);
	static vector<LongIntegerUP> DivThreeHalvesByTwo(LongIntegerUP& a1, LongIntegerUP& a2, LongIntegerUP& a3, LongIntegerUP& b1, LongIntegerUP& b2, UINT uNumDigits);
	static vector<LongIntegerUP> DivTwoDigitsByOne(LongIntegerUP& AHigh, LongIntegerUP& ALow, LongIntegerUP& B, UINT uNumDigits);
	// A generic splitting function. Split the input LongInteger into UINT parts of UINT length and return as an array of LongIntegers
	static vector<LongIntegerUP> split(LongIntegerUP&, UINT, UINT);

	LongInteger* pSharedLongInt;

public:
	bool multiplyInternal(const LongInteger&);

	// comparison operators
public:

	inline bool operator<(const LongInteger& rhs) const {
		if (bOverflow || rhs.bOverflow) {
			return false;
		}

		if (bPositive != rhs.bPositive) {
			// If this is + and rhs is -, then return -, else return +. So return opposite of bPositive flag
			return !bPositive;
		}

		if (size < rhs.size) {
			return true;
		}
		if (size > rhs.size) {
			return false;
		}

		bool bLessThan = false;
		bool bLoop = true;
		// Some amendments as the index is now a UINT and thus we can't go negative
		if (size > 0) {
			UINT index = size - 1;
			while (index > 0 && bLoop) {
				if (digits[index] < rhs.digits[index]) {
					bLessThan = true;
					bLoop = false;
				}
				else if (digits[index] > rhs.digits[index]) {
					bLoop = false;
				}
				--index;
			}
		}
		// Scenario for index = 0 and we haven't finished checking
		if (bLoop) {
			if (digits[0] < rhs.digits[0]) {
				bLessThan = true;
			}
		}

		return (bLessThan ^ !bPositive); // Return the opposite result if the numbers are negative
	}

	inline bool operator>(const LongInteger& rhs) const {
		return rhs < (*this);
	}

	inline bool operator<=(const LongInteger& rhs) const {
		return !((*this) > rhs);
	}

	inline bool operator>=(const LongInteger& rhs) const {
		return !((*this) < rhs);
	}

	inline bool operator==(const LongInteger& rhs) const {
		if (bOverflow || rhs.bOverflow) {
			return false;
		}
		if (size != rhs.size) {
			return false;
		}
		if (bPositive != rhs.bPositive) {
			return false;
		}
		bool bEqual = true;
		// Amendments for using UINT as index instead of int. Can no longer check for less than zero
		// so we have to do this in 2 bits
		if (size > 0) {
			UINT iLoop = size - 1;
			while (iLoop > 0 && bEqual) {
				if (digits[iLoop] != rhs.digits[iLoop])
					bEqual = false;
				--iLoop;
			}
		}
		// Extra bit for the zeroth digit
		if (bEqual) {
			if (digits[0] != rhs.digits[0])
				bEqual = false;
		}
		return bEqual;
	}

	inline bool operator==(int rhs) const {
		if (rhs == 0) {
			return equalsZero();
		}
		return ((*this) == LongInteger(rhs));
	}

	inline bool operator!=(const LongInteger& rhs) {
		return !((*this) == rhs);
	}

	inline bool operator!=(int rhs) {
		return !((*this) == rhs);
	}


};

inline bool operator==(int lhs, const LongInteger& rhs) {
	return rhs == lhs;
}

inline bool operator!=(int lhs, const LongInteger& rhs) {
	return !(rhs == lhs);
}

inline bool operator>(int lhs, const LongInteger& rhs) {
	return rhs < lhs;
}

inline bool operator<(int lhs, const LongInteger& rhs) {
	return rhs > lhs;
}

inline LongInteger operator+(int lhs, const LongInteger& rhs) {
	return rhs + lhs;
}

inline LongInteger operator-(int lhs, const LongInteger& rhs) {
	LongInteger value = lhs;
	return value - rhs;
}

inline LongInteger operator*(int lhs, const LongInteger& rhs) {
	return rhs * lhs;
}

inline LongInteger operator/(int lhs, const LongInteger& rhs) {
	LongInteger value = lhs;
	return value / rhs;
}

inline LongInteger operator&(int lhs, const LongInteger& rhs) {
	LongInteger value = lhs;
	return value & rhs;
}

inline LongInteger operator|(int lhs, const LongInteger& rhs) {
	LongInteger value = lhs;
	return value | rhs;
}

inline LongInteger operator^(int lhs, const LongInteger& rhs) {
	LongInteger value = lhs;
	return value ^ rhs;
}

inline LongInteger operator >> (int lhs, const LongInteger& rhs) {
	LongInteger value = lhs;
	return value >> rhs;
}
