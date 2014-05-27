#ifndef DATABUFFER_H
#define DATABUFFER_H

#include <unordered_map>

namespace EM {
	struct Data {
		Data(char *d = nullptr, size_t l = 0) : data(d), length(l) {}

		char *data;
		size_t length;
	};
}

class DataBuffer
{
public:
	DataBuffer(size_t size);
	~DataBuffer();

	size_t get_max_size() const;
	size_t get_size() const;

	uint get_index() const;

	void insert(EM::Data data);
	EM::Data get_data(uint number, size_t length);

	EM::Data raw_read(size_t length);
	void raw_move(size_t offset);

	void init();
	void clear();

private:
	size_t size;
	size_t end;
	size_t current;

	size_t raw_ptr;

	char *data;

	uint index;

	std::unordered_map<uint, size_t> indexes;
};

#endif // DATABUFFER_H
