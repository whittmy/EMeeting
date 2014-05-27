#include <limits>

#include "System/Mixer.h"

void Mixer::mixer(
	Mixer::MixerInput *inputs,
	size_t queues_number,
	void *output_buffer,
	size_t *output_size,
	long unsigned int tx_interval_ms)
{
	for (size_t in = 0; in < queues_number; ++in)
		inputs[in].consumed = 0;

	/** This isn't entirely my idea of mixer, but it is clever */
	for (size_t i = 0; i < *output_size / sizeof(data_t); i += queues_number) {
		uint32_t sum = 0;
		for (size_t in = 0; in < queues_number; ++in) {
			if (inputs[in].consumed + sizeof(data_t) <= inputs[in].length) {
				sum += ((data_t *) inputs[in].data)[i];
				inputs[in].consumed += sizeof(data_t);
			}
		}

		if (sum > (uint32_t) std::numeric_limits<size_t>::max())
			sum = (uint32_t) std::numeric_limits<size_t>::max();
		((data_t *) output_buffer)[i] = (data_t) sum;
	}

	std::cerr << "produced " << *output_size << " bytes\n";
}
