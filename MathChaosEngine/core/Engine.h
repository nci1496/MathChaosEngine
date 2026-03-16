#pragma once

#include "Module.h"

class Engine
{

public:

    Module* currentModule;

    Engine();

    void setModule(Module* m);

    void update(double dt);

    void render(CDC* pDC);
};