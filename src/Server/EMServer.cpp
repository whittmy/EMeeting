#include <boost/bind.hpp>
#include <iostream>
#include <thread>

#include "Server/EMServer.h"
#include "System/Logging.h"
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

	current_nr(0)
{
	ClientObject *dummy = new ClientObject(0, get_fifo_size(), get_fifo_low_watermark(),
		get_fifo_high_watermark());
	clients[0] = dummy;
}

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
	warn() << "Accepting connections on port " << port << " (IPv4).\n";
	start_accept();

	std::thread (&EMServer::mixer_routine, this).detach();
	std::thread (&EMServer::send_info_routine, this).detach();
	EMServer::udp_receive_routine();
	std::thread (&EMServer::send_routine, this).detach();

	io_service.run();
}

void EMServer::quit()
{}

uint EMServer::get_next_cid()
{
	while (true) {
		uint cid = AbstractServer::get_next_cid();
		bool used = false;

		for (auto c : clients)
			if (c.second->get_cid() == cid)
				used = true;
		if (!used && cid != 0)
			return cid;
	}
	error() << "get_next_cid: error\n";
	exit(EXIT_SUCCESS);
}

void EMServer::add_client(uint cid)
{
	clients[cid] =
		new ClientObject(cid,
			get_fifo_size(),
			get_fifo_low_watermark(),
			get_fifo_high_watermark());
}

void EMServer::on_connection_established(uint cid, Connection *connection)
{
	add_client(cid);
	clients[cid]->set_connection(
		dynamic_cast<TcpConnection *>(connection)->shared_from_this());
}

void EMServer::on_connection_lost(uint cid)
{
	if (clients[cid]->is_connected()) {
		info() << "Client " << cid << " disconnected.\n";
		clients[cid]->set_connection(TcpConnection::Pointer(nullptr));
	}
}

void EMServer::start_accept()
{
	TcpConnection::Pointer new_connection =
		TcpConnection::create(this, tcp_acceptor->get_io_service());
	log() << "Waiting for connections...\n";
	tcp_acceptor->async_accept(
		new_connection->get_socket(),
		boost::bind(&EMServer::handle_accept, this, new_connection,
			boost::asio::placeholders::error));
}

void EMServer::handle_accept(
	TcpConnection::Pointer new_connection,
        const boost::system::error_code &err)
{
	if (!err)
		new_connection->start();
	else
		warn() << "handle_accept: error\n";
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
			debug() << "SEND INFO\n";
			std::string report("\n");

			for (auto p : clients)
				if (p.second->is_connected())
					report += p.second->get_report();

			for (auto p : clients)
				if (p.second->is_connected())
					p.second->get_connection()->send_info(report);
		}
	}
}

uint EMServer::get_connected_clients_number() const
{
	uint cnt = 0;
	for (auto p : clients)
		cnt += (int) (p.second->is_connected());
	return cnt;
}

uint EMServer::get_active_clients_number() const
{
	uint cnt = 0;
	for (auto p : clients)
		cnt += (int) (p.second->is_active());
	return cnt;
}

std::string EMServer::get_address_from_endpoint(boost::asio::ip::udp::endpoint &endpoint) const
{
	return endpoint.address().to_string() + ":" + std::to_string(endpoint.port());
}

uint EMServer::get_cid_from_address(const std::string &address)
{
	for (auto p : clients) {
		if (p.second->get_name().substr(0, address.size()) == address) {
			if (!p.second->is_connected())
				return 0;
			else
				return p.first;
		}
	}
	return 0;
}

void EMServer::udp_receive_routine()
{
	udp_socket.async_receive_from(
		boost::asio::buffer(input_buffer), udp_endpoint,
		boost::bind(&EMServer::handle_receive, this,
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));
}

void EMServer::handle_receive(const boost::system::error_code &ec, size_t bytes_received)
{
	if (ec || bytes_received == 0) {
		warn() << "server error in udp\n";
	} else {
		std::string message(input_buffer.begin(), input_buffer.begin() + bytes_received);
		EM::Messages::Type type = EM::Messages::get_type(message);
		log() << "message from: " << get_address_from_endpoint(udp_endpoint) << "\n";
		switch (type) {
			case EM::Messages::Type::Client: {
				uint cid = 0;
				if (EM::Messages::read_client(message, cid)) {
					log() << "READ " << message << " from "
						<< get_address_from_endpoint(udp_endpoint) << ".\n";;
					clients[cid]->set_udp_endpoint(udp_endpoint);
					info() << "Added client: " << clients[cid]->get_name() << "\n";
				} else {
					info() << "READ invalid CLIENT datagram from "
						<< get_address_from_endpoint(udp_endpoint) << ".\n";
				}
				break;
			}
			case EM::Messages::Type::Upload: {
				uint nr = 0;
				uint cid =
					get_cid_from_address(
						get_address_from_endpoint(udp_endpoint));
				if (EM::Messages::read_upload(message, nr) && cid != 0) {
					size_t index =
						message.find('\n');

					if (index == message.size()) {
						info() << "READ invalid UPLOAD datagram from "
							<< clients[cid]->get_name() << ".\n";
						break;
					}

					log() << "READ UPLOAD " << nr << " from "
						<< clients[cid]->get_name()
						<< " (" << bytes_received - index - 1 << ")\n";

					ClientQueue &queue = clients[cid]->get_queue();
					if (queue.insert(message.substr(index + 1), nr))
						send_ack(udp_endpoint,
							queue.get_expected_nr(),
							queue.get_available_space_size());
					else
						info() << "READ invalid UPLOAD datagram from "
							<< clients[cid]->get_name() << ".\n";
				} else {
					info() << "READ invalid UPLOAD datagram from "
						<< clients[cid]->get_name() << ".\n";
				}
				break;
			}
			case EM::Messages::Type::Retransmit: {
				uint nr;
				uint cid =
					get_cid_from_address(
						get_address_from_endpoint(udp_endpoint));
				if (EM::Messages::read_retransmit(message, nr) && cid != 0) {
					log() << "READ " << message;
					if (current_nr - nr <= get_buffer_length()) {
						for (uint i = nr; i < current_nr; ++i) {
							ClientQueue q = clients[cid]->get_queue();
							send_data(udp_endpoint, cid, i,
								q.get_expected_nr(),
								q.get_available_space_size(),
								messages[i]);
						}
					}
				} else {
					info() << "READ invalid RETRANSMIT datagram.\n";
				}
				break;
			}
			case EM::Messages::Type::KeepAlive: {
				break;
			}
			default: {
				info() << "READ Unrecognized datagram: " << message << " ("
					<< bytes_received << ")\n";
			}
		}
	}
	udp_receive_routine();
}

void EMServer::send_ack(boost::asio::ip::udp::endpoint endpoint, uint nr, size_t win)
{
	std::string message(EM::Messages::LENGTH, ' ');
	std::sprintf(&message[0], EM::Messages::Ack.c_str(), nr, win);
	message = message.substr(0, message.find("\n") + 1);

	add_to_send(message, endpoint);
}

void EMServer::send_data(
	boost::asio::ip::udp::endpoint endpoint,
	uint cid,
	uint nr,
	uint ack,
	size_t win,
	const std::string &data)
{
	std::string message(BUFFER_SIZE, ' ');
	std::sprintf(&message[0], EM::Messages::Data.c_str(), nr, ack, win);
	message = message.substr(0, message.find("\n") + 1);

	add_to_send(message + data, endpoint);
}

void EMServer::send_routine()
{
	boost::system::error_code ec;
	boost::asio::socket_base::message_flags flags = 0;

	while (true) {
		send_mutex.lock();
		if (!to_send_list.empty()) {
			std::string msg = to_send_list.front().first;
			boost::asio::ip::udp::endpoint endpoint = to_send_list.front().second;

			log() << "SEND " << msg.substr(0, msg.find("\n")) << " to "
				<< get_address_from_endpoint(endpoint) << "\n";

			udp_socket.send_to(boost::asio::buffer(msg), endpoint, flags, ec);
			to_send_list.pop();

			if (ec)
				warn() << "error in send\n";
		}
		send_mutex.unlock();
	}
}

void EMServer::add_to_send(const std::string &message, boost::asio::ip::udp::endpoint endpoint)
{
	send_mutex.lock();
	to_send_list.push({message, endpoint});
	send_mutex.unlock();
}

void EMServer::mixer_routine()
{
	boost::asio::deadline_timer timer(io_service);

	timer.expires_from_now(boost::posix_time::milliseconds(get_tx_interval()));
	timer.async_wait(boost::bind(&EMServer::mixer_routine, this));

	if (get_active_clients_number() == 0)
		return;

	Mixer::MixerInput inputs[get_active_clients_number()];
	uint client_number[get_active_clients_number()];

	size_t data_length = get_tx_interval() * Mixer::DATA_MS_SIZE;;
	char data[data_length];

	char input_data_array[data_length * get_active_clients_number()];

	/** Collect the data from the queues */
	size_t active_client = 0;
	for (auto p : clients) {
		ClientObject *client = p.second;

		if (client->is_active()) {
			client_number[active_client] = client->get_cid();
			std::string input_data = client->get_queue().get(data_length);

			std::memmove(input_data_array + data_length * active_client,
				&input_data[0], input_data.size());

			inputs[active_client].data   =
				input_data_array + data_length * active_client;
			inputs[active_client].length = input_data.length();

			++active_client;
		}
	}

	/** Mix it */
	Mixer::mixer(inputs, get_active_clients_number(), data, &data_length,
		get_tx_interval());

	for (size_t i = 0; i < get_active_clients_number(); ++i)
		clients[client_number[i]]->get_queue().move(inputs[i].consumed);

	/** Add the message to the sent list and erase the old one */
	messages[current_nr] = std::string(data, data_length);
	if (messages.find(current_nr - get_buffer_length()) != messages.end())
		messages.erase(messages.find(current_nr - get_buffer_length()));

	/** Iterate through the clients and send them mixed data */
	for (auto p : clients) {
		if (p.second->is_connected())
			send_data(p.second->get_udp_endpoint(),
				p.second->get_cid(), current_nr,
				p.second->get_queue().get_expected_nr(),
				p.second->get_queue().get_available_space_size(),
				messages[current_nr]);
	}
	++current_nr;
}
