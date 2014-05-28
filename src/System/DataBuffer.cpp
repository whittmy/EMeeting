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
	if (full)
		return get_max_size();
	if (indexes.find(index) == indexes.end())
		return current;
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

	if (data.length == get_max_size() - get_size())
		full = true;

	std::memcpy(this->data + current, data.data, data.length);

	current += data.length;

	if (current == end || current == get_max_size())
		current = 0;
}

EM::Data DataBuffer::get_data(uint number, size_t length)
{
	static const uint INDEX_MEMORY = 10;

	if (indexes.find(number - INDEX_MEMORY) != indexes.end())
		indexes.erase(indexes.find(number - INDEX_MEMORY));

	if (length > 0)
		full = false;

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

void DataBuffer::init()
{
	data = new char[size];
	indexes.clear();
	current = 0;
	index = 0;
	indexes[index] = 0;
	end = size;
	full = false;
}

void DataBuffer::clear()
{
	delete[] data;
}

/**
 * \class RawBuffer
 */

RawBuffer::RawBuffer(size_t size) :
	DataBuffer(size)
{
	init();
}

size_t RawBuffer::get_max_size() const
{
	return DataBuffer::get_max_size();
}

size_t RawBuffer::get_size() const
{
	if (full)
		return get_max_size();
	return (current >= ptr)
		? (current - ptr)
		: (end - ptr + current);
}

void RawBuffer::write(EM::Data data)
{
	DataBuffer::insert(data);
}

EM::Data RawBuffer::read(size_t length)
{
	size_t last_index;

	if (current > ptr)
		last_index = current;
	else
		last_index = end;

	if (last_index - ptr < length)
		return EM::Data(data + ptr, last_index - ptr);
	else
		return EM::Data(data + ptr, length);
}

void RawBuffer::move(size_t offset)
{
	std::cerr << "offset: " << offset << "\n";
	bool a = false;
	if (offset == get_size()) {
		a = true;
		std::cerr << "ptr: " << ptr << ", current: " << current << ", getsize: " << get_size() << "\n";
	}

	if (offset > 0)
		full = false;
	ptr += offset;
	if (ptr >= end) {
		ptr %= end;
		end = get_max_size();
	}
	if (a)
		std::cerr << "ptr: " << ptr << ", current: " << current << ", getsize: " << get_size() << "\n";
}

void RawBuffer::init()
{
	ptr = 0;
	DataBuffer::init();
}

void RawBuffer::clear()
{
	DataBuffer::clear();
}
