#pragma once

struct ResourceData
{
	const unsigned char* ptr;
	const size_t length;
};

class Resource
{
public:
	virtual void init(ResourceData data) = 0;
	virtual void deinit() = 0;
};
