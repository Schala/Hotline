#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <exception>
#include <iostream>

#include "server.hpp"

int main(int argc, char **argv)
{
	using namespace boost::asio;
	
	try
	{
		io_service io;
		tcp::endpoint ep(tcp::v4(), 5500);
		Server *s = new Server(io, ep);
		boost::thread io_thread(boost::bind(&io_service::run, &io));
		io.run();
		io_thread.detach();
	}
	catch (std::exception &e)
	{
		std::cerr << "Exception caught: " << e.what() << '\n';
	}
	
	return 0;
}
