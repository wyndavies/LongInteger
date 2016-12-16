#pragma once
#include <vector>
#include <memory>
using std::vector;
using std::shared_ptr;
using std::make_shared;

// Header for LongInteger file. Can handle positive integers up to 4 billion digits in length.
// It holds the number as a byte array and indexes it using an uint (hence the size limit)
// Addition, subtraction, multiplication and division and powers are handled.
// Standard operators - +,-,*,/,+=,-=,/=,*=,<,>,<=,>=,!=,== are handled
// The = operator is overriden to provide copy by value instead of the default copy by reference
// It is also overloaded, so that strings and ints can be assigned to a LongInteger
//
// The karatsuba algorithm is used to multiply large numbers. It is more complex than long multiplication,
// but the number of steps needed doesn't increase as rapidly as the number length increases.
// The karatsuba algorithm is only accessed via the multiplyNumber method

class LongInteger;
typedef shared_ptr<LongInteger> LongIntegerSP; // Started using shared_ptr because the number of deletes was getting out of hand

class LongInteger
{
public:
	// Have these listed here whilst still working on them

	// Still in testing. Performance is weird (improves dramatically at multiples of 350 and then drops rapidly until the next multiple is hit)
	static LongInteger* ToomCook3(const LongInteger&, const LongInteger&);

	// Restoring division seems to work fine. Some more testing needed to see what the performance is
	static void RestoringDivision(LongInteger&, LongInteger&, LongInteger*, LongInteger*);

	// This is under development. The code is rather messy as I'm struggling to follow the paper due to the heavy usage of maths
	// terminology I'm unfamiliar with.
	static void BurnikelZiegler(LongInteger&, LongInteger&, LongInteger*, LongInteger*);

	static shared_ptr<LongInteger> merge(vector<LongIntegerSP> vList, UINT uNumParts, UINT uSizeParts);
	static vector<LongIntegerSP> DivThreeHalvesByTwo(LongIntegerSP a1, LongIntegerSP a2, LongIntegerSP a3, LongIntegerSP b1, LongIntegerSP b2, UINT uNumDigits);
	static vector<LongIntegerSP> DivTwoDigitsByOne(LongIntegerSP AHigh, LongIntegerSP ALow, LongIntegerSP B, UINT uNumDigits);
	// A generic splitting function. Split the input LongInteger into UINT parts of UINT length and return as an array of LongIntegers
	static vector<shared_ptr<LongInteger>> split(LongIntegerSP, UINT, UINT);

private:
	static LongInteger karatsuba(const LongInteger&, const LongInteger&); // This is the front end which also tidies up any memory allocated
	static LongInteger* karatsubaMain(const LongInteger&, const LongInteger&); // This is the recursive part of the karatsuba algorithm

	// Constants
public:
	static const int BASEVAL = 256; // The effective base of the numbers (each digit is 1 byte)
	static const int BASEVALBITS = 8; // Size of each digit in bits
	static const byte BASEMAX = 255; // Highest value that can be stored in each digit
	static const UINT ABSMAXSIZE = 4294967295; // Largest number of values that can be stored (index is by uint, so 2^32-1)
	static const UINT SIZESTEP = 10000; // Internal byte array is increased or decreased in block of this size
	static const UINT KARATSBUACUTOFF = 50; // Karatsuba cutoff size. Numbers below this size will use long multiplication
	static const UINT TOOMCOOK3CUTOFF = 150; // Testing shows this is the optimal value. Update - 150 is the best in debug builds. In release builds it is not. In release it is all over the place.

public:
	// For testing only
	void assignByteArray(byte*, UINT);

	// Constructors
	LongInteger();
	LongInteger(const int);
	LongInteger(const UINT);
	LongInteger(const LongInteger&);
	LongInteger(CString&);
	~LongInteger();

	// Useful functions
	const UINT getSize() const;

	bool assignNumber(int);
	bool assignNumber(UINT);
	bool assignNumber(CString&);

	byte getDigit(UINT) const;
	bool setDigit(UINT, byte);
	// Split the current LongInt. Send in 2 LongInts to hold the result. Returns false if it fails
	bool split(LongInteger*, LongInteger*, UINT) const;
	// Split 'this' into 3 parts of UINT size and return as an array of LongIntegers
	UINT toomsplit(LongInteger**, UINT) const;


	// Conversion methods
	CString toDecimal();
	CString toHexString() const; // Useful for testing
	CString toArrayNumbers();    // Also useful for testing
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
    //	LongInteger operator+() const; // Unary plus
	LongInteger operator-(const LongInteger&) const;
	//	LongInteger operator-() const; // Unary negate
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
	LongInteger operator>>(const LongInteger&);
	LongInteger operator>>(int);
	LongInteger& operator>>=(const LongInteger&);
	LongInteger& operator>>=(int);
	LongInteger operator<<(const LongInteger&);
	LongInteger operator<<(int);
	LongInteger& operator<<=(const LongInteger&);
	LongInteger& operator<<=(int);

	// Unary operators
	LongInteger operator-() const;
	LongInteger operator+() const;

	LongInteger abs(const LongInteger&);

	bool equalsZero() const;
	bool isProcessing();
	void setProcessing(bool, CWinThread* = nullptr);
	HANDLE getProcessingHandle();
	bool isShuttingDown();
	void setShuttingDown(bool);

	inline bool overflow() const { return bOverflow; }

private:
	byte* digits = nullptr;
	bool bPositive = true;
	UINT maxSize = 0;
	UINT size = 0;
	bool bOverflow = false;
	bool static bShuttingDown;
	bool bProcessing;
	CWinThread *thrProcessingRef = nullptr;

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
public:
	bool multiplyInternal(const LongInteger&);

	// comparison operators
public:
	inline bool operator<(const LongInteger& rhs) const
	{
		if (bOverflow || rhs.bOverflow) {
			return false;
		}

		if (bPositive != rhs.bPositive)
		{
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
		int index = size - 1;
		while (index >= 0 && bLoop) {
			if (digits[index] < rhs.digits[index]) {
				bLessThan = true;
				bLoop = false;
			}
			else if (digits[index] > rhs.digits[index]) {
				bLoop = false;
			}
			--index;
		}
		return (bLessThan ^ !bPositive); // Return the opposite result if the numbers are negative
	}

	inline bool operator>(const LongInteger& rhs) const { return rhs < (*this); }
	inline bool operator<=(const LongInteger& rhs) const { return !((*this) > rhs); }
	inline bool operator>=(const LongInteger& rhs) const { return !((*this) < rhs); }
	inline bool operator==(const LongInteger& rhs) const
	{
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
		int iLoop = size - 1;
		while (iLoop >= 0 && bEqual) {
			if (digits[iLoop] != rhs.digits[iLoop])
				bEqual = false;
			--iLoop;
		}
		return bEqual;
	}
	inline bool operator==(int rhs) const
	{
		if (rhs == 0) {
			return equalsZero();
		}
		return ((*this) == LongInteger(rhs));
	}
	
	inline bool operator!=(const LongInteger& rhs) { return !((*this) == rhs); }
	inline bool operator!=(int rhs) { return !((*this) == rhs); }
	

};

inline bool operator==(int lhs, const LongInteger& rhs) { return rhs == lhs; }
inline bool operator!=(int lhs, const LongInteger& rhs) { return !(rhs == lhs); }
inline bool operator>(int lhs, const LongInteger& rhs) { return rhs < lhs; }
inline bool operator<(int lhs, const LongInteger& rhs) { return rhs > lhs; }

inline LongInteger operator+(int lhs, const LongInteger& rhs) { return rhs + lhs;}
inline LongInteger operator-(int lhs, const LongInteger& rhs) { LongInteger value = lhs; return value - rhs; }
inline LongInteger operator*(int lhs, const LongInteger& rhs) { return rhs * lhs; }
inline LongInteger operator/(int lhs, const LongInteger& rhs) { LongInteger value = lhs; return value / rhs; }
inline LongInteger operator&(int lhs, const LongInteger& rhs) { LongInteger value = lhs; return value & rhs; }
inline LongInteger operator|(int lhs, const LongInteger& rhs) { LongInteger value = lhs; return value | rhs; }
inline LongInteger operator^(int lhs, const LongInteger& rhs) { LongInteger value = lhs; return value ^ rhs; }
inline LongInteger operator>>(int lhs, const LongInteger& rhs) { LongInteger value = lhs; return value >> rhs; }