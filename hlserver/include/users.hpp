#ifndef _USERS_H
#define _USERS_H

#include <bitset>
#include <boost/asio.hpp>
#include <boost/endian/arithmetic.hpp>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <mutex>
#include <openssl/sha.h>
#include <utility>

using boost::asio::ip::tcp;
using namespace boost::endian;

enum UserAccess: uint8_t
{
	UA_DELETEFILE = 0,
	UA_UPLOADFILE,
	UA_DOWNLOADFILE,
	UA_RENAMEFILE,
	UA_MOVEFILE,
	UA_CREATEFOLDER,
	UA_DELETEFOLDER,
	UA_RENAMEFOLDER,
	UA_MOVEFOLDER,
	UA_READCHAT,
	UA_SENDCHAT,
	UA_OPENCHAT,
	UA_CLOSECHAT,
	UA_SHOWINLIST,
	UA_CREATEUSER,
	UA_DELETEUSER,
	UA_OPENUSER,
	UA_MODIFYUSER,
	UA_CHANGEOWNPASS,
	UA_SENDPRIVMSG,
	UA_NEWSREADART,
	UA_NEWSPOSTART,
	UA_DISCONUSER,
	UA_CANNOTBEDISCON,
	UA_GETCLIENTINFO,
	UA_UPLOADANYWHERE,
	UA_ANYNAME,
	UA_NOAGREEMENT,
	UA_SETFILECOMMENT,
	UA_SETFOLDERCOMMENT,
	UA_VIEWDROPBOXES,
	UA_MAKEALIAS,
	UA_BROADCAST,
	UA_NEWSDELETEART,
	UA_NEWSCREATECAT,
	UA_NEWSDELETECAT,
	UA_NEWSCREATEFLDR,
	UA_NEWSDELETEFLDR,
	UA_UPLOADFOLDER,
	UA_DOWNLOADFOLDER,
	UA_SENDMESSAGE, // synonymous with UA_SENDPRIVMSG?
	UA_NEWSEDITART,
	UA_NEWMOVEFLDR,
	UA_NEWSMOVECAT,
	UA_SPAM,
	UA_EMPTYTRASH,
	UA_VIEWHIDDENUSERS,
	UA_MODIFYFOLDERACCESS,
	UA_ASSIGNFOLDERACCESS,
	USER_ACCESS_BITS
};

enum ExtraAccess: uint8_t
{
	XA_CHATPRIVATE = 0,
	XA_MSG,
	XA_USERGETLIST,
	XA_FILELIST,
	XA_FILEGETINFO,
	XA_FILEHASH,
	XA_CANLOGIN,
	XA_USERVISIBILITY,
	XA_USERCOLOR,
	XA_CANSPAM,
	XA_SETSUBJECT,
	XA_DEBUG,
	XA_USERACCESS,
	XA_ACCESSVOLATILE,
	XA_USER0WN,
	XA_IS0WN3D,
	XA_MANAGEUSERS,
	XA_INFOGETADDRESS,
	XA_INFOGETLOGIN,
	XA_NAMELOCK,
	XA_CANAGREE,
	XA_CANPING,
	XA_BANNERGET,
	XA_IGNOREQUEUE,
	EXTRA_ACCESS_BITS
};

enum UserFlags: uint8_t
{
	UF_VISIBLE = 0,
	UF_AWAY,
	UF_ISHL,
	UF_ISTIDE,
	UF_ISANICLIENT,
	UF_ISHEIDRUN,
	UF_ISFROGBLAST,
	UF_ISIRC,
	UF_ISKDX,
	UF_INPING,
	UF_INLOGIN,
	UF_SOCKUNIX,
	USER_FLAGS
};

enum FolderAccess: uint8_t
{
	FA_SEEFOLDER = 0,
	FA_CREATEFOLDERS,
	FA_UPLOADFILES,
	FA_UPLOADFOLDERS,
	FA_MOVEINITEMS,
	FA_ALIASINITEMS,
	FA_DUPLICATEINITEMS,
	FA_DELETEFILES = 9,
	FA_DELETEFOLDERS,
	FA_MOVEOUTITEMS,
	FA_SEEFOLDERCONTENT = 19,
	FA_DOWNLOADFILES,
	FA_DOWNLOADFOLDERS,
	FA_ALIASOUTITEMS,
	FA_DUPLICATEOUTITEMS,
	FA_RENAMEITEMS = 29,
	FA_SETITEMATTRIBUTES,
	FA_MODIFYFILECONTENTS,
	FOLDER_ACCESS_BITS
};

struct User final
{
	std::string name, login, host, auto_reply;
	uint8_t pw_sum[SHA256_DIGEST_LENGTH];
	tcp::socket sock;
	std::mutex lock;
	std::bitset<USER_ACCESS_BITS> access;
	std::bitset<EXTRA_ACCESS_BITS> extra_access;
	std::bitset<FOLDER_ACCESS_BITS> folder_access;
	std::bitset<USER_FLAGS> flags;
	big_uint32_t last_trans_id;
	uint32_t nreplies;
	big_uint16_t id, icon, color, client_ver;
	
	User(boost::asio::io_service&);
	~User();
	void Disconnect();
	std::string InfoText() const;
	
	bool ComparePassword(const uint8_t *sum) const
	{
		return strncmp(reinterpret_cast<const char*>(pw_sum),
			reinterpret_cast<const char*>(sum), SHA256_DIGEST_LENGTH) == 0 ? true : false;
	}
	
	std::string PasswordSumString() const
	{
		std::string s(SHA256_DIGEST_LENGTH+1, 0);
		for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) sprintf(&s[i], "%02x", pw_sum[i]);
		return s;
	}
	
	std::string VersionString() const
	{
		std::string s(std::to_string(client_ver));
		s.insert(1, 1, '.'); s.insert(3, 1, '.');
		return s;
	}
};

#endif // _USERS_H
