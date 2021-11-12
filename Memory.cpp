#include "Memory.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

Memory::Memory()
{
	p_Mem = NULL;
}

Memory::Memory(unsigned int uiSizeParam)
{
}

Memory::~Memory()
{
	if (p_Mem)
	{
		free(p_Mem);
		p_Mem = NULL;
	}
}

// Implementation of [] operator.  This function must return a
// reference as array element can be put on left side
unsigned char& Memory::operator[](int iIndexParam)
{
	if (iIndexParam >= uiSize) 
	{
		printf("Array index out of bound, exiting");
		exit(0);
	}
	return p_Mem[iIndexParam];
}


void Memory::Init(unsigned int uiSizeParam, eMemoryEnum eNumParam)
{
	if (!p_Mem)
	{
		uiSize = uiSizeParam;
		eMemType = eNumParam;
		p_Mem = (unsigned char *)malloc(sizeof(unsigned char) * uiSizeParam);
		memset(p_Mem, 0, sizeof(unsigned char) * uiSizeParam);
	}
}

void Memory::Write(unsigned short wAddressParam, unsigned char byByteParam)
{
	if (eMemType == eMemoryEnum::eCPUMEM)
	{
		p_Mem[wAddressParam] = byByteParam;
	}
	else
	{
		p_Mem[wAddressParam] = byByteParam;
	}
}

unsigned char Memory::Read(unsigned short wAddressParam)
{
	return p_Mem[wAddressParam];
}

