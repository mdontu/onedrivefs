// SPDX-License-Identifier: GPL-2.0

#include <unistd.h>
#include <sys/types.h>
#include <cstring>
#include "fuse.h"
#include "log.h"

namespace {

} // anonymouse namespace

namespace OneDrive {

CFuse::CFuse()
{
	fuseOps_.init    = fuseInit;
	fuseOps_.destroy = fuseDestroy;
	fuseOps_.getattr = fuseGetAttr;
	fuseOps_.open    = fuseOpen;
	fuseOps_.release = fuseRelease;
	fuseOps_.readdir = fuseReadDir;
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

int CFuse::fuseOpen(const char * /*path*/, struct fuse_file_info * /*fileInfo*/)
{
	return -ENOSYS;
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

} // namespace OneDrive
