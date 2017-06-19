#ifndef DELETIONCOMMAND_H
#define DELETIONCOMMAND_H
#include "commandinterface.h"
#include "face.h"


class DeletionCommand : public CommandInterface
{
	struct DeletionData
	{
		DeletionData(Face* face);
		Face * face;
		Face * adjacent[4];
		Face * near[4];
		uint8_t permeability[2][4];
	};

	std::vector<DeletionData> m_list;
	bool owns_faces;

public:
	DeletionCommand(const Face::List_t & faces);
	~DeletionCommand();

	void rollBack(MapEditor * window);
	void rollForward(MapEditor * window);
	void initialize(MapEditor * window);
};

#endif // DELETIONCOMMAND_H
