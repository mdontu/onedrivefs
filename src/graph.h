// SPDX-License-Identifier: GPL-2.0

#ifndef __GRAPH_H_INCLUDED__
#define __GRAPH_H_INCLUDED__

#include <fstream>
#include "appconfig.h"
#include "curl.h"

namespace OneDrive {

class CGraph
{
public:
	CGraph()
	{
	}

	~CGraph()
	{
	}

	CGraph(const CGraph &) = delete;
	CGraph & operator=(const CGraph &) = delete;

	void init();

	std::string request(const std::string &resource);

	void request(const std::string &resource, std::ofstream &file);

	size_t request(const std::string &url, void *buf, size_t size, off_t offset);

	void deleteRequest(const std::string &resource);

	void upload(const std::string &resource, const std::string &body);

private:
	CCurl httpClient_;

	void refreshToken();
};

} // namespace OneDrive

#endif // __GRAPH_H_INCLUDED__
