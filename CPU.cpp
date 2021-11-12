#include "CPU.h"

CPU::CPU()
{
    //ctor
    pr_NES = NULL;
    A = 0x24;
    X = 0x04;
    Y = 0x28;
    SP = 0xFA;
    S = 0x04;
    ulClockTicks = 0;
    pr_PPU = NULL;
    p_byValue= NULL;
}


CPU::~CPU()
{
    //dtor
}


void CPU::Init(NES *pr_NESParam,PPU *pr_PPUParam, unsigned char* pr_CPUMemParam)
{
    pr_NES = pr_NESParam;
    pr_CPUMem = pr_CPUMemParam;
    pr_PPU = pr_PPUParam;

    for (int i = 0; i < 256; i++)
    {
        p_vOpcodes[i] = NULL;
    }
    for (int i = 0; i < 256; i++)
    {
        if (stOpVector[i].byOpcode)
        {
            p_vOpcodes[stOpVector[i].byOpcode] = &stOpVector[i];
        }
    }

#ifdef GENERATE_LOG
    fopen_s(&fp, "log.txt", "wt");
#endif
}

// execute RESET interrupt, to start a game
void CPU::Reset()
{
    PC = (pr_CPUMem[RESET + 1] << 8) + pr_CPUMem[RESET];
    ulClockTicks += 8;
    this->pr_PPU->UpdateStatus(false,ulClockTicks);
}

bool CPU::Run()
{
    byOpcode = pr_CPUMem[PC];


#ifdef GENERATE_LOG
    if (this->ulClockTicks >= 626110) //0xC91E
    {

        printf("EXIT FORCADO POR CLOCKTICKS\n");
        fclose(fp);
        exit(0);
    }
#endif

    bool bSamePage = true;
    int iAddCycles;
    iAddCycles = 0;
    wAddress = 0;

    if (p_vOpcodes[byOpcode])
    {
        /* Getting data from addressing modes*/
        switch (p_vOpcodes[byOpcode]->AddressMode)
        {
        case eImmedt:
            p_byValue = &pr_CPUMem[PC + 1];
            break;
        case eAccumu:
            p_byValue = &A;
            break;
        case eImplid:

            break;
        case eAbsolu:
            wAddress = (pr_CPUMem[PC + 2] << 8) + pr_CPUMem[PC + 1];
            p_byValue = &pr_CPUMem[wAddress];
            break;
        case eIndrct:
            wAddress = pr_CPUMem[PC + 1];
            wAddress = (pr_CPUMem[PC + 2] << 8) + wAddress;
            wAddress = (pr_CPUMem[wAddress + 1] << 8) + pr_CPUMem[wAddress];

            break;
        case eZeroPg:
            wAddress = pr_CPUMem[PC + 1];
            p_byValue = &pr_CPUMem[wAddress];
            break;
        case eRelatv:
            wAddress = PC + (signed char)pr_CPUMem[PC + 1];
            wAddress += 2;
            p_byValue = &pr_CPUMem[wAddress];
            bSamePage = (((PC + p_vOpcodes[byOpcode]->byNumBytes) & 0xFF00) == (wAddress & 0xFF00)) ? true : false;
            break;
        case eZPIdxX: // Zero Page Indexed with X : $addr8 + X
            wAddress = pr_CPUMem[PC + 1];
            wAddress += X;
            p_byValue = &pr_CPUMem[wAddress];
            break;
        case eZPIdxY: // Zero Page Indexed with Y : $addr8 + Y
            wAddress = pr_CPUMem[PC + 1];
            wAddress += Y;
            p_byValue = &pr_CPUMem[wAddress];
            break;
        case eAbIdxX:   // Absolute Indexed with X : $addr16 + X
            wAddress = (pr_CPUMem[PC + 2] << 8) + pr_CPUMem[PC + 1];
            wAddress += X;
            p_byValue = &pr_CPUMem[wAddress];
            break;
        case eAbIdxY:   // Absolute Indexed with Y : $addr16 + Y
            wAddress = (pr_CPUMem[PC + 2] << 8) + pr_CPUMem[PC + 1];
            wAddress += Y;
            p_byValue = &pr_CPUMem[wAddress];
            break;
        case eIndIdx:   // Indirect Indexed with Y : ($addr8) + Y
            wAddress = pr_CPUMem[pr_CPUMem[PC + 1]];
            bSamePage = ((wAddress + Y) & 0xFF00) ? false : true;
            wAddress = (pr_CPUMem[pr_CPUMem[PC + 1] + 1] << 8) + wAddress;
            wAddress += Y;
            p_byValue = &pr_CPUMem[wAddress];
            break;
        default:
            printf("\nADDRESSING MODE NOT IMPLEMENTED YET\n");
            printf("\nOPCODE 0x%X \n", pr_CPUMem[PC]);
            printf("\nElapsed Cycles: %d \n", ulClockTicks);
            printf("\nPC: 0x%X \n", PC);
            exit(0);
            return false;
            break;
        }

        if (wAddress == 0)
        {
            int ke;
            ke = 4;
        }
#ifdef GENERATE_LOG
        char buffer[200];
        if (p_byValue)
        {
            sprintf_s(buffer, 200, "%X $%02X  ADDR:%04X DATA:%02X                A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%d  SL:%d  CPU Cycle:%d\n", PC, byOpcode, wAddress, *p_byValue, A, X, Y, S, SP, pr_PPU->GetCycle(), pr_PPU->GetCurrentScanLine(), ulClockTicks);
        }
        else
        {
            sprintf_s(buffer, 200, "%X $%02X  ADDR:%04X DATA:%02X                A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%d  SL:%d  CPU Cycle:%d\n", PC, byOpcode, wAddress, 0, A, X, Y, S, SP, pr_PPU->GetCycle(), pr_PPU->GetCurrentScanLine(), ulClockTicks);
        }
        fwrite(buffer, sizeof(char), strlen(buffer), fp);
#endif

        /* need to increment PC ?*/
        bool bIncPC = true;
        
        switch (p_vOpcodes[byOpcode]->OpcName)
        {
        case ADC: // Operation:  A + M + C -> A, C
            unsigned short wResult;
            wResult = A + *p_byValue + GET_BIT(S, CRY_POS);
            SP_NEGATIVE(wResult);
            SP_ZERO(wResult);
            if ((wResult & 0xFF00) != 0)
            {
                BIT_SET(S, CRY_POS);
            }
            else
            {
                BIT_CLEAR(S, CRY_POS);
            }
            // With r = a + b, overflow occurs if both a and b are negative and r is positive,
            // or both a and b are positive and r is negative. Looking at sign bits of a, b, r,
            // overflow occurs when 0 0 1 or 1 1 0, so we can use simple xor logic to figure it out.
            if (((unsigned short)A ^ wResult) & ((unsigned short)*p_byValue ^ wResult) & 0x0080)
            {
                BIT_SET(S, OVR_POS);
            }
            else
            {
                BIT_CLEAR(S, OVR_POS);
            }
            A = (unsigned char)wResult;
            break;
        case AND:
            A = A & (*p_byValue);
            SP_NEGATIVE(A);
            SP_ZERO(A);
            break;
        case ASL: // must verify address mode to use memory or accumulator
            if (p_vOpcodes[byOpcode]->AddressMode == eAccumu)
            {
                if (GET_BIT(A, 7))
                {
                    BIT_SET(S, CRY_POS);
                }
                else
                {
                    BIT_CLEAR(S, CRY_POS);
                }
                A = A << 1;
                SP_NEGATIVE(A);
                SP_ZERO(A);
            }
            else
            {
                if (GET_BIT(*p_byValue, 7))
                {
                    BIT_SET(S, CRY_POS);
                }
                else
                {
                    BIT_CLEAR(S, CRY_POS);
                }
                *p_byValue = (*p_byValue) << 1;
                SP_NEGATIVE(*p_byValue);
                SP_ZERO(*p_byValue);
            }
            break;
        case BCC:
            if (GET_BIT(S, CRY_POS) == 0)
            {
                PC = wAddress;
                bIncPC = false;
                iAddCycles = (bSamePage) ? 1 : 2;
            }
            break;
        case BCS:
            if (GET_BIT(S, CRY_POS) == 1)
            {
                PC = wAddress;
                bIncPC = false;
                iAddCycles = (bSamePage) ? 1 : 2;
            }
            break;
        case BEQ:
            if (GET_BIT(S, ZRO_POS) == 1)
            {
                PC = wAddress;
                bIncPC = false;
                iAddCycles = (bSamePage) ? 1 : 2;
            }
            break;
        case BNE:
            if (GET_BIT(S, ZRO_POS) == 0)
            {
                PC = wAddress;
                bIncPC = false;
                iAddCycles = (bSamePage) ? 1 : 2;
            }
            break;
        case BMI:
            if (GET_BIT(S, NEG_POS) == 1)
            {
                PC = wAddress;
                bIncPC = false;
                iAddCycles = (bSamePage) ? 1 : 2;
            }
            break;
        case BPL:
            if (GET_BIT(S, NEG_POS) == 0)
            {
                PC = wAddress;
                bIncPC = false;
                iAddCycles = (bSamePage) ? 1 : 2;
            }
            break;
        case BRK:
            printf("BRK NAO IMPLEMENTADO");
            exit(0);
            break;
        case CMP:
            if (A >= *p_byValue)
            {
                BIT_SET(S, CRY_POS);
            }
            else BIT_CLEAR(S, CRY_POS);
            SP_NEGATIVE(A - *p_byValue);
            SP_ZERO(A - *p_byValue);

            break;
        case BVC:
            if (GET_BIT(S, OVR_POS) == 0)
            {
                PC = wAddress;
                bIncPC = false;
                iAddCycles++;
                iAddCycles = (bSamePage) ? 0 : p_vOpcodes[byOpcode]->byPageCycles;
            }
            break;
        case BVS:
            if (GET_BIT(S, OVR_POS) == 1)
            {
                PC = wAddress;
                bIncPC = false;
                iAddCycles++;
                iAddCycles = (bSamePage) ? 0 : p_vOpcodes[byOpcode]->byPageCycles;
            }
            break;
        case CLV:
            BIT_CLEAR(S, OVR_POS);
            break;
        case CPX:
            if (X >= *p_byValue)
            {
                BIT_SET(S, CRY_POS);
            }
            else BIT_CLEAR(S, CRY_POS);
            SP_NEGATIVE(X - *p_byValue);
            SP_ZERO(X - *p_byValue);
            break;
        case CPY:
            if (Y >= *p_byValue)
            {
                BIT_SET(S, CRY_POS);
            }
            else BIT_CLEAR(S, CRY_POS);
            SP_NEGATIVE(Y - *p_byValue);
            SP_ZERO(Y - *p_byValue);
            break;
        case BIT:
            if (*p_byValue & 0x80)
            {
                BIT_SET(S, NEG_POS);
            }
            else
            {
                BIT_CLEAR(S, NEG_POS);
            }
            if (*p_byValue & 0x40)
            {
                BIT_SET(S, OVR_POS);
            }
            else
            {
                BIT_CLEAR(S, OVR_POS);
            }
            if (A & *p_byValue)
            {
                BIT_CLEAR(S, ZRO_POS);
            }
            else
            {
                BIT_SET(S, ZRO_POS);
            }
            break;
        case DEC:
            pr_CPUMem[wAddress]--;
            SP_NEGATIVE(pr_CPUMem[wAddress]);
            SP_ZERO(pr_CPUMem[wAddress]);
            break;
        case DEX:
            X--;
            SP_NEGATIVE(X);
            SP_ZERO(X);
            break;
        case DEY:
            Y--;
            SP_NEGATIVE(Y);
            SP_ZERO(Y);
            break;
        case EOR:
            A ^= *p_byValue;
            SP_NEGATIVE(A);
            SP_ZERO(A);
            break;
        case CLC:
            BIT_CLEAR(S, CRY_POS);
            break;
        case INC:
            *p_byValue = *p_byValue + 1;
            SP_NEGATIVE(A);
            SP_ZERO(A);
            break;
        case INX:
            X++;
            SP_NEGATIVE(X);
            SP_ZERO(X);
            break;
        case INY:
            Y++;
            SP_NEGATIVE(Y);
            SP_ZERO(Y);
            break;
        case JMP:
            PC = wAddress;
            bIncPC = false;
            break;
        case JSR:
            PC += 3 - 1;
            pr_CPUMem[STACK_PAGE + SP] = PC & 0x00FF;
            SP--;
            pr_CPUMem[STACK_PAGE + SP] = (PC & 0xFF00) >> 8;
            SP--;
            PC = wAddress;
            bIncPC = false;
            break;
        case LDA:
            iAddCycles = (bSamePage) ? 0 : p_vOpcodes[byOpcode]->byPageCycles;
            pr_PPU->UpdateStatus(false,this->ulClockTicks + p_vOpcodes[byOpcode]->byNumCycles + iAddCycles);
            if (wAddress == 0x4016)
            {
                pr_PPU->HandleIORegisters(false, wAddress, p_byValue);
                A = *p_byValue;
            }
            else
            {
                A = *p_byValue;
                pr_PPU->HandleIORegisters(false, wAddress, p_byValue);
            }
            SP_NEGATIVE(A);
            SP_ZERO(A);
            break;
        case LDX:
            X = *p_byValue;
            SP_NEGATIVE(*p_byValue);
            SP_ZERO(*p_byValue);
            pr_PPU->HandleIORegisters(false, wAddress, p_byValue);
            
            break;
        case LDY:
            Y = *p_byValue;
            SP_NEGATIVE(*p_byValue);
            SP_ZERO(*p_byValue);
            pr_PPU->HandleIORegisters(false, wAddress, p_byValue);
            break;
        case LSR:
            if ((*p_byValue & 0x01) == 0x01)
            {
                BIT_SET(S, CRY_POS);
            }
            else
            {
                BIT_CLEAR(S, CRY_POS);
            }
            A = A >> 1;
            SP_NEGATIVE(A);
            SP_ZERO(A);
            break;
        case NOP:
            break;
        case ORA:
            A = A | *p_byValue;
            SP_NEGATIVE(A);
            SP_ZERO(A);
            break;
        case ROL:
            unsigned char bNewCarry;
            bNewCarry = (A & 0x80) >> 7;
            *p_byValue = *p_byValue << 1;
            *p_byValue = (GET_BIT(S, CRY_POS)) ? *p_byValue + 1 : *p_byValue;
            if (bNewCarry)
            {
                BIT_SET(S, CRY_POS);
            }
            else
            {
                BIT_CLEAR(S, CRY_POS);
            }
            SP_NEGATIVE(*p_byValue);
            SP_ZERO(*p_byValue);
            break;
        case ROR:
            bNewCarry = (*p_byValue & 0x01);
            *p_byValue = *p_byValue >> 1;
            *p_byValue = (GET_BIT(S, CRY_POS)) ? *p_byValue | 0x80 : *p_byValue;
            if (bNewCarry)
            {
                BIT_SET(S, CRY_POS);
            }
            else
            {
                BIT_CLEAR(S, CRY_POS);
            }
            SP_NEGATIVE(*p_byValue);
            SP_ZERO(*p_byValue); 
            break;
        case RTS:
            SP++;
            PC = pr_CPUMem[STACK_PAGE + SP];
            SP++;
            PC = pr_CPUMem[STACK_PAGE + SP] + ((PC & 0x00FF) << 8);
            break;
        case RTI:
            SP++;
            PC = pr_CPUMem[STACK_PAGE + SP] << 8;
            SP++;
            PC += pr_CPUMem[STACK_PAGE + SP];
            SP++;
            S = pr_CPUMem[STACK_PAGE + SP];
            bIncPC = false;
            break;
        case SBC:
            wResult = (unsigned short)A - (unsigned short)*p_byValue - (unsigned short)GET_BIT(S, CRY_POS);
            SP_NEGATIVE(wResult);
            SP_ZERO(wResult);
            if ((wResult & 0xFF00) != 0)
            {
                BIT_SET(S, CRY_POS);
            }
            else
            {
                BIT_CLEAR(S, CRY_POS);
            }
            if (((unsigned short)A ^ wResult) & ((unsigned short)*p_byValue ^ wResult) & 0x0080)
            {
                BIT_SET(S, OVR_POS);
            }
            else
            {
                BIT_CLEAR(S, OVR_POS);
            }
            A = (unsigned char)wResult;

            break;
        case STA:
            pr_CPUMem[wAddress] = A;
            pr_PPU->HandleIORegisters(true,wAddress, &A);
            
            Mirroring(wAddress, A);
            break;
        case STX:
            pr_CPUMem[wAddress] = X;
            pr_PPU->HandleIORegisters(true, wAddress, &X);
            
            Mirroring(wAddress, X);
            break;
        case SEC:
            BIT_SET(S, CRY_POS);
            break;
        case SED:
            BIT_SET(S, DCM_POS);
            break;
        case SEI:
            BIT_SET(S, IRQ_POS);
            break;
        case CLD:
            BIT_CLEAR(S, DCM_POS);
            break;
        case STY:
            pr_CPUMem[wAddress] = Y;
            pr_PPU->HandleIORegisters(true, wAddress, &Y);
            
            Mirroring(wAddress, Y);
            break;
        case PLA:
            SP++;
            A = pr_CPUMem[STACK_PAGE + SP];
            SP_NEGATIVE(A);
            SP_ZERO(A);
            break;
        case PLP:
            SP++;
            S = pr_CPUMem[STACK_PAGE + SP];
            break;
        case PHA:
            pr_CPUMem[STACK_PAGE + SP] = A;
            SP--;
            break;
        case PHP:
            pr_CPUMem[STACK_PAGE + SP] = S;
            SP--;
            break;
        case TAX:
            X = A;
            SP_NEGATIVE(X);
            SP_ZERO(X);
            break;
        case TAY:
            Y = A;
            SP_NEGATIVE(Y);
            SP_ZERO(Y);
            break;
        case TSX:
            X = SP;
            SP_NEGATIVE(X);
            SP_ZERO(X);
            break;
        case TXA:
            A = X;
            SP_NEGATIVE(A);
            SP_ZERO(A);
            break;
        case TXS:
            SP = X;
            break;
        case TYA:
            A = Y;
            SP_NEGATIVE(A);
            SP_ZERO(A);
            break;
        default:
            printf("\nOPCODE 0x%X NOT IMPLEMENTED YET\n", pr_CPUMem[PC]);
            fclose(fp);
            exit(0);
            break;
        }
        this->ulClockTicks += p_vOpcodes[byOpcode]->byNumCycles;
        this->ulClockTicks += iAddCycles;
        this->PC = (bIncPC)? PC + p_vOpcodes[byOpcode]->byNumBytes: PC;
    }
    else
    {
        printf("\nOPCODE 0x%X NOT IMPLEMENTED YET\n", pr_CPUMem[PC]);
        printf("\nElapsed Cycles: %d \n", ulClockTicks);
        printf("\nPC: 0x%X \n", PC);
#ifdef GENERATE_LOG
        fclose(fp);
#endif
        exit(0);
        return false;
    }
    return true;
}

unsigned long int CPU::GetElapsedClockTicks()
{
    return ulClockTicks;
}

void CPU::ExecuteNMI()
{
#ifdef GENERATE_LOG
    char buffer[200];
    sprintf_s(buffer, 200, "[NMI - Cycle: 86963]\n");
    fwrite(buffer, sizeof(char), strlen(buffer), fp);
#endif // GENERATE_LOG

    pr_CPUMem[STACK_PAGE + SP] = S;
    SP--;
    pr_CPUMem[STACK_PAGE + SP] = PC & 0x00FF;
    SP--;
    pr_CPUMem[STACK_PAGE + SP] = (PC & 0xFF00) >> 8;
    SP--;
    PC = (pr_CPUMem[NMI + 1] << 8) + pr_CPUMem[NMI];
    ulClockTicks += 7;
    pr_PPU->UpdateStatus(false,ulClockTicks);
}

void CPU::Mirroring(unsigned short wAddressParam, unsigned char byDataParam)
{
    // mirroring PRG RAM
    if (wAddressParam >= 0x0000 && wAddressParam < 0x0800)
    {
        pr_CPUMem[wAddressParam + 0x0800] = byDataParam;
        pr_CPUMem[wAddressParam + (0x0800 * 2)] = byDataParam;
        pr_CPUMem[wAddressParam + (0x0800 * 3)] = byDataParam;
    }
    // mirroring IO Registeres
    if (wAddressParam >= 0x2000 && wAddressParam < 0x2008)
    {
        for (int i = 1; i < 1024; i++)
        {
            pr_CPUMem[wAddressParam + (i * 8)] = byDataParam;
        }
        int ee;
        ee = 4;

    }
}
