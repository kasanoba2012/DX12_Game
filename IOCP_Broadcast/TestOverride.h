#pragma once
#include <iostream>
#include "Base.h"

class TestOverride : public Base
{
	void OnTest() override
	{
		std::cout << "TestOverride\n";
	}
};

