#include <algorithm>
#include <cstring>
#include <vector>

#include "globals.hpp"

enum Opcode: uint16_t
{
	OP_ERROR = 100,
	OP_GETMSGS,
	OP_NEWMSG,
	OP_OLDPOSTNEWS,
	OP_SERVERMSG,
	OP_CHATSEND,
	OP_CHATMSG,
	OP_LOGIN,
	OP_SENDINSTANTMSG,
	OP_SHOWAGREEMENT,
	OP_DISCONNECTUSER,
	OP_DISCONNECTMSG,
	OP_INVITENEWCHAT,
	OP_INVITETOCHAT,
	OP_REJECTCHATINVITE,
	OP_JOINCHAT,
	OP_LEAVECHAT,
	OP_NOTIFYCHATCHANGEUSER,
	OP_NOTIFYCHATDELETEUSER,
	OP_NOTIFYCHATSUBJECT,
	OP_SETCHATSUBJECT,
	OP_AGREED,
	OP_SERVERBANNER,
	OP_GETFILENAMELIST = 200,
	OP_DOWNLOADFILE = 202,
	OP_UPLOADFILE,
	OP_DELETEFILE,
	OP_NEWFOLDER,
	OP_GETFILEINFO,
	OP_SETFILEINFO,
	OP_MOVEFILE,
	OP_MAKEFILEALIAS,
	OP_DOWNLOADFLDR,
	OP_DOWNLOADINFO,
	OP_DOWNLOADBANNER,
	OP_UPLOADFLDR,
	OP_GETUSERNAMELIST = 300,
	OP_NOTIFYCHANGEUSER,
	OP_NOTIFYDELETEUSER,
	OP_GETCLIENTINFOTEXT,
	OP_SETCLIENTUSERINFO,
	OP_NEWUSER = 350,
	OP_DELETEUSER,
	OP_GETUSER,
	OP_SETUSER,
	OP_USERACCESS,
	OP_USERBROADCAST,
	OP_GETNEWSCATNAMELIST = 370,
	OP_GETNEWSARTNAMELIST,
	OP_DELNEWSITEM = 380,
	OP_NEWNEWSFLDR,
	OP_NEWNEWSCAT,
	OP_GETNEWSARTDATA = 400,
	OP_POSTNEWSART = 410,
	OP_DELNEWSART,
	OP_EDITNEWSART,
	OP_SENDKEEPALIVE = 500,
	OP_INDEX = 513,
	OP_INDEXSTATUS = 801,
	OP_EMPTYTRASH = 803,
	OP_FOLDERACCESS,
	OP_SETFLDRACCESSLIST,
	OP_SCRIPT
};

enum Field: uint16_t
{
	F_ERRORTEXT = 100,
	F_DATA,
	F_USERNAME,
	F_USERID,
	F_USERICONID,
	F_USERLOGIN,
	F_USERPASSWORD,
	F_REFNUM,
	F_TRANSFERSIZE,
	F_CHATOPTIONS,
	F_USERACCESS,
	F_USERALIAS,
	F_USERFLAGS,
	F_OPTIONS,
	F_CHATID,
	F_CHATSUBJECT,
	F_WAITINGCOUNT,
	F_SERVERAGREEMENT = 150,
	F_SERVERBANNER,
	F_SERVERBANNERTYPE,
	F_SERVERBANNERURL,
	F_NOSERVERAGREEMENT,
	F_VERS = 160,
	F_COMMUNITYBANNERID,
	F_SERVERNAME,
	F_FILENAMEWITHINFO = 200,
	F_FILENAME,
	F_FILEPATH,
	F_FILERESUMEDATA,
	F_FILEXFEROPTIONS,
	F_FILETYPESTRING,
	F_FILECREATORSTRING,
	F_FILESIZE,
	F_FILECREATEDATE,
	F_FILEMODIFYDATE,
	F_FILECOMMENT,
	F_FILENEWNAME,
	F_FILENEWPATH,
	F_FILETYPE,
	F_QUOTINGMSG,
	F_AUTOMATICRESPONSE,
	F_FLDRITEMCOUNT = 220,
	F_USERNAMEWITHINFO = 300,
	F_NEWSCATGUID = 319,
	F_NEWSARTLISTDATA = 321,
	F_NEWSCATNAME,
	F_NEWSCATLISTDATA,
	F_NEWSPATH = 325,
	F_NEWSARTID,
	F_NEWSARTDATAFLAV,
	F_NEWSARTTITLE,
	F_NEWSARTPOSTER,
	F_NEWSARTDATE,
	F_NEWSARTPREVART,
	F_NEWSARTNEXTART,
	F_NEWSARTDATA,
	F_NEWSARTFLAGS,
	F_NEWSARTPARENTART,
	F_NEWSART1STCHILDART,
	F_NEWSARTRECURSEDEL
};

struct Parameter
{
	big_uint16_t type;
	
	Parameter(uint16_t type): type(type) {}
	
	Parameter(std::istream &s)
	{
		s.read(reinterpret_cast<char*>(&type), 2);
		boost::endian::big_to_native_inplace(type);
	}
	
	virtual void Write(std::ostream&) const;
	virtual uint16_t GetSize() const = 0;
	virtual uint16_t AsInt16() const;
	virtual uint32_t AsInt32() const;
	virtual uint64_t AsInt64() const;
	virtual std::string AsString() const;
	virtual std::vector<uint8_t> AsByteArray() const;
	virtual std::chrono::system_clock::time_point AsTime() const;
};

struct Int16Param final: Parameter
{
	big_uint16_t ordinal;
	
	Int16Param(uint16_t t, uint16_t i): Parameter(t), ordinal(i) {}
	
	Int16Param(std::istream &s): Parameter(s)
	{
		s.ignore(2); // already know it's 2 bytes
		s.read(reinterpret_cast<char*>(&ordinal), 2);
	}
	
	void Write(std::ostream&) const;
	uint16_t GetSize() const override;
	uint16_t AsInt16() const override;
};

struct Int32Param final: Parameter
{
	big_uint32_t ordinal;
	
	Int32Param(uint16_t t, uint32_t i): Parameter(t), ordinal(i) {}
	
	Int32Param(std::istream &s): Parameter(s)
	{
		s.ignore(2); // already know it's 4 bytes
		s.read(reinterpret_cast<char*>(&ordinal), 4);
	}
	
	void Write(std::ostream&) const;
	uint16_t GetSize() const override;
	uint32_t AsInt32() const override;
};

struct Int64Param final: Parameter
{
	big_uint64_t ordinal;
	
	Int64Param(uint16_t t, uint64_t i): Parameter(t), ordinal(i) {}
	
	Int64Param(std::istream &s): Parameter(s)
	{
		s.ignore(2); // already know it's 8 bytes
		s.read(reinterpret_cast<char*>(&ordinal), 8);
	}
	
	void Write(std::ostream&) const;
	uint16_t GetSize() const override;
	uint64_t AsInt64() const override;
};

struct StringParam final: Parameter
{
	std::string text;
	
	StringParam(uint16_t t, const char *s): Parameter(t), text(s) {}
	
	StringParam(std::istream &s): Parameter(s)
	{
		big_uint16_t size;
		s.read(reinterpret_cast<char*>(&size), 2);
		text.resize(size);
		s.read(&text[0], size);
	}
	
	void Write(std::ostream&) const;
	uint16_t GetSize() const override;
	std::string AsString() const override;
};

struct ByteArrayParam final: Parameter
{
	std::vector<uint8_t> bytes;
	
	ByteArrayParam(uint16_t t, const uint8_t *b, uint16_t len): Parameter(t)
	{
		bytes.insert(bytes.begin(), b, b+len);
	}
	
	ByteArrayParam(std::istream &s): Parameter(s)
	{
		big_uint16_t size;
		s.read(reinterpret_cast<char*>(&size), 2);
		bytes.resize(size);
		s.read(reinterpret_cast<char*>(&bytes[0]), size);
	}
	
	void Write(std::ostream&) const;
	uint16_t GetSize() const override;
	std::vector<uint8_t> AsByteArray() const override;
};

struct TimeParam final: Parameter
{
	DateTime dt;
	
	TimeParam(uint16_t t, const std::chrono::system_clock::time_point &tp):
		Parameter(t), dt(DateTime(tp)) {}
	
	TimeParam(std::istream &s): Parameter(s), dt(DateTime(s))
	{
		s.ignore(2); // timestamps are 8 bytes
	}
	
	void Write(std::ostream&) const;
	uint16_t GetSize() const override;
	std::chrono::system_clock::time_point AsTime() const override;
};

struct UserInfoParam final: Parameter
{
	char *name;
	big_uint16_t id, icon, flags;
	
	UserInfoParam(class User*);
	
	~UserInfoParam()
	{
		delete[] name;
	}
	
	void Write(std::ostream&) const;
	uint16_t GetSize() const override;
};

struct Transaction final
{
	std::vector<Parameter*> params;
	class User *user;
	big_uint32_t id, error, size;
	big_uint16_t type;
	bool reply;
	
	Transaction(class User *user, uint16_t type, bool reply, uint32_t id, uint32_t error = 0):
		user(user), type(type), reply(reply), id(id), error(error) {}
	Transaction(class User*, std::istream&);
	
	~Transaction()
	{
		for (auto p: params)
			delete p;
	}
	
	void ReadParams(std::istream&);
	uint32_t GetSize() const;
	void Write(std::ostream&, bool preserve_id = false);
};
