#pragma once

class Base
{
public:
	Base(void) {};

	void Testinit() { OnTest(); };

	virtual void OnTest(){}
};