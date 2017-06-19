#include "deletioncommand.h"
#include "mapeditor.h"


DeletionCommand::DeletionData::DeletionData(Face * face) :
	face(face),
	adjacent{0,0,0,0},
	near{0,0,0,0},
	permeability{{0,0,0,0},{0,0,0,0}}
{
	for(int i = 0; i < 4; ++i)
	{
		near[i]     = face->adjacent[i];
		permeability[0][i] = face->permeability[i];
	}
}

DeletionCommand::DeletionCommand(const Face::List_t & faces) :
	owns_faces(false)
{
	m_list.reserve(faces.size());

	for(auto i = faces.begin(); i != faces.end(); ++i)
	{
		m_list.push_back(DeletionData(*i));
	}
}

DeletionCommand::~DeletionCommand()
{
	if(owns_faces)
	{
		for(auto i = m_list.begin(); i != m_list.end(); ++i)
		{
			delete i->face;
		}
	}
}

void DeletionCommand::rollBack(MapEditor * window)
{
	owns_faces = false;

	Vertex::List_t verts;
//	verts.reserve(m_list.size());

	for(auto i = m_list.begin(); i != m_list.end(); ++i)
	{
		i->face->setFaceList(&(window->selection.allFaces));

		for(int j = 0; j < 4; ++j)
		{
			if(i->near[j])
			{
				i->face->setAdjacent    ((Direction) j,  i->near[j]);
				i->face->setPermeability((Direction) j,  i->permeability[0][j]);
			}

			verts.push_back(&(i->face->verticies[j]));
		}
	}

	window->selection.selectVerticies(std::move(verts));
}

void DeletionCommand::rollForward(MapEditor * window)
{
	owns_faces = true;

	for(auto i = m_list.begin(); i != m_list.end(); ++i)
	{
		i->face->setFaceList(0L);

		for(int j = 0; j < 4; ++j)
		{
			if(i->near[j])
			{
				i->near[j]->setAdjacent    ((Direction) (j^1), i->adjacent[j]);
				i->near[j]->setPermeability((Direction) (j^1), i->permeability[1][j]);
			}
		}
	}

	window->selection.selectVerticies(Vertex::List_t());
}

void DeletionCommand::initialize(MapEditor *)
{
	for(auto i = m_list.begin(); i != m_list.end(); ++i)
	{
		for(int j = 0; j < 4; ++j)
		{
			if(i->near[j])
			{
				i->adjacent		  [j] = i->near[j]->adjacent[j^1];
				i->permeability[1][j] = i->near[j]->permeability[j^1];
			}
		}
	}
}
