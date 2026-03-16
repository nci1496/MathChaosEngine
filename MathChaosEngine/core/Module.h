#pragma once
#include<afxwin.h>

class Module {
public:
    virtual ~Module() {};
    //virtual void reset() = 0;

    //virtual void update(double dt) = 0;

    //virtual void render(CDC* pDC) = 0;
    virtual void reset()=0;

    virtual void update(double dt)=0;

    virtual void render(CDC* pDC)=0;


    virtual void onMouseDown(int x, int y)=0;
    virtual void onMouseMove(int x, int y) {};




};