// SPDX-License-Identifier: GPL-2.0

#include <fstream>
#include "curl.h"
#include "graph.h"
#include "log.h"

namespace OneDrive {

void CGraph::init()
{
	// Get an authorization code
	if (gConfig.authorizationCode().empty()) {
		std::list<std::pair<std::string, std::string>> params;

		params.emplace_back(std::make_pair("client_id", gConfig.clientId()));
		params.emplace_back(std::make_pair("scope", "files.readwrite.all offline_access"));
		params.emplace_back(std::make_pair("response_type", "code"));
		params.emplace_back(std::make_pair("redirect_uri", gConfig.redirectUri()));

		std::string url = httpClient_.buildUrl(gConfig.authorityUrl() + gConfig.authEndpoint(), params);

		throw std::runtime_error(std::string("missing authorization code\n\n"
			"Open the following URL in your browser (private window) and retrieve the authorization code:\n" + url));
	}

	// Redeem the code for access tokens
	if (gConfig.token().empty() || gConfig.refreshToken().empty()) {
		std::list<std::pair<std::string, std::string>> params;

		params.emplace_back(std::make_pair("client_id", gConfig.clientId()));
		params.emplace_back(std::make_pair("redirect_uri", gConfig.redirectUri()));
		params.emplace_back(std::make_pair("code", gConfig.authorizationCode()));
		params.emplace_back(std::make_pair("grant_type", "authorization_code"));

		std::string url = gConfig.authorityUrl() + gConfig.tokenEndpoint();
		std::string body = httpClient_.formEncode(params);
		std::list<std::string> headers;
		long respCode = 0;

		std::string data = httpClient_.post(url, headers, body, respCode);

		if (respCode != 200)
			throw std::runtime_error("the server responded with: " + data);

		std::ofstream f("token.json", std::ios::binary);

		if (!f)
			throw std::runtime_error("failed to save the token JSON");

		f << data;
	}
}

void CGraph::refreshToken()
{
	std::list<std::pair<std::string, std::string>> params;

	params.emplace_back(std::make_pair("client_id", gConfig.clientId()));
	params.emplace_back(std::make_pair("redirect_uri", gConfig.redirectUri()));
	params.emplace_back(std::make_pair("refresh_token", gConfig.refreshToken()));
	params.emplace_back(std::make_pair("grant_type", "refresh_token"));

	std::string url = gConfig.authorityUrl() + gConfig.tokenEndpoint();
	std::string body = httpClient_.formEncode(params);
	std::list<std::string> headers;
	long respCode = 0;

	std::string data = httpClient_.post(url, headers, body, respCode);

	if (respCode != 200)
		throw std::runtime_error("the server responded with: " + data);

	std::ofstream f(gConfig.configDir() + "/token.json", std::ios::binary);

	if (!f)
		throw std::runtime_error("failed to save the token JSON");

	f << data;
}

std::string CGraph::request(const std::string &resource)
{
	std::string url = "https://graph.microsoft.com/v1.0" + resource;

	std::string data;

	long respCode = 0;

	unsigned int retries = 3;

	do {
		std::list<std::string> headers;

		headers.emplace_back(std::string("Authorization: " + gConfig.tokenType() + " " + gConfig.token()));

		respCode = 0;

		data = httpClient_.get(url, headers, respCode);

		if (respCode == 401) {
			refreshToken();
			gConfig.readToken();
		} else if (respCode != 200)
			throw std::runtime_error("the server responded with: " + data);
	} while (respCode != 200 && retries-- > 0);

	if (respCode != 200)
		throw std::runtime_error("the server responded with: " + data);

	return data;
}

void CGraph::request(const std::string &resource, std::ofstream &file)
{
	std::string url = "https://graph.microsoft.com/v1.0" + resource;

	long respCode;

	unsigned int retries = 3;

	do {
		std::list<std::string> headers;

		headers.emplace_back(std::string("Authorization: " + gConfig.tokenType() + " " + gConfig.token()));

		respCode = 0;

		httpClient_.download(url, headers, file, respCode);

		if (respCode == 401) {
			refreshToken();
			gConfig.readToken();
		} else if (respCode != 200)
			throw std::runtime_error("the server responded with: " + std::to_string(respCode));
	} while (respCode != 200 && retries-- > 0);

	if (respCode != 200)
		throw std::runtime_error("the server responded with: " + std::to_string(respCode));
}

size_t CGraph::request(const std::string &url, void *buf, size_t size, off_t offset)
{
	size_t ret = 0;

	long respCode;

	unsigned int retries = 3;

	do {
		std::list<std::string> headers;

		headers.emplace_back(std::string("Authorization: " + gConfig.tokenType() + " " + gConfig.token()));
		headers.emplace_back(std::string("Range: bytes=" + std::to_string(offset) + "-" + std::to_string(offset + size - 1)));

		respCode = 0;

		ret = httpClient_.get(url, headers, buf, size, respCode);

		if (respCode == 401) {
			refreshToken();
			gConfig.readToken();
		} else if (respCode != 206 && respCode != 416)
			throw std::runtime_error("HTTP error while downloading: " + std::to_string(respCode));
	} while (respCode == 401 && retries-- > 0);

	if (respCode != 206 && respCode != 416)
		throw std::runtime_error("HTTP error while downloading: " + std::to_string(respCode));

	return ret;
}

void CGraph::deleteRequest(const std::string &resource)
{
	std::string url = "https://graph.microsoft.com/v1.0" + resource;

	long respCode;

	unsigned int retries = 3;

	do {
		std::list<std::string> headers;

		headers.emplace_back(std::string("Authorization: " + gConfig.tokenType() + " " + gConfig.token()));

		respCode = 0;

		httpClient_.deleteRequest(url, headers, respCode);

		if (respCode == 401) {
			refreshToken();
			gConfig.readToken();
		} else if (respCode != 204)
			throw std::runtime_error("HTTP error while deleting: " + std::to_string(respCode));
	} while (respCode == 401 && retries-- > 0);

	if (respCode != 204)
		throw std::runtime_error("HTTP error while deleting: " + std::to_string(respCode));
}

void CGraph::patchRequest(const std::string &resource, const std::string &body)
{
	std::string url = "https://graph.microsoft.com/v1.0" + resource;

	long respCode;

	unsigned int retries = 3;

	do {
		std::list<std::string> headers;

		headers.emplace_back(std::string("Authorization: " + gConfig.tokenType() + " " + gConfig.token()));
		headers.emplace_back(std::string("Content-Type: application/json"));

		respCode = 0;

		httpClient_.patchRequest(url, headers, body, respCode);

		if (respCode == 401) {
			refreshToken();
			gConfig.readToken();
		} else if (respCode != 200)
			throw std::runtime_error("HTTP error while patching: " + std::to_string(respCode));
	} while (respCode == 401 && retries-- > 0);

	if (respCode != 200)
		throw std::runtime_error("HTTP error while patching: " + std::to_string(respCode));
}

void CGraph::upload(const std::string &resource, const std::string &body)
{
	std::string url = "https://graph.microsoft.com/v1.0" + resource;

	long respCode;

	unsigned int retries = 3;

	do {
		std::list<std::string> headers;

		headers.emplace_back(std::string("Authorization: " + gConfig.tokenType() + " " + gConfig.token()));
		headers.emplace_back(std::string("Content-Type: application/octet-stream"));

		respCode = 0;

		httpClient_.putRequest(url, headers, body, respCode);

		if (respCode == 401) {
			refreshToken();
			gConfig.readToken();
		} else if (respCode != 200)
			throw std::runtime_error("HTTP error while uploading: " + std::to_string(respCode));
	} while (respCode == 401 && retries-- > 0);

	if (respCode != 200)
		throw std::runtime_error("HTTP error while uploading: " + std::to_string(respCode));
}

} // namespace OneDrive
