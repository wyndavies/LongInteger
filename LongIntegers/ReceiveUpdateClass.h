#pragma once

// Interface class to define common messages the subclasses can receive

template<class T>
class ReceiveUpdateClass
{
public:
	// If I make the constructor virtual Visual Studio refuses to compile. Dunno why. GCC has no issues.
	ReceiveUpdateClass();
	// Virtual destructor needed so I can assign a subclass to a ReceiveUpdateClass pointer and not get memory leaks
	// if delete is called on that pointer
	virtual ~ReceiveUpdateClass();

	virtual void iHaveFinished(unsigned int) = 0;
	virtual void iAmWaiting() = 0;
	virtual void iHaveStoppedWaiting() = 0;
	
};

#include "ReceiveUpdateClass.hpp"