#ifdef _WIN32
#include "stdafx.h"
#endif
#include "LongInteger.h"

#include <stdexcept>
#undef min
#undef max
#include <limits>

#include <functional>


// min and max are undefined before the inclusion of <limits> as Microsoft reused the min and max macros and this causes
// conflicts with (C++ standard) definitions in <limits>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

bool LongInteger::bShuttingDown = false;
UINT LongInteger::BURKINELZIEGLERCUTOFF = 50;
UINT LongInteger::TOOMCOOK3CUTOFF = 100;
UINT LongInteger::TOOMCOOK3THREADING = 100;
UINT LongInteger::KARATSUBACUTOFF = 50;
UINT LongInteger::KARATSUBATHREADING = 1000;

void LongInteger::init() {
	// Create the byte array. Default size of SIZESTEP
	size = 0;
	maxSize = SIZESTEP;
	reset();
}

void LongInteger::reset() {
	// Create the byte array with the new size
	if (digits != nullptr)
		delete[] digits;
	digits = new byte[maxSize];
	memset(digits, 0, sizeof(byte) * maxSize);
	bOverflow = false;
	bProcessing = false;
	bPositive = true; // Can only be true or false, so we'll treat zero as positive
}

LongInteger LongInteger::multiplyChooser(const LongInteger& liOne, const LongInteger &liTwo) {
	// Wrapper class to handle memory management
	// The karatsuba and ToomCook3 functions needs to put memory on the heap, so the memory needs
	// to be managed manually. But we want all memory management within the class


	// Additional section added as part of multi-threading
	// As the size increases the number of threads increases exponentially if the thread cut-off is a fixed value.
	// So we will calculate a cut-off value that will allow a suitable number of threads to run - 
	// not too many and not too few.
	// Each time a call to Karatsuba is made where iHalfSize ends up less than the threading cutoff we get
	// approx 3 times the number of threads
	// So take the number of cores, multiply by a fixed value (10 maybe?) to give enough threads that 100% core usage
	// happens without the queue getting ridiculous.
	// Then work out what iHalfSize will be and half it n times until 3^n is greater than the value from above.

	UINT minCalc = 500;
	if (liOne.getSize() > 500 || liTwo.getSize() > 500)
	{

		// There is only a single instance of QOT
		QueueOfThreads<LongIntWrapper> *qot = LongIntWrapper::getQOT();
		UINT deviceCores = qot->getDeviceCores();
		UINT targetThreads = deviceCores * 10;

		UINT uKCutOff = ((liOne.size > liTwo.size) ? liOne.size : liTwo.size) / 2;
		UINT uTCcutOff = uKCutOff;

		UINT numThreads = 1;
		while (numThreads < targetThreads) {
			numThreads = numThreads * 3;
			uKCutOff /= 2;
		}
		numThreads = 1;
		while (numThreads < targetThreads) {
			numThreads = numThreads * 5;
			uTCcutOff /= 2;
		}

		// Not sure if this is the optimal cutoff value
		if (uKCutOff < 500) {
			uKCutOff = 500;
		}
		if (uTCcutOff < 500) {
			uTCcutOff = 500;
		}

		// Comment out during performance testing
		LongInteger::KARATSUBATHREADING = uKCutOff;
		LongInteger::TOOMCOOK3THREADING = uTCcutOff;
	}
	else
	{
		LongInteger::KARATSUBATHREADING = LongInteger::ABSMAXSIZE;
		LongInteger::TOOMCOOK3THREADING = LongInteger::ABSMAXSIZE;
	}

	// Call ToomCook3, Karatsuba or Long Multiplication?

	LongInteger* returnValue;
	if ((liOne.size > TOOMCOOK3CUTOFF) || (liTwo.size > TOOMCOOK3CUTOFF))
	{
		returnValue = ToomCook3(liOne, liTwo);
	}
	else {
		if ((liOne.size > KARATSUBACUTOFF) || (liTwo.size > KARATSUBACUTOFF))
		{
			returnValue = karatsuba(liOne, liTwo);
		}
		else
		{
			returnValue = new LongInteger(liOne);
			returnValue->multiplyInternal(liTwo);
		}
	}

	return returnValue;
}


// Implementation of the Karatsuba algorithm for multiplication
// This is a static method

LongInteger* LongInteger::karatsuba(const LongInteger &liOne, const LongInteger &liTwo, bool bBackgroundThread)
{
	if ((liOne.size < KARATSUBACUTOFF) && (liTwo.size < KARATSUBACUTOFF))
	{
		LongInteger *returnValue = new LongInteger(liOne);
		returnValue->multiplyInternal(liTwo);
		return returnValue;
	}

	// Determine the size of the numbers, so we know where to split them
	UINT iSize = (liOne.size > liTwo.size) ? liOne.size : liTwo.size;
	UINT iHalfSize = iSize / 2;

	// Split the digit sequences about the middle
	LongInteger* liHighOne = new LongInteger();
	LongInteger* liLowOne = new LongInteger();
	LongInteger* liHighTwo = new LongInteger();
	LongInteger* liLowTwo = new LongInteger();
	liOne.split(liLowOne, liHighOne, iHalfSize);
	liTwo.split(liLowTwo, liHighTwo, iHalfSize);

	LongInteger* liZ0;
	LongInteger* liZ1;
	LongInteger* liZ2;

	if (iHalfSize > KARATSUBATHREADING) {
		QueueOfThreads<LongIntWrapper> *qot = LongIntWrapper::getQOT();
		LongIntWrapper* liw0 = new LongIntWrapper;
		LongIntWrapper* liw1 = new LongIntWrapper;
		LongIntWrapper* liw2 = new LongIntWrapper;
		liw0->setParams(*liLowOne, *liLowTwo);
		liw1->setParams((*liLowOne + *liHighOne), (*liLowTwo + *liHighTwo));
		liw2->setParams(*liHighOne, *liHighTwo);
		LIfunction fp = &LongInteger::karatsuba;
		liw0->setStartMethod(fp);
		liw1->setStartMethod(fp);
		liw2->setStartMethod(fp);
		qot->addToQueue(liw0);
		qot->addToQueue(liw1);
		qot->addToQueue(liw2);
		if (bBackgroundThread) {
			qot->iAmWaiting(); // Only call this if this process is called in a background thread.
		}
		//CString strOutput;
		// We will log which thread we are waiting on. The way this has been implemented, this process doesn't know
		// what its own thread ID is, so we can't log that
		//strOutput.Format(L"Waiting for thread %d to finish\n", liw0->getID());
		//qot->logwithlock(strOutput);
		qot->waitForThread(liw0);
		//strOutput.Format(L"Stopped waiting on thread %d\n", liw0->getID());
		//qot->logwithlock(strOutput);
		//strOutput.Format(L"Waiting for thread %d to finish\n", liw1->getID());
		//qot->logwithlock(strOutput);
		qot->waitForThread(liw1);
		//strOutput.Format(L"Stopped waiting on thread %d\n", liw1->getID());
		//qot->logwithlock(strOutput);
		//strOutput.Format(L"Waiting for thread %d to finish\n", liw2->getID());
		//qot->logwithlock(strOutput);
		qot->waitForThread(liw2);
		//strOutput.Format(L"Stopped waiting on thread %d - Resuming processing\n", liw2->getID());
		//qot->logwithlock(strOutput);
		if (bBackgroundThread) {
			qot->iHaveStoppedWaiting();
		}
		liZ0 = liw0->getResult();
		liZ1 = liw1->getResult();
		liZ2 = liw2->getResult();
		delete liw0;
		delete liw1;
		delete liw2;
	}
	else {
		// 3 calls made to numbers approximately half the size
		liZ0 = karatsuba(*liLowOne, *liLowTwo, bBackgroundThread);
		liZ1 = karatsuba((*liLowOne + *liHighOne), (*liLowTwo + *liHighTwo), bBackgroundThread);
		liZ2 = karatsuba(*liHighOne, *liHighTwo, bBackgroundThread);
	}
	// The next step is this calculation:
	// return (z2*10^(2*m2))+((z1-z2-z0)*10^(m2))+(z0)
	// This calc is in base 10 whereas we are in base BASEVAL, which is the size of 1 byte
	// The numbers are represented internally as a byte array
	// So we will start with most complex bit - z1-z2-z0 * BASEVAL^(halfsize)
	// We will do the sums, then move liZ1 left by halfsize bytes
	(*liZ1) -= (*liZ2);
	(*liZ1) -= (*liZ0);
	liZ1->multHelper2(iHalfSize);
	// Then move liZ2 left by iSize bytes
	liZ2->multHelper2(iHalfSize * 2);
	// Then we add liZ0, liZ1 and liZ2 together

	LongInteger* returnValue = new LongInteger((*liZ2) + (*liZ1) + (*liZ0));

	delete liLowOne;
	delete liLowTwo;
	delete liHighOne;
	delete liHighTwo;
	delete liZ0;
	delete liZ1;
	delete liZ2;

	return returnValue;
}

bool LongInteger::split(LongInteger* liFront, LongInteger* liBack, UINT iSplit) const {
	// Split the current LongInteger into 2 bits at the point specified by iSplit

	if (iSplit >= size) {
		(*liFront) = (*this);
		liBack->init();
		return false;
	}

	// Put the first half of the current LongInteger in liFront
	liFront->size = iSplit;
	liFront->maxSize = (iSplit - (iSplit % SIZESTEP)) + SIZESTEP;
	liFront->reset();
	memcpy(liFront->digits, digits, iSplit * sizeof(byte));
	liFront->bPositive = bPositive;

	// Put the second half in liBack
	liBack->size = size - iSplit;
	liBack->maxSize = (iSplit - (iSplit % SIZESTEP)) + SIZESTEP;
	liBack->reset();
	memcpy(liBack->digits, digits + (iSplit * sizeof(byte)), ((size - iSplit) * sizeof(byte)));
	liBack->bPositive = bPositive;

	return true;
}

LongInteger::LongInteger() {
	init();
}

LongInteger::LongInteger(const int iNew) {
	assignNumber(iNew);
}

LongInteger::LongInteger(const UINT iNew) {
	assignNumber(iNew);
}

LongInteger::LongInteger(const LongInteger& oldLongInt) {
	size = oldLongInt.size;
	maxSize = oldLongInt.maxSize;
	reset();
	bPositive = oldLongInt.bPositive;
	bOverflow = oldLongInt.bOverflow;
	memcpy(digits, oldLongInt.digits, sizeof(byte) * maxSize);
}

LongInteger::LongInteger(LongInteger* pliCopyInt) {
	// Copy constructor for a pointer.
	// I've had to create some tortuous workarounds for assignments of points returned from a method
	size = pliCopyInt->size;
	maxSize = pliCopyInt->maxSize;
	reset();
	bPositive = pliCopyInt->bPositive;
	bOverflow = pliCopyInt->bOverflow;
	memcpy(digits, pliCopyInt->digits, sizeof(byte) * maxSize);
	delete pliCopyInt;
}

#ifdef _WIN32

LongInteger::LongInteger(CString& strInput) {
	assignNumber(strInput);
}
#else

LongInteger::LongInteger(const string& strInput) {
	assignNumber(strInput);
}

#endif

LongInteger::~LongInteger() {
	delete[] digits;
}

const UINT LongInteger::getSize() const {
	return size;
}

bool LongInteger::assignNumber(UINT inNum) {
	// Clear out any existing number
	init();

	// Turn the int into base 16 digits and store in the internal array
	UINT index = 0;
	while (index < maxSize && inNum > 0) {
		digits[index] = inNum % BASEVAL;
		inNum /= BASEVAL;
		++index;
	}

	// It will be unlikely for an int to overflow the starting size of a LongInteger, so not bother checking for that
	size = index;
	return bOverflow;
}

bool LongInteger::assignNumber(int inNum) {
	// Clear out any existing number
	init();

	if (inNum < 0) {
		bPositive = false;
		inNum = -inNum;
	}

	// Turn the int into base 16 digits and store in the internal array
	UINT index = 0;
	while (index < maxSize && inNum > 0) {
		digits[index] = inNum % BASEVAL;
		inNum /= BASEVAL;
		++index;
	}

	// It will be unlikely for an int to overflow the starting size of a LongInteger, so not bother checking for that
	size = index;
	return bOverflow;
}

#ifdef _WIN32
bool LongInteger::assignNumber(CString& in_number)
#else

bool LongInteger::assignNumber(const string& in_number)
#endif
{
	// Accept a (potentially very large) number in a CString
	// Assume it is in decimal and turn it into an 8-bit byte array
	// Easy!

	// So create a temporary list to hold the digits we calculate
	// Put all the decimal digits in there, then loop through dividing by 256 and adding the remainder
	// to the digits list until the templist is empty
	init();

#ifdef _WIN32
	if (in_number.GetLength() == 0) {
#else
	if (in_number.length() == 0) {
#endif
		assignNumber(0);
		return !bOverflow;
	}

#ifdef _WIN32
	byte* tempList = new byte[in_number.GetLength()];
	UINT listLength = in_number.GetLength();
	wchar_t s[2];
#else
	byte* tempList = new byte[in_number.length()];
	UINT listLength = in_number.length();
	char s[2];
#endif

	UINT index = listLength - 1;
	s[1] = 0;
	UINT iStart = 0;
	if (in_number[0] == '-') {
		bPositive = false;
		iStart = 1;
		index--;
	}
	for (UINT i = iStart; i < listLength; i++, index--) {
		s[0] = in_number[i];
#ifdef _WIN32
		tempList[index] = _wtoi(s);
#else
		tempList[index] = atoi(s);
#endif
	}
	if (!bPositive) listLength--;

	index = 0;
	bool finished = false;
	unsigned short remainder;
	unsigned short value;
	while (!finished && !bOverflow) {
		remainder = 0;
		// Loop through and divide each digit by BASEVAL, putting the remainder in the next digit down
		for (UINT i = listLength; i > 0; i--) {
			value = tempList[i - 1] + (remainder * 10);
			//			tempList[i] = value / BASEVAL;
			tempList[i - 1] = value >> BASEVALBITS;
			//			remainder = value % BASEVAL;
			remainder = (byte)value; // Only works when using base 256
		}
		// Anything left in remainder is the new digit
		digits[index] = (byte)remainder;

		//digits[index] = remainder;
		index++;

		// Check the temp list to see if we've finished
		while (listLength > 0 && tempList[listLength - 1] == 0) {
			listLength--;
		}
		if (listLength == 0 && tempList[0] == 0) {
			finished = true;
		}

		size = index + 1;
		if (size > maxSize) {
			increaseSize();
			if (bOverflow)
				return !bOverflow;
			if (bShuttingDown)
				return false;
		}
	}

	checkSize();


	delete[] tempList;

	return !bOverflow;
}

byte LongInteger::getDigit(UINT index) const {
	if (index < 0 || index >= maxSize) {
		return -1;
	}

	return digits[index];
}

bool LongInteger::setDigit(UINT index, byte newValue) {
	if (index < 0 || index >= LongInteger::ABSMAXSIZE) {
		return false;
	}

	if (index > size) {
		size = index + 1;
		recalcMaxSize();
	}
	digits[index] = newValue;
	return true;
}


#ifdef _WIN32
CString LongInteger::toDecimal()
#else

string LongInteger::toDecimal()
#endif
{
	// Convert the internal number into a decimal string
	// Create a copy of the number and start dividing each digit by 10, putting the overflow into the
	// number below, with the last overflow dropping out into the string

	// Trying to find some way to improve the algorithm below as this is by far the slowest part of the program
	// Converting large digits to decimal takes an unreasonable amount of time

	// Sanity check
	if (bOverflow) {
#ifdef _WIN32
		return (L"NULL");
#else
		return "NULL";
#endif

	}
#ifdef _WIN32
	CString output = L"";
#else
	string output = "";
	stringstream outputstream;
#endif

	if (equalsZero()) {
#ifdef _WIN32
		output = L"0";
#else
		output = "0";
#endif
		return output;
	}

	// Find out how big we need to make the copy
	UINT listSize;
	// Check we don't have leading zeroes
	for (listSize = size - 1; listSize > 0 && digits[listSize] == 0; listSize--);
	if (listSize == 0 && digits[0] == 0) return output;
#ifdef _WIN32
	output.Preallocate(listSize * 3); // An attempt to speed things up a little bit
#endif
	byte* listCopy = new byte[listSize + 1];

	memcpy(listCopy, digits, sizeof(byte) * (listSize + 1));

	bool finished = false;
	UINT value = 0;
	UINT remainder = 0;
	while (!finished) {
		remainder = 0;
		// The use of 2 variables below is because UINTs can't be negative, so need to test for more than zero
		// This then leads to every index reference being (counter-1), unless we've already calculated it.
		// The approach below produces a tiny improvement over using a single variable. Every little bit helps
		for (UINT counter = listSize + 1, index = listSize; counter > 0; counter--, index--) {
			value = listCopy[index] + (remainder << BASEVALBITS); // Slight improvement over multiplying by BASEVAL (256)
			listCopy[index] = value / 10; // Surprisingly using bit hacking slowed this function down by a factor of 4
			remainder = value - (listCopy[index] * 10); // Using bit manipulation to multiply also slows this down by nearly 4 times
		}
		// The append below has a tiny impact on performance - less than 1%
#ifdef _WIN32
		output.AppendFormat(L"%d", remainder);
#else
		outputstream << remainder;
#endif

		// Check the temp list to see if we've finished
		while (listSize > 0 && listCopy[listSize] == 0) {
			listSize--;
		}
		if (listSize == 0 && listCopy[0] == 0) {
			finished = true;
		}
		if (bShuttingDown) {
			delete[] listCopy;
#ifdef _WIN32
			return CString(L"Shutting Down");
#else
			return "Shutting Down";
#endif
		}
	}
	// The string is done, but it is backwards
#ifdef _WIN32
	output.MakeReverse();
#else
	output = outputstream.str();
	std::reverse(output.begin(), output.end());
#endif

	if (!bPositive) {
#ifdef _WIN32
		output = L"-" + output;
#else
		output = "-" + output;
#endif
	}
	delete[] listCopy;

	return output;
}

#ifdef _WIN32
CString LongInteger::toDecimal() const
#else

string LongInteger::toDecimal() const
#endif
{
	// Convert the internal number into a decimal string
	// Create a copy of the number and start dividing each digit by 10, putting the overflow into the
	// number below, with the last overflow dropping out into the string

	// Trying to find some way to improve the algorithm below as this is by far the slowest part of the program
	// Converting large digits to decimal takes an unreasonable amount of time

	// Sanity check
	if (bOverflow) {
#ifdef _WIN32
		return (L"NULL");
#else
		return "NULL";
#endif

	}
#ifdef _WIN32
	CString output = L"";
#else
	string output = "";
	stringstream outputstream;
#endif

	if (equalsZero()) {
#ifdef _WIN32
		output = L"0";
#else
		output = "0";
#endif
		return output;
	}

	// Find out how big we need to make the copy
	UINT listSize;
	// Check we don't have leading zeroes
	for (listSize = size - 1; listSize > 0 && digits[listSize] == 0; listSize--);
	if (listSize == 0 && digits[0] == 0) return output;
#ifdef _WIN32
	output.Preallocate(listSize * 3); // An attempt to speed things up a little bit
#endif
	byte* listCopy = new byte[listSize + 1];

	memcpy(listCopy, digits, sizeof(byte) * (listSize + 1));

	bool finished = false;
	UINT value = 0;
	UINT remainder = 0;
	while (!finished) {
		remainder = 0;
		// The use of 2 variables below is because UINTs can't be negative, so need to test for more than zero
		// This then leads to every index reference being (counter-1), unless we've already calculated it.
		// The approach below produces a tiny improvement over using a single variable. Every little bit helps
		for (UINT counter = listSize + 1, index = listSize; counter > 0; counter--, index--) {
			value = listCopy[index] + (remainder << BASEVALBITS); // Slight improvement over multiplying by BASEVAL (256)
			listCopy[index] = value / 10; // Surprisingly using bit hacking slowed this function down by a factor of 4
			remainder = value - (listCopy[index] * 10); // Using bit manipulation to multiply also slows this down by nearly 4 times
		}
		// The append below has a tiny impact on performance - less than 1%
#ifdef _WIN32
		output.AppendFormat(L"%d", remainder);
#else
		outputstream << remainder;
#endif

		// Check the temp list to see if we've finished
		while (listSize > 0 && listCopy[listSize] == 0) {
			listSize--;
		}
		if (listSize == 0 && listCopy[0] == 0) {
			finished = true;
		}
		if (bShuttingDown) {
			delete[] listCopy;
#ifdef _WIN32
			return CString(L"Shutting Down");
#else
			return "Shutting Down";
#endif
		}
	}
	// The string is done, but it is backwards
#ifdef _WIN32
	output.MakeReverse();
#else
	output = outputstream.str();
	std::reverse(output.begin(), output.end());
#endif

	if (!bPositive) {
#ifdef _WIN32
		output = L"-" + output;
#else
		output = "-" + output;
#endif
	}
	delete[] listCopy;

	return output;
}


LongInteger::operator int() const
{
	// Check the number held can be stored in an int
	if (size > sizeof(int))
		return 0;
	if (this->bPositive) {
		if ((*this) > (std::numeric_limits<int>::max()))
			return 0;
	}
	else {
		if(((*this) * (-1)) > (std::numeric_limits<int>::max()))
			return 0;
	}
	

	int iResult = (UINT)(*this); // Call the UINT conversion so we don't have to write the conversion code twice
	if (!bPositive)
		iResult *= -1;

	return iResult;
}


LongInteger::operator UINT() const
{
	// Convert the LongInteger into an unsigned int. If it is too big, return 0
	// Assume the user wants the absolute value, so we won't worry about pesky negative signs
	if (size > sizeof(UINT))
		return 0;
	// Use an unsigned int to hold the converted LongInteger
	UINT uResult = 0;
	for (UINT i = 0; i < size; i++) {
		uResult += (digits[i] * UINTpower(BASEVAL, i));
	}

	return uResult;
}


UINT LongInteger::UINTpower(UINT base, UINT power) const {
	UINT result = 1;
	for (UINT i = 0; i < power; i++) {
		result *= base;
	}
	return result;
}

bool LongInteger::addNumber(int iAdd) {
	// Sanity checks
	if (iAdd == 0) {
		return true;
	}

	// The simplest way to handle this is to turn the int into a LongInteger and call that addition
	// (Surprising complications can occur otherwise)
	LongInteger liAdd = iAdd;

	return addNumber(liAdd);
}

bool LongInteger::addNumber(const LongInteger& liAdd) {
	// Adding is easy. Assume each digit in the input is in the range 0-255.
	// We just add each digit in the ranges and then balance the numbers

	// Some sanity checks.
	while (liAdd.size > maxSize) {
		increaseSize();
		if (bOverflow)
			return false;
	}
	// If this and liAdd are positive, carry on
	// If this is + and liAdd is -, then call subtract with liAdd as +
	if (bPositive) {
		if (!liAdd.bPositive) {
			return this->subtractNumber(-liAdd);
		}
	}
	else
		// If this is - and liAdd is +, then subtract liAdd from this
		// If both are - then add them
	{
		if (liAdd.bPositive) {
			return this->subtractNumber(-liAdd);
		}
	}

	UINT loopSize = liAdd.size;
	if (loopSize > size) {
		memset(digits + size, 0, (loopSize - size));
		size = loopSize;
		if (size >= maxSize) {
			increaseSize();
		}
	}

	short workingVal = 0;
	short overflow = 0;
	for (UINT index = 0; index < loopSize; index++) {
		workingVal = digits[index] + liAdd.digits[index] + overflow;
		if (workingVal >= BASEVAL) {
			digits[index] = workingVal % BASEVAL;
			overflow = workingVal / BASEVAL;
		}
		else {
			digits[index] = (byte)workingVal;
			overflow = 0;
		}
	}

	UINT iDigit;
	UINT liDigit;
	while (overflow > 0 && size <= maxSize && !bShuttingDown) {
		iDigit = (loopSize < size) ? digits[loopSize] : 0;
		liDigit = (loopSize < liAdd.size) ? liAdd.digits[loopSize] : 0;
		workingVal = iDigit + liDigit + (overflow % BASEVAL);
		digits[loopSize] = workingVal % BASEVAL;
		overflow = workingVal / BASEVAL;
		++loopSize;
		if (loopSize >= size) {
			size = loopSize + 1;
			if (size >= maxSize) {
				increaseSize();
			}
		}
	}
	checkSize();

	return true;
}

bool LongInteger::subtractNumber(int iAdd) {
	// Sanity checks
	if (iAdd < 0) {
		return addNumber(-iAdd);
	}
	if (iAdd == 0) {
		return true;
	}
	subtractNumber(LongInteger(iAdd));

	return bOverflow;
}

bool LongInteger::subtractNumber(const LongInteger& liMinus) {
	// We just subtract each digit in the ranges and then balance the numbers

	// Some sanity checks.
	if (liMinus.size == 0) {
		return false;
	}

	// Subtracting a negative is the same as adding a positive
	if ((bPositive && !liMinus.bPositive) || (!bPositive && liMinus.bPositive)) {
		addNumber(-liMinus);
		return bOverflow;
	}
	// Check for going negative
	// If liMinus is greater than *this, we will go negative
	// So set the flag for negative and subtract *this from liMinus
	LongInteger liWorking;
	// Some manipulations to avoid making a copy. Looks might complicated, but it is checking for
	// abs(liMinus) > abs(*this). The abs operator makes a copy with bPositive set to positive, so
	// it is a bit of an overhead. Hence the flag manipulation below
	bool bGreater = false;
	// Test manually as the non-abs function depends on the signs and we want a sign-independant test that doesn't do copying
	if (liMinus.size > size) {
		bGreater = true;
	}
	else {
		/* This needs fixed
		It needs to check all digits
		*/
		if (liMinus.size == size) {
			bool bLoop = true;
			UINT uIndex = liMinus.size - 1;
			while (bLoop) {
				if (liMinus.digits[uIndex] > digits[uIndex]) {
					bGreater = true;
					bLoop = false;
				}
				else
					if (liMinus.digits[uIndex] < digits[uIndex]) {
						bLoop = false;
					}
					else {
						// Digits are the same
						if (uIndex > 0)
							uIndex--;
						else
							bLoop = false;
					}
			}
		}
	}

	if (bGreater) {
		LongInteger liTemp = (*this);
		*this = liMinus;
		liWorking = liTemp;
		bPositive = !bPositive;
	}
	else {
		liWorking = liMinus;
	}

	// Start subtracting
	UINT indexSize = liWorking.size;

	short workingVal = 0;
	short overflow = 0;
	for (UINT index = 0; index < indexSize; index++) {
		if (bShuttingDown) {
			return false; // Bailout condition if the program is shutting down mid-calculation
		}
		workingVal = digits[index] - liWorking.digits[index] - overflow;
		overflow = 0;
		while (workingVal < 0) {
			workingVal += BASEVAL;
			++overflow;
		}
		digits[index] = (byte)workingVal;
	}
	// Check if the subtracting needs to borrow from higher digits
	while (overflow > 0 && indexSize < size) {
		workingVal = digits[indexSize] - overflow;
		overflow = 0;
		while (workingVal < 0) {
			workingVal += BASEVAL;
			++overflow;
		}
		digits[indexSize] = (byte)workingVal;
		++indexSize;
	}

	if (overflow > 0) {
		// If the overflow is still non-zero and we've reached the end of the numbers, then something has gone wrong
		bOverflow = true;
		return false;
	}

	// The size of the number might be smaller
	checkSize();
	return bOverflow;
}

LongInteger LongInteger::abs(const LongInteger& liValue) {
	LongInteger returnValue(liValue);
	returnValue.bPositive = true;
	return returnValue;
}

LongInteger& LongInteger::operator=(const LongInteger& li) {
	// Detect the scenario of self-assignment
	if (this == &li)
		return *this;
	// Delete the existing byte array and copy all info from the new LongInteger
	copy(li);
	return *this;
}

void LongInteger::copy(const LongInteger& li) {
	maxSize = li.maxSize;
	delete[] digits;
	digits = new byte[maxSize];
	size = li.size;
	memcpy(digits, li.digits, sizeof(byte) * maxSize);
	bPositive = li.bPositive;
	bOverflow = li.bOverflow;
}

bool LongInteger::increment() {
	bool bDone = false;

	UINT index = 0;
	while (!bDone) {
		if (digits[index] == BASEMAX) {
			digits[index] = 0;
			++index;
			if (index >= maxSize) {
				increaseSize();
				bDone = bOverflow;
			}
			if (index >= size)
				digits[index] = 0;
		}
		else {
			++digits[index];
			bDone = true;
		}
	}
	if (index >= size)
		size = (index + 1);
	return bOverflow;
}

// Prefix ++ operator

LongInteger& LongInteger::operator++() {
	if (bPositive)
		increment();
	else
		decrement();
	return *this;
}

// Postfix ++ operator

LongInteger LongInteger::operator++(int) {
	LongInteger liTemp(*this);
	operator++();
	return liTemp;
}

bool LongInteger::decrement() {
	bool bDone = false;

	// Sanity checks
	if (size == 1 && digits[0] == 1) {
		size = 0;
		digits[0] = 0;
		bPositive = true; // Zero is always treated as positive
		return bOverflow;
	}
	if (size == 0 || (size == 1 && digits[0] == 0)) {
		// Decrementing zero gives us -1
		size = 1;
		digits[0] = 1;
		bPositive = false;
		return bOverflow;
	}

	UINT index = 0;
	while (!bDone) {
		if (digits[index] == 0) {
			digits[index] = BASEMAX;
			++index;
			if (index >= size) {
				bDone = true;
				bOverflow = true;
			}
		}
		else {
			--digits[index];
			bDone = true;
		}
	}
	if (index != 0) checkSize();
	return bOverflow;
}

LongInteger& LongInteger::operator--() {
	if (bPositive)
		decrement();
	else
		increment();
	return *this;
}

LongInteger LongInteger::operator--(int) {
	LongInteger liTemp(*this);
	operator--();
	return liTemp;
}

LongInteger& LongInteger::operator+=(const LongInteger& liAdd) {
	addNumber(liAdd);
	return *this;
}

LongInteger& LongInteger::operator+=(int iAdd) {
	addNumber(iAdd);
	return *this;
}

LongInteger& LongInteger::operator-=(const LongInteger& liAdd) {
	subtractNumber(liAdd);
	return *this;
}

LongInteger& LongInteger::operator-=(int iAdd) {
	subtractNumber(iAdd);
	return *this;
}

LongInteger& LongInteger::operator*=(const LongInteger& liAdd) {
	multiplyNumber(liAdd);
	return *this;
}

LongInteger& LongInteger::operator*=(int iAdd) {
	multiplyNumber(iAdd);
	return *this;
}

LongInteger& LongInteger::operator/=(const LongInteger& liDivide) {
	divideNumber(liDivide);
	return *this;
}

LongInteger& LongInteger::operator/=(int iDivide) {
	divideNumber(iDivide);
	return *this;
}

LongInteger LongInteger::operator+(const LongInteger& rhs) const {
	LongInteger value = *this;
	value += rhs;
	return value;
}

LongInteger LongInteger::operator-(const LongInteger& rhs) const {
	LongInteger value = *this;
	value -= rhs;
	return value;
}

LongInteger LongInteger::operator*(const LongInteger& rhs) const {
	LongInteger value = *this;
	value *= rhs;
	return value;
}

LongInteger LongInteger::operator/(const LongInteger& rhs) const {
	LongInteger value = *this;
	value /= rhs;
	return value;
}

bool LongInteger::multiplyNumber(const LongInteger& liMult) {
	bool bFinal;
	// Sort out the signs
	if (bPositive != liMult.bPositive)
		bFinal = false;
	else
		bFinal = true;

	(*this) = multiplyChooser(*this, liMult);

	// Handle the special case of multiplying zero by a negative number
	if (size == 0) bFinal = true;

	bPositive = bFinal;

	return bOverflow;
}

bool LongInteger::multiplyNumber(int iMult) {
	if (iMult == BASEVAL) {
		multHelper(iMult);
		return !bOverflow;
	}

	LongInteger liMult(iMult);

	return (multiplyNumber(liMult));
}

bool LongInteger::multiplyInternal(const LongInteger& liMult) {
	// We will assume that the digits of liMult are all in range 0-255. This means that all values will be held by a byte
	// Multiplying needs to be done 1 digit of the multiplier at a time and we need another memory storage area to hold
	// the result


	bool bFinalSign = !(bPositive ^ liMult.bPositive);

	// How big will the resulting number be?
	UINT inSize = liMult.size;
	UINT tempSize = size + inSize;

	byte* tempArea = new byte[tempSize];
	memset(tempArea, 0, sizeof(byte) * tempSize);
	UINT workingVal; // uint is at least 32 bits
	UINT overflow = 0;
	UINT tempIndex = 0;

	// Go through each digit in the input
	for (UINT inIndex = 0; inIndex < inSize; ++inIndex) {
		if (bShuttingDown) {
			delete[] tempArea;
			return false;
		}
		// Now loop through each value in the current structure, multiply and add the result to the temporary store
		tempIndex = inIndex;
		for (UINT index = 0; index < size; ++index) {
			workingVal = digits[index] * liMult.digits[inIndex];
			addInternal(tempArea, tempIndex, workingVal, 2); // 2 bytes is the max size of a sum of 2x1 byte numbers)
			++tempIndex;
		}
	}

	// Now put the temporary values into the original
	while (maxSize < tempSize) {
		increaseSize();
		if (bOverflow)
			return false;
	}
	memcpy(digits, tempArea, sizeof(byte) * tempSize);

	delete[] tempArea;

	size = tempSize;
	checkSize();

	bPositive = bFinalSign;

	return true;
}

void LongInteger::addInternal(byte* list, UINT offset, UINT value, UINT length) {
	// Add an unsigned int (which could hold up to length bytes) to the list starting from offset index

	byte* byteValues = (byte*)& value;
	UINT result;
	byte overflow = 0;

	for (UINT counter = 0; counter < length; ++counter) {
		result = byteValues[counter] + list[offset] + overflow;
		list[offset] = (byte)result;
		overflow = result / BASEVAL;
		++offset;
		if (offset > (maxSize - 1)) {
			increaseSize();
			if (bOverflow)
				return;
		}
	}

	while (overflow > 0) {
		result = list[offset] + overflow;
		list[offset] = (byte)result;
		overflow = result / BASEVAL;
		++offset;
	}
}

void LongInteger::checkSize() {
	if (size == 0) {
		if (maxSize > SIZESTEP)
			decreaseSize();
		return;
	}
	UINT startSize = size;
	UINT index = size - 1;
	while (index > 0 && digits[index] == 0) {
		--size;
		--index;
	}
	if (index == 0 && digits[index] == 0) {
		size = 0;
		bPositive = true; // Zero is positive for the purposes of this class
	}
	if (startSize != size) decreaseSize();
}

void LongInteger::recalcMaxSize() {
	// Used when maxSize needs to be recalculated
	UINT oldMaxSize = maxSize;
	maxSize = ((size / SIZESTEP) * SIZESTEP) + SIZESTEP;

	if (maxSize != oldMaxSize) {
		UINT copySize = oldMaxSize;
		if (maxSize < oldMaxSize) copySize = maxSize;
		byte* tempDigits = digits;
		digits = new byte[maxSize];
		memset(digits, 0, sizeof(byte) * maxSize);
		memcpy(digits, tempDigits, sizeof(byte) * copySize);
		delete[] tempDigits;
	}
}

bool LongInteger::arrowCalc(UINT arrows, int powerValue) {
	LongInteger liValue = powerValue;
	LongInteger baseValue = *this;
	bool returnValue = arrowCalc(baseValue, arrows, liValue);
	*this = liValue;
	return returnValue;
}

bool LongInteger::arrowCalc(const LongInteger& baseValue, UINT arrows, LongInteger& liValue) {
	if (bOverflow) return false;
	LongInteger liWorking;

	if (arrows == 1) {
		// If the level is 1, then calculate the answer, which is 2^value
		// So that is 2*2 repeated 'value'-1 times

		// powerCalc updates the 1st parameter, but for this scenario we want to retain the 1st param and update the 2nd
		liWorking = baseValue;
		powerCalc(liWorking, liValue);
		if (bShuttingDown) {
			return false;
		}
		liValue = liWorking;
	}
	else {
		// x^x (y times) and arrow level
		if (arrows > 2) {
			arrowCalc(baseValue, (arrows - 1), liValue);
		}

		// calc baseValue to the power baseValue, loopValue times. Put the answer in liValue
		LongInteger loopValue = liValue;
		LongInteger runningValue = baseValue;
		--loopValue;
		while (!(loopValue == 0)) {
			powerCalcHelper(baseValue, runningValue);
			if (bShuttingDown) {
				return false;
			}
			--loopValue;
		}
		liValue = runningValue;
	}

	return !bOverflow;
}

bool LongInteger::equalsZero() const {
	if (size == 0 || (size == 1 && digits[0] == 0))
		return true;
	else
		return false;
}

bool LongInteger::powerCalc(LongInteger& liValue, const LongInteger& liPower) {
	// Calculate the power by using powers of 2
	// We will have a loop where we divide liPower by 2 and multiply a copy of liValue by itself,
	// adding this copy to a running total every time liPower is odd
	// Just realised this is a bit like old-fashioned multiplication

	LongInteger liMultiples = liValue;
	LongInteger liTotal = 1;
	LongInteger liCounter = liPower;

	while (liCounter > 1) {
		// Find out if liCounter is odd
		UINT lastDigit = liCounter.getDigit(0);
		UINT oddCheck = lastDigit / 2;
		if (oddCheck * 2 < lastDigit) {
			liTotal *= liMultiples;
		}
		liCounter /= 2;
		liMultiples *= liMultiples;
	}
	// Last value is alway 1, thus always odd
	liTotal *= liMultiples;

	liValue = liTotal;

	return !bOverflow;
}

bool LongInteger::powerCalcHelper(const LongInteger& value, LongInteger& powerIn) {
	// Calculate a power by performing multiplication repeatedly
	// This is a helper for the arrow function - it changes the 2nd parameter rather than the first as would normally be expected

	LongInteger liMult = value;
	bOverflow = !(powerCalc(liMult, powerIn));
	powerIn = liMult;

	return !bOverflow;
}

bool LongInteger::powerCalc(int powerIn) {
	return powerCalc(*this, powerIn);
}

bool LongInteger::powerCalc(const LongInteger& powerIn) {
	return powerCalc(*this, powerIn);
}

bool LongInteger::divideNumber(const LongInteger& liDivide) {
	LongIntegerUP upliQuotient = make_unique<LongInteger>(0);
	LongIntegerUP upliModulus = make_unique<LongInteger>(0);

	bool bSuccess = DivAndMod(*this, liDivide, upliQuotient, upliModulus);

	if (!bSuccess) {
		*this = 0;
		bOverflow = true;
	}
	else {
		*this = *upliQuotient;
	}

	return bOverflow;
}

bool LongInteger::DivAndMod(const LongInteger& liValue, const LongInteger& liDivide, LongIntegerUP& upliQuotient, LongIntegerUP& upliModulus) {
	// The divide and modulus functions were created separately, but it will save a lot of hassle by combining them - especially as
	// it is the same code in the 2 functions


	// How to divide? Can you divide 1 digit at a time?
	// Division is big subtraction, but if the divisor is small that will be a lot of
	// subtractions.
	// However... the divisor can be multiplied until it is just a bit smaller than
	// the number being divided and then subtracted. This will reduce the number of subtractions

	bool bSuccess = true;
	// Dividing zero doesn't get you anywhere
	if (liValue.size == 0)
		return !bSuccess;

	// Compare sizes
	// If the divisor is bigger than the number being divided then result is zero (we don't have decimals or fractions)
	if (liDivide.size > liValue.size) {
		return !bSuccess;
	}
	// Don't divide by zero
	if (liDivide.equalsZero()) {
		return !bSuccess;
	}

	// If the value is large and the divisor is at least half the size of the value, then Burnikel-Zeigler division is quicker
	if (liValue.size > BURKINELZIEGLERCUTOFF) {
		if (liDivide.size > (liValue.size / 2)) {
			BurnikelZiegler(liValue, liDivide, upliQuotient, upliModulus);
			return bSuccess;
		}
		else {
			// Increase the size of the divisor until it is at least half the size of the value
			// Then do the division
			// Then divide the modulus by the amount we had "buffed" the divisor, noting the quotient
			// Then multiply the original quotient by the "buffed" amount and add the new quotient
			// This seems like a lot of work, but it is quicker

			UINT uIncrement = liValue.size - liDivide.size;
			if (uIncrement <= 2) {
				uIncrement = 2;
			}
			else {
				uIncrement /= 2;
			}

			LongInteger liNewDivide = liDivide;
			LongInteger liOffset = uIncrement;
			liOffset *= LongInteger::BASEVALBITS;
			liNewDivide <<= liOffset;

			DivAndMod(liValue, liNewDivide, upliQuotient, upliModulus);

			liNewDivide.bitshiftright(liOffset);
			LongIntegerUP upliNewQuotient = make_unique<LongInteger>(0);
			LongIntegerUP upliNewModulus = make_unique<LongInteger>(0);
			DivAndMod(*upliModulus, liNewDivide, upliNewQuotient, upliNewModulus);

			upliQuotient->bitshiftleft(liOffset);
			*upliQuotient += *upliNewQuotient;
			upliModulus = move(upliNewModulus);

			return bSuccess;
		}
	}

	// Use a LongInteger for the subtractor as the number could potentially end up being very large
	LongInteger liNumOfSubtractions = 1;
	LongInteger liSubtractor = liDivide;
	LongInteger liWorkingMod = liValue;
	LongInteger liWorkingQuot = 0;

	// Turn the calculation into a +ve dividing a +ve and also work out what the final sign will be
	bool bFinal = !(liWorkingMod.bPositive ^ liDivide.bPositive);
	liSubtractor.bPositive = true;
	liWorkingMod.bPositive = true;

	// multiply the divisor by 256 until the size is the same as the target.
	// Subtract until the divider is greater than the target.
	// Then divide by 256 and repeat until the divisor is back down to it's original size
	// We don't have fractions or decimals so that will be the end
	UINT iLevel = 0;
	while (liSubtractor.size < liWorkingMod.size) {
		liNumOfSubtractions *= BASEVAL;
		liSubtractor *= BASEVAL;
		++iLevel;
	}

	bool doLoop = true;
	while (doLoop) {
		while (liWorkingMod >= liSubtractor) {
			if (bShuttingDown) {
				return false;
			}
			liWorkingMod -= liSubtractor;
			liWorkingQuot += liNumOfSubtractions;
		}
		if (iLevel > 0) {
			liNumOfSubtractions /= BASEVAL;
			liSubtractor /= BASEVAL;
		}
		if (iLevel == 0) {
			doLoop = false;
		}
		else {
			--iLevel;
		}
	}

	upliQuotient = make_unique<LongInteger>(liWorkingQuot);
	upliQuotient->bPositive = bFinal;
	upliModulus = make_unique<LongInteger>(liWorkingMod);

	return bSuccess;
}

bool LongInteger::divideNumber(int iDivide) {
	// Divide using an integer

	// If the divisor is BASEVAL then call a special function
	// This is because the division operation uses division by BASEVAL internally,
	// so you would get endless recursion if you didn't handle this specific value differently
	if (iDivide == BASEVAL) {
		divHelper(iDivide);
		return !bOverflow;
	}
	if (iDivide > 0 && iDivide < 256) {
		this->divByByte((byte)iDivide);
		return !bOverflow;
	}
	if (iDivide < 0 && iDivide > -256) {
		this->divByByte((byte)std::abs(iDivide));
		this->bPositive = !(this->bPositive);
		return !bOverflow;
	}

	*this /= LongInteger(iDivide);

	return !bOverflow;
}

bool LongInteger::divHelper(int iDivide) {
	if (iDivide != BASEVAL) {
		return !bOverflow;
	}

	// This division is by the unit storage amount - i.e. move each digit down one
	// Slightly quicker version of shifting right by 8 bits
	if (size == 1) {
		digits[0] = 0;
	}
	else {
		for (UINT i = 1; i < size; ++i) {
			digits[i - 1] = digits[i];
		}
	}
	--size;
	digits[size] = 0;

	decreaseSize();

	return !bOverflow;
}

bool LongInteger::multHelper(int iMult) {
	if (iMult != BASEVAL) {
		return !bOverflow;
	}
	if (size >= maxSize) {
		increaseSize();
		if (bOverflow)
			return !bOverflow;
	}

	// This multiplication is by the unit storage amount - i.e. move each digit up one
	for (int i = size; i > 0; --i) {
		digits[i] = digits[i - 1];
	}
	digits[0] = 0;
	++size;

	return !bOverflow;
}

bool LongInteger::multHelper2(UINT iMult) {
	// We want to increase the size of this number by iMult bytes
	UINT oldSize = size;
	size += iMult;
	recalcMaxSize();

	byte* newDigits = new byte[maxSize];
	memset(newDigits, 0, maxSize * sizeof(byte));
	// Copy the data. This works because it copies from zero upwards and the lowest digit is in place zero.
	memcpy(newDigits + iMult, digits, sizeof(byte) * oldSize);
	delete[] digits;
	digits = newDigits;

	return true;
}

bool LongInteger::increaseSize() {
	// The current 'maxSize' is not big enough. Let us see if we can increase the size
	// without making it bigger than 'ABSMAXSIZE'
	if (maxSize == ABSMAXSIZE) {
		bOverflow = true;
		return bOverflow;
	}

	UINT newMaxSize = maxSize + SIZESTEP;
	if (newMaxSize > ABSMAXSIZE)
		newMaxSize = ABSMAXSIZE;

	// Now to create a new data array of bytes on the heap, copy the existing values across and delete the original
	byte* newDigits = new byte[newMaxSize];
	// Copy the data. This works because it copies from zero upwards and the lowest digit is in place zero.
	memcpy(newDigits, digits, sizeof(byte) * maxSize);
	// Set the remaining digits to zero
	memset(newDigits + (maxSize * sizeof(byte)), 0, (newMaxSize - maxSize) * sizeof(byte));

	delete digits;
	digits = newDigits;
	maxSize = newMaxSize;

	return bOverflow;
}

bool LongInteger::decreaseSize() {
	// Can we decrease the size? The smallest size allowed is 'SIZESTEP'
	if (maxSize <= SIZESTEP)
		return false;

	// Check the size. Subtract 10 from maxSize to prevent borderline cases, where the program
	// starts repeatedly adding and removing data blocks
	if ((size + SIZESTEP) > (maxSize - 10))
		return false;

	UINT newMaxSize = maxSize - SIZESTEP;
	if (newMaxSize < SIZESTEP)
		newMaxSize = SIZESTEP;

	// Check that the number can fit inside the new smaller structure
	bool allZero = true;
	for (UINT i = newMaxSize; i < maxSize && allZero; ++i) {
		if (digits[i] != 0) allZero = false;
	}
	if (!allZero)
		return false;

	// Create the new smaller structure
	byte* newDigits = new byte[newMaxSize];

	// Copy the data.
	memcpy(newDigits, digits, sizeof(byte) * newMaxSize);

	delete[] digits;
	digits = newDigits;
	maxSize = newMaxSize;

	return true;
}

bool LongInteger::isProcessing() {
	return bProcessing;
}


void LongInteger::setProcessing(bool bValue)
{
	bProcessing = bValue;
}

// Commenting these out as I need a better way of waiting for process completion

/*
void LongInteger::setProcessing(bool bValue, CWinThread *thrRef)
{
bProcessing = bValue;
if (bProcessing && thrRef != nullptr)
thrProcessingRef = thrRef;
if (!bProcessing)
thrProcessingRef = nullptr;
}


HANDLE LongInteger::getProcessingHandle()
{
return thrProcessingRef->m_hThread;
}
*/

bool LongInteger::isShuttingDown() {
	return bShuttingDown;
}

void LongInteger::setShuttingDown(bool bValue) {
	bShuttingDown = bValue;
}

bool LongInteger::modulus(const LongInteger& liDivide) {
	LongIntegerUP upliQuotient = make_unique<LongInteger>(0);
	LongIntegerUP upliModulus = make_unique<LongInteger>(0);

	bool bSuccess = DivAndMod(*this, liDivide, upliQuotient, upliModulus);

	if (!bSuccess) {
		*this = 0;
	}
	else {
		*this = *upliModulus;
	}

	return bOverflow;
}

LongInteger& LongInteger::operator%=(const LongInteger& liMod) {
	modulus(liMod);
	return *this;
}

LongInteger& LongInteger::operator%=(int iMod) {
	if (iMod <= 0)
		return *this;
	LongInteger value = iMod;
	modulus(value);
	return *this;
}

LongInteger LongInteger::operator%(const LongInteger& liMod) const {
	LongInteger value = *this;
	value %= liMod;
	return value;
}

LongInteger LongInteger::operator%(int iMod) const {
	if (iMod <= 0)
		return *this;
	LongInteger value = *this;
	value %= iMod;
	return value;
}

LongInteger LongInteger::operator+(int iAdd) const {
	if (iAdd == 0)
		return *this;
	LongInteger value = *this;
	value += iAdd;
	return value;
}

LongInteger LongInteger::operator-(int iMinus) const {
	if (iMinus == 0)
		return *this;
	LongInteger value = *this;
	value -= iMinus;
	return value;
}

LongInteger LongInteger::operator*(int iMult) const {
	LongInteger value = *this;
	value *= iMult;
	return value;
}

LongInteger LongInteger::operator/(int iDiv) const {
	if (iDiv == 0) {
		LongInteger liReturn(0);
		liReturn.bOverflow = true;
		return liReturn;
	}
	LongInteger value = *this;
	value /= iDiv;
	return value;
}

bool LongInteger::bitwiseand(const LongInteger& liAnd) {
	// Bitwise AND method
	// AND this with liAnd
	// 
	// Loop through each byte in the digits array and AND them
	if (size <= 0 || liAnd.size <= 0)
		return bOverflow;

	UINT numLoops = (size > liAnd.size ? size : liAnd.size);

	for (UINT i = 0; i < numLoops; i++) {
		digits[i] &= liAnd.digits[i];
	}

	if (size > liAnd.size) {
		for (UINT i = liAnd.size; i < size; i++) {
			digits[i] = 0; // anything AND zero is zero
		}
	}

	checkSize();

	return bOverflow;
}

LongInteger LongInteger::operator&(const LongInteger& liAnd) {
	// Bitwise AND operator
	LongInteger value = *this;
	value &= liAnd;
	return value;
}

LongInteger LongInteger::operator&(int iAnd) {
	LongInteger value = *this;
	value &= iAnd;
	return value;
}

LongInteger& LongInteger::operator&=(const LongInteger& liAnd) {
	bitwiseand(liAnd);
	return *this;
}

LongInteger& LongInteger::operator&=(int iAnd) {
	bitwiseand(iAnd);
	return *this;
}

bool LongInteger::bitwiseor(const LongInteger& liOR) {

	// Bitwise OR method
	// OR this with liOR
	// 
	// Loop through each byte in the digits array and OR them
	if (size <= 0 || liOR.size <= 0)
		return bOverflow;

	// Don't want to overflow either of the numbers' internal byte arrays
	UINT numLoops = (size > liOR.size ? liOR.size : size);

	for (UINT i = 0; i < numLoops; i++) {
		digits[i] |= liOR.digits[i];
	}

	if (size > liOR.size) {
		for (UINT i = liOR.size; i < size; i++) {
			digits[i] = digits[i]; // anything OR zero is itself
		}
	}

	if (size < liOR.size) {
		for (UINT i = size; i < liOR.size; i++) {
			if (i >= maxSize) increaseSize();
			digits[i] = liOR.digits[i]; // Anything OR zero is itself
			size++;
		}
	}

	checkSize();

	return bOverflow;
}

LongInteger LongInteger::operator|(const LongInteger& liOR) {
	// Bitwise OR operator
	LongInteger value = *this;
	value |= liOR;
	return value;
}

LongInteger LongInteger::operator|(int iOR) {
	LongInteger value = *this;
	value |= iOR;
	return value;
}

LongInteger& LongInteger::operator|=(const LongInteger& liOR) {
	bitwiseor(liOR);
	return *this;
}

LongInteger& LongInteger::operator|=(int iOR) {
	bitwiseor(iOR);
	return *this;
}

bool LongInteger::bitwisenot() {
	// Invert all the bits
	for (UINT i = 0; i < size; i++) {
		digits[i] = ~(digits[i]);
	}

	checkSize(); // In case any leading digits have been set to zero

	return bOverflow;
}

LongInteger LongInteger::operator~() const {
	LongInteger value = *this;
	value.bitwisenot();
	return value;
}

bool LongInteger::bitwisexor(const LongInteger& liXOR) {

	// Bitwise XOR method
	// XOR this with liXOR
	// 
	// Loop through each byte in the digits array and XOR them
	if (size <= 0 || liXOR.size <= 0)
		return bOverflow;

	// Don't want to overflow either of the numbers' internal byte arrays
	UINT numLoops = (size > liXOR.size ? liXOR.size : size);

	for (UINT i = 0; i < numLoops; i++) {
		digits[i] ^= liXOR.digits[i];
	}

	if (size > liXOR.size) {
		for (UINT i = liXOR.size; i < size; i++) {
			digits[i] = digits[i]; // anything XOR zero is itself
		}
	}

	if (size < liXOR.size) {
		for (UINT i = size; i < liXOR.size; i++) {
			if (i >= maxSize) increaseSize();
			digits[i] = liXOR.digits[i]; // Anything OR zero is itself
			size++;
		}
	}

	checkSize();

	return bOverflow;
}

LongInteger LongInteger::operator^(const LongInteger& liXOR) {
	// Bitwise XOR operator
	LongInteger value = *this;
	value ^= liXOR;
	return value;
}

LongInteger LongInteger::operator^(int iXOR) {
	LongInteger value = *this;
	value ^= iXOR;
	return value;
}

LongInteger& LongInteger::operator^=(const LongInteger& liXOR) {
	bitwisexor(liXOR);
	return *this;
}

LongInteger& LongInteger::operator^=(int iXOR) {
	bitwisexor(iXOR);
	return *this;
}

bool LongInteger::bitshiftright(const LongInteger& liShift) {
	// Rightshift all the bits by liShift amount
	// Every multiple of 8 in liShift is a full byte to the right
	// So start by getting liShift div 8 and moving all bytes by that amount

	// This is an implementation that assumes the indexing can be by LongInteger
	// and not UINT as is assumed elsewhere (but has to use a cast as the array
	// uses UINT as an index

	// First a sanity check. A little like a sanity clause, but without the reindeer.
	// Shifting zero or shifting by zero achieves nothing - so just return
	if (size == 0 || liShift.size == 0) {
		return bOverflow;
	}

	LongInteger liMoveBytes = liShift;
	liMoveBytes.bitshiftright(BASEVALBITSSHIFT); // This should call the UNIT version
	UINT iMoveBits = liShift.digits[0] % BASEVALBITS;

	// And another check
	if (liMoveBytes > size) {
		init();
		checkSize();
		return bOverflow;
	}

	// The digits need to be shifted down by liMoveBytes, starting from zero
	for (UINT i = 0, j = (UINT)liMoveBytes; j < size; ++i, ++j) {
		digits[i] = digits[j];
	}
	// Blank out the other digits
	for (UINT uLoop = (size - (UINT)liMoveBytes); uLoop < size; ++uLoop) {
		digits[uLoop] = 0;
	}
	checkSize();

	// Create a bitmask
	byte bBitMask = 0;
	for (UINT i = 0; i < iMoveBits; i++) {
		bBitMask <<= 1;
		bBitMask += 1;
	}

	if (bBitMask == 0) return bOverflow;

	// Now move each digit by any bits left over
	UINT iUnderflow = 0;
	UINT iTemp;
	UINT index;
	for (UINT uLoop = size; uLoop > 0; --uLoop) {
		index = uLoop; // Index of digit to adjust converted to a UINT
		index--; // A workaround to deal with LongIntegers (and UINT) not being negative and the end test for the loop
		iTemp = digits[index] >> iMoveBits;
		iTemp += iUnderflow;
		iUnderflow = digits[index] & bBitMask;
		iUnderflow <<= (BASEVALBITS - iMoveBits);
		digits[index] = (byte)iTemp;
	}

	checkSize();
	return bOverflow;
}

bool LongInteger::bitshiftright(UINT uShift) {
	// Rightshift all the bits by uShift amount
	// Every multiple of 8 in uShift is a full byte to the right
	// So start by getting uShift div 8 and moving all bytes by that amount

	// First a sanity check. A little like a sanity clause, but without the reindeer.
	UINT uMoveBytes = uShift / 8;
	UINT iMoveBits = uShift % 8;
	if (uMoveBytes > size) {
		init();
		checkSize();
		return bOverflow;
	}

	// And another check
	// Shifting zero or shifting by zero achieves nothing - so just return
	if (size == 0 || uShift == 0) {
		return bOverflow;
	}

	// The digits need to be shifted down by uMoveBytes, starting from zero
	for (UINT i = 0, j = uMoveBytes; j < size; ++i, ++j) {
		digits[i] = digits[j];
	}
	// Blank out the other digits
	for (UINT uLoop = (size - uMoveBytes); uLoop < size; ++uLoop) {
		digits[uLoop] = 0;
	}
	checkSize();

	// Create a bitmask
	byte bBitMask = 0;
	for (UINT i = 0; i < iMoveBits; i++) {
		bBitMask <<= 1;
		bBitMask += 1;
	}

	if (bBitMask == 0) return bOverflow;

	// Now move each digit by any bits left over
	UINT iUnderflow = 0;
	UINT iTemp;
	// A separate index variable is a workaround to deal with LongIntegers (and UINT) not being negative and the end test for the loop
	for (UINT uLoop = size, index = size - 1; uLoop > 0; --uLoop, --index) {
		iTemp = digits[index] >> iMoveBits;
		iTemp += iUnderflow;
		iUnderflow = digits[index] & bBitMask;
		iUnderflow <<= (BASEVALBITS - iMoveBits);
		digits[index] = (byte)iTemp;
	}

	checkSize();
	return bOverflow;
}

bool LongInteger::bitshiftleft(const LongInteger& liShift) {
	// Leftshift all the bits by liShift amount
	// Every multiple of 8 in liShift is a full byte to the left
	// So start by getting liShift div 8 and moving all bytes by that amount

	// This is an implementation that assumes the indexing can be by LongInteger
	// and not UINT as is assumed elsewhere

	// First some sanity checks. A little like a sanity clause, but without the reindeer.
	// Left shifting zero doens't do anything (except cause issues with the loops)
	if (size == 0 || (size == 1 && digits[0] == 0))
		return bOverflow;
	// Check we aren't going to increase the number beyond max size
	LongInteger liMoveBytes = liShift;
	liMoveBytes.bitshiftright(BASEVALBITSSHIFT);
	UINT iMoveBits = liShift.digits[0] % BASEVALBITS;
	if (liMoveBytes > LongInteger::ABSMAXSIZE) {
		bOverflow = true;
		return bOverflow;
	}

	// And another check
	// Shifting zero or shifting by zero achieves nothing - so just return
	if (size == 0 || liShift.size == 0) {
		return bOverflow;
	}

	// Work out the new size needed
	UINT newSize = (UINT)(size + liMoveBytes);
	UINT oldSize = size;
	size = newSize;
	recalcMaxSize();

	// The bits below are all wrong
	// Got to here

	// The digits need to be shifted up by liMoveBytes
	// Some artificial manipulation of uMoveTo as we can't check for it being less than zero
	// as UINTs can't be negative (i.e stick in -1 and +1 where needed)
	UINT uMoveTo = (size - 1);
	UINT uMoveFrom = (uMoveTo - (size - oldSize)) + 1;

	while (uMoveFrom > 0) {
		digits[uMoveTo] = digits[(uMoveFrom - 1)];
		--uMoveFrom;
		--uMoveTo;
	}
	// Blank out the other digits
	for (UINT uLoop = 0; uLoop < (size - oldSize); ++uLoop) {
		digits[uLoop] = 0;
	}
	checkSize();

	// Create a bitmask
	byte bBitMask = 0;
	for (UINT i = 0; i < iMoveBits; i++) {
		bBitMask <<= 1;
		bBitMask += 1;
	}

	if (bBitMask == 0) return bOverflow;

	// Now move each digit by any bits left over
	UINT iMoveUp = 0;
	UINT iTemp;
	UINT index;
	for (UINT uLoop = 0; uLoop < size; ++uLoop) {
		index = uLoop;
		iTemp = digits[index] << iMoveBits;
		iTemp += iMoveUp;
		iMoveUp = iTemp / 256;
		digits[uLoop] = (byte)iTemp;
	}
	if (iMoveUp != 0) {
		size++;
		if (size > maxSize) {
			recalcMaxSize();
		}
		digits[size - 1] = (byte)iMoveUp;
	}
	checkSize();

	return bOverflow;
}

bool LongInteger::bitshiftleft(UINT uShift) {
	// Leftshift all the bits by liShift amount
	// Every multiple of 8 in liShift is a full byte to the left
	// So start by getting liShift div 8 and moving all bytes by that amount

	// First some sanity checks. A little like a sanity clause, but without the reindeer.
	// Left shifting zero doens't do anything (except cause issues with the loops)
	if (size == 0 || (size == 1 && digits[0] == 0))
		return bOverflow;
	// Check we aren't going to increase the number beyond max size
	UINT uMoveBytes = uShift / 8;
	UINT iMoveBits = uShift % 8;
	if (uMoveBytes > (LongInteger::ABSMAXSIZE - size)) {
		bOverflow = true;
		return bOverflow;
	}

	// And another check
	// Shifting zero or shifting by zero achieves nothing - so just return
	if (size == 0 || uShift == 0) {
		return bOverflow;
	}

	// Work out the new size needed
	UINT newSize = size + uMoveBytes;
	UINT oldSize = size;
	size = newSize;
	recalcMaxSize();

	// The bits below are all wrong
	// Got to here

	// The digits need to be shifted up by uMoveBytes
	// Some artificial manipulation of uMoveTo as we can't check for it being less than zero
	// as UINTs can't be negative (i.e stick in -1 and +1 where needed)
	UINT uMoveTo = (size - 1);
	UINT uMoveFrom = (uMoveTo - (size - oldSize)) + 1;

	while (uMoveFrom > 0) {
		digits[uMoveTo] = digits[(uMoveFrom - 1)];
		--uMoveFrom;
		--uMoveTo;
	}
	// Blank out the other digits
	for (UINT uLoop = 0; uLoop < (size - oldSize); ++uLoop) {
		digits[uLoop] = 0;
	}
	checkSize();

	// Create a bitmask
	byte bBitMask = 0;
	for (UINT i = 0; i < iMoveBits; i++) {
		bBitMask <<= 1;
		bBitMask += 1;
	}

	if (bBitMask == 0) return bOverflow;

	// Now move each digit by any bits left over
	UINT iMoveUp = 0;
	UINT iTemp;
	UINT index;
	for (UINT uLoop = 0; uLoop < size; ++uLoop) {
		index = uLoop;
		iTemp = digits[index] << iMoveBits;
		iTemp += iMoveUp;
		iMoveUp = iTemp / 256;
		digits[uLoop] = (byte)iTemp;
	}
	if (iMoveUp != 0) {
		size++;
		if (size > maxSize) {
			recalcMaxSize();
		}
		digits[size - 1] = (byte)iMoveUp;
	}
	checkSize();

	return bOverflow;
}

LongInteger LongInteger::operator >> (const LongInteger& rhs) {
	LongInteger value = *this;
	value >>= rhs;
	return value;
}

LongInteger LongInteger::operator >> (UINT rhs) {
	LongInteger value = *this;
	value >>= rhs;
	return value;
}

LongInteger& LongInteger::operator>>=(const LongInteger& rhs) {
	bitshiftright(rhs);
	return *this;
}

LongInteger& LongInteger::operator>>=(UINT rhs) {
	bitshiftright(rhs);
	return *this;
}

LongInteger LongInteger::operator<<(const LongInteger& rhs) {
	LongInteger value = *this;
	value <<= rhs;
	return value;
}

LongInteger LongInteger::operator<<(UINT rhs) {
	LongInteger value = *this;
	value <<= rhs;
	return value;
}

LongInteger& LongInteger::operator<<=(const LongInteger& rhs) {
	bitshiftleft(rhs);
	return *this;
}

LongInteger& LongInteger::operator<<=(UINT rhs) {
	bitshiftleft(rhs);
	return *this;
}

LongInteger* LongInteger::ToomCook3(const LongInteger& liOne, const LongInteger& liTwo, bool bBackgroundThread) {
	// A sanity check
	if (liOne.equalsZero() || liTwo.equalsZero())
		return new LongInteger(0);

	// kFactor is the split level. This will be initially implemented as Toom-3, which uses a kFactor of 3
	const UINT kFactor = 3;

	// Note the signs of the two inputs for the final result
	bool bFinalSign = !(liOne.bPositive ^ liTwo.bPositive);

	// The ToomCook algorithm has 5 main steps
	// 1 - Splitting
	// 2 - Evaluation
	// 3 - Pointwise multiplication
	// 4 - Interpolation
	// 5 - Recomposition

	// Implement this as Toom-3 initially

	// 1 - Splitting
	// We split the numbers up into kFactor bits.
	LongInteger * liM[3];
	LongInteger * liN[3];
	for (UINT i = 0; i < 3; i++) {
		liM[i] = new LongInteger;
		liN[i] = new LongInteger;
	}


	UINT uSplit;

	// Split the numbers into 3 parts
	uSplit = liOne.size > liTwo.size ? liOne.size : liTwo.size;
	uSplit /= 3;
	if (uSplit == 0) uSplit = 1;

	uSplit = liOne.toomsplit(liM, uSplit);
	uSplit = liTwo.toomsplit(liN, uSplit);


	// 2 - Evaluation
	//
	// For Toom-3, we will end up with the equations:
	// p(x) = m2*x^2 + m1*x^1 + m0
	// q(x) = n2*x^2 + n1*x^1 + n0
	//
	// For evaluation, we will use the points 0, 1, -1, -2 & infinity
	//
	// p(0) = m0 + m1(0) + m2(0^2) = 0
	// p(1) = m0 + m1(1) + m2(1^2) = m0 + m1 + m2
	// p(-1) = m0 + m1(-1) + m2(-1^2) = m0 - m1 + m2
	// p(-2) = m0 + m1(-2) + m2(-2^2) = m0 - 2m1 + 4m2
	// p(@) = m2
	//
	// Faster evaluation
	// p0 = m0 + m2
	// p(0) = m0
	// p(1) = p0 + m1
	// p(-1) = p0 - m1
	// p(-2) = (p(-1) + m2) * 2 - m0
	// p(@) = m2

	// Note that the example above (taken from wikipedia) has the elements of m & n in the opposite order from our array

	// Array: 0=0, 1=1, 2=-1, 3=-2, 4=infinity
	LongInteger * liEvalM[5];
	LongInteger * liEvalN[5];
	LongInteger *liP0M = new LongInteger;
	LongInteger *liP0N = new LongInteger;
	for (UINT i = 0; i < 5; i++) {
		liEvalM[i] = new LongInteger;
		liEvalN[i] = new LongInteger;
	}

	*liP0M = *liM[0] + *liM[2];
	*liP0N = *liN[0] + *liN[2];
	*liEvalM[0] = *liM[0];
	*liEvalN[0] = *liN[0];
	*liEvalM[1] = *liP0M + *liM[1];
	*liEvalN[1] = *liP0N + *liN[1];
	*liEvalM[2] = *liP0M - *liM[1];
	*liEvalN[2] = *liP0N - *liN[1];
	*liEvalM[3] = ((*liEvalM[2] + *liM[2]) << 1) - *liM[0];
	*liEvalN[3] = ((*liEvalN[2] + *liN[2]) << 1) - *liN[0];
	*liEvalM[4] = *liM[2];
	*liEvalN[4] = *liN[2];

	// 3 - Pointwise multiplication
	//
	// This is where some recursion comes into effect.
	// If the operands are small enough we switch to a different algorithm

	// We will also check for threading

	LongInteger * liPointwise[5];
	
	if (LongInteger::TOOMCOOK3THREADING > liOne.size)
	{
		for (UINT i = 0; i < 5; i++) {
			if (liEvalM[i]->size > TOOMCOOK3CUTOFF || liEvalN[i]->size > TOOMCOOK3CUTOFF) {
				liPointwise[i] = ToomCook3(*liEvalM[i], *liEvalN[i]);
			}
			else {
				liPointwise[i] = karatsuba(*liEvalM[i], *liEvalN[i], bBackgroundThread);
			}
		}
	}
	else
	{
		QueueOfThreads<LongIntWrapper> *qot = LongIntWrapper::getQOT();
		LongIntWrapper* liw0 = new LongIntWrapper;
		LongIntWrapper* liw1 = new LongIntWrapper;
		LongIntWrapper* liw2 = new LongIntWrapper;
		LongIntWrapper* liw3 = new LongIntWrapper;
		LongIntWrapper* liw4 = new LongIntWrapper;

		liw0->setParams(*liEvalM[0], *liEvalN[0]);
		liw1->setParams(*liEvalM[1], *liEvalN[1]);
		liw2->setParams(*liEvalM[2], *liEvalN[2]);
		liw3->setParams(*liEvalM[3], *liEvalN[3]);
		liw4->setParams(*liEvalM[4], *liEvalN[4]);
		
		LIfunction fp = &LongInteger::ToomCook3;
		
		liw0->setStartMethod(fp);
		liw1->setStartMethod(fp);
		liw2->setStartMethod(fp);
		liw3->setStartMethod(fp);
		liw4->setStartMethod(fp);
		
		qot->addToQueue(liw0);
		qot->addToQueue(liw1);
		qot->addToQueue(liw2);
		qot->addToQueue(liw3);
		qot->addToQueue(liw4);

		if (bBackgroundThread) {
				qot->iAmWaiting(); // Only call this if this process is called in a background thread.
		}

		qot->waitForThread(liw0);
		qot->waitForThread(liw1);
		qot->waitForThread(liw2);
		qot->waitForThread(liw3);
		qot->waitForThread(liw4);

		if (bBackgroundThread) {
			qot->iHaveStoppedWaiting();
		}

		liPointwise[0] = liw0->getResult();
		liPointwise[1] = liw1->getResult();
		liPointwise[2] = liw2->getResult();
		liPointwise[3] = liw3->getResult();
		liPointwise[4] = liw4->getResult();

		delete liw0;
		delete liw1;
		delete liw2;
		delete liw3;
		delete liw4;
	}

	// 4 - Interpolation
	//
	// This is the most complex step. It is the reverse of the Evaluation step
	// We need to determine the coefficients for the polynomial given the points used
	// This leads to a matrix
	// |r(0)  |     |0^0   0^1   0^2   0^3   0^4 |    |r0|
	// |r(1)  |     |1^0   1^1   1^2   1^3   1^4 |    |r1|
	// |r(-1) |  =  |-1^0  -1^1  -1^2  -1^3  -1^4|  = |r2|
	// |r(-2) |     |-2^0  -2^1  -2^2  -2^3  -2^4|    |r3|
	// |r(@)  |     |0     0     0     0     1   |    |r4|
	//
	// | 1   0   0   0   0 |   |r0|
	// | 1   1   1   1   1 |   |r1|
	// | 1   -1  1   -1  1 |   |r2|
	// | 1   -2  4   -8  16|   |r3|
	// | 0   0   0   0   1 |   |r4|
	//
	// We can make use of the fact that due to the choice of evaluation points, the matrix is invertable
	//
	// |r0|  | 1     0     0     0     0   |   |r(0) |
	// |r1|  | 1/2   1/3   -1    1/6   -2  |   |r(1) |
	// |r2|  | -1    1/2   1/2   0     -1  |   |r(-1)|
	// |r3|  | -1/2  1/6   1/2   -1/6  2   |   |r(-2)|
	// |r4|  | 0     0     0     0     1   |   |r(@) |
	//
	// The difficulty is in finding an efficient method to compute the above
	// A sequence for Toom-3 given by Bodrato will be used

	LongInteger * liResult[5];
	for (UINT i = 0; i < 5; i++)
		liResult[i] = new LongInteger;

	*liResult[0] = *liPointwise[0];
	*liResult[4] = *liPointwise[4];
	*liResult[3] = (*liPointwise[3] - *liPointwise[1]) / 3;
	*liResult[1] = (*liPointwise[1] - *liPointwise[2]) >> 1;
	*liResult[2] = *liPointwise[2] - *liPointwise[0];
	*liResult[3] = (*liResult[2] - *liResult[3]) / 2 + (*liPointwise[4] << 1);
	*liResult[2] = *liResult[2] + *liResult[1] - *liResult[4];
	*liResult[1] = *liResult[1] - *liResult[3];


	// 5 - Recomposition
	//
	// We have the parts of the final values. Time to put it back together
	for (UINT i = 1; i < 5; i++) {
		*liResult[i] = *liResult[i] << (BASEVALBITS * (i * uSplit));
	}

	LongInteger *liReturn = new LongInteger;
	for (UINT i = 0; i < 5; i++) {
		*liReturn += *liResult[i];
	}

	// Memory clean up
	for (UINT i = 0; i < 3; i++) {
		delete liM[i];
		delete liN[i];
	}
	for (UINT i = 0; i < 5; i++) {
		delete liEvalM[i];
		delete liEvalN[i];
		delete liPointwise[i];
		delete liResult[i];
	}
	delete liP0M;
	delete liP0N;

	liReturn->bPositive = bFinalSign;
	if (liReturn->equalsZero()) liReturn->bPositive = true;


	return liReturn;
}

UINT LongInteger::toomsplit(LongInteger** liList, UINT uDigits) const {
	// liList is an vector of LongIntegers. We will split up *this into chunks and 
	// put these in liList
	// We assume that the members of liList have already been created, so we just need to fill them in

	UINT uStep = uDigits;
	UINT uNumSplits = 3;
	if ((uStep * 2) >= size) uNumSplits = 2;
	if (uStep >= size) uNumSplits = 1;

	for (UINT i = 0; i < uNumSplits; i++) {
		UINT uStart = uStep * i;
		UINT uEnd = uStart + uStep;
		if (uEnd > size) uEnd = size;
		// Handle numbers where the digits don't divide into 3 (i.e. the highest number chunk has more digits than the others)
		if (i == 2) uEnd = size;

		// Put the current chunk into the LongInteger
		liList[i]->size = (uEnd - uStart);
		liList[i]->maxSize = (uStep - (uStep % SIZESTEP)) + SIZESTEP;
		liList[i]->reset();
		memcpy(liList[i]->digits, digits + uStart, uEnd - uStart);
		liList[i]->checkSize(); // Get rid of any leading zeroes
	}

	if (uNumSplits < 3)
		liList[2]->assignNumber(0);
	if (uNumSplits < 2)
		liList[1]->assignNumber(0);

	return uStep; // Return the number of digits each has been divided into
}

vector<LongIntegerUP> LongInteger::split(LongIntegerUP& liToSplit, UINT uNumParts, UINT uDigits) {
	// We will split up the input LongInteger into uNumParts and return as 
	// an array of LongIntegers
	// We will create the LongIntegers in this method

	vector<LongIntegerUP> vList(uNumParts);
	for (UINT i = 0; i < uNumParts; i++) {
		vList[i] = make_unique<LongInteger>(0);
	}

	UINT uNumSplits = uNumParts;
	while ((uDigits * (uNumParts - 1) >= liToSplit->size) && (uNumSplits > 1))
		uNumSplits--;

	if (uNumSplits == 0) return vList;

	for (UINT i = 0; i < uNumSplits; i++) {
		UINT uStart = uDigits * i;
		UINT uEnd = uStart + uDigits;
		// Handle numbers that don't evenly divide into uDigit parts
		if (uEnd > liToSplit->size) uEnd = liToSplit->size;
		if (i == (uNumSplits - 1)) uEnd = liToSplit->size;

		// Put the current chunk into the LongInteger
		vList[i]->size = (uEnd - uStart);
		vList[i]->maxSize = (vList[i]->size - (vList[i]->size % SIZESTEP)) + SIZESTEP;
		vList[i]->reset();
		memcpy(vList[i]->digits, liToSplit->digits + uStart, uEnd - uStart);
	}

	while (uNumSplits < uNumParts) {
		vList[uNumSplits]->assignNumber(0);
		uNumSplits++;
	}

	return move(vList);
}

LongInteger LongInteger::operator-() const {
	// Negate the LongInteger
	// We will return a copy with the sign inverted
	LongInteger liReturn(*this);
	liReturn.bPositive = !liReturn.bPositive;
	return liReturn;
}

LongInteger LongInteger::operator+() const {
	// Added for completeness
	// Makes no change to the number
	return LongInteger(*this);
}

void LongInteger::assignByteArray(byte* newArray, UINT newSize) {
	// Copy a byte array
	size = newSize;
	recalcMaxSize();
	memcpy(digits, newArray, newSize);
	checkSize();
}

bool LongInteger::divByByte(byte bDiv) {
	// Divide this by a number between 1 and 255
	if (size == 0) return !bOverflow;

	unsigned short remainder = 0;
	unsigned short working;
	for (UINT i = size; i != 0; i--) {
		remainder += digits[i - 1];
		working = remainder / bDiv;
		remainder = remainder % bDiv;
		digits[i - 1] = (byte)working;
		remainder *= 256;
	}

	checkSize();
	return !bOverflow;
}

#ifdef _WIN32

CString LongInteger::toHexString() const {
	// Output the number represented as a hex string (helps in testing really big numbers)
	wchar_t hexchars[] = L"0123456789ABCDEF";

	CString result = L"";
	byte highValue;
	byte lowValue;
	CString index;
	for (UINT i = 0; i < size; i++) {
		highValue = digits[i] / 16;
		lowValue = digits[i] % 16;

		if (i != 0) result.Append(L".");
		index.Format(L"[%d]", i);
		result.Append(index);
		result.AppendChar(hexchars[highValue]);
		result.AppendChar(hexchars[lowValue]);
	}
	return result;
}
#else

string LongInteger::toHexString() const {
	// Output the number represented as a hex string (helps in testing really big numbers)
	char hexchars[] = "0123456789ABCDEF";

	stringstream resultstream;
	byte highValue;
	byte lowValue;
	for (UINT i = 0; i < size; i++) {
		highValue = digits[i] / 16;
		lowValue = digits[i] % 16;

		if (i != 0) resultstream << ".";
		resultstream << i;
		resultstream << highValue;
		resultstream << lowValue;
	}
	return resultstream.str();
}
#endif

#ifdef _WIN32

CString LongInteger::toArrayNumbers() {
	// Output the number represented as each decimal number in the array (helps in testing really big numbers)
	CString result = L"";
	CString index;
	for (UINT i = 0; i < size; i++) {
		if (i != 0) result.Append(L".");
		index.Format(L"[%d]%d", i, digits[i]);
		result.Append(index);
	}
	return result;
}
#else

string LongInteger::toArrayNumbers() {
	// Output the number represented as each decimal number in the array (helps in testing really big numbers)
	stringstream result;
	for (UINT i = 0; i < size; i++) {
		if (i != 0) result << ".";
		result << "[" << i << "]" << digits[i];
	}
	return result.str();
}
#endif

void LongInteger::RestoringDivision(LongInteger& liValue, LongInteger& liDiv, LongInteger* liResult, LongInteger* liModulus) {
	// Slow division - restoring division
	// Divide N by D, placing the quotient in Q and the remainder in R

	// n = number of digits in the quotient
	// q(n) is the nth digit of the quotient


	// P = N
	// D = D << n
	// for i = n-1 to 0
	//   P = 2P - D
	//   if P >= 0 then
	//		q(i) = 1
	// else
	//   q(i) = 0
	//   P = P + D

	/*
	7/2=111/10
	P=111
	D=1000
	i=1
	P=2P-D=1110-1000=110
	q(1)=1
	i=0
	P=2P-D=1100-1000=100
	q(0)=1
	P>>2=1
	Q=11

	9/2=1001/10
	P=1001
	D=10<<3=10000
	i=2
	P=2P-D=10010-10000=10
	q(2)=1
	i=1
	P=2P-D=100-10000=-1100
	q(1)=0
	P=-1100+10000=100
	i=0
	P=2P-D=1000-10000=-1000
	q(0)=0
	P=-1000+10000=1000
	P>>3=1

	8/2=1000/10
	P=1000
	D=10<<3=10000
	i=2
	P=2P-D=10000-10000=0
	q(2)=1
	i=1
	P=2P-D=0-10000=-10000
	q(1)=0
	P=-10000+10000=0
	i=0
	P=2P-D=0-10000=-100000
	q(0)=0
	P=-10000+10000=0
	P>>3=0
	*/

	// I understand how this works in binary. Still puzzled about higher bases
	// So let us implement this non-disaster-in-progress in binaryness

	// The number must be larger than the divisor for it to work
	// Well, technically if the divisor is larger then the remainder is the original number
	if (liValue < liDiv) {
		*liResult = 0;
		*liModulus = liValue;
		return;
	}

	LongInteger P = liValue;
	LongInteger D = liDiv << (liValue.size * BASEVALBITS);
	byte* Q = new byte[liValue.size];
	for (UINT i = liValue.size; i > 0; i--) {
		Q[i - 1] = 0;
		for (UINT j = 8; j > 0; j--) {
			P <<= 1;
			P -= D;
			if (P >= 0) {
				// Set the appropriate bit in Q
				byte addbit = 1 << (j - 1);
				Q[i - 1] += addbit;
			}
			else {
				P += D;
			}
		}
	}
	P >>= (liValue.size * BASEVALBITS);

	liResult->assignByteArray(Q, liValue.size);
	*liModulus = P;
	delete Q;
}

void LongInteger::BurnikelZiegler(const LongInteger& liValue, const LongInteger& liDiv, LongIntegerUP& liResult, LongIntegerUP& liModulus) {
	// Looked at other algorithm (e.g. Newton-Raphson division) and many of them use floating point numbers
	// So we shall move on to Burnikel-Ziegler division

	// The first part is DivTwoDigitsByOne
	// It needs to split the numbers, so we shall test the split code

	// Very much in the testing phase, hence the chaotic layout (plus it is rather hard to understand the paper as I don't really
	// know maths notation - so a lot of trial and error)



	// DivTwoDigitsByOne(AHigh, ALow, B), return quotient Q and remainder S
	// 
	// 1) Let [a1,a2] = AH, [a3,a4] = AL, and [b1,b2] = B
	// 2) [q1,R] = DivThreeHalvesByTwo(a1,a2,a3,b1,b2)
	// 3) Let [r1,r2]=R
	// 4) [q2,S] = DivThreeHalvesByTwo(r1,r2,a4,b1,b2)
	// 5) Return Q=[q1,q2] and S

	// DivThreeHalvesByTwo(a1,a2,a3,b1,b2)
	//
	// 6) q=[a1,a2]/b1;
	// 7) c=[a1,a2]-q*b1
	// 8) D=q*b2
	// 9) R=[r1,r2]=[c,a3]-D
	//10) if(R<0) {
	//11)   q--;
	//12)   R+=B;
	//13)   if(R<0) {
	//14)      q--;
	//15)      R+=B;
	//16)   }
	//17) }
	//18) Return quotient q and remainder R

	UINT uDigits = (liValue.size + 3) / 4;
	vector<LongIntegerUP> vASplit(2);

	// tempUP added to make gcc compiler happy. VS accepts this as an anonymous
	// parameter, but gcc refuses to compile
	LongIntegerUP tempUP = make_unique<LongInteger>(liValue);
	vASplit = move(split(tempUP, 2, uDigits * 2));

	LongIntegerUP tempUP2 = make_unique<LongInteger>(liDiv);
	vector<LongIntegerUP> vResult = DivTwoDigitsByOne(vASplit[1], vASplit[0], tempUP2, uDigits);

	liResult = move(vResult[0]);
	liModulus = move(vResult[1]);

}

vector<LongIntegerUP> LongInteger::DivTwoDigitsByOne(LongIntegerUP& AHigh, LongIntegerUP& ALow, LongIntegerUP& B, UINT uNumDigits) {
	// DivTwoDigitsByOne(AHigh, ALow, B), return quotient Q and remainder S
	// 
	// 1) Let [a1,a2] = AH, [a3,a4] = AL, and [b1,b2] = B
	// 2) [q1,R] = DivThreeHalvesByTwo(a1,a2,a3,b1,b2)
	// 3) Let [r1,r2]=R
	// 4) [q2,S] = DivThreeHalvesByTwo(r1,r2,a4,b1,b2)
	// 5) Return Q=[q1,q2] and S


	/*
	Input Two nonnegative integers A and B, such that A<βnB and βn/2 ≤ B<βn.
	n must be even.
	Output The quotient A/B and the remainder A mod B.
	1. Let A = A3β3n/2 +A2βn +A1βn/2 +A0 and B = B1βn/2 +B0, with 0 ≤ Ai <
	βn/2 and 0 ≤ Bi < βn/2.
	2. Compute the high half Q1 of the quotient as Q1 = A3βn+A2βn/2+A1
	B with remainder
	R1 using algorithm 3.4.
	3. Compute the low half Q0 of the quotient as Q0 = R1βn/2+A4
	B with remainder
	R0 using algorithm 3.4.
	4. Return the quotient Q = Q1βn/2 + Q0 and the remainder R = R0.

	*/
	vector<LongIntegerUP> vReturn(2); // Return Q & S

									  // Decide how many digits we are chopping up the numbers by (it needs to be consistent)
	if (uNumDigits == 0) uNumDigits = 1;

	vector<LongIntegerUP> vWorking;
	vWorking = move(LongInteger::split(AHigh, 2, uNumDigits));
	LongIntegerUP a1 = move(vWorking[1]);
	LongIntegerUP a2 = move(vWorking[0]);
	vWorking = move(LongInteger::split(ALow, 2, uNumDigits));
	LongIntegerUP a3 = move(vWorking[1]);
	LongIntegerUP a4 = move(vWorking[0]);
	vWorking = move(LongInteger::split(B, 2, uNumDigits));
	LongIntegerUP b1 = move(vWorking[1]);
	LongIntegerUP b2 = move(vWorking[0]);

	vector<LongIntegerUP> vResult1, vResult2;
	// 2) [q1,R] = DivThreeHalvesByTwo(a1,a2,a3,b1,b2)
	LongInteger a1a2a3 = (*a1 << (uNumDigits * BASEVALBITS * 2)) + (*a2 << (uNumDigits * BASEVALBITS)) + *a3;

	if (a1a2a3 < *B) {
		vResult1.push_back(make_unique<LongInteger>(LongInteger()));
		vResult1.push_back(make_unique<LongInteger>(a1a2a3));
	}
	else
		vResult1 = DivThreeHalvesByTwo(a1, a2, a3, b1, b2, uNumDigits);

	// 3) Let [r1,r2]=R
	vWorking = move(LongInteger::split(vResult1[1], 2, uNumDigits));
	LongIntegerUP r1 = move(vWorking[1]);
	LongIntegerUP r2 = move(vWorking[0]);

	// 4) [q2,S] = DivThreeHalvesByTwo(r1,r2,a4,b1,b2)
	a1a2a3 = (*vResult1[1] << (uNumDigits * BASEVALBITS)) + *a4;
	if (a1a2a3 < *B) {
		vResult2.push_back(make_unique<LongInteger>(LongInteger()));
		vResult2.push_back(make_unique<LongInteger>(a1a2a3));
	}
	else {
		vResult2 = DivThreeHalvesByTwo(r1, r2, a4, b1, b2, uNumDigits);
	}
	// 5) Return Q=[q1,q2] and S
	vReturn[1] = move(vResult2[1]);
	vector<LongIntegerUP> Q(2);
	Q[1] = move(vResult1[0]);
	Q[0] = move(vResult2[0]);
	vReturn[0] = move(merge(Q, 2, uNumDigits));

	return move(vReturn);
}

vector<LongIntegerUP> LongInteger::DivThreeHalvesByTwo(LongIntegerUP& a2, LongIntegerUP& a1, LongIntegerUP& a0, LongIntegerUP& b1, LongIntegerUP& b0, UINT uNumDigits) {
	// DivThreeHalvesByTwo(a1,a2,a3,b1,b2)
	//
	// 6) q=[a1,a2]/b1;
	// 7) c=[a1,a2]-q*b1
	// 8) D=q*b2
	// 9) R=[r1,r2]=[c,a3]-D
	//10) if(R<0) {
	//11)   q--;
	//12)   R+=B;
	//13)   if(R<0) {
	//14)      q--;
	//15)      R+=B;
	//16)   }
	//17) }
	//18) Return quotient q and remainder R

	/*
	Input Two nonnegative integers A and B, such that A<βnB and β2n/2 ≤ B <
	β2n. n must be even.
	Output The quotient A/B and the remainder A mod B.
	1. Let A = A2β2n + A1βn + A0 and B = B1βn + B0, with 0 ≤ Ai < βn and
	0 ≤ Bi < βn.
	2. If A2 < B1, compute Q = A2βn+A1 / B1  with remainder R1 using algorithm 3.3;
	otherwise, let Q = βn − 1 and R1 = (A2 − B1)βn + A1 + B1.
	3. R ← R1βn + A4 − QB 0
	4. If R < 0, R ← R + B and Q ← Q − 1.
	5. If R < 0, R ← R + B and Q ← Q − 1.
	6. Return Q = Q and R = R.
	*/

	/*
	For A/B, B needs to be less than A, but more than A/10.
	The second criteria - B >= (BASE^2n)/2 & B < BASE^2n doesn't seem necessary in my tests so far. Will check further

	If B is less than A/10, then it needs to be multiplied by 10 (for base 256, this equates to left shifted)
	and then the final result also needs left shifted by the same amount, the high digits of the remainder added (and removed from the remainder)
	Checking for this is best done in the calling program
	*/

	/*
	This has ended up diverging from the algorithm described and from an example implementation I found, which
	worries me somewhat. Part of it is because I'm working in base 256 and the descriptions
	are for binary, but even so there shouldn't have been major changes.
	There is even a completely new logic block to handle scenarios not mentioned in the original algorithm,
	and an optimisation attempt that is getting so complex I'm losing track of what it does.
	This is all going horribly wrong.
	*/

	vector<LongIntegerUP> vResult(2); // To hold q & R
	vector<LongIntegerUP> vMerge(2);

	LongIntegerUP B(nullptr);
	LongIntegerUP Q, R;

	Q = make_unique<LongInteger>(0);
	R = make_unique<LongInteger>(0);

	if (*b1 != 0) {
		*Q = ((*a2) << (uNumDigits * LongInteger::BASEVALBITS)) + (*a1);
		// The next 2 lines can be done recursively as they are division and modulus

		if (Q->size > BURKINELZIEGLERCUTOFF) {
			LongIntegerUP temp;
			BurnikelZiegler(*Q, *b1, temp, R);
			Q = move(temp);
		}
		else {
			LongIntegerUP Q2 = make_unique<LongInteger>(0);
			DivAndMod(*Q, *b1, Q2, R);
			Q = move(Q2);
		}
	}
	else {
		// I am baffled. The line below doesn't work as expected and instead gives the result 255
		// no matter what value uNumDigits is. However, the calculation works.
		// I've worked out why the number goes wrong - it is calculating 1 rightshifted as a UINT,
		// which can't be shifted more than 3 bytes left.
		// However, I cannot explain why the calculation still works.
		*Q = (1 << (uNumDigits * BASEVALBITS)) - 1;
		*R = ((*a2) << (uNumDigits * LongInteger::BASEVALBITS)) + *a1;
	}

	*R = (*R << (uNumDigits * LongInteger::BASEVALBITS)) + *a0 - (*Q * *b0);

	vMerge[0] = make_unique<LongInteger>(*b0);
	vMerge[1] = make_unique<LongInteger>(*b1);
	B = move(merge(vMerge, 2, uNumDigits));

	// Added to handle a scenario not mentioned in the algorithm
	// Need to speed this up a bit as it can loop for up to 256^uNumDigits-1 times - which can be a lot!
	// Speed up code added by adding left-shifts. uDiff is the number of bits to left-shift
	// There are still awkward scenarios. The worst case is that this loop runs for log2(B) times, which isn't great
	// but a lot less than before

	while (*R >= *B || *R < 0) {
		if (*R >= *B) {
			if (R->size < B->size) {
				*R -= *B;
				(*Q)++;
			}
			else {
				UINT uDiff = R->size - B->size;
				uDiff *= BASEVALBITS;
				// Try to get the exact number of bits different
				if (R->digits[(R->size - 1)] > B->digits[(B->size - 1)]) {
					int uDiffBits = R->digits[(R->size - 1)] / B->digits[(B->size - 1)];
					while (uDiffBits > 1) {
						uDiffBits /= 2;
						uDiff++;
					}
				}
				else {
					int uDiffBits = B->digits[(B->size - 1)] / R->digits[(R->size - 1)];
					while (uDiffBits > 1) {
						uDiffBits /= 2;
						uDiff--;
					}
				}
				// *temp created to check we aren't deleting too large a number
				// It is very hard to get the balance between too large and too small
				// This is an overhead, but it should reduce the number of loops and so
				// save more than it costs
				// Changed the test as I encountered a scenario where the overall while loop went into an infinite cycle
				LongInteger* temp = new LongInteger(*B << uDiff);
				while (temp->size == R->size && temp->digits[temp->size - 1] > R->digits[R->size - 1] && uDiff > 0) {
					*temp >>= 1;
					uDiff--;
				}
				if (temp->size > 1 && temp->size == R->size && temp->digits[temp->size - 1] == R->digits[R->size - 1] &&
					temp->digits[temp->size - 2] > R->digits[R->size - 2] && uDiff > 0) {
					*temp >>= 1;
					uDiff--;
				}

				*R -= *temp;
				delete temp;
				*Q += (LongInteger(1) << uDiff);
			}
		}
		else {
			// In some scenarios R can end up massively negative, so make B a lot bigger
			if (R->size < B->size) {
				// This can happen in rare cases
				*R += *B;
				(*Q)--;
			}
			else {
				UINT uDiff = uDiff = R->size - B->size;
				uDiff *= BASEVALBITS;
				// Try to get the exact number of bits different
				int uDiffBits = R->digits[(R->size - 1)] / B->digits[(B->size - 1)];
				while (uDiffBits > 1) {
					uDiffBits /= 2;
					uDiff++;
				}
				if (uDiff > 0)
					uDiff--;
				if (B->size == 53) {
					if (B->digits[52] == 3) {
						int breakpointint = 0;
					}
				}
				*R += (*B << uDiff);
				*Q -= (LongInteger(1) << uDiff);
			}
		}
	}


	vResult[0] = move(Q);
	vResult[1] = move(R);

	return vResult;

}

LongIntegerUP LongInteger::merge(vector<LongIntegerUP>& vList, UINT uNumParts, UINT uSizeParts) {
	// Merge the contents of liList into a single LongInteger

	// Need to rework this as I've hit scenarios where the components can be bigger than uSizeParts
	// It originally used memcpy for speed, but with potentially overlapping values that can't be used
	LongIntegerUP liReturn;

	LongInteger liWorking;

	for (UINT i = 0; i < uNumParts; i++) {
		liWorking += (*vList[i] << (i * uSizeParts * BASEVALBITS));
	}

	liReturn = make_unique<LongInteger>(liWorking);

	return liReturn;
}


bool LongInteger::factorial()
{
	// Can't find any way other than multiplying sequentially, but I'll have a think

	if (!bPositive)
	{
		bOverflow = true;
		return false;
	}

	if (equalsZero())
	{
		assignNumber(1);
		return !bOverflow;
	}

	LongInteger *counter = new LongInteger(this);
	LongInteger *runningTotal = new LongInteger(1);

	while (*counter > 1 && runningTotal->bPositive)
	{
		*runningTotal *= *counter;
		*runningTotal--;
	}

	if (runningTotal->bPositive)
	{
		copy(*runningTotal);
	}
	else
	{
		bOverflow = true;
	}
	return !bOverflow;
}


LongInteger LongInteger::sqrt(const LongInteger& liInput)
{
	// Calculate the positive square root (rounded to the nearest integer) of the input value

	// Now the question is ... how do you calculate square roots for a number held in a byte array?

	// The square root will have approximately half the number of digits and the leading digit will be approximately the
	// square root of the leading digit (if odd number of digits) or 2 leading digits (if even no of digits)

	LongInteger liGuess; // The guess at what the square root is

	UINT index = 0;
	for (UINT i = 0; i < liInput.size; i+=2)
	{
		UINT temp = liInput.digits[i];
		if ((i + 1) < liInput.size)
			temp += (BASEVAL * liInput.digits[i + 1]);
		temp = std::sqrt(temp);
		LongInteger liTemp(temp);
		liTemp <<= (BASEVALBITS * index);
		liGuess += liTemp;
		index++;
	}

	// We now have a guess for the squareroot.
	// Using the Babylonian method:
	// To calculate sqrt(a), we take guess x and see if x^2 == a.
	// If not, we take (x + a/x)/2 as the new x and try again
	//
	// This won't work as we don't have decimals and what is an acceptable difference when dealing with huge numbers?
	// Also rounding down or up? We need to round down.
	// Solution in terms of decimals is the modulus.
	// So....
	// Take guess x
	// Divide a by x (taking modulus as well)
	// Result is rd.rm
	// answerhigh = rd*rd
	// answermid = rd*rm*2
	// answerlow = rm*rm
	// am2d.am2m = answermid/x
	// ah2 = answerhigh + am2d
	// am2m2 = am2m * x
	// addmods = answerlow + am2m2
	// addmodsdiv = addmods / x (discard modulus)
	// addmodsdiv2 = addmodsdiv / x (discard modulus)
	// final = ah2 + addmodsdiv2
	// 
	// If abs(final - a) is less than x then we are in the right range. If final >= a then this is the answer,
	// else subtract one from final for the answer.
	// If abs(final - a) is greater or equal to x then try a new estimate:
	// x = (x + a/x) / 2


	bool foundX = false;
	while (!foundX)
	{
		// We have a guess at X in liGuess





	}



}