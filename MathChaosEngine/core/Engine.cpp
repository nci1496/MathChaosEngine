#include "pch.h" 
#include "Engine.h"

Engine::Engine()
{
    currentModule = nullptr;
}

void Engine::setModule(Module* m)
{
    currentModule = m;

    if (currentModule)
    {
        currentModule->reset();
    }
}

void Engine::update(double dt)
{
    if (currentModule)
    {
        currentModule->update(dt);
    }
}

void Engine::render(CDC* pDC)
{
    if (currentModule)
    {
        currentModule->render(pDC);
    }
}