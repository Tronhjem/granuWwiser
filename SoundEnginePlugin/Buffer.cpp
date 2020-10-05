#include "Buffer.h"
Buffer::Buffer(int size)
{
	bufferL = new float[size] {0};
	bufferR = new float[size] {0};
}

Buffer::~Buffer()
{
	delete[]bufferL;
	delete[]bufferR;
}

float* Buffer::GetWritePointer(int channel)
{
	switch (channel)
	{
	case 0:
		return bufferL;
		break;
	case 1:
		return bufferR;
		break;
	default:
		return nullptr;
		break;
	}
}