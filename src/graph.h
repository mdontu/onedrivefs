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

private:
	CCurl httpClient_;

	void refreshToken();
};

} // namespace OneDrive

#endif // __GRAPH_H_INCLUDED__
