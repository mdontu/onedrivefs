// SPDX-License-Identifier: GPL-2.0

#ifndef __LOG_H_INCLUDED__
#define __LOG_H_INCLUDED__

#include <ctime>
#include <fstream>
#include <sstream>
#include "appconfig.h"

namespace OneDrive {

class CLog
{
public:
	CLog()
	{
	}

	~CLog()
	{
	}

	CLog(const CLog &) = delete;
	CLog & operator=(const CLog &) = delete;

	void init(const std::string &path)
	{
		f_.open(path, std::ios::out | std::ios::app);
	}

	void debug(const std::string &s)
	{
		f_ << timestamp() << " DEBUG: " << s << std::endl;
	}

	void info(const std::string &s)
	{
		f_ << timestamp() << " INFO: " << s << std::endl;
	}

	void warn(const std::string &s)
	{
		f_ << timestamp() << " WARN: " << s << std::endl;
	}

	void error(const std::string &s)
	{
		f_ << timestamp() << " ERROR: " << s << std::endl;
	}

private:
	std::fstream f_;

	// rfc3339, rfc5424
	std::string timestamp() const
	{
		time_t t = std::time(nullptr);
		struct tm *tm = std::localtime(&t);

		if (!tm)
			return "1970-01-01T00:00:00.000Z";

		char buf[64];

		std::strftime(buf, sizeof(buf), "%FT%T.000Z", tm);

		return buf;
	}
};

} // namespace OneDrive

extern OneDrive::CLog gLog;

#define LOG_DEBUG(args)       \
({                            \
	std::stringstream ss; \
	ss << args;           \
	gLog.debug(ss.str()); \
})

#define LOG_INFO(args)        \
({                            \
	std::stringstream ss; \
	ss << args;           \
	gLog.info(ss.str());  \
})

#define LOG_WARN(args)        \
({                            \
	std::stringstream ss; \
	ss << args;           \
	gLog.warn(ss.str());  \
})

#define LOG_ERROR(args)       \
({                            \
	std::stringstream ss; \
	ss << args;           \
	gLog.error(ss.str()); \
})

#endif // __LOG_H_INCLUDED__
