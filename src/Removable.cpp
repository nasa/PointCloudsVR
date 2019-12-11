#include "PCVR_Scene.hpp"

#include "Removable.hpp"

std::list<Removable*> Removable::_Removables;

void Removable::RemoveAll()
{
	PCVR_Scene::GetFrameManager()->lock();
	while (!_Removables.empty())
	{
		_Removables.front()->remove();
	}
	PCVR_Scene::GetFrameManager()->unlock();
}

Removable::Removable()
{
	_Removables.push_back(this);
}

void Removable::remove()
{
	std::list<Removable*> temp;
	for (Removable* r : _Removables)
	{
		if (r != this)
		{
			temp.push_back(r);
		}
	}
	_Removables = temp;
}