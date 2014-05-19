#ifndef EMSERVER_H
#define EMSERVER_H

#include <string>

class EMServer
{
public:
	EMServer();

	void set_port(uint port);

	void set_fifo_size(int fifo_size);
	void set_fifo_low_watermark(int fifo_low_watermark);
	void set_fifo_high_watermark(int fifo_high_watermark);

	void set_buffer_length(uint buffer_length);

	void set_tx_interval(uint tx_interval);

private:
	uint port;

	uint fifo_size;
	uint fifo_low_watermark;
	uint fifo_high_watermark;

	uint buffer_length;

	uint tx_interval;
};

#endif // EMSERVER_H
