#ifndef ROOMTYPECOMMAND_H
#define ROOMTYPECOMMAND_H

#include "commandinterface.h"
#include "selection.h"

class RoomTypeCommand : public CommandInterface
{
	struct RoomTypeState
	{
		Face * face;
		uint8_t init_type;
	};


typedef std::vector<RoomTypeState> RoomTypeState_t;
	Selection::List_t              selected;
	RoomTypeState_t op;
	uint8_t final_type;

	RoomTypeCommand(Selection & selection, RoomTypeState_t && op, int permeability);

public:
	static RoomTypeCommand * Create(Selection & selection, int permeability);

	void rollBack(MapEditor * window);
	void rollForward(MapEditor * window);
	void initialize(MapEditor * window);
};

#endif // ROOMTYPECOMMAND_H
