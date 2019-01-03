// SPDX-License-Identifier: GPL-2.0

#ifndef __CURL_H_INCLUDED__
#define __CURL_H_INCLUDED__

#include <stddef.h>
#include <curl/curl.h>
#include <fstream>
#include <list>
#include <map>
#include <string>
#include <utility>

namespace OneDrive {

class CCurl
{
public:
	CCurl();
	~CCurl();

	CCurl(const CCurl &) = delete;
	CCurl & operator=(const CCurl &) = delete;

	std::string get(const std::string &url, const std::list<std::string> &headers,
			long &respCode);

	size_t get(const std::string &url, const std::list<std::string> &headers,
		   void *buf, size_t size, long &respCode);

	std::string post(const std::string &url, const std::list<std::string> &headers,
			 const std::string &body, long &respCode);

	void download(const std::string &url, const std::list<std::string> &headers,
		      std::ofstream &file, long &respCode);

	void deleteRequest(const std::string &url, const std::list<std::string> &headers,
			   long &respCode);

	void patchRequest(const std::string &url, const std::list<std::string> &headers,
			  const std::string &body, long &respCode);

	void putRequest(const std::string &url, const std::list<std::string> &headers,
			const std::string &body, long &respCode);

	std::string escape(const std::string &str);

	std::string buildUrl(const std::string &url,
			     const std::list<std::pair<std::string, std::string>> &params);

	std::string formEncode(const std::list<std::pair<std::string, std::string>> &params);

private:
	void setopt(CURLoption option, long arg);
	void setopt(CURLoption option, const std::string &arg);
	void setopt(CURLoption option, void *arg);

	long perform();

	static size_t writeCallback(char *ptr, size_t size, size_t nmemb, void *userdata);

	static size_t writeBufferCallback(char *ptr, size_t size, size_t nmemb, void *userdata);

	static size_t downloadCallback(char *ptr, size_t size, size_t nmemb, void *userdata);

	CURL *handle_{};
};

} // namespace OneDrive

#endif // __CURL_H_INCLUDED__
