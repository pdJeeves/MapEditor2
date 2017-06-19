#ifndef PERMEABILITYCOMMAND_H
#define PERMEABILITYCOMMAND_H
#include "commandinterface.h"
#include "selection.h"
#include <vector>
#include <cstdint>

class PermeabilityCommand : public CommandInterface
{
	struct PermeabilityState
	{
		Face * face;
		uint8_t edge;
		uint8_t initPerm;
	};

typedef std::vector<PermeabilityState> PermeabilityState_t;
	Selection::List_t              selected;
	PermeabilityState_t op;
	uint8_t finalPerm;

	PermeabilityCommand(Selection & selection, PermeabilityState_t && op, int permeability);

public:
	static PermeabilityCommand * Create(Selection & selection, int permeability);

	void rollBack(MapEditor * window);
	void rollForward(MapEditor * window);
	void initialize(MapEditor * window);
};

#endif // PERMEABILITYCOMMAND_H
