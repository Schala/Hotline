#include "users.hpp"

User::User(boost::asio::io_service &io):
	sock(io),
	last_trans_id(0),
	nreplies(0)
{
}

User::~User()
{
	// TODO: socket stuff
}
