#ifndef INSERTIONCOMMAND_H
#define INSERTIONCOMMAND_H
#include "commandinterface.h"
#include "face.h"

class InsertionCommand : public CommandInterface
{
	struct InsertionData
	{
		InsertionData(Face* face);
		Face * face;
		Face * adjacent[4];
		uint8_t permeability[4];
	};

	Vertex::List_t verts;
	std::vector<InsertionData> m_list;
	bool owns_faces;

public:
	InsertionCommand(const Face::List_t & faces, const Vertex::List_t & verts);
	~InsertionCommand();

	void rollBack(MapEditor * window);
	void rollForward(MapEditor * window);
	void initialize(MapEditor * );
};

#endif // INSERTIONCOMMAND_H
