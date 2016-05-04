#include <boost/endian/conversion.hpp>
#include <boost/predef.h>
#include <iostream>
#include <stdexcept>

#include "transactions.hpp"
#include "users.hpp"

void Parameter::Write(std::ostream &s) const
{
	s.write(reinterpret_cast<const char*>(&type), 2);
}

uint16_t Parameter::AsInt16() const
{
	return 0;
}

uint32_t Parameter::AsInt32() const
{
	return 0;
}

uint64_t Parameter::AsInt64() const
{
	return 0;
}

char* Parameter::AsString() const
{
	return nullptr;
}

uint8_t* Parameter::AsByteArray() const
{
	return nullptr;
}

std::chrono::system_clock::time_point Parameter::AsTime() const
{
	return std::chrono::system_clock::now();
}

void Int16Param::Write(std::ostream &s) const
{
	big_uint16_t size = GetSize();
	
	Parameter::Write(s);
	s.write(reinterpret_cast<const char*>(&size), 2);
	s.write(reinterpret_cast<const char*>(&ordinal), 2);
}

uint16_t Int16Param::GetSize() const
{
	return 2;
}

uint16_t Int16Param::AsInt16() const
{
	return ordinal;
}

void Int32Param::Write(std::ostream &s) const
{
	big_uint16_t size = GetSize();
	
	Parameter::Write(s);
	s.write(reinterpret_cast<const char*>(&size), 2);
	s.write(reinterpret_cast<const char*>(&ordinal), 4);
}

uint16_t Int32Param::GetSize() const
{
	return 4;
}

uint32_t Int32Param::AsInt32() const
{
	return ordinal;
}

void Int64Param::Write(std::ostream &s) const
{
	big_uint16_t size = GetSize();
	
	Parameter::Write(s);
	s.write(reinterpret_cast<const char*>(&size), 2);
	s.write(reinterpret_cast<const char*>(&ordinal), 8);
}

uint16_t Int64Param::GetSize() const
{
	return 8;
}

uint64_t Int64Param::AsInt64() const
{
	return ordinal;
}

void StringParam::Write(std::ostream &s) const
{
	big_uint16_t size = GetSize();
	std::string str = LF2CR(std::string(text));
	std::copy(str.begin(), str.end(), text);
	
	Parameter::Write(s);
	s.write(reinterpret_cast<const char*>(&size), 2);
	s.write(text, GetSize());
}

uint16_t StringParam::GetSize() const
{
	return strlen(text);
}

char* StringParam::AsString() const
{
#if BOOST_OS_UNIX
	std::string str = CR2LF(std::string(text));
	std::copy(str.begin(), str.end(), text);
#endif // BOOST_OS_UNIX
	return text;
}

void ByteArrayParam::Write(std::ostream &s) const
{
	Parameter::Write(s);
	s.write(reinterpret_cast<const char*>(&size), 2);
	s.write(reinterpret_cast<const char*>(bytes), size);
}

uint16_t ByteArrayParam::GetSize() const
{
	return size;
}

uint8_t* ByteArrayParam::AsByteArray() const
{
	return bytes;
}

void TimeParam::Write(std::ostream &s) const
{
	big_uint16_t size = GetSize();
	
	Parameter::Write(s);
	s.write(reinterpret_cast<const char*>(&size), 2);
	dt.Write(s);
}

uint16_t TimeParam::GetSize() const
{
	return 8;
}

std::chrono::system_clock::time_point TimeParam::AsTime() const
{
	return dt.TimePoint();
}

UserInfoParam::UserInfoParam(User *u): Parameter(F_USERNAMEWITHINFO)
{
	id = u->id;
	icon = u->icon;
	flags = 0; // TODO: chat flags
	name = new char[u->name.size()];
	std::copy(u->name.begin(), u->name.end(), name);
}

void UserInfoParam::Write(std::ostream &s) const
{
	big_uint16_t size = GetSize();
	big_uint16_t nlen = strlen(name);
	
	Parameter::Write(s);
	s.write(reinterpret_cast<const char*>(&size), 2);
	s.write(reinterpret_cast<const char*>(&id), 2);
	s.write(reinterpret_cast<const char*>(&icon), 2);
	s.write(reinterpret_cast<const char*>(&flags), 2);
	s.write(reinterpret_cast<const char*>(&nlen), 2);
	s.write(name, nlen);
}

uint16_t UserInfoParam::GetSize() const
{
	return strlen(name)+8;
}

Transaction::Transaction(User *user, std::istream &s)
{
	s.ignore(1); // reserved
	reply = static_cast<bool>(s.get());
	s.read(reinterpret_cast<char*>(&type), 2);
	s.read(reinterpret_cast<char*>(&id), 4);
	s.read(reinterpret_cast<char*>(&error), 4);
	s.read(reinterpret_cast<char*>(&size), 4);
	s.ignore(4); // duplicate size
	
	this->user = user;
	user->last_trans_id = id;
}

void Transaction::ReadParams(std::istream &s)
{
	using namespace boost::endian;
	
	uint16_t nparams;
	
	s.read(reinterpret_cast<char*>(&nparams), 2);
	big_to_native_inplace(nparams);
	
	if (nparams > 0)
	{
		while (nparams--)
		{
			big_uint16_t pid;
			s.read(reinterpret_cast<char*>(&pid), 2);
			s.seekg(-2, s.cur);
			
			try
			{
				switch (pid)
				{
					case F_USERICONID:
					case F_CHATOPTIONS:
					case F_USERFLAGS:
					case F_OPTIONS:
					case F_VERS:
						params.push_back(new Int16Param(s));
						break;
					case F_FILESIZE:
					case F_FILETYPE:
						params.push_back(new Int32Param(s));
						break;
					case F_USERACCESS:
						params.push_back(new Int64Param(s));
						break;
					case F_USERNAME:
					case F_USERLOGIN:
					case F_CHATSUBJECT:
					case F_FILENAME:
					case F_FILETYPESTRING:
					case F_FILECREATORSTRING:
					case F_FILECOMMENT:
					case F_FILENEWNAME:
					case F_AUTOMATICRESPONSE:
					case F_NEWSCATNAME:
					case F_NEWSARTDATAFLAV:
					case F_NEWSARTTITLE:
					case F_NEWSARTPOSTER:
					case F_NEWSARTDATA:
						params.push_back(new StringParam(s));
						break;
					case F_DATA:
					case F_USERPASSWORD:
					case F_NEWSCATGUID:
						params.push_back(new ByteArrayParam(s));
						break;
					case F_FILECREATEDATE:
					case F_FILEMODIFYDATE:
					case F_NEWSARTDATE:
						params.push_back(new TimeParam(s));
						break;
					default:
						throw std::runtime_error("[Transaction]: Unknown parameter encountered");
				}
			}
			catch (std::runtime_error &e)
			{
				std::cerr << e.what() << '\n';
			}
		}
	}
}

uint32_t Transaction::GetSize() const
{
	uint32_t len = 2; // always count uint16(# of params)
	
	for (auto p: params)
		len += p->GetSize()+4;
	return len;
}

void Transaction::Write(std::ostream &s, bool preserve_id)
{
	big_uint32_t size = GetSize();
	big_uint16_t nparams = params.size();
	
	s.put(0); // reserved
	s.put(static_cast<char>(reply));
	
	if (reply)
		s.put(0).put(0);
	else
		s.write(reinterpret_cast<const char*>(&type), 2);
	
	if (reply)
		++user->nreplies;
	else if (!preserve_id)
		id = ++user->last_trans_id;
	else ;
	
	s.write(reinterpret_cast<const char*>(&id), 4);
	s.write(reinterpret_cast<const char*>(&error), 4);
	s.write(reinterpret_cast<const char*>(&size), 4); // this data
	s.write(reinterpret_cast<const char*>(&size), 4); // total data (this data again)
	s.write(reinterpret_cast<const char*>(&nparams), 2);
	for (auto p: params) p->Write(s);
}
