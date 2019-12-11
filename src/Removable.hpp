#pragma once

#include <functional>
#include <list>

#include "PCVR_Controller.hpp"
#include "PCVR_Tool.hpp"

class Removable
{
public:
	static void RemoveAll();

	Removable();
	virtual void remove() = 0;

private:
	static std::list<Removable*> _Removables;
};