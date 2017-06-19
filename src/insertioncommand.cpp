#include "insertioncommand.h"
#include "mapeditor.h"

InsertionCommand::InsertionData::InsertionData(Face* face) :
	face(face),
	adjacent{0L, 0L, 0L},
	permeability{0, 0, 0, 0}
{
}


InsertionCommand::InsertionCommand(const Face::List_t & faces, const Vertex::List_t & verts) :
	verts(verts),
	owns_faces(false)
{
	for(auto i = faces.begin(); i != faces.end(); ++i)
	{
		m_list.push_back(InsertionData(*i));
	}
}

InsertionCommand::~InsertionCommand()
{
	if(owns_faces)
	{
		for(auto i = m_list.begin(); i != m_list.end(); ++i)
		{
			delete i->face;
		}
	}
}

void InsertionCommand::rollBack(MapEditor * window)
{
	owns_faces = true;

	for(auto i = m_list.begin(); i != m_list.end(); ++i)
	{
		i->face->setFaceList(0L);

		for(int j = 0; j < 4; ++j)
		{
			i->face->setAdjacent((Direction) j, 0L);
			i->face->setPermeability((Direction) j, 0L);
		}
	}

	window->selection.setSelected(verts);
}

void InsertionCommand::rollForward(MapEditor * window)
{
	owns_faces = false;

	Vertex::List_t verts;

	for(auto i = m_list.begin(); i != m_list.end(); ++i)
	{
		i->face->setFaceList(&(window->selection.allFaces));

		for(int j = 0; j < 4; ++j)
		{
			i->face->setAdjacent((Direction) j, i->adjacent[j]);
			i->face->setPermeability((Direction) j, i->permeability[j]);
			verts.push_back(&(i->face->verticies[j]));
		}
	}

	window->selection.selectVerticies(std::move(verts));
}

void InsertionCommand::initialize(MapEditor * )
{
	for(auto i = m_list.begin(); i != m_list.end(); ++i)
	{
		for(int j = 0; j < 4; ++j)
		{
			i->adjacent    [j] = i->face->adjacent    [j];
			i->permeability[j] = i->face->permeability[j];
		}
	}
}

