// SPDX-License-Identifier: GPL-2.0

#include <cstring>
#include <memory>
#include <stdexcept>
#include "curl.h"

namespace {

class CCurlGlobal
{
public:
	CCurlGlobal()
	{
		if (curl_global_init(CURL_GLOBAL_ALL) != CURLE_OK)
			throw std::runtime_error("the cURL global initializer has failed");
	}

	~CCurlGlobal()
	{
		curl_global_cleanup();
	}

private:
	CCurlGlobal(const CCurlGlobal &) = delete;
	CCurlGlobal & operator=(const CCurlGlobal &) = delete;
} globalCurl;

struct DownloadBuffer {
	void *buf;
	size_t size;
	size_t pos;
};

} // anonymous namespace

namespace OneDrive {

CCurl::CCurl()
{
	handle_ = curl_easy_init();
	if (!handle_)
		throw std::runtime_error("failed to obtain a cURL handle");
}

CCurl::~CCurl()
{
	curl_easy_cleanup(handle_);
}

void CCurl::setopt(CURLoption option, long arg)
{
	CURLcode err = curl_easy_setopt(handle_, option, arg);

	if (err != CURLE_OK)
		throw std::runtime_error(std::string("curl_easy_setopt() has failed: ") + curl_easy_strerror(err));
}

void CCurl::setopt(CURLoption option, const std::string &arg)
{
	CURLcode err = curl_easy_setopt(handle_, option, arg.c_str());

	if (err != CURLE_OK)
		throw std::runtime_error(std::string("curl_easy_setopt() has failed: ") + curl_easy_strerror(err));
}

void CCurl::setopt(CURLoption option, void *arg)
{
	CURLcode err = curl_easy_setopt(handle_, option, arg);

	if (err != CURLE_OK)
		throw std::runtime_error(std::string("curl_easy_setopt() has failed: ") + curl_easy_strerror(err));
}

std::string CCurl::get(const std::string &url, const std::list<std::string> &headers, long &respCode)
{
	struct curl_slist *slist = nullptr;

	for (auto &&h : headers)
		slist = curl_slist_append(slist, h.c_str());

	std::unique_ptr<struct curl_slist, decltype(&curl_slist_free_all)> sp(slist, &curl_slist_free_all);

	setopt(CURLOPT_HTTPHEADER, slist);

	std::string buf;
	setopt(CURLOPT_WRITEDATA, static_cast<void *>(&buf));
	setopt(CURLOPT_WRITEFUNCTION, reinterpret_cast<void *>(writeCallback));

	setopt(CURLOPT_HTTPGET, 1);
	setopt(CURLOPT_URL, url);

	setopt(CURLOPT_SSL_VERIFYPEER, 1);
	setopt(CURLOPT_SSL_VERIFYHOST, 2);
	setopt(CURLOPT_TIMEOUT, 10);
	setopt(CURLOPT_CONNECTTIMEOUT, 30);
	setopt(CURLOPT_FOLLOWLOCATION, 1);

	respCode = perform();

	return buf;
}

size_t CCurl::get(const std::string &url, const std::list<std::string> &headers,
		  void *buf, size_t size, long &respCode)
{
	struct curl_slist *slist = nullptr;

	for (auto &&h : headers)
		slist = curl_slist_append(slist, h.c_str());

	std::unique_ptr<struct curl_slist, decltype(&curl_slist_free_all)> sp(slist, &curl_slist_free_all);

	setopt(CURLOPT_HTTPHEADER, slist);

	DownloadBuffer db{};
	db.buf = buf;
	db.size = size;

	setopt(CURLOPT_WRITEDATA, static_cast<void *>(&db));
	setopt(CURLOPT_WRITEFUNCTION, reinterpret_cast<void *>(writeBufferCallback));

	setopt(CURLOPT_HTTPGET, 1);
	setopt(CURLOPT_URL, url);

	setopt(CURLOPT_SSL_VERIFYPEER, 1);
	setopt(CURLOPT_SSL_VERIFYHOST, 2);
	setopt(CURLOPT_TIMEOUT, 10);
	setopt(CURLOPT_CONNECTTIMEOUT, 30);
	setopt(CURLOPT_FOLLOWLOCATION, 1);

	respCode = perform();

	return db.pos;
}

std::string CCurl::post(const std::string &url, const std::list<std::string> &headers, const std::string &body,
			long &respCode)
{
	struct curl_slist *slist = nullptr;

	for (auto &&h : headers)
		slist = curl_slist_append(slist, h.c_str());

	std::unique_ptr<struct curl_slist, decltype(&curl_slist_free_all)> sp(slist, &curl_slist_free_all);

	setopt(CURLOPT_HTTPHEADER, slist);

	setopt(CURLOPT_POSTFIELDSIZE, body.length());
	setopt(CURLOPT_COPYPOSTFIELDS, body.c_str());

	std::string buf;
	setopt(CURLOPT_WRITEDATA, static_cast<void *>(&buf));
	setopt(CURLOPT_WRITEFUNCTION, reinterpret_cast<void *>(writeCallback));

	setopt(CURLOPT_URL, url);

	setopt(CURLOPT_SSL_VERIFYPEER, 1);
	setopt(CURLOPT_SSL_VERIFYHOST, 2);
	setopt(CURLOPT_TIMEOUT, 10);
	setopt(CURLOPT_CONNECTTIMEOUT, 30);
	setopt(CURLOPT_FOLLOWLOCATION, 1);

	respCode = perform();

	return buf;
}

void CCurl::download(const std::string &url, const std::list<std::string> &headers, std::ofstream &file,
		     long &respCode)
{
	struct curl_slist *slist = nullptr;

	for (auto &&h : headers)
		slist = curl_slist_append(slist, h.c_str());

	std::unique_ptr<struct curl_slist, decltype(&curl_slist_free_all)> sp(slist, &curl_slist_free_all);

	setopt(CURLOPT_HTTPHEADER, slist);

	setopt(CURLOPT_WRITEDATA, static_cast<void *>(&file));
	setopt(CURLOPT_WRITEFUNCTION, reinterpret_cast<void *>(downloadCallback));

	setopt(CURLOPT_HTTPGET, 1);
	setopt(CURLOPT_URL, url);

	setopt(CURLOPT_SSL_VERIFYPEER, 1);
	setopt(CURLOPT_SSL_VERIFYHOST, 2);
	setopt(CURLOPT_TIMEOUT, 10);
	setopt(CURLOPT_CONNECTTIMEOUT, 30);
	setopt(CURLOPT_FOLLOWLOCATION, 1);

	respCode = perform();
}

long CCurl::perform()
{
	CURLcode err = curl_easy_perform(handle_);
	if (err != CURLE_OK)
		throw std::runtime_error(std::string("curl_easy_perform() has failed: ") + curl_easy_strerror(err));

	long respCode = 0;
	err = curl_easy_getinfo(handle_, CURLINFO_RESPONSE_CODE, &respCode);
	if (err != CURLE_OK)
		throw std::runtime_error(std::string("curl_easy_getinfo() has failed: ") + curl_easy_strerror(err));

	curl_easy_reset(handle_);

	return respCode;
}

size_t CCurl::writeCallback(char *ptr, size_t size, size_t nmemb, void *userData)
{
	if (!userData)
		return 0;

	std::string *buf = static_cast<std::string *>(userData);

	try {
		buf->append(ptr, size * nmemb);
	} catch (...) {
		return 0;
	}

	return size * nmemb;
}

size_t CCurl::writeBufferCallback(char *ptr, size_t size, size_t nmemb, void *userData)
{
	if (!userData)
		return 0;

	DownloadBuffer *db = static_cast<DownloadBuffer *>(userData);

	size_t n = std::min(size * nmemb, db->size - db->pos);

	memcpy(static_cast<char *>(db->buf) + db->pos, ptr, n);

	db->pos += n;

	return n;
}

size_t CCurl::downloadCallback(char *ptr, size_t size, size_t nmemb, void *userData)
{
	if (!userData)
		return 0;

	std::ofstream *file = static_cast<std::ofstream *>(userData);

	try {
		file->write(ptr, size * nmemb);
	} catch (...) {
		return 0;
	}

	return size * nmemb;
}

std::string CCurl::escape(const std::string &str)
{
	char *ptr = curl_easy_escape(handle_, str.c_str(), str.length());

	if (!ptr)
		throw std::runtime_error("attempted to URL encode an invalid string");

	std::string res(ptr);

	curl_free(ptr);

	return res;
}

std::string CCurl::buildUrl(const std::string &url, const std::list<std::pair<std::string, std::string>> &params)
{
	std::string p;

	for (auto &&i : params) {
		if (!p.empty())
			p += "&";
		p += i.first + "=" + escape(i.second);
	}

	if (p.empty())
		return url;

	return url + "?" + p;
}

std::string CCurl::formEncode(const std::list<std::pair<std::string, std::string>> &params)
{
	std::string p;

	for (auto &&i : params) {
		if (!p.empty())
			p += "&";
		p += i.first + "=" + escape(i.second);
	}

	return p;
}

} // namespace OneDrive
