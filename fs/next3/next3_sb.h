/*
 *  linux/fs/next3/next3_sb.h
 *
 * Copyright (C) 1992, 1993, 1994, 1995
 * Remy Card (card@masi.ibp.fr)
 * Laboratoire MASI - Institut Blaise Pascal
 * Universite Pierre et Marie Curie (Paris VI)
 *
 *  from
 *
 *  linux/include/linux/minix_fs_sb.h
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 *
 *  Added snapshot support, Amir Goldstein <amir73il@users.sf.net>, 2008
 */

#ifndef _LINUX_NEXT3_SB
#define _LINUX_NEXT3_SB

#ifdef __KERNEL__
#include <linux/timer.h>
#include <linux/wait.h>
#include <linux/blockgroup_lock.h>
#include <linux/percpu_counter.h>
#ifdef CONFIG_NEXT3_FS_SNAPSHOT_FILE
#include <linux/mutex.h>
#endif
#endif
#include <linux/rbtree.h>

#ifdef CONFIG_NEXT3_FS_SNAPSHOT_FILE
/*
 * third extended-fs per-block-group data in memory
 */
struct next3_group_info {
	/*
	 * Fast cache for location of exclude/COW bitmap blocks.
	 * Exclude bitmap blocks are allocated offline by mke2fs/tune2fs.
	 * Location of exclude bitmap blocks is read from exclude inode to
	 * initialize bg_exclude_bitmap on mount time.
	 * bg_cow_bitmap is reset to zero on mount time and on every snapshot
	 * take and initialized lazily on first block group write access.
	 * bg_cow_bitmap is protected by sb_bgl_lock().
	 */
	unsigned long bg_exclude_bitmap;/* Exclude bitmap cache */
	unsigned long bg_cow_bitmap;	/* COW bitmap cache */
};

#endif
/*
 * third extended-fs super-block data in memory
 */
struct next3_sb_info {
	unsigned long s_frag_size;	/* Size of a fragment in bytes */
	unsigned long s_frags_per_block;/* Number of fragments per block */
	unsigned long s_inodes_per_block;/* Number of inodes per block */
	unsigned long s_frags_per_group;/* Number of fragments in a group */
	unsigned long s_blocks_per_group;/* Number of blocks in a group */
	unsigned long s_inodes_per_group;/* Number of inodes in a group */
	unsigned long s_itb_per_group;	/* Number of inode table blocks per group */
	unsigned long s_gdb_count;	/* Number of group descriptor blocks */
	unsigned long s_desc_per_block;	/* Number of group descriptors per block */
	unsigned long s_groups_count;	/* Number of groups in the fs */
	unsigned long s_overhead_last;  /* Last calculated overhead */
	unsigned long s_blocks_last;    /* Last seen block count */
	struct buffer_head * s_sbh;	/* Buffer containing the super block */
	struct next3_super_block * s_es;	/* Pointer to the super block in the buffer */
	struct buffer_head ** s_group_desc;
	unsigned long  s_mount_opt;
	next3_fsblk_t s_sb_block;
	uid_t s_resuid;
	gid_t s_resgid;
	unsigned short s_mount_state;
	unsigned short s_pad;
	int s_addr_per_block_bits;
	int s_desc_per_block_bits;
	int s_inode_size;
	int s_first_ino;
	spinlock_t s_next_gen_lock;
	u32 s_next_generation;
	u32 s_hash_seed[4];
	int s_def_hash_version;
	int s_hash_unsigned;	/* 3 if hash should be signed, 0 if not */
	struct percpu_counter s_freeblocks_counter;
	struct percpu_counter s_freeinodes_counter;
	struct percpu_counter s_dirs_counter;
	struct blockgroup_lock *s_blockgroup_lock;

	/* root of the per fs reservation window tree */
	spinlock_t s_rsv_window_lock;
	struct rb_root s_rsv_window_root;
	struct next3_reserve_window_node s_rsv_window_head;

	/* Journaling */
	struct inode * s_journal_inode;
	struct journal_s * s_journal;
	struct list_head s_orphan;
#ifdef CONFIG_NEXT3_FS_SNAPSHOT_FILE
	struct next3_group_info *s_group_info;	/* [ sb_bgl_lock ] */
	struct mutex s_snapshot_mutex;		/* protects 2 fields below: */
	struct inode *s_active_snapshot;	/* [ s_snapshot_mutex ] */
#ifdef CONFIG_NEXT3_FS_SNAPSHOT_LIST
	struct list_head s_snapshot_list;	/* [ s_snapshot_mutex ] */
#endif
#endif
	unsigned long s_commit_interval;
	struct block_device *journal_bdev;
#ifdef CONFIG_JBD_DEBUG
	struct timer_list turn_ro_timer;	/* For turning read-only (crash simulation) */
	wait_queue_head_t ro_wait_queue;	/* For people waiting for the fs to go read-only */
#endif
#ifdef CONFIG_QUOTA
	char *s_qf_names[MAXQUOTAS];		/* Names of quota files with journalled quota */
	int s_jquota_fmt;			/* Format of quota to use */
#endif
};

static inline spinlock_t *
sb_bgl_lock(struct next3_sb_info *sbi, unsigned int block_group)
{
	return bgl_lock_ptr(sbi->s_blockgroup_lock, block_group);
}

#endif	/* _LINUX_NEXT3_SB */