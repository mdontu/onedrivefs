#include <json/json.h>
#include <fstream>
#include <stdexcept>
#include "appconfig.h"

namespace OneDrive {

CAppConfig::CAppConfig()
{
	readConfig();
	readToken();
}

void CAppConfig::readConfig()
{
	std::ifstream f("config.json", std::ios::binary);

	if (!f)
		throw std::runtime_error("failed to read the configuration file");

	Json::Value root;

	f >> root;

	if (!!root["authority_url"])
		authorityUrl_ = root["authority_url"].asString();
	if (authorityUrl_.empty())
		throw std::runtime_error("the authority URL was not found in the configuration file");

	if (!!root["auth_endpoint"])
		authEndpoint_ = root["auth_endpoint"].asString();
	if (authEndpoint_.empty())
		throw std::runtime_error("the authorization endpoint was not found in the configuration file");

	if (!!root["token_endpoint"])
		tokenEndpoint_ = root["token_endpoint"].asString();
	if (tokenEndpoint_.empty())
		throw std::runtime_error("the token endpoint was not found in the configuration file");

	if (!!root["client_id"])
		clientId_ = root["client_id"].asString();
	if (clientId_.empty())
		throw std::runtime_error("the client ID was not found in the configuration file");

	if (!!root["redirect_uri"])
		redirectUri_ = root["redirect_uri"].asString();
	if (redirectUri_.empty())
		throw std::runtime_error("the redirection URI was not found in the configuration file");

	authorizationCode_ = root["authorization_code"].asString();
}

void CAppConfig::readToken()
{
	std::ifstream f("token.json", std::ios::binary);

	if (!f)
		return;

	Json::Value root;

	f >> root;

	tokenType_       = root["token_type"].asString();
	tokenScope_      = root["scope"].asString();
	tokenExpires_    = root["expires_in"].asString();
	tokenExtExpires_ = root["ext_expires_in"].asString();
	token_           = root["access_token"].asString();
	refreshToken_    = root["refresh_token"].asString();
}

} // namespace OneDrive
