#include <exception>
#include <string>
#include <iostream>
#include <stdio.h>
#include "NES.h"
#include "CPU.h"
#include "Cartridge.h"
#include "main.h"



int main(int argc,char **args)
{
    NES Nes;
    Cartridge cart;

    cart.Init("C:\\Users\\robertocm\\Documents\\GitHub\\Nes\\NESTEST.nes");
 

    Nes.InsertCartridge(&cart);

    Nes.PowerUp();

    

    while (1) 
    {
        
        Nes.Update();
     
    }

return 1;

}