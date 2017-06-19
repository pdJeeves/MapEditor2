#ifndef GRABCOMMAND_H
#define GRABCOMMAND_H
#include "selection.h"
#include "commandinterface.h"

class GrabCommand : public CommandInterface
{
	struct GrabState
	{
		GrabState(Face * face);

		Face * face;
		QPoint verticies[4];
		Face * adjacent[4];
		uint8_t permeability[4];
	};

	std::vector<GrabState> state;
	Vertex::List_t list;

protected:
	void swapPositions(MapEditor * window);

public:
	GrabCommand(Selection & selection);

	void rollBack(MapEditor * window);
	void rollForward(MapEditor * window);
	void initialize(MapEditor * window);
};

#endif // GRABCOMMAND_H
