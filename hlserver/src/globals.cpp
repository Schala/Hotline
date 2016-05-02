#include <boost/endian/conversion.hpp>
#include <ctime>
#include <iomanip>
#include <iostream>

#include "globals.hpp"

static const uint32_t MONTH_SECS[12] =
{
	0, 2678400, 5097600, 7776000, 10368000, 13046400,
	15638400, 18316400, 20995200, 23587200, 26265600,
	28857600
};

DateTime::DateTime(std::istream &s)
{
	s.read(reinterpret_cast<char*>(&year), 2);
	s.read(reinterpret_cast<char*>(&msecs), 2);
	s.read(reinterpret_cast<char*>(&secs), 4);
}

std::chrono::system_clock::time_point DateTime::TimePoint() const
{
	return std::chrono::system_clock::from_time_t(static_cast<time_t>(secs*(year-1900)));
}

bool DateTime::operator<(const DateTime &rhs) const
{
	if (year == rhs.year)
		return secs < rhs.secs;
	else if (year < rhs.year)
		return true;
	else
		return false;
}

bool DateTime::operator<=(const DateTime &rhs) const
{
	if (year == rhs.year)
		return secs <= rhs.secs;
	else if (year < rhs.year)
		return true;
	else
		return false;
}

bool DateTime::operator>(const DateTime &rhs) const
{
	if (year == rhs.year)
		return secs > rhs.secs;
	else if (year > rhs.year)
		return true;
	else
		return false;
}

bool DateTime::operator>=(const DateTime &rhs) const
{
	if (year == rhs.year)
		return secs >= rhs.secs;
	else if (year > rhs.year)
		return true;
	else
		return false;
}

bool DateTime::operator==(const DateTime &rhs) const
{
	return (year == rhs.year) && (secs == rhs.secs);
}

bool DateTime::operator!=(const DateTime &rhs) const
{
	return (year != rhs.year) && (secs != rhs.secs);
}

void DateTime::FromTimePoint(const std::chrono::system_clock::time_point &tp)
{
	using namespace std::chrono;

	time_t ct = system_clock::to_time_t(tp);
	tm st = *localtime(&ct);
	year = st.tm_year+1900;

	msecs = duration_cast<milliseconds>(tp.time_since_epoch()).count();

	secs = MONTH_SECS[st.tm_mon] + (st.tm_mon > 1 && !(year%4) ? 86400 : 0) +
		(st.tm_sec + (60 * (st.tm_min+60 * ((st.tm_hour+24 * (st.tm_mday-1))))));
}

void Log(std::string txt)
{
	using namespace std::chrono;
	
	static time_t present;
	present = system_clock::to_time_t(system_clock::now());
	
	std::cout << '[' << std::put_time(localtime(&present), "%D %T") << "]: " << txt << '\n';
}
