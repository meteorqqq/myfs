#include "../include/bitmap.h"
#include "../include/myfs.h"

extern struct myfs_super myfs_super;
extern struct custom_options myfs_options;

/**
 * @brief 获取文件名
 *
 * @param path
 * @return char*
 */
char *myfs_get_fname(const char *path) {
    char ch = '/';
    char *q = strrchr(path, ch) + 1;
    return q;
}

/**
 * @brief 计算路径的层级
 * exm: /av/c/d/f
 * -> lvl = 4
 * @param path
 * @return int
 */
int myfs_calc_lvl(const char *path) {
    char *str = path;
    int lvl = 0;
    if (strcmp(path, "/") == 0) {
        return lvl;
    }
    while (*str != NULL) {
        if (*str == '/') {
            lvl++;
        }
        str++;
    }
    return lvl;
}

/**
 * @brief 驱动读
 *
 * @param offset
 * @param out_content
 * @param size
 * @return int
 */
int myfs_driver_read(int offset, uint8_t *out_content, int size) {
    int offset_aligned = MYFS_ROUND_DOWN(offset, MYFS_IO_SZ());
    int bias = offset - offset_aligned;
    int size_aligned = MYFS_ROUND_UP((size + bias), MYFS_IO_SZ());
    uint8_t *temp_content = (uint8_t *) malloc(size_aligned);
    uint8_t *cur = temp_content;
    // lseek(MYFS_DRIVER(), offset_aligned, SEEK_SET);
    ddriver_seek(MYFS_DRIVER(), offset_aligned, SEEK_SET);
    while (size_aligned != 0) {
        // read(SFS_DRIVER(), cur, SFS_IO_SZ());
        // 每次设备只可以读写512B
        ddriver_read(MYFS_DRIVER(), cur, MYFS_IO_SZ());
        cur += MYFS_IO_SZ();
        size_aligned -= MYFS_IO_SZ();
    }
    memcpy(out_content, temp_content + bias, size);
    free(temp_content);
    return MYFS_ERROR_NONE;
}

/**
 * @brief 驱动写
 *
 * @param offset
 * @param in_content
 * @param size
 * @return int
 */
int myfs_driver_write(int offset, uint8_t *in_content, int size) {
    int offset_aligned = MYFS_ROUND_DOWN(offset, MYFS_IO_SZ());
    int bias = offset - offset_aligned;
    int size_aligned = MYFS_ROUND_UP((size + bias), MYFS_IO_SZ());
    uint8_t *temp_content = (uint8_t *) malloc(size_aligned);
    uint8_t *cur = temp_content;
    myfs_driver_read(offset_aligned, temp_content, size_aligned);
    memcpy(temp_content + bias, in_content, size);

    // lseek(SFS_DRIVER(), offset_aligned, SEEK_SET);
    ddriver_seek(MYFS_DRIVER(), offset_aligned, SEEK_SET);
    while (size_aligned != 0) {
        // write(SFS_DRIVER(), cur, SFS_IO_SZ());
        // 每次设备只可以读写512B
        ddriver_write(MYFS_DRIVER(), cur, MYFS_IO_SZ());
        cur += MYFS_IO_SZ();
        size_aligned -= MYFS_IO_SZ();
    }

    free(temp_content);
    return MYFS_ERROR_NONE;
}

/**
 * @brief 为一个inode分配dentry，采用头插法
 *
 * @param inode
 * @param dentry
 * @return int
 */
int myfs_alloc_dentry(struct myfs_inode *inode, struct myfs_dentry *dentry) {
    if (inode->dentrys == NULL) {
        inode->dentrys = dentry;
    } else {
        dentry->brother = inode->dentrys;
        inode->dentrys = dentry;
    }
    inode->dir_cnt++;
    return inode->dir_cnt;
}

/**
 * @brief 将dentry从inode的dentrys中取出
 *
 * @param inode
 * @param dentry
 * @return int
 */
int myfs_drop_dentry(struct myfs_inode *inode, struct myfs_dentry *dentry) {
    boolean is_find = FALSE;
    struct myfs_dentry *dentry_cursor;
    dentry_cursor = inode->dentrys;

    if (dentry_cursor == dentry) {
        inode->dentrys = dentry->brother;
        is_find = TRUE;
    } else {
        while (dentry_cursor) {
            if (dentry_cursor->brother == dentry) {
                dentry_cursor->brother = dentry->brother;
                is_find = TRUE;
                break;
            }
            dentry_cursor = dentry_cursor->brother;
        }
    }
    if (!is_find) {
        return -MYFS_ERROR_NOTFOUND;
    }
    inode->dir_cnt--;
    return inode->dir_cnt;
}

// The upper same as simplefs

/**
 * @brief 分配一个inode，占用位图
 *
 * @param dentry 该dentry指向分配的inode
 * @return myfs_inode
 */
struct myfs_inode *myfs_alloc_inode(struct myfs_dentry *dentry) {
    struct myfs_inode *inode;
    int ino_curse = get_first_unset_bit(myfs_super.map_inode, MYFS_BLKS_SZ(myfs_super.map_inode_blks));
    if (ino_curse == -1) {
        return -MYFS_ERROR_NOSPACE;
    }
    set_bit(&myfs_super.map_inode, ino_curse);
    inode = (struct myfs_inode *) malloc(sizeof(struct myfs_inode));
    for (int i = 0; i < MYFS_DATA_PER_FILE; i++) {
        int data_curse = get_first_unset_bit(myfs_super.map_data, MYFS_BLKS_SZ(myfs_super.map_data_blks));
        if (data_curse == -1) {
            return -MYFS_ERROR_NOSPACE;
        }
        set_bit(&myfs_super.map_data, data_curse);
        inode->block_pointer[i] = data_curse;
    }
    inode->ino = ino_curse;
    inode->size = 0;
    // dentry指向inode
    dentry->inode = inode;
    dentry->ino = ino_curse;
    // inode指回dentry
    inode->dentry = dentry;
    inode->dir_cnt = 0;
    inode->dentrys = NULL;

    if (MYFS_IS_REG(inode)) {
        inode->data = (uint8_t *) malloc(MYFS_BLKS_SZ(MYFS_DATA_PER_FILE));
    }

    return inode;
}

/**
 * @brief 将内存inode及其下方结构全部刷回磁盘
 *
 * @param inode
 * @return int
 */
int myfs_sync_inode(struct myfs_inode *inode) {
    struct myfs_inode_d inode_d;
    struct myfs_dentry *dentry_cursor;
    struct myfs_dentry_d dentry_d;
    int ino = inode->ino;
    inode_d.ino = ino;
    inode_d.size = inode->size;
    memcpy(inode_d.target_path, inode->target_path, MYFS_MAX_FILE_NAME);
    inode_d.ftype = inode->dentry->ftype;
    inode_d.dir_cnt = inode->dir_cnt;
    int offset;
    for (int i = 0; i < MYFS_DATA_PER_FILE; i++) {
        inode_d.block_pointer[i] = inode->block_pointer[i];
    }
    // 写入
    if (myfs_driver_write(MYFS_INO_OFS(ino), (uint8_t *) &inode_d, sizeof(struct myfs_inode_d)) != MYFS_ERROR_NONE) {
        MYFS_DBG("[%s] io error\n", __func__);
        return -MYFS_ERROR_IO;
    }
    if (MYFS_IS_DIR(inode)) {
        int index = 0;
        dentry_cursor = inode->dentrys;
        offset = MYFS_DATA_OFS(inode->block_pointer[index]);
        while (dentry_cursor != NULL) {
            memcpy(dentry_d.fname, dentry_cursor->fname, MYFS_MAX_FILE_NAME);
            dentry_d.ftype = dentry_cursor->ftype;
            dentry_d.ino = dentry_cursor->ino;

            if (myfs_driver_write(offset, (uint8_t *) &dentry_d, sizeof(struct myfs_dentry_d)) != MYFS_ERROR_NONE) {
                MYFS_DBG("[%s] io error\n", __func__);
                return -MYFS_ERROR_IO;
            }

            // 偏移量超过块大小，则在新数据块记录数据
            offset += sizeof(struct myfs_dentry_d);
            if ((offset >= MYFS_DATA_OFS(inode->block_pointer[index]) + MYFS_BLK_SZ())) {
                if (index < MYFS_DATA_PER_FILE - 1) {
                    offset = MYFS_DATA_OFS(inode->block_pointer[index++]);
                } else {
                    return -MYFS_ERROR_NOSPACE;
                }
            }

            if (dentry_cursor->inode != NULL) {
                myfs_sync_inode(dentry_cursor->inode);
            }

            dentry_cursor = dentry_cursor->brother;
        }
    } else if (MYFS_IS_REG(inode)) {
        for (int i = 0; i < MYFS_DATA_PER_FILE; i++) {
            // 内存中数据连续
            if (myfs_driver_write(MYFS_DATA_OFS(inode->block_pointer[i]), (uint8_t *) (inode->data + i * MYFS_BLK_SZ()),
                                  MYFS_BLK_SZ()) != MYFS_ERROR_NONE) {
                MYFS_DBG("[%s] io error\n", __func__);
                return -MYFS_ERROR_IO;
            }
        }
    }
}

/**
 * @brief 删除内存中的一个inode， 暂时不释放
 * Case 1: Reg File
 *
 *                  Inode
 *                /      \
 *            Dentry -> Dentry (Reg Dentry)
 *                       |
 *                      Inode  (Reg File)
 *
 *  1) Step 1. Erase Bitmap
 *  2) Step 2. Free Inode                      (Function of myfs_drop_inode)
 * ------------------------------------------------------------------------
 *  3) *Step 3. Free Dentry belonging to Inode (Outsider)
 * ========================================================================
 * Case 2: Dir
 *                  Inode
 *                /      \
 *            Dentry -> Dentry (Dir Dentry)
 *                       |
 *                      Inode  (Dir)
 *                    /     \
 *                Dentry -> Dentry
 *
 *   Recursive
 * @param inode
 * @return int
 */
int myfs_drop_inode(struct myfs_inode *inode) {
    struct myfs_dentry *dentry_cursor;
    struct myfs_dentry *dentry_to_free;
    struct myfs_inode *inode_cursor;

    if (inode == myfs_super.root_dentry->inode) {
        return MYFS_ERROR_INVAL;
    }

    if (MYFS_IS_DIR(inode)) {
        dentry_cursor = inode->dentrys;
        clear_bit(&myfs_super.map_inode, inode->ino);

        while (dentry_cursor) {
            inode_cursor = dentry_cursor->inode;
            myfs_drop_inode(inode_cursor);
            myfs_drop_dentry(inode, dentry_cursor);

            for (int i = 0; i < MYFS_DATA_PER_FILE; i++) {
                clear_bit(&myfs_super.map_data, inode->block_pointer[i]);
            }

            dentry_to_free = dentry_cursor;
            dentry_cursor = dentry_cursor->brother;
            free(dentry_to_free);
        }
    } else if (MYFS_IS_REG(inode) || MYFS_IS_SYM_LINK(inode)) {
        clear_bit(&myfs_super.map_inode, inode->ino);
        if (inode->data) {
            for (int i = 0; i < MYFS_DATA_PER_FILE; i++) {
                clear_bit(&myfs_super.map_data, inode->block_pointer[i]);
            }
            free(inode->data);
        }
        free(inode);
    }
    return MYFS_ERROR_NONE;
}

/**
 * @brief
 *
 * @param dentry dentry指向ino，读取该inode
 * @param ino inode唯一编号
 * @return struct myfs_inode*
 */
struct myfs_inode *myfs_read_inode(struct myfs_dentry *dentry, int ino) {
    struct myfs_inode *inode = (struct myfs_inode *) malloc(sizeof(struct myfs_inode));
    struct myfs_inode_d inode_d;
    struct myfs_dentry *sub_dentry;
    struct myfs_dentry_d dentry_d;
    int dir_cnt = 0, i;
    int offset;

    if (myfs_driver_read(MYFS_INO_OFS(ino), (uint8_t *) &inode_d, sizeof(struct myfs_inode_d)) != MYFS_ERROR_NONE) {
        MYFS_DBG("[%s] io error\n", __func__);
        return NULL;
    }

    inode->dir_cnt = 0;
    inode->ino = inode_d.ino;
    inode->size = inode_d.size;
    memcpy(inode->target_path, inode_d.target_path, MYFS_MAX_FILE_NAME);
    inode->dentry = dentry;
    inode->dentrys = NULL;
    for (int j = 0; j < MYFS_DATA_PER_FILE; j++) {
        inode->block_pointer[j] = inode_d.block_pointer[j];
    }
    if (MYFS_IS_DIR(inode)) {
        dir_cnt = inode_d.dir_cnt;
        int index = 0;
        offset = MYFS_DATA_OFS(inode->block_pointer[index]);
        for (i = 0; i < dir_cnt; i++) {
            if (myfs_driver_read(MYFS_DATA_OFS(ino) + i * sizeof(struct myfs_dentry_d), (uint8_t *) &dentry_d,
                                 sizeof(struct myfs_dentry_d)) != MYFS_ERROR_NONE) {
                MYFS_DBG("[%s] io error\n", __func__);
                return NULL;
            }

            // 偏移量超过块大小，则在新数据块记录数据
            offset += sizeof(struct myfs_dentry_d);
            if ((offset >= MYFS_DATA_OFS(inode->block_pointer[index]) + MYFS_BLK_SZ())) {
                if (index < MYFS_DATA_PER_FILE - 1) {
                    offset = MYFS_DATA_OFS(inode->block_pointer[index++]);
                } else {
                    return -MYFS_ERROR_NOSPACE;
                }
            }

            sub_dentry = new_dentry(dentry_d.fname, dentry_d.ftype);
            sub_dentry->parent = inode->dentry;
            sub_dentry->ino = dentry_d.ino;
            myfs_alloc_dentry(inode, sub_dentry);
        }
    } else if (MYFS_IS_REG(inode)) {
        inode->data = (uint8_t *) malloc(MYFS_BLKS_SZ(MYFS_DATA_PER_FILE));
        for (int j = 0; j < MYFS_DATA_PER_FILE; j++) {
            // 内存中数据连续
            if (myfs_driver_read(MYFS_DATA_OFS(inode->block_pointer[j]), (uint8_t *) (inode->data + j * MYFS_BLK_SZ()),
                                 MYFS_BLK_SZ()) != MYFS_ERROR_NONE) {
                MYFS_DBG("[%s] io error\n", __func__);
                return -MYFS_ERROR_IO;
            }
        }
    }
    return inode;
}

/**
 * @brief
 *
 * @param inode
 * @param dir [0...]
 * @return struct myfs_dentry*
 */
struct myfs_dentry *myfs_get_dentry(struct myfs_inode *inode, int dir) {
    struct myfs_dentry *dentry_cursor = inode->dentrys;
    int cnt = 0;
    while (dentry_cursor) {
        if (dir == cnt) {
            return dentry_cursor;
        }
        cnt++;
        dentry_cursor = dentry_cursor->brother;
    }
    return NULL;
}

/**
 * @brief
 * path: /qwe/ad  total_lvl = 2,
 *      1) find /'s inode       lvl = 1
 *      2) find qwe's dentry
 *      3) find qwe's inode     lvl = 2
 *      4) find ad's dentry
 *
 * path: /qwe     total_lvl = 1,
 *      1) find /'s inode       lvl = 1
 *      2) find qwe's dentry
 *
 * @param path
 * @return struct myfs_inode*
 */
struct myfs_dentry *myfs_lookup(const char *path, boolean *is_find, boolean *is_root) {
    struct myfs_dentry *dentry_cursor = myfs_super.root_dentry;
    struct myfs_dentry *dentry_ret = NULL;
    struct myfs_inode *inode;
    int total_lvl = myfs_calc_lvl(path);
    int lvl = 0;
    boolean is_hit;
    char *fname = NULL;
    char *path_cpy = (char *) malloc(sizeof(path));
    *is_root = FALSE;
    strcpy(path_cpy, path);

    if (total_lvl == 0) { /* 根目录 */
        *is_find = TRUE;
        *is_root = TRUE;
        dentry_ret = myfs_super.root_dentry;
    }
    fname = strtok(path_cpy, "/");
    while (fname) {
        lvl++;
        if (dentry_cursor->inode == NULL) { /* Cache机制 */
            myfs_read_inode(dentry_cursor, dentry_cursor->ino);
        }

        inode = dentry_cursor->inode;

        if (MYFS_IS_REG(inode) && lvl < total_lvl) {
            MYFS_DBG("[%s] not a dir\n", __func__);
            dentry_ret = inode->dentry;
            break;
        }
        if (MYFS_IS_DIR(inode)) {
            dentry_cursor = inode->dentrys;
            is_hit = FALSE;

            while (dentry_cursor) {
                if (memcmp(dentry_cursor->fname, fname, strlen(fname)) == 0) {
                    is_hit = TRUE;
                    break;
                }
                dentry_cursor = dentry_cursor->brother;
            }

            if (!is_hit) {
                *is_find = FALSE;
                MYFS_DBG("[%s] not found %s\n", __func__, fname);
                dentry_ret = inode->dentry;
                break;
            }

            if (is_hit && lvl == total_lvl) {
                *is_find = TRUE;
                dentry_ret = dentry_cursor;
                break;
            }
        }
        fname = strtok(NULL, "/");
    }

    if (dentry_ret->inode == NULL) {
        dentry_ret->inode = myfs_read_inode(dentry_ret, dentry_ret->ino);
    }

    return dentry_ret;
}

/**
 * @brief 挂载myfs, Layout 如下
 *
 * Layout
 * | Super(1) | Inode Map(1) | Data Map(1) | Inodes | Data |
 *  BLK_SZ = 2 * IO_SZ
 * 每个Inode占用1个Blk
 * @param options
 * @return int
 */
int myfs_mount(struct custom_options options) {
    int ret = MYFS_ERROR_NONE;
    int driver_fd;
    struct myfs_super_d myfs_super_d;
    struct myfs_dentry *root_dentry;
    struct myfs_inode *root_inode;

    int inode_num;
    int inode_blks;
    int map_inode_blks;
    int map_data_blks;

    int super_blks;
    boolean is_init = FALSE;

    myfs_super.is_mounted = FALSE;

    driver_fd = ddriver_open(options.device);
    if (driver_fd < 0) {
        return driver_fd;
    }
    myfs_super.driver_fd = driver_fd;
    ddriver_ioctl(MYFS_DRIVER(), IOC_REQ_DEVICE_SIZE, &myfs_super.sz_disk);
    ddriver_ioctl(MYFS_DRIVER(), IOC_REQ_DEVICE_IO_SZ, &myfs_super.sz_io);
    myfs_super.sz_blk = 2 * myfs_super.sz_io;
    MYFS_DBG("sz_disk: %d, sz_io: %d\n", myfs_super.sz_disk, myfs_super.sz_io);
    root_dentry = new_dentry("/", MYFS_DIR);

    if (myfs_driver_read(MYFS_SUPER_OFS, (uint8_t *) (&myfs_super_d), sizeof(struct myfs_super_d)) != MYFS_ERROR_NONE) {
        return -MYFS_ERROR_IO;
    }

    if (myfs_super_d.magic_num != MYFS_MAGIC_NUM) {
        // | Super(1) | Inode Map(1) | Data Map(1) | Inodes(585) | Data(*) |
        super_blks = MYFS_ROUND_UP(sizeof(struct myfs_super_d), MYFS_BLK_SZ()) / MYFS_BLK_SZ();
        inode_num = MYFS_DISK_SZ() / ((MYFS_DATA_PER_FILE + MYFS_INODE_PER_FILE) * MYFS_BLK_SZ());
        map_inode_blks = MYFS_ROUND_UP(MYFS_ROUND_UP(inode_num, UINT32_BITS) / UINT8_BITS, MYFS_IO_SZ()) / MYFS_IO_SZ();
        map_data_blks =
                MYFS_ROUND_UP(MYFS_ROUND_UP(MYFS_DISK_SZ() / MYFS_BLK_SZ(), UINT32_BITS) / UINT8_BITS, MYFS_BLK_SZ()) /
                MYFS_BLK_SZ();

        myfs_super.max_ino = (inode_num - super_blks - map_data_blks - map_inode_blks);
        myfs_super_d.map_inode_offset = MYFS_SUPER_OFS + MYFS_BLKS_SZ(super_blks);
        myfs_super_d.map_data_offset = myfs_super_d.map_inode_offset + MYFS_BLKS_SZ(map_inode_blks);
        myfs_super_d.inode_offset = myfs_super_d.map_data_offset + MYFS_BLKS_SZ(map_data_blks);
        myfs_super_d.data_offset = myfs_super_d.inode_offset + MYFS_BLKS_SZ(myfs_super.max_ino);
        myfs_super_d.map_inode_blks = map_inode_blks;
        myfs_super_d.map_data_blks = map_data_blks;
        myfs_super_d.sz_usage = 0;
        MYFS_DBG("max_ino: %d\n",myfs_super.max_ino);
        MYFS_DBG("super_blks: %d, inode_num: %d\n", super_blks, inode_num);
        MYFS_DBG("map_inode_offset: %d, map_data_offset: %d\n", myfs_super_d.map_inode_offset, myfs_super_d.map_data_offset);
        MYFS_DBG("map_inode_blks: %d, map_data_blks: %d\n", myfs_super_d.map_inode_blks, myfs_super_d.map_data_blks);
        MYFS_DBG("inode_offset: %d, data_offset: %d\n", myfs_super_d.inode_offset, myfs_super_d.data_offset);
        is_init = TRUE;
    }
    myfs_super.sz_usage = myfs_super_d.sz_usage;
    myfs_super.map_inode = (uint8_t *) malloc(MYFS_BLKS_SZ(myfs_super_d.map_inode_blks));
    myfs_super.map_inode_blks = myfs_super_d.map_inode_blks;
    myfs_super.map_inode_offset = myfs_super_d.map_inode_offset;
    myfs_super.map_data = (uint8_t *) malloc(MYFS_BLKS_SZ(myfs_super_d.map_data_blks));
    myfs_super.map_data_blks = myfs_super_d.map_data_blks;
    myfs_super.map_data_offset = myfs_super_d.map_data_offset;

    myfs_super.inode_offset = myfs_super_d.inode_offset;
    myfs_super.data_offset = myfs_super_d.data_offset;

    // 从磁盘读数据和索引位图
    if (myfs_driver_read(myfs_super_d.map_inode_offset, (uint8_t *) (myfs_super.map_inode),
                         MYFS_BLKS_SZ(myfs_super_d.map_inode_blks)) != MYFS_ERROR_NONE) {
        return -MYFS_ERROR_IO;
    }

    if (myfs_driver_read(myfs_super_d.map_data_offset, (uint8_t *) (myfs_super.map_data),
                         MYFS_BLKS_SZ(myfs_super_d.map_data_blks)) != MYFS_ERROR_NONE) {
        return -MYFS_ERROR_IO;
    }

    if (is_init) {
        root_inode = myfs_alloc_inode(root_dentry);
        myfs_sync_inode(root_inode);
    }

    root_inode = myfs_read_inode(root_dentry, MYFS_ROOT_INO);
    root_dentry->inode = root_inode;
    myfs_super.root_dentry = root_dentry;
    myfs_super.is_mounted = TRUE;

    myfs_dump_map_inode();
    return ret;
}

/**
 * @brief
 *
 * @return int
 */
int myfs_umount() {
    struct myfs_super_d myfs_super_d;

    if (!myfs_super.is_mounted) {
        return MYFS_ERROR_NONE;
    }

    myfs_sync_inode(myfs_super.root_dentry->inode);

    myfs_super_d.magic_num = MYFS_MAGIC_NUM;
    myfs_super_d.map_inode_blks = myfs_super.map_inode_blks;
    myfs_super_d.map_inode_offset = myfs_super.map_inode_offset;
    myfs_super_d.map_data_blks = myfs_super.map_data_blks;
    myfs_super_d.map_data_offset = myfs_super.map_data_offset;
    myfs_super_d.inode_offset = myfs_super.inode_offset;
    myfs_super_d.data_offset = myfs_super.data_offset;
    myfs_super_d.sz_usage = myfs_super.sz_usage;
    // write back superblock
    if (myfs_driver_write(MYFS_SUPER_OFS, (uint8_t *) &myfs_super_d,
                          sizeof(struct myfs_super_d)) != MYFS_ERROR_NONE) {
        return -MYFS_ERROR_IO;
    }
    // write back map_inode
    if (myfs_driver_write(myfs_super_d.map_inode_offset, (uint8_t *) (myfs_super.map_inode),
                          MYFS_BLKS_SZ(myfs_super_d.map_inode_blks)) != MYFS_ERROR_NONE) {
        return -MYFS_ERROR_IO;
    }
    // write back map_data
    if (myfs_driver_write(myfs_super_d.map_data_offset, (uint8_t *) (myfs_super.map_data),
                          MYFS_BLKS_SZ(myfs_super_d.map_data_blks)) != MYFS_ERROR_NONE) {
        return -MYFS_ERROR_IO;
    }

    free(myfs_super.map_inode);
    free(myfs_super.map_data);
    ddriver_close(MYFS_DRIVER());

    return MYFS_ERROR_NONE;
}
