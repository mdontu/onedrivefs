// SPDX-License-Identifier: GPL-2.0

#ifndef __ONEDRIVE_H_INCLUDED__
#define __ONEDRIVE_H_INCLUDED__

#include <fstream>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include "graph.h"

namespace OneDrive {

class COwner
{
public:
	COwner()
	{
	}

	COwner(const std::string &displayName, const std::string &id): displayName_{displayName}, id_{id}
	{
	}

	COwner(const COwner &owner): displayName_{owner.displayName_}, id_{owner.id_}
	{
	}

	~COwner()
	{
	}

	COwner & operator=(const COwner &owner)
	{
		displayName_ = owner.displayName_;
		id_          = owner.id_;

		return *this;
	}

	std::string displayName() const
	{
		return displayName_;
	}

	std::string id() const
	{
		return id_;
	}

private:
	std::string displayName_;
	std::string id_;
};

class CQuota
{
public:
	CQuota()
	{
	}

	CQuota(const std::string &deleted, const std::string &remaining, const std::string &state,
	       const std::string &total, const std::string &used): deleted_{deleted}, remaining_{remaining},
			state_{state}, total_{total}, used_{used}
	{
	}

	CQuota(const CQuota &quota): deleted_{quota.deleted_}, remaining_{quota.remaining_},
		state_{quota.state_}, total_{quota.total_}, used_{quota.used_}
	{
	}

	~CQuota()
	{
	}

	CQuota & operator=(const CQuota &quota)
	{
		deleted_   = quota.deleted_;
		remaining_ = quota.remaining_;
		state_     = quota.state_;
		total_     = quota.total_;
		used_      = quota.used_;

		return *this;
	}

	std::string deleted() const
	{
		return deleted_;
	}

	std::string remaining() const
	{
		return remaining_;
	}

	std::string state() const
	{
		return state_;
	}

	std::string total() const
	{
		return total_;
	}

	std::string used() const
	{
		return used_;
	}

private:
	std::string deleted_;
	std::string remaining_;
	std::string state_;
	std::string total_;
	std::string used_;
};

class CDrive
{
public:
	CDrive()
	{
	}

	CDrive(const std::string &id, const std::string type): id_{id}, type_{type}
	{
	}

	CDrive(const CDrive &drive): id_{drive.id_}, type_{drive.type_},
		owner_{drive.owner_}, quota_{drive.quota_}
	{
	}

	~CDrive()
	{
	}

	CDrive & operator=(const CDrive &drive)
	{
		id_    = drive.id_;
		type_  = drive.type_;
		owner_ = drive.owner_;
		quota_ = drive.quota_;

		return *this;
	}

	std::string id() const
	{
		return id_;
	}

	std::string type() const
	{
		return type_;
	}

	COwner owner() const
	{
		return owner_;
	}

	void setOwner(const COwner &owner)
	{
		owner_ = owner;
	}

	CQuota quota() const
	{
		return quota_;
	}

	void setQuota(const CQuota &quota)
	{
		quota_ = quota;
	}

private:
	std::string id_;
	std::string type_;
	COwner      owner_;
	CQuota      quota_;
};

class CDriveItem
{
public:
	enum DriveItemType {
		DRIVE_ITEM_FOLDER,
		DRIVE_ITEM_FILE,
		DRIVE_ITEM_UNKNOWN
	};

	CDriveItem()
	{
	}

	CDriveItem(const std::string &id, const std::string &name, const std::string &size,
		   const std::string &createTime, const std::string &modifiedTime,
		   const std::string &url, DriveItemType type):
			id_{id}, name_{name}, size_{size}, createTime_{createTime}, modifiedTime_{modifiedTime},
			url_{url}, type_{type}
	{
	}

	CDriveItem(const CDriveItem &driveItem): id_{driveItem.id_}, name_{driveItem.name_},
		size_{driveItem.size_}, createTime_{driveItem.createTime_}, modifiedTime_{driveItem.modifiedTime_},
		url_{driveItem.url_}, type_{driveItem.type_}, hash_{driveItem.hash_}, cacheTime_{driveItem.cacheTime_}
	{
	}

	CDriveItem & operator=(const CDriveItem &driveItem)
	{
		id_           = driveItem.id_;
		name_         = driveItem.name_;
		size_         = driveItem.size_;
		createTime_   = driveItem.createTime_;
		modifiedTime_ = driveItem.modifiedTime_;
		url_          = driveItem.url_;
		type_         = driveItem.type_;
		hash_         = driveItem.hash_;
		cacheTime_    = driveItem.cacheTime_;

		return *this;
	}

	~CDriveItem()
	{
	}

	std::string id() const
	{
		return id_;
	}

	std::string name() const
	{
		return name_;
	}

	std::string size() const
	{
		return size_;
	}

	std::string createTime() const
	{
		return createTime_;
	}

	std::string modifiedTime() const
	{
		return modifiedTime_;
	}

	std::string url() const
	{
		return url_;
	}

	DriveItemType type() const
	{
		return type_;
	}

	void setDriveItemType(DriveItemType type)
	{
		type_ = type;
	}

	std::string hash() const
	{
		return hash_;
	}

	void setHash(const std::string &hash)
	{
		hash_ = hash;
	}

	time_t cacheTime() const
	{
		return cacheTime_;
	}

	void setCacheTime(time_t cacheTime)
	{
		cacheTime_ = cacheTime;
	}

private:
	std::string   id_;
	std::string   name_;
	std::string   size_;
	std::string   createTime_;
	std::string   modifiedTime_;
	std::string   url_;
	DriveItemType type_{DRIVE_ITEM_UNKNOWN};
	std::string   hash_;
	time_t        cacheTime_;
};

class COneDrive
{
public:
	COneDrive()
	{
	}

	~COneDrive()
	{
	}

	COneDrive(const COneDrive &) = delete;
	COneDrive & operator=(const COneDrive &) = delete;

	CDrive drive();

	void drives(std::list<CDrive> &drives);

	void listChildren(std::list<CDriveItem> &driveItems);

	void listChildren(const CDriveItem &driveItem, std::list<CDriveItem> &driveItems);

	void download(const CDriveItem &driveItem, std::ofstream &file);

	CDriveItem root();

	CDriveItem itemFromPath(const std::string &path);

	void driveItemTime(const std::string &s, struct timespec &ts);

	size_t read(const CDriveItem &driveItem, void *buf, size_t size, off_t offset);

	void deleteItem(const CDriveItem &driveItem);

	CDriveItem queryCache(const std::string &path);

	void cache(const std::string &path, CDriveItem &driveItem);

	void cacheLocked(const std::string &path, CDriveItem &driveItem)
	{
		std::lock_guard<std::mutex> lock(mutex_);

		cache(path, driveItem);
	}

private:
	CGraph                            graph_;
	std::mutex                        mutex_;
	std::map<std::size_t, CDriveItem> cache_;
};

} // namespace OneDrive

#endif // __ONEDRIVE_H_INCLUDED__
