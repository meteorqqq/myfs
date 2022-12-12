/**
 *  仿照simplefs，定义各结构体，在sfs基础上添加数据位图。
 */

#ifndef _TYPES_H_
#define _TYPES_H_

/******************************************************************************
* SECTION: Type def
*******************************************************************************/
typedef int boolean;
typedef uint16_t flag16;

typedef enum myfs_file_type {
    MYFS_REG_FILE,
    MYFS_DIR,
    MYFS_SYM_LINK
} MYFS_FILE_TYPE;

/******************************************************************************
* SECTION: Macro
*******************************************************************************/
#define TRUE                    1
#define FALSE                   0
#define UINT32_BITS             32
#define UINT8_BITS              8

#define MYFS_MAGIC_NUM           0x52415453
#define MYFS_SUPER_OFS           0
#define MYFS_ROOT_INO            0

#define MYFS_ERROR_NONE          0
#define MYFS_ERROR_ACCESS        EACCES
#define MYFS_ERROR_SEEK          ESPIPE
#define MYFS_ERROR_ISDIR         EISDIR
#define MYFS_ERROR_NOSPACE       ENOSPC
#define MYFS_ERROR_EXISTS        EEXIST
#define MYFS_ERROR_NOTFOUND      ENOENT
#define MYFS_ERROR_UNSUPPORTED   ENXIO
#define MYFS_ERROR_IO            EIO     /* Error Input/Output */
#define MYFS_ERROR_INVAL         EINVAL  /* Invalid Args */

#define MYFS_MAX_FILE_NAME       128
#define MYFS_INODE_PER_FILE      1
#define MYFS_DATA_PER_FILE       4
#define MYFS_DEFAULT_PERM        0777

#define MYFS_IOC_MAGIC           'S'
#define MYFS_IOC_SEEK            _IO(SFS_IOC_MAGIC, 0)
#define MYFS_FLAG_BUF_DIRTY      0x1
#define MYFS_FLAG_BUF_OCCUPY     0x2

/******************************************************************************
* SECTION: Macro Function
*******************************************************************************/
#define MYFS_IO_SZ()                     (myfs_super.sz_io)
#define MYFS_BLK_SZ()                    (myfs_super.sz_blk)
#define MYFS_DISK_SZ()                   (myfs_super.sz_disk)
#define MYFS_DRIVER()                    (myfs_super.driver_fd)

#define MYFS_ROUND_DOWN(value, round)    (value % round == 0 ? value : (value / round) * round)
#define MYFS_ROUND_UP(value, round)      (value % round == 0 ? value : (value / round + 1) * round)

#define MYFS_BLKS_SZ(blks)               (blks * MYFS_BLK_SZ())
#define MYFS_ASSIGN_FNAME(pmyfs_dentry, _fname)\
                                     memcpy(pmyfs_dentry->fname, _fname, strlen(_fname))
#define MYFS_INO_OFS(ino)                (myfs_super.inode_offset + (ino) * MYFS_BLKS_SZ(1))
#define MYFS_DATA_OFS(ino)               (myfs_super.data_offset + (ino) * MYFS_BLKS_SZ(1))

#define MYFS_IS_DIR(pinode)              (pinode->dentry->ftype == MYFS_DIR)
#define MYFS_IS_REG(pinode)              (pinode->dentry->ftype == MYFS_REG_FILE)
#define MYFS_IS_SYM_LINK(pinode)         (pinode->dentry->ftype == MYFS_SYM_LINK)

/******************************************************************************
* SECTION: FS Specific Structure - In memory structure
*******************************************************************************/
struct myfs_dentry;
struct myfs_inode;
struct myfs_super;

struct custom_options {
    const char *device;
};

struct myfs_super {
    int driver_fd;

    int sz_io;
    int sz_disk;
    int sz_blk;
    int sz_usage;

    int max_ino;
    uint8_t *map_inode;
    int map_inode_blks;
    int map_inode_offset;

    uint8_t *map_data;
    int map_data_blks;
    int map_data_offset;

    int inode_offset;
    int data_offset;

    boolean is_mounted;

    struct myfs_dentry *root_dentry;
};

struct myfs_inode {
    int ino;                           /* 在inode位图中的下标 */
    int size;                          /* 文件已占用空间 */
    char target_path[MYFS_MAX_FILE_NAME];/* store target path when it is a symlink */
    int dir_cnt;
    struct myfs_dentry *dentry;                        /* 指向该inode的dentry */
    struct myfs_dentry *dentrys;                       /* 所有目录项 */
    uint8_t *data;
    int block_pointer[MYFS_DATA_PER_FILE];
};

struct myfs_dentry {
    char fname[MYFS_MAX_FILE_NAME];
    struct myfs_dentry *parent;                        /* 父亲Inode的dentry */
    struct myfs_dentry *brother;                       /* 兄弟 */
    int ino;
    struct myfs_inode *inode;                         /* 指向inode */
    MYFS_FILE_TYPE ftype;
};

static inline struct myfs_dentry *new_dentry(char *fname, MYFS_FILE_TYPE ftype) {
    struct myfs_dentry *dentry = (struct myfs_dentry *) malloc(sizeof(struct myfs_dentry));
    memset(dentry, 0, sizeof(struct myfs_dentry));
    MYFS_ASSIGN_FNAME(dentry, fname);
    dentry->ftype = ftype;
    dentry->ino = -1;
    dentry->inode = NULL;
    dentry->parent = NULL;
    dentry->brother = NULL;
}

/******************************************************************************
* SECTION: FS Specific Structure - Disk structure
*******************************************************************************/
struct myfs_super_d {
    uint32_t magic_num;
    int sz_usage;

    int max_ino;
    int map_inode_blks;
    int map_inode_offset;

    int map_data_blks;
    int map_data_offset;

    int inode_offset;
    int data_offset;
};

struct myfs_inode_d {
    int ino;                           /* 在inode位图中的下标 */
    int size;                          /* 文件已占用空间 */
    char target_path[MYFS_MAX_FILE_NAME];/* store target path when it is a symlink */
    int dir_cnt;
    MYFS_FILE_TYPE ftype;
    int block_pointer[MYFS_DATA_PER_FILE];
};

struct myfs_dentry_d {
    char fname[MYFS_MAX_FILE_NAME];
    MYFS_FILE_TYPE ftype;
    int ino;                           /* 指向的ino号 */
    int valid;
};

#endif /* _TYPES_H_ */
