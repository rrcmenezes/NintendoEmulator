#pragma once

enum eMemoryEnum
{
	eCPUMEM,
	ePPUMEM
};

class Memory
{
	unsigned char *p_Mem;
	unsigned int uiSize;
	eMemoryEnum eMemType;
public:
	Memory();
	Memory(unsigned int uiSizeParam);
	~Memory();
	unsigned char& operator[](int);
	void Init(unsigned int uiSizeParam, eMemoryEnum eNumParam);
	void Write(unsigned short wAddressParam, unsigned char byByteParam);
	unsigned char Read(unsigned short wAddressParam);
};

