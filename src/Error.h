#pragma once

struct Error
{
	Error(const char* message, size_t position) 
		: message(message)
		, position(position)
	{}

	size_t position;
	const char* message;
};