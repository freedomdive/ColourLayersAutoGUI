#include "pch.h"
#include "Result.h"

Result::Result()
	: data(nullptr), data_length(0), error(nullptr), error_length(0)
{  
}

Result::Result(const std::string& error_message)
	: data(nullptr), data_length(0), error_length(error_message.length())
{
	error = new char[error_length + 1];
	error[error_length] = 0;

	if (error_length != 0)
		error_message.copy(error, error_length);
}

Result::Result(void* data, unsigned data_length)
	: data(data), data_length(data_length), error(nullptr), error_length(0)
{
}

