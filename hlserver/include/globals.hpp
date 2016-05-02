#ifndef _GLOBALS_H
#define _GLOBALS_H

#include <boost/endian/arithmetic.hpp>
#include <chrono>
#include <istream>
#include <ostream>
#include <sstream>
#include <string>

using namespace boost::endian;

struct DateTime final
{
	big_uint16_t year;
	big_uint16_t msecs;
	big_uint32_t secs;
	
	DateTime(std::istream&);
	std::chrono::system_clock::time_point TimePoint() const;
	void FromTimePoint(const std::chrono::system_clock::time_point&);
	
	void Write(std::ostream &s) const
	{
		s.write(reinterpret_cast<const char*>(&year), 2);
		s.write(reinterpret_cast<const char*>(&msecs), 2);
		s.write(reinterpret_cast<const char*>(&secs), 4);
	}
	
	bool operator<(const DateTime&) const;
	bool operator<=(const DateTime&) const;
	bool operator>(const DateTime&) const;
	bool operator>=(const DateTime&) const;
	bool operator==(const DateTime&) const;
	bool operator!=(const DateTime&) const;
	
	DateTime(const std::chrono::system_clock::time_point &tp)
	{
		FromTimePoint(tp);
	}
	
	void operator=(const std::chrono::system_clock::time_point &tp)
	{
		FromTimePoint(tp);
	}
};

void Log(std::string);

inline std::string ConvertString(std::string s)
{
	for (int i = 0; i < s.size(); i++) s[i] = ~s[i];
	return s;
}

inline std::string LF2CR(std::string s)
{
	for (int i = 0; i < s.size(); i++)
		if (s[i] == '\n') s[i] = '\r';
	return s;
}

inline std::string CR2LF(std::string s)
{
	for (int i = 0; i < s.size(); i++)
		if (s[i] == '\r') s[i] = '\n';
	return s;
}

#endif // _GLOBALS_H
