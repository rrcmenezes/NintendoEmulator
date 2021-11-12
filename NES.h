#pragma once


#include "Cartridge.h"
#include "CPU.h"
#include "PPU.h"
#include "Memory.h"

enum eKeys
{
    eKey_A          = 0x01,
    eKey_B          = 0x02,
    eKey_SELECT     = 0x04,
    eKey_START      = 0x08,
    eKey_UP         = 0x10,
    eKey_DOWN       = 0x20,
    eKey_LEFT       = 0x40,
    eKey_RIGHT      = 0x80,
};
#define TOTAL_CALLING_BUTTONS = 24




class NES
{
    CPU CPU;
    unsigned char CPUMem[0x10000];
    PPU PPU;
    unsigned char PPUMem[0x4000];
    Cartridge *p_RefCartridge;
public:
    int iCurrentScanningButton;
    unsigned char byButtonsPressed;
    NES();
    virtual ~NES();
    bool InsertCartridge(Cartridge *p_CartParam);
    void PowerUp();
    bool Update();
    void Mirroring(unsigned short wAddressParam, unsigned char byDataParam);
};
