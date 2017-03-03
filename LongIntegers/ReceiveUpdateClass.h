#pragma once

// Interface class to define common messages the subclasses can receive

class ReceiveUpdateClass
{
public:
	ReceiveUpdateClass();
	virtual ~ReceiveUpdateClass();

	virtual void iHaveFinished(unsigned int) = 0;
};

