#ifndef _SERVER_H
#define _SERVER_H

#include <boost/asio.hpp>
#include <boost/endian/arithmetic.hpp>
#include <boost/thread.hpp>
#include <cstdint>
#include <map>
#include <openssl/sha.h>
#include <sqlite3.h>
#include <vector>

using boost::asio::ip::tcp;
using namespace boost::endian;

struct TrackerEntry final
{
	tcp::socket sock;
	std::string login, pw;
};

class Server final
{
public:
	Server(boost::asio::io_service&, const tcp::endpoint&);
	//~Server();
private:
	std::map<uint16_t, class User*> users;
	std::string name, description;
	std::vector<TrackerEntry*> trackers;
	boost::thread_group threads;
	tcp::acceptor listener;
	boost::asio::io_service &io;
	sqlite3 *db;
	SHA256_CTX *ctx;
	big_uint16_t fake_users, last_user_id;
	
	void ReadTransaction(class User*);
	void Listen();
	//void StartUser(UserPtr);
	//void CheckUser(UserPtr);
	void ValidateHello(class User*);
	void HandleLogin(class User*, class Transaction*);
	void HandleAgreed(class User*, class Transaction*);
	void HandleGetUserNameList(class User*);
};

Server const* GlobalInstance();

#endif // _SERVER_H
