#include <cstring>
#include <iostream>

#include "System/DataBuffer.h"

/**
 * \class DataBuffer
 */

DataBuffer::DataBuffer(size_t size) :
	size(size)
{
	init();
}

DataBuffer::~DataBuffer()
{
	clear();
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

uint DataBuffer::get_index() const
{
	return index;
}

void DataBuffer::insert(EM::Data data)
{
	if ((size_t) (size - current) < data.length) {
		end = current;
		current = 0;
	}
	std::memcpy(this->data + current, data.data, data.length);

	current += data.length;
}

EM::Data DataBuffer::get_data(uint number, size_t length)
{
	/** number cannot(!) be greater than index + 1 */
	if (index > number)
		index = number;

	if (end - indexes[index] < length) {
		indexes[index++] = 0;
		if (current > indexes[index - 1]) {
			end = current;
			current = 0;
			return {data + indexes[index - 1],
				current - indexes[index - 1]};
		} else {
			return {data + indexes[index - 1],
				end - indexes[index - 1]};
		}
	} else {
		size_t len = std::min(length, current - indexes[index]);
		indexes[index + 1] = indexes[index] + len;

		return {data + indexes[index++], len};
	}
}

EM::Data DataBuffer::raw_read(size_t length)
{
	size_t last_index;

	if (current > raw_ptr)
		last_index = current;
	else
		last_index = end;

	if (last_index - raw_ptr < length)
		return EM::Data(data + raw_ptr, last_index - raw_ptr);
	else
		return EM::Data(data + raw_ptr, length);
}

void DataBuffer::raw_move(size_t offset)
{
	raw_ptr = (raw_ptr + offset) % end;
}

void DataBuffer::init()
{
	data = new char[size];
	indexes.clear();
	current = 0;
	raw_ptr = 0;
	index = 0;
	indexes[index] = 0;
	end = size;
}

void DataBuffer::clear()
{
	delete[] data;
}
