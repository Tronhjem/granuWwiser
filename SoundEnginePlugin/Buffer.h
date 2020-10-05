#pragma once
class Buffer
{
public:
	Buffer(int m_size);
	~Buffer();
	float* Buffer::GetWritePointer(int channel);
private:
	float* bufferL;
	float* bufferR;
};

