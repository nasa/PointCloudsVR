#include "PCVR_Selection.hpp"

std::ostream& PCVR_Selectable::writeToStream(std::ostream& o) const
{
	return o << getPos().x() << ','
			 << getPos().y() << ','
			 << getPos().z() << std::endl;
}

std::list<PCVR_Selection*> PCVR_Selection::_Selections;

void PCVR_Selection::SaveAllSelections(const std::string& path, const std::vector<PCVR_Selectable*>& points)
{
	for (PCVR_Selection* sel : _Selections)
	{
		if (!sel->_saved)
		{
			sel->save(path, points);
			sel->_saved = true;
		}
	}
}

void PCVR_Selection::ShowAllSelections(bool b)
{
	for (PCVR_Selection* sel : _Selections)
	{
		sel->show(b);
	}
}


PCVR_Selection::PCVR_Selection()
	: Removable()
{
	_Selections.push_back(this);
}

void PCVR_Selection::remove()
{
	Removable::remove();

	_Selections.remove(this);
}