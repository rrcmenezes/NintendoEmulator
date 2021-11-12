#include "NES.h"

NES::NES()
{
    p_RefCartridge = NULL;
    
    CPU.Init(this,&PPU,CPUMem);
    PPU.Init(this, &CPU, CPUMem, PPUMem);
    iCurrentScanningButton = 0;
}

NES::~NES()
{
}

bool NES::InsertCartridge(Cartridge *p_CartParam)
{
    p_RefCartridge = p_CartParam;
    p_RefCartridge->SetCPUMem(CPUMem);
    p_RefCartridge->SetPPUMem(PPUMem);

    return false;
}

void NES::PowerUp()
{
    CPU.Reset();
    PPU.Reset();
}

bool NES::Update()
{
    bool CPUok = false;
    SDL_Event event;
    SDL_PollEvent(&event);
    switch (event.type)
    {
    case SDL_KEYDOWN:
        switch (event.key.keysym.sym) 
        {
            case SDLK_LEFT:
                byButtonsPressed = byButtonsPressed | eKey_LEFT;
                break;
            case SDLK_RIGHT:
                byButtonsPressed = byButtonsPressed | eKey_RIGHT;
                break;
            case SDLK_UP:
                byButtonsPressed = byButtonsPressed | eKey_UP;
                break;
            case SDLK_a:
                byButtonsPressed = byButtonsPressed | eKey_A;
                break;
            case SDLK_s:
                byButtonsPressed = byButtonsPressed | eKey_B;
                break;
            case SDLK_z:
                byButtonsPressed = byButtonsPressed | eKey_SELECT;
                break;
            case SDLK_x:
                byButtonsPressed = byButtonsPressed | eKey_START;
                break;
            case SDLK_DOWN:
                byButtonsPressed = byButtonsPressed | eKey_DOWN;
                break;
            default:
                break;
        }
        break;
    default:
        break;
    }
    
    CPUok = CPU.Run();
    PPU.UpdateStatus(true,CPU.GetElapsedClockTicks());
    PPU.SpriteEvaluation();
#ifdef DRAW_SOMETHING
    PPU.Render();
#endif

    if (CPUok)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void NES::Mirroring(unsigned short wAddressParam, unsigned char byDataParam)
{
    return;
}
