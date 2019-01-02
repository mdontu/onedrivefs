#include <json/json.h>
#include <iostream>
#include <sstream>
#include "onedrive.h"

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
				       node["createdDateTime"].asString(), node["lastModifiedDateTime"].asString(), type);

	if (!!node["file"] && !!node["file"]["hashes"] && !!node["file"]["hashes"]["sha1Hash"])
		driveItem.setHash(node["file"]["hashes"]["sha1Hash"].asString());

	return driveItem;
}

} // anonymous namespace

namespace OneDrive {

CDrive COneDrive::drive()
{
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
	std::stringstream data;

	data << graph_.request("/me/drive/root/children");

	Json::Value root;

	data >> root;

	for (unsigned int i = 0; i < root["value"].size(); i++) {
		const Json::Value node(root["value"][i]);

		CDriveItem driveItem(driveItemFromJson(node));

		driveItems.push_back(driveItem);
	}
}

void COneDrive::listChildren(const CDriveItem &driveItem, std::list<CDriveItem> &driveItems)
{
	std::stringstream data;

	data << graph_.request("/me/drive/items/" + driveItem.id() + "/children");

	Json::Value root;

	data >> root;

	for (unsigned int i = 0; i < root["value"].size(); i++) {
		const Json::Value node(root["value"][i]);

		CDriveItem driveItem(driveItemFromJson(node));

		driveItems.push_back(driveItem);
	}
}

void COneDrive::download(const CDriveItem &driveItem, std::ofstream &file)
{
	graph_.request("/me/drive/items/" + driveItem.id() + "/content", file);
}

} // namespace OneDrive
