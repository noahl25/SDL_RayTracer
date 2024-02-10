 

#include <iostream>

#include <SDL.h>
#include "RayTracer.h"
 

int main(int argc, char** args)
{
    float scale = 1;
    
    RayTracer rayTracer("RayTracer", 800 * scale, 500 * scale);
    rayTracer.Run();

    return 0;

}

 
