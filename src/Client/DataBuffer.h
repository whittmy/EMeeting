#ifndef DATABUFFER_H
#define DATABUFFER_H

#include <unordered_map>

class DataBuffer
{
public:
	DataBuffer(size_t size);

	size_t get_max_size() const;
	size_t get_size() const;

	void insert(char *ptr, size_t length);
	std::pair<char *, size_t> get_data(uint number, size_t length);

	void init();
	void clear();

private:
	size_t size;
	size_t end;
	size_t current;

	char *data;

	uint index;

	std::unordered_map<uint, size_t> indexes;
};

#endif // DATABUFFER_H
