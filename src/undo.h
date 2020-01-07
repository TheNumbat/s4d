
#pragma once

#include <memory>
#include <stack>

#include "scene/scene.h"

class Action_Base {
    virtual void undo() = 0;
    virtual void redo() = 0;
    friend class Undo;
public:
    virtual ~Action_Base() {}
};

template<typename R, typename U>
class Action : public Action_Base {
public:
    Action(R r, U u) : _undo(u), _redo(r) {};
    ~Action() {}

private:
    U _undo;
    R _redo;
    void undo() {_undo();}
    void redo() {_redo();}
};

class Undo {
public:
    Undo();
    ~Undo();
    
    template<typename T>
    void set(T& old, T next) {
        action(
            [&old, next]() {
                old = next;
            },
            [&old, prev=old]() {
                old = prev;
            });
    }

    template<typename R, typename U> 
    void action(R redo, U undo) {
        action(std::make_unique<Action<R,U>>(redo, undo));
    }

    void undo();
    void redo();
    
private:
    void action(std::unique_ptr<Action_Base> action);
    std::stack<std::unique_ptr<Action_Base>> undos;
    std::stack<std::unique_ptr<Action_Base>> redos;
};
