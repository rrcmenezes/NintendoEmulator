#pragma once
#include "main.h"
#include "PPU.h"
#include <string>

class NES;

#define STACK_PAGE 0x0100
#define NMI     0xFFFA
#define RESET   0xFFFC
#define IRQ_BRK 0xFFFE
#define CRY_POS 0
#define ZRO_POS 1
#define IRQ_POS 2
#define DCM_POS 3
#define BRK_POS 4
#define OVR_POS 6
#define NEG_POS 7
#define BIT_SET(a,b) ((a) |= (1ULL<<(b)))
#define BIT_CLEAR(a,b) ((a) &= ~(1ULL<<(b)))
#define BIT_FLIP(a,b) ((a) ^= (1ULL<<(b)))
//#define BIT_CHECK(a,b) (!!((a) & (1ULL<<(b))))        // '!!' to make sure this returns 0 or 1
#define SP_NEGATIVE(a) if (GET_BIT(a, NEG_POS)) BIT_SET(S, NEG_POS); else BIT_CLEAR(S, NEG_POS)
#define SP_ZERO(a) (a==0)? BIT_SET(S, ZRO_POS) : BIT_CLEAR(S, ZRO_POS)
#define GET_BIT(a,b) ((a >> b) & 0x01) 

enum eAdressMode
{
    eImmedt = 0x0001, // Immediate : #value
    eImplid = 0x0002, // Implied : no operand
    eAccumu = 0x0004, // Accumulator : no operand
    eRelatv = 0x0008, // Relative : $addr8 used with branch instructions
    eZeroPg = 0x0010, // Zero Page : $addr8
    eZPIdxX = 0x0020, // Zero Page Indexed with X : $addr8 + X
    eZPIdxY = 0x0040, // Zero Page Indexed with Y : $addr8 + Y
    eAbsolu = 0x0080, // Absolute : $addr16
    eAbIdxX = 0x0100, // Absolute Indexed with X : $addr16 + X
    eAbIdxY = 0x0200, // Absolute Indexed with Y : $addr16 + Y
    eIndrct = 0x0400, // Indirect : ($addr8) used only with JMP
    eIdxInd = 0x0800, // Indexed with X Indirect : ($addr8 + X)
    eIndIdx = 0x1000, // Indirect Indexed with Y : ($addr8) + Y
};

namespace OpCodeName
{
    enum Type
    {
        ADC, AND, ASL,
        BCC, BCS, BEQ, BIT, BMI, BNE, BPL, BRK, BVC, BVS,
        CLC, CLD, CLI, CLV, CMP, CPX, CPY, DEC, DEX, DEY,
        EOR, INC, INX, INY,
        JMP, JSR,
        LDA, LDX, LDY, LSR,
        NOP,
        ORA,
        PHA, PHP, PLA, PLP,
        ROL, ROR, RTI, RTS,
        SBC, SEC, SED, SEI, STA, STX, STY,
        TAX, TAY, TSX, TXA, TXS, TYA,
        NumTypes
    };

    static const char* String[] =
    {
        "ADC", "AND", "ASL",
        "BCC", "BCS", "BEQ", "BIT", "BMI", "BNE", "BPL", "BRK", "BVC", "BVS",
        "CLC", "CLD", "CLI", "CLV", "CMP", "CPX", "CPY", "DEC", "DEX", "DEY",
        "EOR", "INC", "INX", "INY",
        "JMP", "JSR",
        "LDA", "LDX", "LDY", "LSR",
        "NOP",
        "ORA",
        "PHA", "PHP", "PLA", "PLP",
        "ROL", "ROR", "RTI", "RTS",
        "SBC", "SEC", "SED", "SEI", "STA", "STX", "STY",
        "TAX", "TAY", "TSX", "TXA", "TXS", "TYA",
    };
}


struct strOpcode
{
    unsigned char byOpcode;
    OpCodeName::Type OpcName;
    eAdressMode AddressMode;
    unsigned char byNumBytes;
    unsigned char byNumCycles;
    unsigned char byPageCycles;
};

using namespace OpCodeName;

/* 256 positions to opcodes*/
static strOpcode stOpVector[] = {
    {0x69,ADC,eImmedt,2,2,0},  // Add with Carry 
    {0x6D,ADC,eAbsolu,3,4,0},
    {0x65,ADC,eZeroPg,2,3,0},
    {0x75,ADC,eZPIdxX,2,4,0},
    {0x7D,ADC,eAbIdxX,3,4,0},
    {0x29,AND,eImmedt,2,2,0},  // AND A
    {0x25,AND,eZeroPg,2,3,0},  
    {0x35,AND,eZPIdxX,2,4,0},
    {0x2D,AND,eAbsolu,3,4,0},
    {0x3D,AND,eAbIdxX,3,4,1},
    {0x0A,ASL,eAccumu,1,2,0},  // Aritmetic shift left
    {0x06,ASL,eZeroPg,2,5,0},
    {0x0E,ASL,eAbsolu,3,6,0},
    {0x1E,ASL,eAbIdxX,3,7,0},
    {0x90,BCC,eRelatv,2,2,0},  // Branch if Carry Set
    {0xB0,BCS,eRelatv,2,2,0},  // Branch if Carry Set 
    {0xF0,BEQ,eRelatv,2,2,0},  // Branch if Equal 
    {0x2C,BIT,eAbsolu,3,4,0},  // Branch if carry clear
    {0x24,BIT,eZeroPg,2,3,0},
    {0xD0,BNE,eRelatv,2,2,0},  // Branch if carry clear
    {0x30,BMI,eRelatv,2,2,0},  // Branch if carry clear
    {0x10,BPL,eRelatv,2,2,0},  // Branch on Plus
    {0x00,BRK,eImplid,1,7,0},  // break interrupt
    {0x50,BVC,eRelatv,2,2,1},  // Branch if Overflow Clear
    {0x70,BVS,eRelatv,2,2,1},  // Branch if Overflow Set
    {0xB8,CLV,eImplid,1,2,0},  // Clear overflow flag
    {0xC9,CMP,eImmedt,2,2,0},  // CoMPare accumulator 
    {0xCD,CMP,eAbsolu,3,4,0},
    {0xD9,CMP,eAbIdxY,3,4,0},
    {0xDD,CMP,eAbIdxX,3,4,1},
    {0xC5,CMP,eZeroPg,2,3,0},
    {0xE4,CPX,eZeroPg,2,3,0},  // Compare X Register
    {0xE0,CPX,eImmedt,2,2,0},
    {0xEC,CPX,eAbsolu,3,4,0},
    {0xC0,CPY,eImmedt,2,2,0},  // ComPare Y register 
    {0xCC,CPY,eAbsolu,3,4,0},
    {0xC4,CPY,eZeroPg,2,3,0},
    {0xC6,DEC,eZeroPg,2,5,0},  // Decrement Memory 
    {0xD6,DEC,eZPIdxX,2,6,0},
    {0xCE,DEC,eAbsolu,3,6,0},
    {0xDE,DEC,eAbIdxX,3,7,0},
    {0xCA,DEX,eImplid,1,2,0},  // Decrement X 
    {0x88,DEY,eImplid,1,2,0},  // Decrement Y 
    {0x45,EOR,eZeroPg,2,3,0},  // Exclusive OR A
    {0x49,EOR,eImmedt,2,2,0},
    {0x4D,EOR,eAbsolu,3,4,0},
    {0x55,EOR,eZPIdxX,2,4,0},
    {0x5D,EOR,eAbIdxX,3,4,0},
    {0xE6,INC,eZeroPg,2,5,0},  // Increment Memory
    {0xEE,INC,eAbsolu,3,6,0},
    {0xFE,INC,eAbIdxX,3,7,0},
    {0xE8,INX,eImplid,1,2,0},  // Increment X
    {0xC8,INY,eImplid,1,2,0},  // Increment Y
    {0x4C,JMP,eAbsolu,3,3,0},  // Jump
    {0x6C,JMP,eIndrct,3,5,0},
    {0x20,JSR,eAbsolu,3,6,0},  // Jump to Subroutine
    {0xA9,LDA,eImmedt,2,2,0},  // Load Accumulator
    {0xAD,LDA,eAbsolu,3,4,0},
    {0xA5,LDA,eZeroPg,2,3,0},
    {0xB5,LDA,eZPIdxX,2,4,0},
    {0xBD,LDA,eAbIdxX,3,4,1},
    {0xB9,LDA,eAbIdxY,3,4,1},
    {0xB1,LDA,eIndIdx,2,5,1},
    {0xA1,LDA,eIdxInd,2,6,0},
    {0xA2,LDX,eImmedt,2,2,0},  // Load X
    {0xA6,LDX,eZeroPg,2,3,0},
    {0xAE,LDX,eAbsolu,3,4,0},
    {0xBE,LDX,eAbIdxY,3,4,1},
    {0xB6,LDX,eZPIdxY,2,4,0},
    {0xA0,LDY,eImmedt,2,2,0},  // Load Y
    {0xAC,LDY,eAbsolu,3,4,0},
    {0xA4,LDY,eZeroPg,2,3,0},
    {0xB4,LDY,eZPIdxX,2,4,0},
    {0xBC,LDY,eAbIdxX,3,4,0},
    {0x4A,LSR,eAccumu,1,2,0},  // Logical Shift Right
    {0x46,LSR,eZeroPg,2,5,0},
    {0x4E,LSR,eAbsolu,3,6,0},
    {0x5E,LSR,eAbIdxX,3,7,0},
    {0xEA,NOP,eImplid,1,2,0},  // No Operation opcode
    {0x09,ORA,eImmedt,2,2,0},  // OR A
    {0x05,ORA,eZeroPg,2,3,0},
    {0x0D,ORA,eAbsolu,3,4,0},
    {0x11,ORA,eIndIdx,2,5,1},
    {0x15,ORA,eZPIdxX,2,4,0},
    {0x1D,ORA,eAbIdxX,3,4,1},
    {0x2A,ROL,eAccumu,1,2,0}, // Rotate Left
    {0x26,ROL,eZeroPg,2,5,0},
    {0x36,ROL,eZPIdxX,2,6,0},
    {0x3E,ROL,eAbIdxX,3,7,0},
    {0x6A,ROR,eAccumu,1,2,0}, // Rotate Right
    {0x66,ROR,eZeroPg,2,5,0},
    {0x7E,ROR,eAbIdxX,3,7,0},
    {0x40,RTI,eImplid,1,6,0}, // Return from Interrupt
    {0x60,RTS,eImplid,1,6,0}, // Return from Subroutine
    {0xE5,SBC,eZeroPg,2,3,0}, // Subtract with carry
    {0xE9,SBC,eImmedt,2,2,0},
    {0x38,SEC,eImplid,1,2,0}, // Set Carry Flag
    {0xED,SBC,eAbsolu,3,4,0},
    {0xF9,SBC,eAbIdxY,3,4,1},
    {0xFD,SBC,eAbIdxX,3,4,1},
    {0x78,SEI,eImplid,1,2,0}, // Set Interrupt Disable
    {0xF8,SED,eImplid,1,2,0}, // Set Decimal Flag
    {0x8D,STA,eAbsolu,3,4,0}, // Store Accumulator
    {0x91,STA,eIndIdx,2,6,0}, 
    {0x85,STA,eZeroPg,2,3,0},
    {0x95,STA,eZPIdxX,2,4,0},
    {0x9D,STA,eAbIdxX,3,5,0},
    {0x99,STA,eAbIdxY,3,5,0},
    {0x86,STX,eZeroPg,2,3,0}, // Store X Register into Memory
    {0x8E,STX,eAbsolu,3,4,0},
    {0x96,STX,eZPIdxY,2,4,0},
    {0x84,STY,eZeroPg,2,3,0}, // Store Y Register into Memory
    {0x8C,STY,eAbsolu,3,4,0},
    {0x94,STY,eZPIdxX,2,4,0},
    {0xD8,CLD,eImplid,1,2,0},
    {0x18,CLC,eImplid,1,2,0}, // Clear Carry Flag
    {0x68,PLA,eImplid,1,4,0}, // Pull Acummulator
    {0x28,PLP,eImplid,1,4,0}, // Pull into Status Flag
    {0x48,PHA,eImplid,1,3,0}, // Push Acummulator
    {0x08,PHP,eImplid,1,3,0}, // Push Status Flag Register
    {0xAA,TAX,eImplid,1,2,0}, // Transfer A to X
    {0xBA,TSX,eImplid,1,2,0}, // Transfer S to X
    {0x8A,TXA,eImplid,1,2,0}, // Transfer X to A
    {0x9A,TXS,eImplid,1,2,0}, // Transfer X to SP
    {0xA8,TAY,eImplid,1,2,0}, // Transfer A to Y
    {0x98,TYA,eImplid,1,2,0}, // Transfer Y to A

    {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {},
    {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {},
    {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {},
    {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {},
    {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {},
    {},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},
    {},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},};



class CPU
{
    NES* pr_NES;
    PPU* pr_PPU;
    unsigned char* pr_CPUMem;
public:
    CPU();
    virtual ~CPU();
    void Init(NES *pr_NESParam,PPU* pr_PPUParam,unsigned char * pr_CPUMemParam);
    void Reset();
    bool Run();
    unsigned long int GetElapsedClockTicks();
    void ExecuteNMI();
    void Mirroring(unsigned short wAddressParam,unsigned char byDataParam);

public:
    FILE* fp;
    strOpcode *p_vOpcodes[256];
    unsigned char byOpcode;
    unsigned short wAddress;
    unsigned char *p_byValue;
    /* registers */
    unsigned short PC;
    /* stack pointer, it points to stack which is located among $0100-$01FF */
    unsigned char SP;
    unsigned char A;
    unsigned char X;
    unsigned char Y;
    /* processor status NEGA | OVER | _ | DECI | IRQ_DIS | ZERO | CARRY */
    /* not worried to be efficient here, just using 1 byte to each signal*/
    unsigned char S;    
    unsigned long int ulClockTicks;
    unsigned char bTmp;
};

