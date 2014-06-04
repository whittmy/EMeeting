#ifndef MIXER_H
#define MIXER_H

#include <boost/asio.hpp>
#include <cstddef>
#include <cstdint>

class Mixer
{
public:
	struct MixerInput {
		void *data;
		size_t length;
		size_t consumed;
	};

	static const int DATA_MS_SIZE = 176;

	static void mixer(
		MixerInput *inputs,
		size_t queues_number,
		void *output_buffer,
		size_t *output_size,
		unsigned long tx_interval_ms);

private:
	Mixer() = delete;
};

#endif // MIXER_H
