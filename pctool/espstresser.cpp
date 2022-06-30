#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <ctime>
#include <iostream>
#include <string>

#include "protocol.h"

using boost::asio::ip::udp;

static constexpr size_t number_frames = 1000;

boost::asio::io_service io_service;
boost::posix_time::milliseconds interval(30);
boost::asio::deadline_timer timer(io_service, interval);

udp::socket sock(io_service, udp::endpoint(udp::v4(), 10000));
udp::endpoint sendto_endpoint;
udp::endpoint recv_endpoint;

static uint32_t lost_frames				   = 0;
static int32_t last_received_frame_counter = -1;

static uint32_t total_ping_pong_cycles = 0;

union
{
	PackageEspToPc content;
	std::array<uint8_t, 1000> raw;
} recv_buffer;

union
{
	PackagePcToEsp content;
	std::array<uint8_t, 1000> raw;
} send_buffer;

void handle_send(const boost::system::error_code& error, std::size_t bytes_transferred)
{
	if (error)
	{
		std::cerr << "handle_send " << error << std::endl;
	}
}

void handle_receive(const boost::system::error_code& error, std::size_t /*bytes_transferred*/)
{
	if (!error)
	{
		if (last_received_frame_counter > 0)
		{
			lost_frames += recv_buffer.content.frame_counter - (last_received_frame_counter + 1);
		}

		last_received_frame_counter = recv_buffer.content.frame_counter;
		total_ping_pong_cycles++;

		printf("RecvFrameCnt:%d LostEspToPc:%d EspRssi:%d LostFramesPcToEsp:%d   %d %d %d %d\n",
			   recv_buffer.content.frame_counter, lost_frames, recv_buffer.content.rssi,
			   recv_buffer.content.lost_frames, recv_buffer.content.err_wrong_order_cnt,
			   recv_buffer.content.err_sendto_cnt, recv_buffer.content.err_recvfrom_cnt,
			   recv_buffer.content.info_reset_cnt);

		sock.async_receive_from(boost::asio::buffer(recv_buffer.raw), recv_endpoint, handle_receive);
	}
	else if (error != boost::asio::error::operation_aborted)
	{
		std::cerr << "handle_receive " << error << std::endl;
	}
}

void timer_tick(const boost::system::error_code& error)
{
	if (!error)
	{
		send_buffer.content.frame_counter++;

		if (send_buffer.content.frame_counter <= number_frames)
		{
			sock.async_send_to(boost::asio::buffer(send_buffer.raw), sendto_endpoint, handle_send);
		}

		if (send_buffer.content.frame_counter == number_frames)
		{
			std::cout << "Waiting for remaining frames ...\n";
		}

		if (send_buffer.content.frame_counter >= number_frames + 10)
		{
			sock.cancel();
		}
		else
		{
			// Reschedule the timer for the future:
			timer.expires_at(timer.expires_at() + interval);
			// Posts the timer event
			timer.async_wait(timer_tick);
		}
	}
	else
	{
		std::cerr << "tick " << error << std::endl;
	}
}

int main(int argc, const char** argv)
{
	send_buffer.content.frame_counter = 0;
	assert(argc == 2);

	recv_buffer.raw.fill(0);
	send_buffer.raw.fill(0);

	try
	{
		udp::resolver resolver(io_service);
		sendto_endpoint = *resolver.resolve(udp::v4(), argv[1], "10000").begin();

		timer.async_wait(timer_tick);

		sock.async_receive_from(boost::asio::buffer(recv_buffer.raw), recv_endpoint, handle_receive);

		io_service.run();
		printf("Finished Test. Total Lost Ping Pong Cycles: %d\n", number_frames - total_ping_pong_cycles);
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}

	return 0;
}
