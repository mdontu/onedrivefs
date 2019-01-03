#include <sys/stat.h>
#include <sys/types.h>
#include <json/json.h>
#include <cerrno>
#include <cstring>
#include <fstream>
#include <stdexcept>
#include "appconfig.h"
#include "utils.h"

namespace OneDrive {

void CAppConfig::setConfigDir(const std::string &configDir)
{
	configDir_ = configDir;

	std::list<std::string> pathMembers;

	stringSplit(configDir_, '/', pathMembers);

	std::string path;

	for (auto &&pm : pathMembers) {
		struct stat st{};

		path += "/" + pm;

		if (stat(path.c_str(), &st) == -1) {
			if (errno != ENOENT)
				throw std::runtime_error(std::string("stat() has failed: ") + std::strerror(errno));
			if (mkdir(path.c_str(), 0755) < 0)
				throw std::runtime_error(std::string("mkdir() has failed: ") + std::strerror(errno));
		}
	}
}

void CAppConfig::readConfig()
{
	const std::string cfg = configDir_ + "/config.json";

	std::ifstream f(cfg, std::ios::binary);

	if (!f)
		throw std::runtime_error("failed to read the configuration file " + cfg);

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
	std::ifstream f(configDir_ + "/token.json", std::ios::binary);

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
