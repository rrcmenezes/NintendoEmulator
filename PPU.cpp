
#include "NES.h"
#include "PPU.h"
#include "CPU.h"
#include <iostream>
#include <windows.h>


void PPU::drawPixel(int xParam, int yParam, RGBColor* pr_RGBParam)
{
    unsigned char* pixels = (unsigned char*)backBuffer->pixels;
    pixels[4 * (yParam * backBuffer->w + xParam) + 3] = 255;
    pixels[4 * (yParam * backBuffer->w + xParam) + 2] = pr_RGBParam->R;
    pixels[4 * (yParam * backBuffer->w + xParam) + 1] = pr_RGBParam->G;
    pixels[4 * (yParam * backBuffer->w + xParam) + 0] = pr_RGBParam->B;
}

PPU::PPU()
{
    pr_NES = NULL;
    pr_CPU = NULL;
    pr_CPUMem = NULL;
    pr_PPUMem = NULL;
    iCurrentScanline = 0;
    iLastScanLine = -1;
    byState = 0;
    wBGPaletteAddr = 0x3F00;
    wSPRPaletteAddr = 0x3F10;
    ulFrame = 0;
    iPPUCycles = 3;
    bScanLineDrawn = false;
    bSpriteEvalDone = false;
    iKeyboardState = -1;
}

PPU::~PPU()
{
    SDL_DestroyWindow(m_window);
    SDL_FreeSurface(backBuffer);
    SDL_Quit();
}

void PPU::Reset()
{
    if (SDL_Init(SDL_INIT_VIDEO ) != 0)
    {
        printf("ERROR");
    }
    m_window = SDL_CreateWindow("NES Emulator", 200, 50, 240*3, 240*3, SDL_WINDOW_SHOWN);
    screenSurface = SDL_GetWindowSurface(m_window);


    backBuffer = SDL_CreateRGBSurface(0, 240, 240, 32, 0, 0, 0, 0);


}

void PPU::Init(NES* pr_NESParam, CPU* pr_CPUParam, unsigned char* pr_CPUMemParam, unsigned char* pr_PPUMemParam)
{
    pr_NES = pr_NESParam;
    pr_CPU = pr_CPUParam;
    pr_CPUMem = pr_CPUMemParam;
    pr_PPUMem = pr_PPUMemParam;
}

int PPU::GetCurrentScanLine()
{
    return (iCurrentScanline == 261) ? -1 : iCurrentScanline;
}

int PPU::GetCycle()
{
    return iPPUCycles;
}


void PPU::UpdateStatus(bool bMayTriggerNMIParam,unsigned long int ulClockTicksElapsedParam)
{
    if (ulClockTicksElapsedParam != ulLastClockTicksElapsed)
    {
        unsigned long int uiDeltaCPUClockTicks;
        uiDeltaCPUClockTicks = ulClockTicksElapsedParam - ulLastClockTicksElapsed;
        ulLastClockTicksElapsed = ulClockTicksElapsedParam;

        iPPUCycles = iPPUCycles + (uiDeltaCPUClockTicks * 3);
        if (iPPUCycles >= PPUCYCLES_PER_SCANLINE)
        {
            iCurrentScanline += iPPUCycles / PPUCYCLES_PER_SCANLINE;
            iPPUCycles = iPPUCycles % PPUCYCLES_PER_SCANLINE;
            bScanLineDrawn = false;
        }
        // entering in VBLANK Period
        if ((((iCurrentScanline == 241) && (iPPUCycles >=1)) || (iCurrentScanline > 241)) && iCurrentScanline < 261 && !bTestSetVBLANK)
        {
            BIT_SET(pr_CPUMem[REG_PPU_STATUS], REG2002_VBLANK);
            bTestSetVBLANK = true;
            if (pr_CPUMem[0x2000] & 0x80) //trigger NMI on VBlank
            {
                this->pr_CPU->ExecuteNMI();
            }
        }

        // leaving VBLANK Period
        if ((((iCurrentScanline == 261) && (iPPUCycles >= 1)) || (iCurrentScanline > 261)) && iPPUCycles >= 1 && !bTestClearVBLANK)
        {
            bTestClearVBLANK = true;
            BIT_CLEAR(pr_CPUMem[REG_PPU_STATUS], REG2002_VBLANK);
            BIT_CLEAR(pr_CPUMem[REG_PPU_STATUS], REG2002_SPRHIT);
            BIT_CLEAR(pr_CPUMem[REG_PPU_STATUS], REG2002_SPROVR);
        }


        if (iCurrentScanline > 261)
        {
            iCurrentScanline = iCurrentScanline % SCANLINES_PER_FRAME;
            ulFrame++;
            SDL_FillRect(backBuffer, NULL, 0x000000);
            bFrameDrawn = false;
            bTestClearVBLANK = false;
            bTestSetVBLANK = false;
        }


        //341 cycles to render 1 scanline
        if (iCurrentScanline != iLastScanLine)
        {
            // draw scanlines that werent drawn
            int iScanLineOri, iScanLineDst;
            iScanLineDst = iCurrentScanline;
            iScanLineOri = (iCurrentScanline < iLastScanLine) ? 0 : iLastScanLine;
            if ((iScanLineDst - iScanLineOri) > 1)
            {
                for (int f = iScanLineOri; f < iScanLineDst; f++)
                {
                    bScanLineDrawn = false;
                    this->Render(f);
                }
            }
            iLastScanLine = iCurrentScanline;
        }
    }
    bSpriteSize = (((pr_CPUMem[0x2000] & 0x20) >> 5) == 1)? true:false;
    wBGPatternTabBaseAddr = (((pr_CPUMem[0x2000] & 0x10) >> 4) == 1)? 0x1000:0x0000;
    wSPRPatternTabBaseAddr = (((pr_CPUMem[0x2000] & 0x08) >> 3) == 1) ? 0x1000 : 0x0000;
    bDrawBG = (pr_CPUMem[0x2001] & 0x08) ? true : false;
    if (bDrawSprites)
    {
        int el;
        el = 4;
    }
    bDrawSprites = (pr_CPUMem[0x2001] & 0x10) ? true : false;
    byPPUIncrement = (((pr_CPUMem[0x2000] & 0x04) >> 2) == 1) ? 32 : 1;
    wNameTableBaseAddr = (pr_CPUMem[0x2000] & 0x03);
    if (wNameTableBaseAddr == 0) wNameTableBaseAddr = 0x2000;
        else if (wNameTableBaseAddr == 1) wNameTableBaseAddr = 0x2400;
            else if (wNameTableBaseAddr == 2) wNameTableBaseAddr = 0x2800;
                else wNameTableBaseAddr = 0x2C00;
}


void PPU :: Render()
{
    Render(iCurrentScanline);
}

void PPU::Render(int iScanLineParam)
{
    // Clear the window with a black background
    //SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 255);
    //SDL_RenderDrawPoint(m_renderer, rand() % 256, iCurrentScanLineParam);
    //nametable 32x30 tiles => each tile is 8x8 pixels
    int iTileIndex = 0;
    int iBGIndex = 0;
    //get attribute table
    int iAtribIndex;
    int iQuadX, iQuadY;
    int i2BitsPaletteHigh;

    // visible scanlines, we need to draw
    if (iScanLineParam >= 0 && iScanLineParam <= 239 && bScanLineDrawn == false)
    {
        int yTiles;
        int iTLine; // tile line to draw

        bScanLineDrawn = true;
        yTiles = iScanLineParam / 8;
        iTLine = iScanLineParam % 8;
        iTileIndex = yTiles * 32;
        // draw sprites with priority greather than BG
        //bDrawSprites = true;
        //bDrawBG = true;
        if (bDrawSprites)
        {
            DrawSprites(true);
        }
        if (bDrawBG)
        {
            for (int xTiles = 0; xTiles < 32; xTiles++)
            {
                // busca o indice attribute table, com base no tile 8x8 pixels que vai ser renderizado
                iAtribIndex = ((yTiles / 4) * 8) + (xTiles / 4);

                // pega o Byte que contem os 2 bits de cada regiao na atributre table
                i2BitsPaletteHigh = pr_PPUMem[wNameTableBaseAddr + 0x3C0 + iAtribIndex];

                // busca o quadrado dentro do quadrado 32x32 pixels pra saber em que parte do byte
                //  do incide que tem que selecionar os 2 bits que vai indicar a palette
                iQuadX = (xTiles % 4) / 2;
                iQuadY = (yTiles % 4) / 2;
                if (iQuadX == 0 && iQuadY == 0) i2BitsPaletteHigh &= 0x03;
                if (iQuadX == 1 && iQuadY == 0) i2BitsPaletteHigh = (i2BitsPaletteHigh & 0x0C) >> 2;
                if (iQuadX == 0 && iQuadY == 1) i2BitsPaletteHigh = (i2BitsPaletteHigh & 0x30) >> 4;
                if (iQuadX == 1 && iQuadY == 1) i2BitsPaletteHigh = (i2BitsPaletteHigh & 0xC0) >> 6;

                // pega o indice do pattern table, na name table
                iBGIndex = pr_PPUMem[wNameTableBaseAddr + iTileIndex];

                // constroi linha por linha do tile 8x8 indo na pattern table fazer a operacao logica dos bytes

                memset(vPixelLine, i2BitsPaletteHigh << 2, sizeof(unsigned char) * 8);
                //assembling pixel color and tile of line tile which is described im pattern table
                byPixelLineL = pr_PPUMem[wBGPatternTabBaseAddr + (iBGIndex * 16) + iTLine];
                byPixelLineH = pr_PPUMem[wBGPatternTabBaseAddr + (iBGIndex * 16) + iTLine + 8];
                vPixelLine[0] += (byPixelLineH & 0x80) ? 2 : 0;
                vPixelLine[0] += (byPixelLineL & 0x80) ? 1 : 0;
                vPixelLine[1] += (byPixelLineH & 0x40) ? 2 : 0;
                vPixelLine[1] += (byPixelLineL & 0x40) ? 1 : 0;
                vPixelLine[2] += (byPixelLineH & 0x20) ? 2 : 0;
                vPixelLine[2] += (byPixelLineL & 0x20) ? 1 : 0;
                vPixelLine[3] += (byPixelLineH & 0x10) ? 2 : 0;
                vPixelLine[3] += (byPixelLineL & 0x10) ? 1 : 0;
                vPixelLine[4] += (byPixelLineH & 0x08) ? 2 : 0;
                vPixelLine[4] += (byPixelLineL & 0x08) ? 1 : 0;
                vPixelLine[5] += (byPixelLineH & 0x04) ? 2 : 0;
                vPixelLine[5] += (byPixelLineL & 0x04) ? 1 : 0;
                vPixelLine[6] += (byPixelLineH & 0x02) ? 2 : 0;
                vPixelLine[6] += (byPixelLineL & 0x02) ? 1 : 0;
                vPixelLine[7] += (byPixelLineH & 0x01) ? 2 : 0;
                vPixelLine[7] += (byPixelLineL & 0x01) ? 1 : 0;
                // draw a line of tile into the screen
                if (vPixelLine[0] & 0x03) drawPixel((xTiles * 8) + 0, (yTiles * 8) + iTLine, &Pallete[pr_PPUMem[wBGPaletteAddr + vPixelLine[0]]]);
                if (vPixelLine[1] & 0x03) drawPixel((xTiles * 8) + 1, (yTiles * 8) + iTLine, &Pallete[pr_PPUMem[wBGPaletteAddr + vPixelLine[1]]]);
                if (vPixelLine[2] & 0x03) drawPixel((xTiles * 8) + 2, (yTiles * 8) + iTLine, &Pallete[pr_PPUMem[wBGPaletteAddr + vPixelLine[2]]]);
                if (vPixelLine[3] & 0x03) drawPixel((xTiles * 8) + 3, (yTiles * 8) + iTLine, &Pallete[pr_PPUMem[wBGPaletteAddr + vPixelLine[3]]]);
                if (vPixelLine[4] & 0x03) drawPixel((xTiles * 8) + 4, (yTiles * 8) + iTLine, &Pallete[pr_PPUMem[wBGPaletteAddr + vPixelLine[4]]]);
                if (vPixelLine[5] & 0x03) drawPixel((xTiles * 8) + 5, (yTiles * 8) + iTLine, &Pallete[pr_PPUMem[wBGPaletteAddr + vPixelLine[5]]]);
                if (vPixelLine[6] & 0x03) drawPixel((xTiles * 8) + 6, (yTiles * 8) + iTLine, &Pallete[pr_PPUMem[wBGPaletteAddr + vPixelLine[6]]]);
                if (vPixelLine[7] & 0x03) drawPixel((xTiles * 8) + 7, (yTiles * 8) + iTLine, &Pallete[pr_PPUMem[wBGPaletteAddr + vPixelLine[7]]]);
                iTileIndex++;
            }
        }
        if (bDrawSprites)
        {
            DrawSprites(false);
        }
    }
    if (iScanLineParam == 240 && bScanLineDrawn == false)
    {
        bScanLineDrawn = true;
        SDL_BlitScaled(backBuffer, NULL, screenSurface, NULL);
        SDL_UpdateWindowSurface(m_window);
    }
}

void PPU :: SpriteEvaluation()
{
    if ((iCurrentScanline >= 0) && (iCurrentScanline < 240) && !bSpriteEvalDone && bDrawSprites)
    {
        bSpriteEvalDone = true;
        for (int i = 0; i < 256; i+=4)
        {
            if ((OAM[i] <= iCurrentScanline) && (OAM[i] + 8) >= iCurrentScanline)
            {
                vScanlineSPrites.push_back(i);
            }
        }
    }
}

void PPU::HandleIORegisters(bool bIsWritingParam,unsigned short wAddressParam, unsigned char* pr_DataParam)
{
    switch (wAddressParam)
    {
    case 0x2000:
        // copy 5 least significant bits to 0x2002 register
        if (bIsWritingParam)
        {
            pr_CPUMem[REG_PPU_STATUS] = (pr_CPUMem[REG_PPU_STATUS] & 0xE0) + (*pr_DataParam & 0x1F);
        }
        break;
    case 0x2001:
        pr_CPUMem[REG_PPU_STATUS] = (pr_CPUMem[REG_PPU_STATUS] & 0xE0) + (*pr_DataParam & 0x1F);
        break;
    case 0x2002:
        if (bIsWritingParam == false)
        {
            // when reading register, resets 7th bit
            pr_CPUMem[REG_PPU_STATUS] &= 0x7F;
        }
        break;
    case 0x2003:
        wOAMAddress = *pr_DataParam;
        pr_CPUMem[REG_PPU_STATUS] = (pr_CPUMem[REG_PPU_STATUS] & 0xE0) + (*pr_DataParam & 0x1F);
        break;
    case 0x2004:
        OAM[wOAMAddress] = *pr_DataParam;
        wOAMAddress ++;
        if (wOAMAddress == 256)
        {
            wOAMAddress = 0;
        }
        break;
    case 0x2005:
        pr_CPUMem[REG_PPU_STATUS] = (pr_CPUMem[REG_PPU_STATUS] & 0xE0) + (*pr_DataParam & 0x1F);
        break;
    case 0x2006:
        switch (byState)
        {
        case 2:
        case 0:
            wVRAMAddress = (*pr_DataParam & 0x00FF) << 8;
            byState=1;
            break;
        case 1:
            wVRAMAddress += *pr_DataParam;
            byState=2;
            break;
        default:
            break;
        }
        pr_CPUMem[REG_PPU_STATUS] = (pr_CPUMem[REG_PPU_STATUS] & 0xE0) + (*pr_DataParam & 0x1F);
        break;
    case 0x2007:
        switch (byState)
        {
        case 2:
            pr_PPUMem[wVRAMAddress % 0x4000] = *pr_DataParam;
            wVRAMAddress += byPPUIncrement;
            // copy 5 least significant bits to 0x2002 register
            pr_CPUMem[REG_PPU_STATUS] = (pr_CPUMem[REG_PPU_STATUS] & 0xE0) + (*pr_DataParam & 0x1F);
            break;
        default:
            break;
        }
        break;
    case 0x4014:
        int iSize;
        iSize = 256 - wOAMAddress;
        memcpy(&OAM[wOAMAddress], &pr_CPUMem[0x100 * *pr_DataParam], sizeof(unsigned char) * iSize);
        pr_CPU->ulClockTicks = (pr_CPU->ulClockTicks % 2 == 1) ? pr_CPU->ulClockTicks + 513 + 1 : pr_CPU->ulClockTicks + 513;
        this->UpdateStatus(true,pr_CPU->ulClockTicks);
        break;
    case 0x4016: // controller 1
        if (bIsWritingParam == false and iKeyboardState == 1)
        {
            *pr_DataParam = 0x40;
            pr_CPUMem[wAddressParam] = 0;
            if (pr_NES->iCurrentScanningButton < 8)
            {
                unsigned char byte;
                if (((pr_NES->byButtonsPressed & eKey_SELECT) && (pr_NES->iCurrentScanningButton == 2)) ||
                   ((pr_NES->byButtonsPressed & eKey_START) && (pr_NES->iCurrentScanningButton == 3)) ||
                   ((pr_NES->byButtonsPressed & eKey_A) && (pr_NES->iCurrentScanningButton == 0)) ||
                    ((pr_NES->byButtonsPressed & eKey_B) && (pr_NES->iCurrentScanningButton == 1)) ||
                    ((pr_NES->byButtonsPressed & eKey_UP) && (pr_NES->iCurrentScanningButton == 4)) ||
                    ((pr_NES->byButtonsPressed & eKey_DOWN) && (pr_NES->iCurrentScanningButton == 5)) ||
                    ((pr_NES->byButtonsPressed & eKey_LEFT) && (pr_NES->iCurrentScanningButton == 6)) ||
                    ((pr_NES->byButtonsPressed & eKey_RIGHT) && (pr_NES->iCurrentScanningButton == 7)))
                {
                    pr_NES->byButtonsPressed = pr_NES->byButtonsPressed & !(0x01 << pr_NES->iCurrentScanningButton);
                    *pr_DataParam = 0x41;
                }
            }
            pr_NES->iCurrentScanningButton++;
            if (pr_NES->iCurrentScanningButton == 8)
            {
                pr_NES->iCurrentScanningButton = 0;
            }
        }
        else
        {
            if ((iKeyboardState == -1 || iKeyboardState == 1) && *pr_DataParam == 1)
            {
                iKeyboardState = 0;
            }
            else if ((iKeyboardState == 0 || iKeyboardState == 1) && *pr_DataParam == 0)
            {
                iKeyboardState = 1;
                pr_NES->iCurrentScanningButton = 0;
            }
            pr_NES->iCurrentScanningButton = 0;
            pr_CPUMem[wAddressParam] = 0;
        }
        break;
    case 0x4017:
        pr_CPUMem[wAddressParam] = 0x00;
        *pr_DataParam = 0x40;
        break;
    default:
        break;
    }
}


void PPU::DrawSprites(bool bBehindBGParam)
{
    int iPosY,iPosX;
    int i2BitsPaletteHigh;
    unsigned char byTileIndex;
        


    for (int i = 0; i < 256; i += 4)
    {
        // make sure wether sprite may be drawn behind or in front
        if (((OAM[i + 2] & 0x20) == 0x20) && bBehindBGParam || 
            ((OAM[i + 2] & 0x20) == 0x00) && (bBehindBGParam == false))
        {
            iPosX = OAM[i + 3]; 
            iPosY = OAM[i];            
            byTileIndex = OAM[i + 1];
            
            i2BitsPaletteHigh = OAM[i + 2] & 0x03;

            // constroi linha por linha do tile 8x8 indo na pattern table fazer a operacao logica dos bytes
            for (int iTLine = 0; iTLine < 8; iTLine++)
            {
                memset(vPixelLine, i2BitsPaletteHigh << 2, sizeof(unsigned char) * 8);
                //assembling pixel color and tile of line tile which is described im pattern table
                byPixelLineL = pr_PPUMem[wSPRPatternTabBaseAddr + (byTileIndex * 16) + iTLine];
                byPixelLineH = pr_PPUMem[wSPRPatternTabBaseAddr + (byTileIndex * 16) + iTLine + 8];
                vPixelLine[0] += (byPixelLineH & 0x80) ? 2 : 0;
                vPixelLine[0] += (byPixelLineL & 0x80) ? 1 : 0;
                vPixelLine[1] += (byPixelLineH & 0x40) ? 2 : 0;
                vPixelLine[1] += (byPixelLineL & 0x40) ? 1 : 0;
                vPixelLine[2] += (byPixelLineH & 0x20) ? 2 : 0;
                vPixelLine[2] += (byPixelLineL & 0x20) ? 1 : 0;
                vPixelLine[3] += (byPixelLineH & 0x10) ? 2 : 0;
                vPixelLine[3] += (byPixelLineL & 0x10) ? 1 : 0;
                vPixelLine[4] += (byPixelLineH & 0x08) ? 2 : 0;
                vPixelLine[4] += (byPixelLineL & 0x08) ? 1 : 0;
                vPixelLine[5] += (byPixelLineH & 0x04) ? 2 : 0;
                vPixelLine[5] += (byPixelLineL & 0x04) ? 1 : 0;
                vPixelLine[6] += (byPixelLineH & 0x02) ? 2 : 0;
                vPixelLine[6] += (byPixelLineL & 0x02) ? 1 : 0;
                vPixelLine[7] += (byPixelLineH & 0x01) ? 2 : 0;
                vPixelLine[7] += (byPixelLineL & 0x01) ? 1 : 0;
                // draw a line of tile into the screen
                if (vPixelLine[0] & 0x03) drawPixel(iPosX + 0, iPosY + iTLine, &Pallete[pr_PPUMem[wSPRPaletteAddr + vPixelLine[0]]]);
                if (vPixelLine[1] & 0x03) drawPixel(iPosX + 1, iPosY + iTLine, &Pallete[pr_PPUMem[wSPRPaletteAddr + vPixelLine[1]]]);
                if (vPixelLine[2] & 0x03) drawPixel(iPosX + 2, iPosY + iTLine, &Pallete[pr_PPUMem[wSPRPaletteAddr + vPixelLine[2]]]);
                if (vPixelLine[3] & 0x03) drawPixel(iPosX + 3, iPosY + iTLine, &Pallete[pr_PPUMem[wSPRPaletteAddr + vPixelLine[3]]]);
                if (vPixelLine[4] & 0x03) drawPixel(iPosX + 4, iPosY + iTLine, &Pallete[pr_PPUMem[wSPRPaletteAddr + vPixelLine[4]]]);
                if (vPixelLine[5] & 0x03) drawPixel(iPosX + 5, iPosY + iTLine, &Pallete[pr_PPUMem[wSPRPaletteAddr + vPixelLine[5]]]);
                if (vPixelLine[6] & 0x03) drawPixel(iPosX + 6, iPosY + iTLine, &Pallete[pr_PPUMem[wSPRPaletteAddr + vPixelLine[6]]]);
                if (vPixelLine[7] & 0x03) drawPixel(iPosX + 7, iPosY + iTLine, &Pallete[pr_PPUMem[wSPRPaletteAddr + vPixelLine[7]]]);
            }
        }
    }
}
