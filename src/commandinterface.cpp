#if 0
#include "commandinterface.h"
#include "face.h"

void CommandInterface::swapAdjacent(Face * face, uint8_t d, Face *&adjacent, uint8_t &permeability)
{
	if(!face) return;

	Face  * adj  = face->adjacent[d];
	uint8_t perm = face->permeability[d];

	face->setAdjacent((Direction) d, adjacent);
	face->setPermeability((Direction) d, permeability);

	adjacent     = adj;
	permeability = perm;
}

#endif
