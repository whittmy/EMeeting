#include <limits>

#include "Server/Mixer.h"
#include "System/Utils.h"

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
	for (size_t i = 0; i < *output_size / sizeof(EM::data_t); i += queues_number) {
		uint32_t sum = 0;
		for (size_t in = 0; in < queues_number; ++in) {
			if (inputs[in].consumed + sizeof(EM::data_t) <= inputs[in].length) {
				sum += ((EM::data_t *) inputs[in].data)[i];
				inputs[in].consumed += sizeof(EM::data_t);
			}
		}

		if (sum > (uint32_t) std::numeric_limits<size_t>::max())
			sum = (uint32_t) std::numeric_limits<size_t>::max();
		((EM::data_t *) output_buffer)[i] = (EM::data_t) sum;
	}
}
