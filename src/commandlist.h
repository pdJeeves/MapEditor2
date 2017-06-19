#ifndef COMMANDLIST_H
#define COMMANDLIST_H
#include "commandinterface.h"
#include <vector>
#include <memory>

class MapEditor;
class CommandInterface;

class CommandList
{
typedef std::vector<std::unique_ptr<CommandInterface> > List;
private:
	List		    list;
	int index;
	int savedIndex;
	MapEditor * window;

public:
	CommandList(MapEditor * window);

	bool isDirty() const;
	void onSave();

	bool canRollBack()    const;
	bool canRollForward() const;

	void rollBack();
	void rollForward();

	void push(CommandInterface *);

	void clear();
};

#endif // COMMANDLIST_H
