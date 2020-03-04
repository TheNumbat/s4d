
#include "undo.h"

#include "lib/log.h"

Undo::Undo() {}
Undo::~Undo() {}

void Undo::reset() {
    undos = {};
    redos = {};
}

void Undo::update_mesh(Scene& scene, Scene_Object::ID id, Halfedge_Mesh&& old_mesh) {
    Scene_Object& obj = *scene.get(id);
    
    Halfedge_Mesh new_mesh;
    obj.copy_mesh(new_mesh);

    action([id, &scene, nm{std::move(new_mesh)}]() {
        Scene_Object& obj = *scene.get(id);
        obj.set_mesh(nm);
    }, [id, &scene, om{std::move(old_mesh)}]() {
        Scene_Object& obj = *scene.get(id);
        obj.set_mesh(om);
    });
}

void Undo::del_obj(Scene& scene, Scene_Object::ID id) {
    scene.erase(id);
    action([id, &scene](){
        scene.erase(id);
    }, [id, &scene](){
        scene.restore(id);
    });
}

void Undo::add_obj(Scene& scene, GL::Mesh&& mesh) {
    Scene_Object::ID id = scene.add({}, std::move(mesh));
    scene.restore(id);
    action([id, &scene](){
        scene.restore(id);
    }, [id, &scene](){
        scene.erase(id);
    });
};

void Undo::update_obj(Scene& scene, Scene_Object::ID id, Pose new_pos) {
    Scene_Object& obj = *scene.get(id);
    obj.pose = new_pos;
    action([id, &scene, new_pos](){
        Scene_Object& obj = *scene.get(id);
        obj.pose = new_pos;
    }, [id, &scene, old_pos=obj.pose](){
        Scene_Object& obj = *scene.get(id);
        obj.pose = old_pos;
    });
}

void Undo::action(std::unique_ptr<Action_Base>&& action) {
    redos = {};
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
