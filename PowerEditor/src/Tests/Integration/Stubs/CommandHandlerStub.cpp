// CommandHandlerStub.cpp - Provides CommandHandler implementation for tests
//
// WHY THIS IS NEEDED:
// The CommandHandler class is defined in NppCommands.h and implemented in
// QtCore/NppCommands.cpp. However, NppCommands.cpp also implements
// NppCommands which depends on Notepad_plus, ScintillaEditView, and the
// entire application core. We only need the CommandHandler methods for
// unit testing, so we replicate them here to avoid pulling in heavy deps.

#include "NppCommands.h"

namespace QtCommands {

void CommandHandler::registerCommand(int id, CommandHandlerFunc handler)
{
	_handlers[id] = std::move(handler);
}

void CommandHandler::executeCommand(int id)
{
	auto it = _handlers.find(id);
	if (it != _handlers.end() && it->second)
	{
		it->second();
	}
}

bool CommandHandler::canExecute(int id) const
{
	auto it = _handlers.find(id);
	return it != _handlers.end() && it->second != nullptr;
}

void CommandHandler::unregisterCommand(int id)
{
	_handlers.erase(id);
}

void CommandHandler::clearCommands()
{
	_handlers.clear();
}

} // namespace QtCommands
