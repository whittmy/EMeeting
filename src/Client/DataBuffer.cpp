#include <cstring>
#include <iostream>

#include "Client/DataBuffer.h"

/**
 * \class DataBuffer
 */

DataBuffer::DataBuffer(size_t size) :
	size(size)
{
	init();
}

size_t DataBuffer::get_max_size() const
{
	return size;
}

size_t DataBuffer::get_size() const
{
	if (indexes.at(index) > current)
		return end - indexes.at(index) + current;
	else
		return current - indexes.at(index);
}

void DataBuffer::insert(char *ptr, size_t length)
{
	if (size_t(size - current) < length) {
		end = current;
		current = 0;
	}
	std::memcpy(data + current, ptr, length);
	std::cerr << "inserted: " << std::string(data + current, length) << "\n";

	current += length;
}

std::pair<char *, size_t> DataBuffer::get_data(uint number, size_t length)
{
	/** number cannot(!) be greater than index + 1 */
	if (index > number)
		index = number;

	if (end - indexes[index] < length) {
		indexes[index++] = 0;
		if (current > indexes[index - 1]) {
			end = current;
			current = 0;
			return std::pair<char *, size_t>(data + indexes[index - 1],
				current - indexes[index - 1]);
		} else {
			return std::pair<char *, size_t>(data + indexes[index - 1],
				end - indexes[index - 1]);
		}
	} else {
		size_t l = std::min(length, current - indexes[index]);
		indexes[index + 1] = indexes[index] + l;

		std::cerr << "get_data: " << std::string(data + indexes[index], l) << "\n";
		return std::pair<char *, size_t>(data + indexes[index++], l);
	}
}

void DataBuffer::init()
{
	data = new char[size];
	indexes.clear();
	current = 0;
	index = 0;
	indexes[index] = 0;
	end = size;
}

void DataBuffer::clear()
{
	delete[] data;
}
