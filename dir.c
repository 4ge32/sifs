#include <linux/module.h>
#include <linux/fs.h>
#include <linux/buffer_head.h>

#include "sifs.h"

static int sifs_readdir(struct file *fp, struct dir_context *ctx) {

	loff_t pos;
	struct super_block *sb;
	struct buffer_head *bh_inode;
	struct sifs_inode *si_inode;
	struct buffer_head *bh_record;
	struct sifs_dir_record *record;
	int i;

	pos = ctx->pos;
	sb = fp->f_inode->i_sb;
	if (pos)
		return 0;

	bh_inode = sb_bread(sb, SIFS_INODE_STORE_BLOCK_NUMBER);
	si_inode = (struct sifs_inode *)bh_inode->b_data;

	bh_record = sb_bread(sb, SIFS_INODE_STORE_BLOCK_NUMBER + 1);
	record = (struct sifs_dir_record *)bh_record->b_data;

	/*
	printk("children_count %lld\n", si_inode->children_count);
	printk("inode no %lld\n", record->inode_no);
	printk("name no %s\n", record->filename);
	*/

	for (i = 0; i < si_inode->children_count; i++) {
		if (!dir_emit(ctx, record->filename, strlen(record->filename),
			                   record->inode_no, DT_UNKNOWN))
			goto out;
		ctx->pos += sizeof(struct sifs_dir_record);
		pos += sizeof(struct sifs_dir_record);
		record++;
	}
	dir_emit_dots(fp, ctx);

//	printk("hmm..\n");

out:
//	printk("oops...\n");
	brelse(bh_inode);
	brelse(bh_record);

	return 0;
}

const struct file_operations sifs_dir_ops = {
	.iterate        = sifs_readdir,
};

static struct dentry *sifs_lookup(struct inode *parent_inode, struct dentry *child_dentry,
				  unsigned int flags)
{
	struct sifs_inode *parent = SIFS_INODE(parent_inode);
	struct super_block *sb = parent_inode->i_sb;
	struct buffer_head *bh;
	struct sifs_dir_record *record;
	int i;
	printk("LOOKUP\n");

	bh = sb_bread(sb, SIFS_INODE_STORE_BLOCK_NUMBER + 1);
	record = (struct sifs_dir_record *)bh->b_data;
	printk("%s\n", record->filename);

	for (i = 0; i < parent->children_count; i++) {
		printk("rec: %s, child: %lld", record->filename, parent->children_count);
		if (!strcmp(record->filename, child_dentry->d_name.name)) {
			struct inode *inode = sifs_iget(sb, record->inode_no);
			inode_init_owner(inode, parent_inode, SIFS_INODE(inode)->mode);
			d_add(child_dentry, inode);
			printk("FOUND\n");
			printk("LOOKUP:%lld", SIFS_INODE(inode)->file_size);
			return NULL;
		}
		record++;
	}

	printk("NOT FOUND\n");
	return NULL;
}

const struct inode_operations sifs_inode_ops = {
	.lookup = sifs_lookup,
};