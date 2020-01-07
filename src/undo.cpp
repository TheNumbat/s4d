
#include "undo.h"

#include "lib/log.h"

Undo::Undo() {}
Undo::~Undo() {}

void Undo::action(std::unique_ptr<Action_Base> action) {
    redos = {};
    action->redo();
    undos.push(std::move(action));
}

void Undo::undo() {
    if (undos.empty()) return;
    undos.top()->undo();
    redos.push(std::move(undos.top()));
    undos.pop();
}

void Undo::redo() {
    if(redos.empty()) return;
    redos.top()->redo();
    undos.push(std::move(redos.top()));
    redos.pop();
}
