// SPDX-License-Identifier: GPL-2.0

#ifndef __FUSE_H_INCLUDED__
#define __FUSE_H_INCLUDED__

#define FUSE_USE_VERSION 31

#include <fuse.h>
#include "appconfig.h"
#include "onedrive.h"

namespace OneDrive {

class CFuse {
public:
	CFuse();

	~CFuse();

	CFuse(const CFuse &) = delete;
	CFuse & operator=(const CFuse &) = delete;

	int init(int argc, const char *argv[]);

private:
	struct fuse_operations fuseOps_{};

	static void *fuseInit(struct fuse_conn_info *conn);

	static void fuseDestroy(void *);

	static int fuseGetAttr(const char *path, struct stat *st);

	static int fuseOpen(const char *path, struct fuse_file_info *fileInfo);

	static int fuseRelease(const char *path, struct fuse_file_info *fileInfo);

	static int fuseReadDir(const char *path, void *buf, fuse_fill_dir_t fillDir,
			       off_t offset, struct fuse_file_info *);

	static int fuseListXAttr(const char *path, char *buf, size_t size);

	static int fuseGetXAttr(const char *path, const char *name, char *buf, size_t size);

	static int fuseStatFs(const char *path, struct statvfs *st);

	static int fuseRead(const char *path, char *buf, size_t size, off_t offset,
			    struct fuse_file_info *fileInfo);

	static int fuseUnlink(const char *path);
};

} // namespace OneDrive

#endif // __FUSE_H_INCLUDED__
