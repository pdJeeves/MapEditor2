#include "commandlist.h"
#include "mapeditor.h"

CommandList::CommandList(MapEditor * window) :
	window(window)
{
	index = 0;
	savedIndex = 0;
}

bool CommandList::isDirty() const
{
	return index != savedIndex;
}

void CommandList::onSave()
{
	savedIndex = index;
}

bool CommandList::canRollBack()    const
{
	return index > 0;
}

bool CommandList::canRollForward() const
{
	return index < (int) list.size();
	window->onCommandPushed();
}

void CommandList::rollBack()
{
	if(canRollBack())
	{
		--index;
		list[index]->rollBack(window);
		window->onCommandPushed();
	}
}

void CommandList::rollForward()
{
	if(canRollForward())
	{
		list[index]->rollForward(window);
		++index;
		window->onCommandPushed();
	}
}

void CommandList::push(CommandInterface * it)
{
	if(!it)
		return;

	if(index != (int) list.size())
	{
		//index is 1 past the last command run
		list.erase(list.begin() + index, list.end());

		if(savedIndex > index)
		{
			savedIndex = -1;
		}
	}

	list.emplace_back(it);
	index = list.size();
	it->initialize(window);

	window->onCommandPushed();
}

void CommandList::clear()
{
	list.clear();
	index = 0;
	savedIndex = 0;
	window->onCommandPushed();
}
