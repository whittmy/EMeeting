#include <boost/bind.hpp>
#include <iostream>
#include <thread>

#include "Server/EMServer.h"
#include "System/Messages.h"
#include "System/Utils.h"

/**
 * \class EMServer
 */

EMServer::EMServer() :
	AbstractServer(),

	port(EM::Default::PORT),

	fifo_size(EM::Default::FIFO_SIZE),
	fifo_low_watermark(EM::Default::FIFO_LOW_WATERMARK),
	fifo_high_watermark(EM::Default::FIFO_HIGH_WATERMARK),

	buffer_length(EM::Default::BUFFER_LENGTH),

	tx_interval(EM::Default::TX_INTERVAL),

	io_service(),

	udp_socket(io_service, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), port)),

	mixer_buffer(BUFFER_SIZE * EXPECTED_CLIENTS_LIMIT)
{}

EMServer::~EMServer()
{
	quit();
}

void EMServer::set_port(uint port)
{
	this->port = port;
}

uint EMServer::get_port() const
{
	return port;
}

void EMServer::set_fifo_size(uint fifo_size)
{
	this->fifo_size = fifo_size;
}

uint EMServer::get_fifo_size() const
{
	return fifo_size;
}

void EMServer::set_fifo_low_watermark(uint fifo_low_watermark)
{
	this->fifo_low_watermark = fifo_low_watermark;
}

uint EMServer::get_fifo_low_watermark() const
{
	return fifo_low_watermark;
}

void EMServer::set_fifo_high_watermark(uint fifo_high_watermark)
{
	this->fifo_high_watermark = fifo_high_watermark;
}

uint EMServer::get_fifo_high_watermark() const
{
	return fifo_high_watermark;
}

void EMServer::set_buffer_length(uint buffer_length)
{
	this->buffer_length = buffer_length;
}

uint EMServer::get_buffer_length() const
{
	return buffer_length;
}

void EMServer::set_tx_interval(uint tx_interval)
{
	this->tx_interval = tx_interval;
}

uint EMServer::get_tx_interval() const
{
	return tx_interval;
}

void EMServer::start()
{
	tcp_acceptor = new boost::asio::ip::tcp::acceptor(
		io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port));
	std::cerr << "Accepting connections on port " << port << " (IPv4).\n";
	start_accept();

	std::thread (&EMServer::send_info_routine, this).detach();
	udp_receive_routine();
	std::thread (&EMServer::mixer_routine, this).detach();

	io_service.run();
}

void EMServer::quit()
{
	std::cerr << "\nClearing server resources.\n";

	/*clients_mutex.lock();
	for (auto obj : clients)
		delete obj.second;
	clients.clear();
	clients_mutex.unlock();*/
}

uint EMServer::get_next_cid()
{
	while (true) {
		uint cid = AbstractServer::get_next_cid();
		bool used = false;

		clients_mutex.lock();
		for (auto c : clients)
			if (c.second->get_cid() == cid)
				used = true;
		clients_mutex.unlock();
		if (!used)
			return cid;
	}
	std::cerr << "get_next_cid: error\n";
	exit(EXIT_SUCCESS);
}

void EMServer::add_client(uint cid)
{
	clients_mutex.lock();
	clients[cid] =
		new ClientObject(cid,
		                 get_fifo_size(),
		                 get_fifo_low_watermark(),
		                 get_fifo_high_watermark());
	clients_mutex.unlock();
	std::cerr << "Added client: " << cid << "\n";
}

void EMServer::on_connection_established(uint cid, Connection *connection)
{
	add_client(cid);
	clients_mutex.lock();
	clients[cid]->set_connection(
		dynamic_cast<TcpConnection *>(connection)->shared_from_this());
	clients_mutex.unlock();
}

void EMServer::on_connection_lost(uint cid)
{
	if (clients[cid]->is_connected()) {
		std::cerr << "Client " << cid << " disconnected.\n";
		clients[cid]->set_connection(TcpConnection::Pointer(nullptr));
	}
}

void EMServer::start_accept()
{
	TcpConnection::Pointer new_connection =
		TcpConnection::create(this, tcp_acceptor->get_io_service());
	std::cerr << "Waiting for connections...\n";
	tcp_acceptor->async_accept(
		new_connection->get_socket(),
		boost::bind(&EMServer::handle_accept, this, new_connection,
		            boost::asio::placeholders::error));
}

void EMServer::handle_accept(
	TcpConnection::Pointer new_connection,
        const boost::system::error_code &error)
{
	if (!error)
		new_connection->start();
	else
		std::cerr << "handle_accept: error\n";
	start_accept();
}

void EMServer::send_info_routine()
{
	boost::asio::deadline_timer timer(
		io_service, boost::posix_time::milliseconds(SEND_INFO_TIMEOUT_MS));
	while (true) {
		timer.expires_from_now(boost::posix_time::milliseconds(SEND_INFO_TIMEOUT_MS));
		timer.wait();
		if (get_connected_clients_number() > 0) {
			std::cerr << "SEND INFO\n";
			std::string report("\n");

			clients_mutex.lock();
			for (auto p : clients)
				if (p.second->is_connected())
					report += p.second->get_report();
			for (auto p : clients)
				if (p.second->is_connected())
					p.second->get_connection()->send_info(report);
			clients_mutex.unlock();
		}
	}
}

uint EMServer::get_connected_clients_number() const
{
	uint cnt = 0;
	clients_mutex.lock();
	for (auto p : clients)
		cnt += (int) (p.second->is_connected());
	clients_mutex.unlock();
	return cnt;
}

uint EMServer::get_active_clients_number() const
{
	uint cnt = 0;
	clients_mutex.lock();
	for (auto p : clients)
		cnt += (int) (p.second->is_active());
	clients_mutex.unlock();
	return cnt;
}

uint EMServer::get_cid_from_address(const std::string &address)
{
	for (auto p : clients)
		if (p.second->get_name().substr(0, address.size()) == address)
			return p.first;
	return 0;
}

void EMServer::udp_receive_routine()
{
	udp_socket.async_receive_from(
		boost::asio::buffer(buffer, get_buffer_length()), udp_endpoint,
		boost::bind(&EMServer::handle_receive, this,
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));
}

void EMServer::handle_receive(const boost::system::error_code &ec, size_t bytes_received)
{
	if (ec || bytes_received == 0) {
		std::cerr << "server error in udp\n";
	} else {
		EM::Messages::Type type = EM::Messages::get_type(buffer);
		switch (type) {
			case EM::Messages::Type::Client: {
				uint cid = 0;
				if (std::sscanf(buffer, EM::Messages::Client.c_str(), &cid) == 1)
					std::cerr << "READ " << buffer;
				else
					std::cerr << "READ invalid CLIENT datagram.\n";
				clients[cid]->set_udp_endpoint(udp_endpoint);
				break;
			}
			case EM::Messages::Type::Upload: {
				uint cid = 0, nr = 0;
				size_t index;
				if (std::sscanf(buffer, EM::Messages::Upload.c_str(), &nr)
					== 1) {
					cid = get_cid_from_address(
						udp_endpoint.address().to_string());
					for (index = 0; index < std::strlen(buffer) &&
						buffer[index] != '\n'; ++index) {}
					++index;

					std::cerr << "READ UPLOAD " << nr << " from " << cid
						<< " (" << bytes_received << ")\n";

					EM::Data data;
					data.data = new char[bytes_received - index - 1];
					std::memcpy(data.data, buffer + index,
						bytes_received - index - 1);
					data.length = bytes_received - index - 1;

					ClientQueue &queue = clients[cid]->get_queue();
					queue.insert(data, nr);
					send_ack(nr, queue.get_available_space_size());
				} else {
					std::cerr << "READ invalid UPLOAD datagram.\n";
				}
				break;
			}
			case EM::Messages::Type::Retransmit: {
				std::cerr << "READ " << buffer;
				break;
			}
			case EM::Messages::Type::KeepAlive: {
				break;
			}
			default: {
				std::cerr << "READ Unrecognized datagram: " << buffer;
			}
		}
	}
	udp_receive_routine();
}

void EMServer::send_ack(uint nr, size_t win)
{
	char message[EM::Messages::LENGTH];
	std::sprintf(message, EM::Messages::Ack.c_str(), nr + 1, win);

	std::cerr << "SEND " << message;

	udp_socket.async_send_to(
		boost::asio::buffer(message, std::strlen(message)), udp_endpoint,
			[this](const boost::system::error_code &ec, size_t bytes_received)
			{
				if (ec)
					std::cerr << "Sending ACK failed.\n";
			}
		);
}

void EMServer::send_data(uint cid, uint nr, uint ack, size_t win, EM::Data data)
{
	udp_endpoint = clients[cid]->get_udp_endpoint();

	char message[EM::Messages::LENGTH + BUFFER_SIZE];
	std::sprintf(message, EM::Messages::Data.c_str(), nr, ack, win);
	size_t message_length = std::strlen(message);

	std::cerr << "SEND " << message;

	std::memcpy(message + message_length, data.data, data.length);

// 	boost::asio::socket_base::message_flags flags(0);
	boost::system::error_code ec;

	udp_socket.async_send_to(
		boost::asio::buffer(message, message_length + data.length), udp_endpoint,
			[this](const boost::system::error_code &ec, size_t bytes_received)
			{
				if (ec)
					std::cerr << "Sending DATA failed.\n";
			}
		);

	/*udp_socket.send_to(
		boost::asio::buffer(message, message_length + data.length), udp_endpoint,
			flags, ec);
	if (ec)
		std::cerr << "Sending DATA failed.\n";*/
}

void EMServer::mixer_routine()
{
	boost::asio::deadline_timer timer(io_service);

	Mixer::MixerInput inputs[get_active_clients_number()];
	uint client_number[get_active_clients_number()];

	size_t bytes_per_client = get_tx_interval() * Mixer::DATA_MS_SIZE;

	EM::Data data;
	data.data =
		new char[bytes_per_client * get_active_clients_number()];

	size_t active_client = 0;
	clients_mutex.lock();
	for (auto p : clients) {
		ClientObject *client = p.second;

		if (client->is_active()) {
			client_number[active_client] = client->get_cid();
			EM::Data input_data = client->get_queue().get(bytes_per_client);
			inputs[active_client].data      = input_data.data;
			inputs[active_client].length    = input_data.length;
			inputs[active_client].consumed  = new size_t;
		}
	}
	clients_mutex.unlock();

	Mixer::mixer(inputs, get_active_clients_number(), data.data, &data.length,
		get_tx_interval());

	clients_mutex.lock();
	for (size_t i = 0; i < get_active_clients_number(); ++i)
		clients[client_number[i]]->get_queue().move(*inputs[i].consumed);
	clients_mutex.unlock();

	mixer_buffer.insert(data);

	delete[] data.data;
	data = mixer_buffer.get_data(mixer_buffer.get_index(), data.length);

	clients_mutex.lock();
	for (auto p : clients)
		if (p.second->is_connected())
			send_data(p.second->get_cid(), mixer_buffer.get_index(),
				p.second->get_queue().get_expected_nr(),
				p.second->get_queue().get_available_space_size(),
				data);
}
