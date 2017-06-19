#include "slicecommand.h"
#include "mapeditor.h"
#include <cassert>
#include <cstring>
#include <iostream>

SliceCommand::SliceData::SliceData(Face * face)
{
	original = face;
	splitFace = 0L;

	for(int i = 0; i < 4; ++i)
	{
		vertex         [i] = face->verticies   [i];
		adjacent    [0][i] = face->adjacent    [i];
		permeability[0][i] = face->permeability[i];
	}

	for(int j = 1; j < 2; ++j)
		for(int i = 0; i < 4; ++i)
		{
			adjacent    [j][i] = 0;
			permeability[j][i] = 0;
		}
}

SliceCommand::SliceCommand(MapEditor * window, const Face::List_t & list) :
	prev_selection(window->selection.verts),
	own_faces(false)
{
	m_slice.reserve(list.size());

	for(auto i = list.begin(); i != list.end(); ++i)
	{
		m_slice.push_back(*i);
	}
}

SliceCommand::~SliceCommand()
{
	if(own_faces)
	{
		for(auto i = m_slice.begin(); i != m_slice.end(); ++i)
		{
			delete i->splitFace;
		}
	}
}

void SliceCommand::rollBack(MapEditor * window)
{
	own_faces = true;

	for(auto i = m_slice.begin(); i != m_slice.end(); ++i)
	{
		i->splitFace->setFaceList(0L);

		assert(i->splitFace->faceList() == 0L);

		for(int j = 0; j < 4; ++j)
		{
			i->original->swapVertex((Corner) j, i->vertex[j]);
			i->original->setAdjacent((Direction) j, i->adjacent[0][j]);
			i->original->setPermeability((Direction) j, i->permeability[0][j]);
		}
	}

	window->selection.setSelected(prev_selection);
}

void SliceCommand::rollForward(MapEditor * window)
{
	own_faces = false;

	for(auto i = m_slice.begin(); i != m_slice.end(); ++i)
	{
		i->splitFace->setFaceList(&(window->selection.allFaces));

		for(int j = 0; j < 4; ++j)
		{
			i->original ->swapVertex ((Corner)    j, i->vertex[j]);
			i->original ->setAdjacent((Direction) j,i->adjacent[1][j]);
			i->splitFace->setAdjacent((Direction) j,i->adjacent[2][j]);

			i->original ->setPermeability((Direction) j,i->permeability[1][j]);
			i->splitFace->setPermeability((Direction) j,i->permeability[2][j]);
		}
	}

	window->selection.setSelected(next_selection);
}

void SliceCommand::initialize(MapEditor * window)
{
	next_selection = window->selection.verts;

	for(auto i = m_slice.begin(); i != m_slice.end(); ++i)
	{
//find split faces
		for(int j = 0; j < 4; ++j)
		{
			if(i->original->adjacent[j] && i->original->adjacent[j]->splitFrom == i->original)
			{
				i->splitFace =	i->original->adjacent[j];
				i->splitFace->splitFrom = 0L;
				break;
			}
		}

//copy data
		for(int j = 0; j < 4; ++j)
		{
			i->adjacent    [1][j] = i->original ->adjacent    [j];
			i->permeability[1][j] = i->original ->permeability[j];

			i->adjacent    [2][j] = i->splitFace->adjacent    [j];
			i->permeability[2][j] = i->splitFace->permeability[j];
		}
	}
}
