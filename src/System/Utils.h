#ifndef UTILS_H
#define UTILS_H

#include <string>

namespace EM {
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
