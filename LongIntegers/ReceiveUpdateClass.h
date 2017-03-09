#pragma once

// Interface class to define common messages the subclasses can receive

template<class T>
class ReceiveUpdateClass
{
public:
	ReceiveUpdateClass();
	virtual ~ReceiveUpdateClass();

	virtual void iHaveFinished(unsigned int);
};

#include "ReceiveUpdateClass.hpp"