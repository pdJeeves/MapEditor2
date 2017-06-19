#ifndef COMMANDBASE_H
#define COMMANDBASE_H

class MapEditor;

struct CommandInterface
{
	virtual ~CommandInterface() {}
	virtual void rollBack(MapEditor * window) = 0;
	virtual void rollForward(MapEditor * window) = 0;
	virtual void initialize(MapEditor * window) { rollForward(window); }
};

#endif // COMMANDBASE_H
