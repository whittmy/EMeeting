#include "System/Mixer.h"

void Mixer::mixer(
	Mixer::MixerInput *inputs,
	size_t queues_number,
	void *output_buffer,
	size_t *output_size,
	long unsigned int tx_interval_ms)
{
	default_data_t *data_ptr;
	size_t data_len;

	default_data_t *output_buf = reinterpret_cast<default_data_t *>(output_buffer);
	default_data_t output_data_size = 0;

	*output_size = 0;

	for (size_t i = 0; i < queues_number; ++i) {
		data_ptr = reinterpret_cast<default_data_t *>(inputs[i].data);
		data_len  = 0;
		while (data_len < tx_interval_ms * DATA_MS_SIZE &&
			inputs[i].length - data_len * sizeof(default_data_t)
			> sizeof(default_data_t))
			output_buf[output_data_size++] = data_ptr[data_len++];
		*(inputs[i].consumed) = data_len * sizeof(default_data_t);

		while (data_len < tx_interval_ms * DATA_MS_SIZE)
			output_buf[output_data_size++] = static_cast<default_data_t>(0);

		*output_size += sizeof(default_data_t) * output_data_size;
	}
}
