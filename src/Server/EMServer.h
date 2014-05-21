#ifndef EMSERVER_H
#define EMSERVER_H

#include <queue>
#include <sys/types.h>
#include <vector>

#include "System/Mixer.h"

class ClientQueue
{
public:
	ClientQueue(uint fifo_size, uint fifo_low_watermark, uint fifo_high_watermark);

	enum class State : uint8_t {Filling, Active};

	void push(EM::MixerInput input);
	EM::MixerInput pop();
	EM::MixerInput front();
	bool full() const;
	size_t size() const;

	State get_current_state() const;

private:
	uint fifo_size;
	uint fifo_low_watermark;
	uint fifo_high_watermark;

	std::queue<EM::MixerInput> queue;

	State state;
};

class EMServer
{
public:
	EMServer();

	void set_port(uint port);
	uint get_port() const;

	void set_fifo_size(int fifo_size);
	uint get_fifo_size() const;
	void set_fifo_low_watermark(int fifo_low_watermark);
	uint get_fifo_low_watermark() const;
	void set_fifo_high_watermark(int fifo_high_watermark);
	uint get_fifo_high_watermark() const;

	void set_buffer_length(uint buffer_length);
	uint get_buffer_length() const;

	void set_tx_interval(uint tx_interval);
	uint get_tx_interval() const;

	void start();
	void quit();

private:
	uint port;

	uint fifo_size;
	uint fifo_low_watermark;
	uint fifo_high_watermark;

	uint buffer_length;

	uint tx_interval;

	std::vector<ClientQueue> client_queues;
};

#endif // EMSERVER_H
