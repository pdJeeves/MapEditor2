#include "roomtypecommand.h"
#include "mapeditor.h"

RoomTypeCommand * RoomTypeCommand::Create(Selection & selection, int room_type)
{
	RoomTypeState_t op;

	for(auto i = selection.faces.begin(); i != selection.faces.end(); ++i)
	{
		if((*i)->isSelected() && (*i)->room_type != room_type)
		{
			RoomTypeState s = { *i, (*i)->room_type };
			op.push_back(s);
		}
	}

	if(!op.size())
		return 0L;

	return new RoomTypeCommand(selection, std::move(op), room_type);
}

RoomTypeCommand::RoomTypeCommand(Selection & selection, RoomTypeState_t && op, int room_type) :
	selected(selection.verts),
	op(op),
	final_type(room_type)
{
}

void RoomTypeCommand::rollBack(MapEditor * window)
{
	for(auto i = op.begin(); i != op.end(); ++i)
	{
		i->face->room_type = i->init_type;
	}

	window->selection.setSelected(selected);
}

void RoomTypeCommand::rollForward(MapEditor * window)
{
	for(auto i = op.begin(); i != op.end(); ++i)
	{
		i->face->room_type = final_type;
	}

	window->selection.setSelected(selected);
}

void RoomTypeCommand::initialize(MapEditor * window)
{
	for(auto i = op.begin(); i != op.end(); ++i)
	{
		i->face->room_type = final_type;
	}
}

