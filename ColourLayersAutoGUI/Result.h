#pragma once

#include <string>

struct Result final
{
	void* data;				//Внутренние данные
	unsigned data_length;	//Размер данных
	char* error;			//Сообщение об ошибках
	unsigned error_length;	//Длина сообщения об ошибках

	explicit Result();
	explicit Result(const std::string& error_message);
	explicit Result(void* data, unsigned data_length);

	~Result() {
		delete[] error;
	}
};
