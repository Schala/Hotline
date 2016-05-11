#include <iomanip>
#include <sstream>

#include "users.hpp"

User::User(boost::asio::io_service &io):
	sock(io),
	last_trans_id(0),
	nreplies(0)
{
}

User::~User()
{
	if (sock.is_open()) Disconnect();
}

void User::Disconnect()
{
	// NOTE: should we care about socket errors here?
	sock.cancel();
	sock.close();
}

std::string User::InfoText() const
{
	std::ostringstream ss;
	
	ss << std::setw(22) << std::left << "Name: " << std::setw(32) << std::left << name << '\r' <<
		std::setw(22) << std::left << "Login: " << std::setw(32) << std::left  << login << '\r' <<
		std::setw(22) << std::left << "Password Hash: " << std::setw(32) << std::left  << PasswordSumString() << '\r' <<
		std::setw(22) << std::left << "Address: " << std::setw(32) << std::left  << sock.remote_endpoint().address().to_string() << '\r' <<
		std::setw(22) << std::left << "Port: " << std::setw(32) << std::left  << sock.remote_endpoint().port() << '\r' <<
		std::setw(22) << std::left << "Hostname: " << std::setw(32) << std::left  << host << '\r' <<
		std::setw(22) << std::left << "User ID: " << std::setw(32) << std::left  << id << '\r' <<
		std::setw(22) << std::left << "Version: " << std::setw(32) << std::left  << client_ver << '\r' <<
		std::setw(22) << std::left << "Icon: " << std::setw(32) << std::left  << icon << '\r' <<
		std::setw(22) << std::left << "Last Transaction ID: " << std::setw(32) << std::left  << last_trans_id << '\r' <<
		std::setw(22) << std::left << "Transaction Replies: " << std::setw(32) << std::left << nreplies << '\r';
	
	return ss.str();
}
