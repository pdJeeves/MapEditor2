#include "grabcommand.h"
#include "mapeditor.h"

GrabCommand::GrabState::GrabState(Face * face) :
	face(face)
{
	for(int i = 0; i < 4; ++i)
	{
		verticies[i] = face->verticies[i].stored;
		adjacent[i] = face->adjacent[i];
		permeability[i] = face->permeability[i];
	}
}

GrabCommand::GrabCommand(Selection & selection) :
	list(selection.verts)
{
	state.reserve(selection.faces.size());

	for(auto i = selection.faces.begin(); i != selection.faces.end(); ++i)
	{
		state.push_back(GrabState(*i));
	}
}


void GrabCommand::swapPositions(MapEditor * window)
{
	for(auto i = state.begin(); i != state.end(); ++i)
	{
		for(int j = 0; j < 4; ++j)
		{
			i->face->swapVertex((Corner) j, i->verticies[j]);
			i->face->swapAdjacent((Direction) j, i->adjacent[j]);
			i->face->swapPermeability((Direction) j, i->permeability[j]);
		}

		i->face->computeBounds();
	}

	window->selection.setSelected(list);
}

void GrabCommand::rollBack(MapEditor * window)
{
	swapPositions(window);
}

void GrabCommand::rollForward(MapEditor * window)
{
	swapPositions(window);
}

void GrabCommand::initialize(MapEditor *)
{
}
