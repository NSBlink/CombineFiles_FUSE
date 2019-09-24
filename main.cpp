#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>
#include <vector>
#include <fstream>
#include <unistd.h>

#ifdef __FreeBSD__
#include <sys/socket.h>
#include <sys/un.h>
#endif
#ifdef HAVE_SETXATTR
#include <sys/xattr.h>
#endif
#include "fuse.h"

std::vector<std::pair<std::string, size_t> >allFiles;
size_t mergedFileSize = 0;
std::string mergedFileName;

size_t getRealFileSize(const std::string& filename) {
    struct stat stat_buf;
    int rc = stat(filename.c_str(), &stat_buf);
    return rc == 0 ? stat_buf.st_size : -1;
}

size_t getMergedFileSize() {
    return mergedFileSize;
}

static int merge_access(const char *path, int mask) {
    int res;
    for (auto&& file: allFiles) {
        res = access(file.first.c_str(), mask);
        if (res == -1) {
            return -errno;
        }
    }
    return 0;
}
static int merge_open(const char *path, struct fuse_file_info *fi) {
    std::cout << "opening" << std::endl;
    /*
    int res, fd;
    fd = open(allFiles[0].first.c_str(), O_RDONLY);
    res = fd;
    close(fd);
     */
    return 0;
}

static int merge_read(const char *path, char *buf, size_t size, off_t offset,
                      struct fuse_file_info *fi) {
    int fd;
    int res;
    size_t restSize = size;
    off_t currentOffset = offset;
    char* currentBuf = buf;
    size_t totalSizeOffset = 0;
    std::cout << restSize << " " << currentOffset << std::endl;
    size_t actuallyReaded = 0;
    for(auto&& file: allFiles) {
        if (currentOffset >= totalSizeOffset && currentOffset < totalSizeOffset + file.second) {
            size_t shouldReadSize = 0;
            if (totalSizeOffset + file.second - currentOffset >= restSize) {
                shouldReadSize = restSize;
            } else {
                shouldReadSize = totalSizeOffset + file.second - currentOffset;
            }
            fd = open(file.first.c_str(), O_RDONLY);
            res = pread(fd, currentBuf, shouldReadSize, currentOffset - totalSizeOffset);
            if (res == -1) {
                return -errno;
            }
            actuallyReaded += res;
            close(fd);
            restSize -= res;
            currentOffset += res;
            currentBuf += res;
            if (restSize == 0) {
                break;
            }
        }
        totalSizeOffset += file.second;
    }
    return actuallyReaded;
}

static int merge_release(const char *path, struct fuse_file_info *fi) {
    return 0;
}

static int merge_getattr(const char *path, struct stat *stbuf) {
    if(strcmp(path, "/") == 0) {
        stbuf->st_mode = 0755 | S_IFDIR;
        return 0;
    }
    else {
        stbuf->st_mode = 0755 | S_IFREG;
        stbuf->st_nlink = 3;
        stbuf->st_size = getMergedFileSize();
    }
    return 0;
}

static int merge_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                         off_t offset, struct fuse_file_info *fi) {
    filler(buf, ".", NULL, 0);
    filler(buf, "..", NULL, 0);
    filler(buf, mergedFileName.c_str(), NULL, 0);
    return 0;
}

static fuse_operations merge_oper = [] {
	fuse_operations ops{};
	ops.readdir = merge_readdir;
	ops.getattr = merge_getattr;
	ops.open    = merge_open;
	ops.read    = merge_read;
	ops.access  = merge_access;
	ops.release = merge_release;
	return ops;
}();



int main(int argc, char *argv[]) {
    std::cout << "Hello, World!" << std::endl;
    std::ifstream configFilein;
    configFilein.open("config.txt");
    std::getline(configFilein, mergedFileName);
    std::string tmpFileName;
    while (std::getline(configFilein, tmpFileName)) {
        size_t tmpFileSize = getRealFileSize(tmpFileName);
        if (tmpFileSize == -1) {
            continue;
        }
        allFiles.emplace_back(std::make_pair(tmpFileName, tmpFileSize));
        mergedFileSize += tmpFileSize;
    }
    if (allFiles.size() == 0) {
        perror("no available file.");
        return 0;
    }
    return fuse_main(argc, argv, &merge_oper, NULL);
}
