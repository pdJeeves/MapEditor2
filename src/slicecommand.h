#ifndef SLICECOMMAND_H
#define SLICECOMMAND_H
#include "commandinterface.h"
#include "face.h"

class SliceCommand : public CommandInterface
{
	struct SliceData
	{
		SliceData(Face * face);

		Face * original;
		Face * splitFace;

		QPoint vertex[4];
		Face * adjacent[3][4];
		uint8_t permeability[3][4];
	};

	Vertex::List_t prev_selection;
	Vertex::List_t next_selection;

	std::vector<SliceData> m_slice;
	bool own_faces;

public:
	SliceCommand(MapEditor * window, const Face::List_t & list);
	~SliceCommand();

	void rollBack(MapEditor * window);
	void rollForward(MapEditor * window);
	void initialize(MapEditor * window);
};

typedef SliceCommand ExtrudeCommand;

#endif // SLICECOMMAND_H
