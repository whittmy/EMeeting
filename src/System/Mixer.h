#ifndef MIXER_H
#define MIXER_H

#include <cstddef>
#include <cstdint>

namespace EM {
	struct MixerInput {
		void *data;
		size_t length;
		size_t consumed;
	};

	typedef int16_t default_data_t;

	const int DATA_MS_SIZE = 176;

	void mixer(MixerInput *inputs, size_t queues_number,
	           void *output_buffer, size_t *output_size,
	           unsigned long tx_interval_ms);
}

#endif // MIXER_H
