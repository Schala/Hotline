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
	return std::string("Name: " + name +
		"\rLogin: " + login +
		"\rPassword Hash: " + PasswordSumString() +
		"\rAddress: " + sock.remote_endpoint().address().to_string() +
		"\rHostname: " + host +
		"\rUser ID: " + std::to_string(id) +
		"\rVersion: " + std::to_string(client_ver) +
		"\rIcon: " + std::to_string(icon) +
		"\rLast Transaction ID: " + std::to_string(last_trans_id) +
		"\rTransaction Replies: " + std::to_string(nreplies) + "\r");
}
