#include <fstream>
#include "curl.h"
#include "graph.h"

namespace OneDrive {

void CGraph::init(AppConfigPtr &&config)
{
	config_ = std::move(config);

	// Get an authorization code
	if (config_->authorizationCode().empty()) {
		std::list<std::pair<std::string, std::string>> params;

		params.emplace_back(std::make_pair("client_id", config_->clientId()));
		params.emplace_back(std::make_pair("scope", "files.readwrite.all offline_access"));
		params.emplace_back(std::make_pair("response_type", "code"));
		params.emplace_back(std::make_pair("redirect_uri", config_->redirectUri()));

		std::string url = httpClient_.buildUrl(config_->authorityUrl() + config_->authEndpoint(), params);

		throw std::runtime_error(std::string("missing authorization code\n\n"
			"Open the following URL in your browser (private window) and retrieve the authorization code:\n" + url));
	}

	// Redeem the code for access tokens
	if (config_->token().empty() || config_->refreshToken().empty()) {
		std::list<std::pair<std::string, std::string>> params;

		params.emplace_back(std::make_pair("client_id", config_->clientId()));
		params.emplace_back(std::make_pair("redirect_uri", config_->redirectUri()));
		params.emplace_back(std::make_pair("code", config_->authorizationCode()));
		params.emplace_back(std::make_pair("grant_type", "authorization_code"));

		std::string url = config_->authorityUrl() + config_->tokenEndpoint();
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

	params.emplace_back(std::make_pair("client_id", config_->clientId()));
	params.emplace_back(std::make_pair("redirect_uri", config_->redirectUri()));
	params.emplace_back(std::make_pair("refresh_token", config_->refreshToken()));
	params.emplace_back(std::make_pair("grant_type", "refresh_token"));

	std::string url = config_->authorityUrl() + config_->tokenEndpoint();
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

std::string CGraph::request(const std::string &resource)
{
	std::string url = "https://graph.microsoft.com/v1.0" + resource;

	std::list<std::string> headers;

	headers.emplace_back(std::string("Authorization: " + config_->tokenType() + " " + config_->token()));

	long respCode = 0;

	std::string data = httpClient_.get(url, headers, respCode);

	if (respCode == 401) {
		refreshToken();
		config_->readToken();
	} else if (respCode != 200)
		throw std::runtime_error("the server responded with: " + data);

	return data;
}

void CGraph::request(const std::string &resource, std::ofstream &file)
{
	std::string url = "https://graph.microsoft.com/v1.0" + resource;

	std::list<std::string> headers;

	headers.emplace_back(std::string("Authorization: " + config_->tokenType() + " " + config_->token()));

	long respCode = 0;

	httpClient_.download(url, headers, file, respCode);

	if (respCode == 401) {
		refreshToken();
		config_->readToken();
	} else if (respCode != 200)
		throw std::runtime_error("HTTP error while downloading: " + std::to_string(respCode));
}

} // namespace OneDrive
