// SPDX-License-Identifier: GPL-2.0

#include <unistd.h>
#include <sys/types.h>
#include <cstring>
#include "fuse.h"
#include "log.h"

namespace {

const char userHashAttr[] = "user.hash";

} // anonymouse namespace

namespace OneDrive {

CFuse::CFuse()
{
	fuseOps_.init      = fuseInit;
	fuseOps_.destroy   = fuseDestroy;
	fuseOps_.getattr   = fuseGetAttr;
	fuseOps_.open      = fuseOpen;
	fuseOps_.read      = fuseRead;
	fuseOps_.release   = fuseRelease;
	fuseOps_.readdir   = fuseReadDir;
	fuseOps_.listxattr = fuseListXAttr;
	fuseOps_.getxattr  = fuseGetXAttr;
	fuseOps_.statfs    = fuseStatFs;
}

CFuse::~CFuse()
{
}

int CFuse::init(int argc, const char *argv[])
{
	struct fuse_args args = FUSE_ARGS_INIT(argc, const_cast<char **>(argv));

	int ret = fuse_main(args.argc, args.argv, &fuseOps_, nullptr);

	return ret;
}

void *CFuse::fuseInit(struct fuse_conn_info * /*conn*/)
{
	return new COneDrive;
}

void CFuse::fuseDestroy(void *ptr)
{
	delete static_cast<COneDrive *>(ptr);
}

int CFuse::fuseGetAttr(const char *path, struct stat *st)
{
	int err = 0;
	COneDrive *oneDrive = static_cast<COneDrive *>(fuse_get_context()->private_data);

	try {
		CDriveItem driveItem = oneDrive->itemFromPath(path);

		if (driveItem.type() == CDriveItem::DRIVE_ITEM_UNKNOWN)
			return -ENOENT;

		std::memset(st, 0, sizeof(*st));

		st->st_uid = getuid();
		st->st_gid = getgid();
		st->st_nlink = 1;
		if (driveItem.type() == CDriveItem::DRIVE_ITEM_FOLDER) {
			st->st_size = 4096;
			st->st_mode = S_IFDIR | S_IXUSR | S_IRUSR | S_IWUSR | S_IXGRP | S_IRGRP | S_IXOTH | S_IROTH;
		} else if (driveItem.type() == CDriveItem::DRIVE_ITEM_FILE) {
			st->st_size = std::stoll(driveItem.size());
			st->st_mode = S_IFREG | S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
		}
		oneDrive->driveItemTime(driveItem.createTime(), st->st_ctim);
		oneDrive->driveItemTime(driveItem.modifiedTime(), st->st_mtim);
		st->st_atim = st->st_mtim;
	} catch (const std::exception &e) {
		LOG_ERROR("an exception was caught: " << e.what());
		err = -EIO;
	} catch (...) {
		LOG_ERROR("an unknown exception was caught");
		err = -EIO;
	}

	return err;
}

int CFuse::fuseOpen(const char *path, struct fuse_file_info * /*fileInfo*/)
{
	int err = 0;
	COneDrive *oneDrive = static_cast<COneDrive *>(fuse_get_context()->private_data);

	try {
		CDriveItem driveItem = oneDrive->itemFromPath(path);

		if (driveItem.type() == CDriveItem::DRIVE_ITEM_UNKNOWN)
			return -ENOENT;
	} catch (const std::exception &e) {
		LOG_ERROR("an exception was caught: " << e.what());
		err = -ENOENT;
	} catch (...) {
		LOG_ERROR("an unknown exception was caught");
		err = -ENOENT;
	}

	return err;
}

int CFuse::fuseRelease(const char * /*path*/, struct fuse_file_info * /*fileInfo*/)
{
	return -ENOSYS;
}

int CFuse::fuseReadDir(const char *path, void *buf, fuse_fill_dir_t fillDir,
		       off_t /*offset*/, struct fuse_file_info * /*fileInfo*/)
{
	int err = 0;
	COneDrive *oneDrive = static_cast<COneDrive *>(fuse_get_context()->private_data);

	try {
		CDriveItem driveItem = oneDrive->itemFromPath(path);

		if (driveItem.type() == CDriveItem::DRIVE_ITEM_UNKNOWN)
			return -ENOENT;

		std::list<CDriveItem> driveItems;

		oneDrive->listChildren(driveItem, driveItems);

		for (auto &&i : driveItems) {
			struct stat st{};

			st.st_uid = getuid();
			st.st_gid = getgid();
			st.st_nlink = 1;
			if (i.type() == CDriveItem::DRIVE_ITEM_FOLDER) {
				st.st_size = 4096;
				st.st_mode = S_IFDIR | S_IXUSR | S_IRUSR | S_IWUSR | S_IXGRP | S_IRGRP | S_IXOTH | S_IROTH;
			} else if (i.type() == CDriveItem::DRIVE_ITEM_FILE) {
				st.st_size = std::stoll(i.size());
				st.st_mode = S_IFREG | S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
			}
			oneDrive->driveItemTime(i.createTime(), st.st_ctim);
			oneDrive->driveItemTime(i.modifiedTime(), st.st_mtim);
			st.st_atim = st.st_mtim;

			std::string fpath(path);
			if (fpath == "/")
				oneDrive->cacheLocked(fpath + i.name(), i);
			else
				oneDrive->cacheLocked(fpath + "/" + i.name(), i);

			if (fillDir(buf, i.name().c_str(), &st, 0))
				break;
		}
	} catch (const std::exception &e) {
		LOG_ERROR("an exception was caught: " << e.what());
		err = -EIO;
	} catch (...) {
		LOG_ERROR("an unknown exception was caught");
		err = -EIO;
	}

	return err;
}

int CFuse::fuseListXAttr(const char *path, char *buf, size_t size)
{
	int err = 0;
	COneDrive *oneDrive = static_cast<COneDrive *>(fuse_get_context()->private_data);

	try {
		CDriveItem driveItem = oneDrive->itemFromPath(path);

		if (driveItem.type() == CDriveItem::DRIVE_ITEM_UNKNOWN)
			return -ENOENT;

		if (driveItem.type() != CDriveItem::DRIVE_ITEM_FILE)
			return 0;

		if (driveItem.hash().empty())
			return 0;

		if (!buf)
			err = sizeof(userHashAttr);
		else if (size < sizeof(userHashAttr))
			err = -ERANGE;
		else {
			memcpy(buf, userHashAttr, sizeof(userHashAttr));
			err = sizeof(userHashAttr);
		}
	} catch (const std::exception &e) {
		LOG_ERROR("an exception was caught: " << e.what());
		err = -EIO;
	} catch (...) {
		LOG_ERROR("an unknown exception was caught");
		err = -EIO;
	}

	return err;
}

int CFuse::fuseGetXAttr(const char *path, const char *name, char *buf, size_t size)
{
	int err = 0;
	COneDrive *oneDrive = static_cast<COneDrive *>(fuse_get_context()->private_data);

	try {
		if (strcmp(name, userHashAttr))
			return -ENODATA;

		CDriveItem driveItem = oneDrive->itemFromPath(path);

		if (driveItem.type() == CDriveItem::DRIVE_ITEM_UNKNOWN)
			return -ENOENT;

		if (driveItem.type() != CDriveItem::DRIVE_ITEM_FILE)
			return 0;

		if (driveItem.hash().empty())
			return 0;

		if (!buf)
			err = driveItem.hash().length();
		else
			err = snprintf(buf, size, "%s", driveItem.hash().c_str());
	} catch (const std::exception &e) {
		LOG_ERROR("an exception was caught: " << e.what());
		err = -EIO;
	} catch (...) {
		LOG_ERROR("an unknown exception was caught");
		err = -EIO;
	}

	return err;
}

int CFuse::fuseStatFs(const char *path, struct statvfs *st)
{
	int err = 0;
	COneDrive *oneDrive = static_cast<COneDrive *>(fuse_get_context()->private_data);

	try {
		CDrive drive = oneDrive->drive();

		std::memset(st, 0, sizeof(*st));

		st->f_bsize = 4096;
		st->f_frsize = 4096;
		st->f_blocks = std::stoll(drive.quota().total()) / st->f_frsize;
		st->f_bfree = (std::stoll(drive.quota().total()) - std::stoll(drive.quota().used())) / st->f_bsize;
		st->f_bavail = st->f_bfree;
		st->f_namemax = 1024;
	} catch (const std::exception &e) {
		LOG_ERROR("an exception was caught: " << e.what());
		err = -EIO;
	} catch (...) {
		LOG_ERROR("an unknown exception was caught");
		err = -EIO;
	}

	return err;
}

int CFuse::fuseRead(const char *path, char *buf, size_t size, off_t offset,
		    struct fuse_file_info *fileInfo)
{
	int err = 0;
	COneDrive *oneDrive = static_cast<COneDrive *>(fuse_get_context()->private_data);

	try {
		CDriveItem driveItem = oneDrive->itemFromPath(path);

		if (driveItem.type() == CDriveItem::DRIVE_ITEM_UNKNOWN)
			return -ENOENT;

		if (driveItem.type() != CDriveItem::DRIVE_ITEM_FILE)
			return -EISDIR;

		err = oneDrive->read(driveItem, buf, size, offset);
	} catch (const std::exception &e) {
		LOG_ERROR("an exception was caught: " << e.what());
		err = -EIO;
	} catch (...) {
		LOG_ERROR("an unknown exception was caught");
		err = -EIO;
	}

	return err;
}

} // namespace OneDrive
