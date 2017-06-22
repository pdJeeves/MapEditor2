#include "permeabilitycommand.h"
#include "mapeditor.h"

PermeabilityCommand * PermeabilityCommand::Create(Selection & selection, int permeability)
{
	PermeabilityState_t op;

	for(auto i = selection.faces.begin(); i != selection.faces.end(); ++i)
	{
		for(int j = 0; j < 4; ++j)
		{
			if((*i)->isEdgeSelected((Direction) j) && (*i)->canSetPermeability((Direction) j))
			{
				PermeabilityState s = { *i, (uint8_t) j, (uint8_t) (*i)->getPermeability((Direction) j) };
				op.push_back(s);
			}
		}
	}

	if(!op.size())
		return 0L;

	return new PermeabilityCommand(selection, std::move(op), permeability);
}

PermeabilityCommand::PermeabilityCommand(Selection & selection, PermeabilityState_t && op, int permeability) :
	selected(selection.verts),
	op(op),
	finalPerm(permeability)
{
}

void PermeabilityCommand::rollBack(MapEditor * window)
{
	for(auto i = op.begin(); i != op.end(); ++i)
	{
		i->face->setPermeability((Direction) i->edge, i->initPerm);
	}

	window->selection.setSelected(selected);
}

void PermeabilityCommand::rollForward(MapEditor * window)
{
	for(auto i = op.begin(); i != op.end(); ++i)
	{
		i->face->setPermeability((Direction) i->edge, finalPerm);
	}

	window->selection.setSelected(selected);
}

void PermeabilityCommand::initialize(MapEditor * window)
{
	for(auto i = op.begin(); i != op.end(); ++i)
	{
		i->face->setPermeability((Direction) i->edge, finalPerm);
	}
}
