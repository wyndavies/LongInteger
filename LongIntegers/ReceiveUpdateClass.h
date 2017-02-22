#pragma once
class ReceiveUpdateClass
{
public:
	ReceiveUpdateClass();
	~ReceiveUpdateClass();

	virtual void receiveUpdate(UINT) = 0;
};

