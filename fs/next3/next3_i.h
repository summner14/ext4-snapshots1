/*
 *  linux/fs/next3/next3_i.h
 *
 * Copyright (C) 1992, 1993, 1994, 1995
 * Remy Card (card@masi.ibp.fr)
 * Laboratoire MASI - Institut Blaise Pascal
 * Universite Pierre et Marie Curie (Paris VI)
 *
 *  from
 *
 *  linux/include/linux/minix_fs_i.h
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 */

#ifndef _LINUX_NEXT3_I
#define _LINUX_NEXT3_I

#include <linux/rwsem.h>
#include <linux/rbtree.h>
#include <linux/seqlock.h>
#include <linux/mutex.h>

/* data type for block offset of block group */
typedef int next3_grpblk_t;

/* data type for filesystem-wide blocks number */
typedef unsigned long next3_fsblk_t;

#define E3FSBLK "%lu"

struct next3_reserve_window {
	next3_fsblk_t	_rsv_start;	/* First byte reserved */
	next3_fsblk_t	_rsv_end;	/* Last byte reserved or 0 */
};

struct next3_reserve_window_node {
	struct rb_node		rsv_node;
	__u32			rsv_goal_size;
	__u32			rsv_alloc_hit;
	struct next3_reserve_window	rsv_window;
};

struct next3_block_alloc_info {
	/* information about reservation window */
	struct next3_reserve_window_node	rsv_window_node;
	/*
	 * was i_next_alloc_block in next3_inode_info
	 * is the logical (file-relative) number of the
	 * most-recently-allocated block in this file.
	 * We use this for detecting linearly ascending allocation requests.
	 */
	__u32                   last_alloc_logical_block;
	/*
	 * Was i_next_alloc_goal in next3_inode_info
	 * is the *physical* companion to i_next_alloc_block.
	 * it the physical block number of the block which was most-recentl
	 * allocated to this file.  This give us the goal (target) for the next
	 * allocation when we detect linearly ascending requests.
	 */
	next3_fsblk_t		last_alloc_physical_block;
};

#define rsv_start rsv_window._rsv_start
#define rsv_end rsv_window._rsv_end

/*
 * third extended file system inode data in memory
 */
struct next3_inode_info {
#ifdef CONFIG_NEXT3_FS_SNAPSHOT_FILE_HUGE
	__le32	i_data[NEXT3_SNAPSHOT_N_BLOCKS]; /* unconverted */
#else
	__le32	i_data[15];	/* unconverted */
#endif
	__u32	i_flags;
#ifdef NEXT3_FRAGMENTS
	__u32	i_faddr;
	__u8	i_frag_no;
	__u8	i_frag_size;
#endif
	next3_fsblk_t	i_file_acl;
	__u32	i_dir_acl;
	__u32	i_dtime;

	/*
	 * i_block_group is the number of the block group which contains
	 * this file's inode.  Constant across the lifetime of the inode,
	 * it is ued for making block allocation decisions - we try to
	 * place a file's data blocks near its inode block, and new inodes
	 * near to their parent directory's inode.
	 */
	__u32	i_block_group;
	unsigned long	i_state_flags;	/* Dynamic state flags for next3 */

	/* block reservation info */
	struct next3_block_alloc_info *i_block_alloc_info;

	__u32	i_dir_start_lookup;
#ifdef CONFIG_NEXT3_FS_XATTR
	/*
	 * Extended attributes can be read independently of the main file
	 * data. Taking i_mutex even when reading would cause contention
	 * between readers of EAs and writers of regular file data, so
	 * instead we synchronize on xattr_sem when reading or changing
	 * EAs.
	 */
	struct rw_semaphore xattr_sem;
#endif

	struct list_head i_orphan;	/* unlinked but open inodes */

#ifdef CONFIG_NEXT3_FS_SNAPSHOT_FILE
#define i_snaplist i_orphan
	/*
	 * In-memory snapshot list overrides i_orphan to link snapshot inodes,
	 * but unlike the real orphan list, the next snapshot inode number
	 * is stored in i_next_snapshot_ino and not in i_dtime
	 */
	__u32	i_next_snapshot_ino;

#endif
	/*
	 * i_disksize keeps track of what the inode size is ON DISK, not
	 * in memory.  During truncate, i_size is set to the new size by
	 * the VFS prior to calling next3_truncate(), but the filesystem won't
	 * set i_disksize to 0 until the truncate is actually under way.
	 *
	 * The intent is that i_disksize always represents the blocks which
	 * are used by this file.  This allows recovery to restart truncate
	 * on orphans if we crash during truncate.  We actually write i_disksize
	 * into the on-disk inode when writing inodes out, instead of i_size.
	 *
	 * The only time when i_disksize and i_size may be different is when
	 * a truncate is in progress.  The only things which change i_disksize
	 * are next3_get_block (growth) and next3_truncate (shrinkth).
	 */
	loff_t	i_disksize;

	/* on-disk additional length */
	__u16 i_extra_isize;

	/*
	 * truncate_mutex is for serialising next3_truncate() against
	 * next3_getblock().  In the 2.4 ext2 design, great chunks of inode's
	 * data tree are chopped off during truncate. We can't do that in
	 * next3 because whenever we perform intermediate commits during
	 * truncate, the inode and all the metadata blocks *must* be in a
	 * consistent state which allows truncation of the orphans to restart
	 * during recovery.  Hence we must fix the get_block-vs-truncate race
	 * by other means, so we have truncate_mutex.
	 */
	struct mutex truncate_mutex;

	/*
	 * Transactions that contain inode's metadata needed to complete
	 * fsync and fdatasync, respectively.
	 */
	atomic_t i_sync_tid;
	atomic_t i_datasync_tid;

	struct inode vfs_inode;
};

#endif	/* _LINUX_NEXT3_I */
