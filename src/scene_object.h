
#pragma once

class Scene_Object {
public:
    typedef int ID;

    Scene_Object();
    ~Scene_Object();

    void render();

    ID id = 0;

private:
};
