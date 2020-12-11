#pragma once

#include <string>

struct Result final
{
	void* data;				//���������� ������
	unsigned data_length;	//������ ������
	char* error;			//��������� �� �������
	unsigned error_length;	//����� ��������� �� �������

	explicit Result();
	explicit Result(const std::string& error_message);
	explicit Result(void* data, unsigned data_length);

	~Result() {
		delete[] error;
	}
};
