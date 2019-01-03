// SPDX-License-Identifier: GPL-2.0

#include <ctime>
#include <json/json.h>
#include <chrono>
#include <iostream>
#include <sstream>
#include "onedrive.h"
#include "log.h"
#include "utils.h"

namespace {

OneDrive::CDrive driveFromJson(const Json::Value &node)
{
	return OneDrive::CDrive(node["id"].asString(), node["driveType"].asString());
}

OneDrive::COwner ownerFromJson(const Json::Value &node)
{
	return OneDrive::COwner(node["owner"]["user"]["displayName"].asString(),
				node["owner"]["user"]["id"].asString());
}

OneDrive::CQuota quotaFromJson(const Json::Value &node)
{
	return OneDrive::CQuota(node["quota"]["deleted"].asString(), node["quota"]["remaining"].asString(),
				node["quota"]["state"].asString(), node["quota"]["total"].asString(),
				node["quota"]["used"].asString());
}

OneDrive::CDriveItem driveItemFromJson(const Json::Value &node)
{
	OneDrive::CDriveItem::DriveItemType type = OneDrive::CDriveItem::DRIVE_ITEM_UNKNOWN;

	if (!!node["folder"])
		type = OneDrive::CDriveItem::DRIVE_ITEM_FOLDER;
	else if (!!node["file"])
		type = OneDrive::CDriveItem::DRIVE_ITEM_FILE;

	OneDrive::CDriveItem driveItem(node["id"].asString(), node["name"].asString(), node["size"].asString(),
				       node["createdDateTime"].asString(), node["lastModifiedDateTime"].asString(),
				       node["@microsoft.graph.downloadUrl"].asString(), type);

	if (!!node["file"] && !!node["file"]["hashes"] && !!node["file"]["hashes"]["sha1Hash"])
		driveItem.setHash(node["file"]["hashes"]["sha1Hash"].asString());

	return driveItem;
}

} // anonymous namespace

namespace OneDrive {

CDrive COneDrive::drive()
{
	std::lock_guard<std::mutex> lock(mutex_);

	std::stringstream data;

	data << graph_.request("/me/drive");

	Json::Value root;

	data >> root;

	CDrive drive(driveFromJson(root));

	drive.setOwner(ownerFromJson(root));

	drive.setQuota(quotaFromJson(root));

	return drive;
}

void COneDrive::drives(std::list<CDrive> &drives)
{
	std::lock_guard<std::mutex> lock(mutex_);

	std::stringstream data;

	data << graph_.request("/me/drives");

	Json::Value root;

	data >> root;

	for (unsigned int i = 0; i < root["value"].size(); i++) {
		const Json::Value node(root["value"][i]);

		CDrive drive(driveFromJson(node));

		drive.setOwner(ownerFromJson(node));

		drive.setQuota(quotaFromJson(node));

		drives.push_back(drive);
	}
}

void COneDrive::listChildren(std::list<CDriveItem> &driveItems)
{
	std::lock_guard<std::mutex> lock(mutex_);

	std::stringstream data;

	data << graph_.request("/me/drive/root/children");

	Json::Value root;

	data >> root;

	for (unsigned int i = 0; i < root["value"].size(); i++) {
		const Json::Value node(root["value"][i]);

		CDriveItem driveItem(driveItemFromJson(node));

		if (driveItem.type() == CDriveItem::DRIVE_ITEM_FILE ||
		    driveItem.type() == CDriveItem::DRIVE_ITEM_FOLDER)
			driveItems.push_back(driveItem);
	}
}

void COneDrive::listChildren(const CDriveItem &driveItem, std::list<CDriveItem> &driveItems)
{
	std::lock_guard<std::mutex> lock(mutex_);

	std::stringstream data;

	data << graph_.request("/me/drive/items/" + driveItem.id() + "/children");

	Json::Value root;

	data >> root;

	for (unsigned int i = 0; i < root["value"].size(); i++) {
		const Json::Value node(root["value"][i]);

		CDriveItem driveItem(driveItemFromJson(node));

		if (driveItem.type() == CDriveItem::DRIVE_ITEM_FILE ||
		    driveItem.type() == CDriveItem::DRIVE_ITEM_FOLDER)
			driveItems.push_back(driveItem);
	}
}

void COneDrive::download(const CDriveItem &driveItem, std::ofstream &file)
{
	std::lock_guard<std::mutex> lock(mutex_);

	graph_.request("/me/drive/items/" + driveItem.id() + "/content", file);
}

CDriveItem COneDrive::root()
{
	std::lock_guard<std::mutex> lock(mutex_);

	std::stringstream data;

	data << graph_.request("/me/drive/root");

	Json::Value root;

	data >> root;

	return CDriveItem(driveItemFromJson(root));
}

CDriveItem COneDrive::itemFromPath(const std::string &path)
{
	CDriveItem driveItem;

	{
		std::lock_guard<std::mutex> lock(mutex_);

		driveItem = queryCache(path);
		if (driveItem.type() != CDriveItem::DRIVE_ITEM_UNKNOWN)
			return driveItem;
	}

	std::list<std::string> items;

	stringSplit(path, '/', items);

	driveItem = root();

	for (auto &&i : items) {
		bool found = false;

		if (i.empty())
			continue;

		std::list<CDriveItem> driveItems;

		listChildren(driveItem, driveItems);

		for (auto &&j : driveItems) {
			if (j.name() == i) {
				driveItem = j;
				found = true;
				break;
			}
		}

		if (!found)
			return CDriveItem();
	}

	{
		std::lock_guard<std::mutex> lock(mutex_);

		cache(path, driveItem);
	}

	return driveItem;
}

// The source timestamp looks like this: 2009-05-06T23:31:32.193Z
void COneDrive::driveItemTime(const std::string &s, struct timespec &ts)
{
	struct tm tm{};

	strptime(s.c_str(), "%FT%T", &tm);

	ts.tv_sec = std::mktime(&tm);
	ts.tv_nsec = 0;
}

size_t COneDrive::read(const CDriveItem &driveItem, void *buf, size_t size, off_t offset)
{
	if (offset > std::stoll(driveItem.size()))
		return 0;

	if ((offset + size) > std::stoull(driveItem.size()))
		size = std::stoull(driveItem.size()) - offset;

	std::lock_guard<std::mutex> lock(mutex_);

	return graph_.request(driveItem.url(), buf, size, offset);
}

void COneDrive::deleteItem(const CDriveItem &driveItem)
{
	std::lock_guard<std::mutex> lock(mutex_);

	graph_.deleteRequest("/me/drive/items/" + driveItem.id());
}

void COneDrive::truncateItem(const CDriveItem &driveItem, off_t offset)
{
	std::lock_guard<std::mutex> lock(mutex_);

	graph_.upload("/me/drive/items/" + driveItem.id() + "/content", "");
}

CDriveItem COneDrive::queryCache(const std::string &path)
{
	time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

	std::size_t hash = std::hash<std::string>{}(path);

	auto driveItem = cache_.find(hash);

	if (driveItem == cache_.end())
		return CDriveItem();

	if (driveItem->second.cacheTime() > now || (now - driveItem->second.cacheTime()) > 30) {
		cache_.erase(driveItem);
		return CDriveItem();
	}

	return driveItem->second;
}

void COneDrive::cache(const std::string &path, CDriveItem &driveItem)
{
	time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

	std::size_t hash = std::hash<std::string>{}(path);

	driveItem.setCacheTime(now);

	cache_[hash] = driveItem;
}

} // namespace OneDrive
