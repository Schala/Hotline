#include <algorithm>
#include <iterator>
#include <sstream>
#include <stdexcept>

#include "server.hpp"
#include "transactions.hpp"
#include "users.hpp"

enum
{
	SERVER_VERSION = 197
};

static Server *global_inst = nullptr;

Server const* GlobalInstance()
{
	return global_inst;
}

Server::Server(boost::asio::io_service &io, const tcp::endpoint &ep):
	io(io),
	listener(io, ep),
	last_user_id(0),
	name("test")
{
	if (global_inst)
		throw std::runtime_error("[Internal error]: Multiple server instances found.");
	else
		global_inst = this;
	
	Log("Server initialised");
	Listen();
}

void Server::Disconnect(User *u)
{
	u->Disconnect();
	
	if ((users.find(u->id) != users.end()) && !u->name.empty())
	{
		Log(std::string(u->name + " has disconnected."));
		users.erase(u->id);
	}
	else
	{
		Log(std::string(u->host + " has disconnected."));
		delete u;
	}
}

void Server::Listen()
{
	auto u = new User(io);
	
	listener.async_accept(u->sock,
		[this, u](boost::system::error_code ec)
		{
			if (ec)
			{
				Resolve(u);
				Disconnect(u);
				Log(ec.message());
			}
			else
			{
				//StartUser(u);
				Resolve(u);
				Log("Incoming connection from " + u->host);
				ValidateHello(u);
			}
			
			Listen();
		});
}

void Server::Resolve(User *u)
{
	tcp::endpoint ep = u->sock.remote_endpoint();
	tcp::resolver rslv(io);
	u->host = rslv.resolve(ep)->host_name();
}

/*void Server::StartUser(User *u)
{
	
}*/

void Server::ValidateHello(User *u)
{
	using namespace boost::asio;
	
	char *hello = new char[12];
	std::fill(hello, hello+12, 0);
	
	async_read(u->sock, buffer(hello, 12),
		[this, u, hello](boost::system::error_code ec, size_t s)
		{
			if (ec)
			{
				Log(ec.message());
				Disconnect(u);
			}
			else if (strncmp(hello, "TRTPHOTL\0\1\0\2", 12) != 0)
			{
				Log("["+u->host+"]: Bad connection greeting");
				Disconnect(u);
			}
			else
			{
				char reply[8] = {'T', 'R', 'T', 'P', 0, 0, 0, 0 };
				async_write(u->sock, buffer(reply, 8),
					[this, u](boost::system::error_code ec, size_t s)
					{
						if (ec)
						{
							Log(ec.message());;
							Disconnect(u);
						}
						else
						{
							u->id = ++last_user_id;
							users.emplace(u->id, u);
							ReadTransaction(u);
						}
					});
			}
			
			delete[] hello;
		});
}

void Server::ReadTransaction(User *u)
{
	using namespace boost::asio;
	
	char header[20];
	
	async_read(u->sock, buffer(header, 20),
		[this, u, &header](boost::system::error_code ec, size_t s)
		{
			u->lock.lock();
			if (ec)
			{
				if (ec.value() != error::eof) Log(ec.message());
				u->lock.unlock();
				Disconnect(u);
			}
			else
			{
				std::istringstream hss(std::string(header, 20));
				Transaction *trans = new Transaction(u, hss);
				char *body = new char[trans->size];
				async_read(u->sock, buffer(body, trans->size),
					[this, u, body, trans](boost::system::error_code ec, size_t s)
					{
						if (ec)
						{
							Log(ec.message());
							u->lock.unlock();
							Disconnect(u);
						}
						else
						{
							std::istringstream tss(std::string(body, trans->size));
							trans->ReadParams(tss);
							delete[] body;
							
							switch (trans->type)
							{
								case OP_LOGIN: HandleLogin(u, trans); break;
								case OP_AGREED: HandleAgreed(u, trans); break;
								case OP_GETUSERNAMELIST:
									delete trans;
									HandleGetUserNameList(u);
									break;
								case OP_GETCLIENTINFOTEXT: HandleGetUserInfo(u, trans); break;
								default: u->lock.unlock();
							}
						}
					});
			}
		});
}

void Server::HandleLogin(User *u, Transaction *trans)
{
	using namespace boost::asio;

	std::ostringstream ss;
	
	if (trans->params.size() > 1)
	{
		u->login = trans->params[0]->AsString();
		uint8_t *password = trans->params[1]->AsByteArray();
		SHA256_Init(ctx);
		SHA256_Update(ctx, password, trans->params[1]->GetSize());
		SHA256_Final(u->pw_sum, ctx);
		u->client_ver = trans->params[2]->AsInt16();
	}
	else
	{
		u->login = "guest";
		std::fill(std::begin(u->pw_sum), std::end(u->pw_sum), 0);
		u->client_ver = trans->params[0]->AsInt16();
	}
	delete trans;
	
	trans = new Transaction(u, 0, true, u->last_trans_id, 0);
	trans->params.push_back(new Int16Param(F_USERID, last_user_id));
	trans->params.push_back(new Int16Param(F_VERS, SERVER_VERSION));
	trans->params.push_back(new Int16Param(F_COMMUNITYBANNERID, 0));
	trans->params.push_back(new StringParam(F_SERVERNAME, name.data()));
	trans->Write(ss, true);
	delete trans;
	
	trans = new Transaction(u, OP_SHOWAGREEMENT, false, u->last_trans_id, 0);
	if (agreement.empty())
		trans->params.push_back(new Int16Param(F_NOSERVERAGREEMENT, 1));
	else
		trans->params.push_back(new StringParam(F_DATA, agreement.data()));
	trans->Write(ss, true);
	delete trans;
	
	async_write(u->sock, buffer(ss.str(), ss.str().size()),
		[this, u](boost::system::error_code ec, size_t s)
		{
			if (ec)
			{
				Log(ec.message());
				u->lock.unlock();
				Disconnect(u);
			}
			else
			{
				u->lock.unlock();
				ReadTransaction(u);
			}
		});
}

void Server::HandleAgreed(User *u, Transaction *trans)
{
	using namespace boost::asio;
	
	std::ostringstream ss;
	uint64_t tmpacc = 0xFFF3CFEFFF800000LL; // TODO: reevaluate access bits
	
	u->name = trans->params[0]->AsString();
	u->icon = trans->params[1]->AsInt16();
	// TODO: 3rd param is chat options, look into that
	delete trans;
	
	trans = new Transaction(u, 0, true, u->last_trans_id, 0);
	trans->Write(ss, true);
	delete trans;
	
	trans = new Transaction(u, OP_USERACCESS, false, u->last_trans_id, 0);
	trans->params.push_back(new Int64Param(F_USERACCESS, tmpacc));
	trans->params.push_back(new UserInfoParam(u));
	trans->Write(ss, true);
	delete trans;
	
	trans = new Transaction(u, OP_SERVERBANNER, false, u->last_trans_id, 0);
	trans->params.push_back(new Int32Param(F_SERVERBANNERTYPE, 0x55524C20)); // TODO: banner handling, using 'URL ' for now
	trans->params.push_back(new StringParam(F_SERVERBANNERURL, "about:blank")); // most likely a 404
	trans->Write(ss);
	delete trans;
	
	async_write(u->sock, buffer(ss.str(), ss.str().size()),
		[this, u](boost::system::error_code ec, size_t s)
		{
			if (ec)
			{
				Log(ec.message());
				u->lock.unlock();
				Disconnect(u);
			}
			else
			{
				u->lock.unlock();
				ReadTransaction(u);
			}
		});
}

void Server::HandleGetUserNameList(User *u)
{
	using namespace boost::asio;
	
	std::ostringstream ss;
	
	auto trans = new Transaction(u, 0, true, u->last_trans_id, 0);
	for (auto user: users) trans->params.push_back(new UserInfoParam(user.second));
	trans->Write(ss, true);
	delete trans;
	
	async_write(u->sock, buffer(ss.str(), ss.str().size()),
		[this, u](boost::system::error_code ec, size_t s)
		{
			if (ec)
			{
				Log(ec.message());
				u->lock.unlock();
				Disconnect(u);
			}
			else
			{
				Log(u->name + " successfully logged in.");
				u->lock.unlock();
				ReadTransaction(u);
			}
		});
}

void Server::HandleGetUserInfo(User *u, Transaction *trans)
{
	using namespace boost::asio;
	
	std::ostringstream ss;
	big_uint16_t uid = trans->params[0]->AsInt16();
	
	delete trans;
	trans = new Transaction(u, 0, true, u->last_trans_id, 0);
	trans->params.push_back(new StringParam(F_USERNAME, users[uid]->name.data()));
	trans->params.push_back(new StringParam(F_DATA, users[uid]->InfoText().data()));
	trans->Write(ss, true);
	delete trans;
	
	async_write(u->sock, buffer(ss.str(), ss.str().size()),
		[this, u](boost::system::error_code ec, size_t s)
		{
			if (ec)
			{
				Log(ec.message());
				u->lock.unlock();
			}
			else
			{
				u->lock.unlock();
				ReadTransaction(u);
			}
		});
}
