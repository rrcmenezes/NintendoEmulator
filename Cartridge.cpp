#include "Cartridge.h"
#include <stdio.h>
#include <stdlib.h>

Cartridge::Cartridge()
{
    //ctor
    fp = NULL;
    p_bPRGROM = NULL;
    p_bCHRROM = NULL;
    cMirroring = "H";
}

Cartridge::~Cartridge()
{
    //dtor
    if (p_bPRGROM)
    {
        free(p_bPRGROM);
    }
    if (p_bCHRROM)
    {
        free(p_bCHRROM);
    }
}

bool Cartridge::Init(std::string strFileNameParam)
{
    unsigned char buffer[16];
    
    fopen_s(&fp,strFileNameParam.c_str(), "rb");
    if (fp)
    {
        fread(buffer, sizeof(unsigned char), 16, fp);
        bNumBanksPRGROM = buffer[4];
        bNumBanksCHRROM = buffer[5];
        bFlag6 = buffer[6];
        if ((bFlag6 & 0x01) == 0x01)
        {
            cMirroring = "V";
        }
        else
        {
            cMirroring = "H";
        }
        bFlag7 = buffer[7];
        bFlag8 = buffer[8];
        bFlag9 = buffer[9];
        bFlag10 = buffer[10];
        if (!p_bPRGROM && !p_bCHRROM)
        {
            // alloc bank of program ROM
            p_bPRGROM = (unsigned char *)malloc(sizeof(unsigned char) * 16384 * bNumBanksPRGROM);
            fread(p_bPRGROM, sizeof(unsigned char), sizeof(unsigned char) * 16384 * bNumBanksPRGROM, fp);
            // alloc bank of CHR ROM
            p_bCHRROM = (unsigned char*)malloc(sizeof(unsigned char) * 8192 * bNumBanksCHRROM);
            fread(p_bCHRROM, sizeof(unsigned char), sizeof(unsigned char) * 8192 * bNumBanksCHRROM, fp);
        }
        fclose(fp);

    }
    else
    {

    }
    return true;
}

void Cartridge::SetCPUMem(unsigned char* p_PRGROMParam)
{
    if (p_PRGROMParam)
    {
        // only 1 16k Program ROM
        if (bNumBanksPRGROM == 1)
        {
            memcpy(&p_PRGROMParam[0xC000], this->p_bPRGROM, sizeof(unsigned char) * 16384 * this->bNumBanksPRGROM);
            memcpy(&p_PRGROMParam[0x8000], this->p_bPRGROM, sizeof(unsigned char) * 16384 * this->bNumBanksPRGROM);
        }
        else
        {
            memcpy(&p_PRGROMParam[0x8000], this->p_bPRGROM, sizeof(unsigned char) * 16384 * this->bNumBanksPRGROM);
        }
    }
}

void Cartridge::SetPPUMem(unsigned char* p_CHRROMParam)
{
    if (p_CHRROMParam)
    {
        // only 1 16k Program ROM
        if (bNumBanksCHRROM == 1)
        {
            memcpy(&p_CHRROMParam[0], this->p_bCHRROM, sizeof(unsigned char) * 8192 * this->bNumBanksCHRROM);
        }
    }
}

char Cartridge::GetMirroring()
{
    return cMirroring;
}