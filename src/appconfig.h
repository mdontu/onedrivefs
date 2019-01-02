#ifndef __APPCONFIG_H_INCLUDED__
#define __APPCONFIG_H_INCLUDED__

#include <memory>
#include <string>

namespace OneDrive {

class CAppConfig
{
public:
	CAppConfig();

	~CAppConfig()
	{
	}

	CAppConfig(const CAppConfig &) = delete;
	CAppConfig & operator=(const CAppConfig &) = delete;

	void readConfig();

	void readToken();

	std::string authorityUrl() const
	{
		return authorityUrl_;
	}

	std::string authEndpoint() const
	{
		return authEndpoint_;
	}

	std::string tokenEndpoint() const
	{
		return tokenEndpoint_;
	}

	std::string clientId() const
	{
		return clientId_;
	}

	std::string redirectUri() const
	{
		return redirectUri_;
	}

	std::string authorizationCode() const
	{
		return authorizationCode_;
	}

	std::string tokenType() const
	{
		return tokenType_;
	}

	std::string tokenScope() const
	{
		return tokenScope_;
	}

	std::string tokenExpires() const
	{
		return tokenExpires_;
	}

	std::string tokenExtExpires() const
	{
		return tokenExtExpires_;
	}

	std::string token() const
	{
		return token_;
	}

	std::string refreshToken() const
	{
		return refreshToken_;
	}

private:
	std::string authorityUrl_;
	std::string authEndpoint_;
	std::string tokenEndpoint_;
	std::string clientId_;
	std::string redirectUri_;
	std::string authorizationCode_;

	std::string tokenType_;
	std::string tokenScope_;
	std::string tokenExpires_;
	std::string tokenExtExpires_;
	std::string token_;
	std::string refreshToken_;
};

using AppConfigPtr = std::unique_ptr<CAppConfig>;

} // namespace OneDrive

#endif // __APPCONFIG_H_INCLUDED__
