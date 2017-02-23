#pragma once

// Interface class to define common messages the subclasses can receive

class ReceiveUpdateClass
{
public:
	ReceiveUpdateClass();
	~ReceiveUpdateClass();

	virtual void iHaveFinished(UINT) = 0;
};

