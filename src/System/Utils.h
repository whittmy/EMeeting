#ifndef UTILS_H
#define UTILS_H

#include <algorithm>
#include <iostream>
#include <string>

#define repeat_twice for (int _index_ = 0; _index_ < 2; ++_index_)

namespace EM {

	typedef int16_t data_t;

	namespace Default {
		static const uint NR_ALB = 334695;

		static const uint PORT = 10000 + NR_ALB % 10000;

		static const uint FIFO_SIZE           = 10560;
		static const uint FIFO_LOW_WATERMARK  = 0;
		static const uint FIFO_HIGH_WATERMARK = FIFO_SIZE;

		static const uint BUFFER_LENGTH    = 10;

		static const uint TX_INTERVAL = 5;

		const uint RETRANSMIT_LIMIT = 10;
	}
}

#endif // UTILS_H
