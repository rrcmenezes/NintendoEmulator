#pragma once
#include <string>

class Cartridge
{
public:
    Cartridge();
    virtual ~Cartridge();

    bool Init(std::string strFileNameParam);

    void SetCPUMem(unsigned char* p_PRGROMParam);
    void SetPPUMem(unsigned char* p_CHRROMParam);
    char GetMirroring();

protected:
    bool cMirroring;
    unsigned char bNumBanksPRGROM; // 16k Page Count
    unsigned char bNumBanksCHRROM; // 8K Page Count
    unsigned char bFlag6, bFlag7, bFlag8, bFlag9, bFlag10;
    unsigned char *p_bPRGROM ;
    unsigned char* p_bCHRROM;

private:
    FILE* fp;
};

