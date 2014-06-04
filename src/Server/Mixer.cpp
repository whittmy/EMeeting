#include <limits>

#include "System/Logging.h"
#include "Server/Mixer.h"
#include "System/Utils.h"

void Mixer::mixer(
	Mixer::MixerInput *inputs,
	size_t queues_number,
	void *output_buffer,
	size_t *output_size,
	long unsigned int tx_interval_ms)
{
	*output_size = DATA_MS_SIZE * tx_interval_ms;

	for (size_t in = 0; in < queues_number; ++in)
		inputs[in].consumed = 0;

	for (size_t i = 0; i < *output_size / sizeof(EM::data_t); ++i) {
		int32_t sum = 0;
		for (size_t in = 0; in < queues_number; ++in) {
			if (inputs[in].consumed + sizeof(EM::data_t) <= inputs[in].length) {
				sum += ((EM::data_t *) inputs[in].data)[i];
				inputs[in].consumed += sizeof(EM::data_t);
			}
		}

		if (sum > (int32_t) std::numeric_limits<EM::data_t>::max())
			sum = (int32_t) std::numeric_limits<EM::data_t>::max();
		if (sum < (int32_t) std::numeric_limits<EM::data_t>::min())
			sum = (int32_t) std::numeric_limits<EM::data_t>::min();
		((EM::data_t *) output_buffer)[i] = (EM::data_t) sum;
	}
}
