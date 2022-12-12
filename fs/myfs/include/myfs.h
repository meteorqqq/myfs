#ifndef _myfs_H_
#define _myfs_H_

#define FUSE_USE_VERSION 26

#include "stdio.h"
#include "stdlib.h"
#include <unistd.h>
#include "fcntl.h"
#include "string.h"
#include "fuse.h"
#include <stddef.h>
#include "ddriver.h"
#include "errno.h"
#include "types.h"

#define MYFS_MAGIC             /* Define by yourself */
#define MYFS_DEFAULT_PERM 0777 /* 全权限打开 */

/******************************************************************************
 * SECTION: macro debug
 *******************************************************************************/
#define MYFS_DBG(fmt, ...)                       \
    do                                           \
    {                                            \
        printf("MYFS_DBG: " fmt, ##__VA_ARGS__); \
    } while (0)

/******************************************************************************
 * SECTION: sfs_utils.c
 *******************************************************************************/
char *myfs_get_fname(const char *path);

int myfs_calc_lvl(const char *path);

int myfs_driver_read(int offset, uint8_t *out_content, int size);

int myfs_driver_write(int offset, uint8_t *in_content, int size);

int myfs_mount(struct custom_options options);

int myfs_umount(void);

int myfs_alloc_dentry(struct myfs_inode *inode, struct myfs_dentry *dentry);

struct myfs_inode *myfs_alloc_inode(struct myfs_dentry *dentry);

int myfs_sync_inode(struct myfs_inode *inode);

struct myfs_inode *myfs_read_inode(struct myfs_dentry *dentry, int ino);

struct myfs_dentry *myfs_get_dentry(struct myfs_inode *inode, int dir);

struct myfs_dentry *myfs_lookup(const char *path, boolean *is_find, boolean *is_root);

/******************************************************************************
 * SECTION: myfs.c
 *******************************************************************************/
void *myfs_init(struct fuse_conn_info *);

void myfs_destroy(void *);

int myfs_mkdir(const char *, mode_t);

int myfs_getattr(const char *, struct stat *);

int myfs_readdir(const char *, void *, fuse_fill_dir_t, off_t, struct fuse_file_info *);

int myfs_mknod(const char *, mode_t, dev_t);

int myfs_write(const char *, const char *, size_t, off_t, struct fuse_file_info *);

int myfs_read(const char *, char *, size_t, off_t, struct fuse_file_info *);

int myfs_access(const char *, int);

int myfs_unlink(const char *);

int myfs_rmdir(const char *);

int myfs_rename(const char *, const char *);

int myfs_utimens(const char *path, const struct timespec tv[2]);

int myfs_truncate(const char *, off_t);

int myfs_open(const char *, struct fuse_file_info *);

int myfs_opendir(const char *, struct fuse_file_info *);

/******************************************************************************
 * SECTION: myfs_debug.c
 *******************************************************************************/
void myfs_dump_map_inode(void);

void myfs_dump_map_data(void);

#endif /* _myfs_H_ */
