// SPDX-License-Identifier: GPL-2.0

#ifndef __UTILS_H_INCLUDED__
#define __UTILS_H_INCLUDED__

#include <list>
#include <string>

namespace OneDrive {

static inline void stringSplit(const std::string &s, char sep, std::list<std::string> &parts)
{
	auto start = s.cbegin();
	auto end = start;
	auto i = start;

	while (i != s.cend()) {
		if (*i == sep) {
			parts.emplace_back(std::string(start, i));
			start = end = ++i;
			while (*start == sep)
				++start;
		} else {
			end = i;
			++i;
		}
	}

	if (start != i)
		parts.emplace_back(std::string(start, i));
}

} // namespace OneDrive

#endif // __UTILS_H_INCLUDED__
