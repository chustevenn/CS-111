/* NAME: Steven Chu
 * EMAIL: schu92620@gmail.com
 * ID: 905094800
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include "ext2_fs.h"

#define SUPERBLOCK_OFFSET 1024
#define GROUP_OFFSET 2048

// Initialize file descriptor of file system image
int sys_fd;

// Helper function to return the jth bit of a binary word (counting from LSB)
int get_jth_bit(unsigned char byte, int j)
{
	return (byte >> j) & 1;
}

// Helper function to output an indirect entry to stdout
int output_indirent(int block_num, int inode_num, int address, int index, int indirection, 
		    int superblock_size)
{
	int block;
	int indirent_check = pread(sys_fd, &block, sizeof(int), block_num * superblock_size + 
				   index * sizeof(int));
	if(indirent_check == -1)
	{
		fprintf(stderr, "Failed to read indirect entry: %s\n", strerror(errno));
		exit(2);
	}
	
	// Don't output invalid blocks
	if(block == 0)
		return block;
	
	// Perform I/O to stdout as specified
	fprintf(stdout, "INDIRECT,%d,%d,%d,%d,%d\n",
		inode_num,
		indirection,
		address,
		block_num,
		block);
	return block;
}

// Helper function to output a direct entry to stdout
void dirent_out(struct ext2_dir_entry *dirent, int inode_num, int address)
{
	fprintf(stdout, "DIRENT,%d,%d,%d,%d,%d,'%s'\n",
		inode_num,
		address,
		dirent->inode,
		dirent->rec_len,
		dirent->name_len,
		dirent->name);
}

// Helper function to retrieve the block number of an indirect entry
int get_blocknum(int block_num, int index, int superblock_size)
{
	int blocknum;
	int blocknum_check = pread(sys_fd, &blocknum, sizeof(int), block_num * superblock_size +
				   index * sizeof(int));
	if(blocknum_check == -1)
	{
		fprintf(stderr, "Failed to read next block number of indirect block: %s\n", strerror(errno));
		exit(2);
	}
	
	return blocknum;
}

// Helper function to read an indirect directory entry
void indirent_helper(int block_num, int address, struct ext2_dir_entry *dirent, int superblock_size)
{
	int indirent_check = pread(sys_fd, dirent, sizeof(struct ext2_dir_entry),
				   block_num * superblock_size + address);
	if(indirent_check == -1)
	{
		fprintf(stderr, "Failed to read an indirect directory entry: %s\n", strerror(errno));
		exit(2);
	}
}

// Helper function to generate the specified output for a single indirect entry
void single_indirent(struct ext2_inode *inode, int inode_num, int superblock_size)
{
	int i;
	for(i = 0; i < (int) (superblock_size/sizeof(int)); i++)
	{
		int address = EXT2_NDIR_BLOCKS + i;
		// Print entry to stdout
		output_indirent(inode->i_block[EXT2_IND_BLOCK], inode_num, address, i, 1,
				superblock_size);
	}
}

// Helper function to generate the specified output for a double indirect entry
void double_indirent(struct ext2_inode *inode, int inode_num, int superblock_size)
{
	int i, j;
	// Loop through all double indirect entries and output them
	for(i = 0; i < (int) (superblock_size/sizeof(int)); i++)
	{
		int address = EXT2_NDIR_BLOCKS + (i + 1) * (int) (superblock_size/sizeof(int));
		int indir_block_num = output_indirent(inode->i_block[EXT2_DIND_BLOCK],
						      inode_num, address, i, 2, superblock_size);
		// Exclude invalid blocks
		if(indir_block_num == 0)
			continue;
		// Loop through all single indirect entries and output them
		for(j = 0; j < (int) (superblock_size/sizeof(int)); j++)
		{
			address = EXT2_NDIR_BLOCKS + (i + 1) * (superblock_size/sizeof(int) + j);
			output_indirent(indir_block_num, inode_num, address, j, 1, superblock_size);
		}
	}
}

// Helper function to generate the specified output for a triple indirect entry
void triple_indirent(struct ext2_inode *inode, int inode_num, int superblock_size)
{
	int i, j, k;
	// Loop through all triple indirect entries and output them
	for(i = 0; i < (int) (superblock_size/sizeof(int)); i++)
	{
		int address = EXT2_NDIR_BLOCKS + ((i + 1) * (superblock_size/sizeof(int) + 1)) *
			      (superblock_size/sizeof(int));
		int double_indir_block_num = output_indirent(inode->i_block[EXT2_TIND_BLOCK],
							     inode_num, address, i, 3, superblock_size);
		// Exclude invalid blocks
		if(double_indir_block_num == 0)
			continue;
		// Loop through all double indirect entries and output them
		for(j = 0; j < (int) (superblock_size/sizeof(int)); j++)
		{
			address = EXT2_NDIR_BLOCKS + ((i + 1) * (superblock_size/sizeof(int) + 1 + j)) *
				  (superblock_size/sizeof(int));
			int indir_block_num = output_indirent(double_indir_block_num,
							      inode_num, address, j, 2, 
							      superblock_size);
			// Exclude invalid blocks
			if(indir_block_num == 0)
				continue;
			// Loop through all single indirect entries and output them
			for(k = 0; k < (int) (superblock_size/sizeof(int)); k++)
			{
				address = EXT2_NDIR_BLOCKS + ((i + 1) * (superblock_size/sizeof(int) + 1 + j))
					  * (superblock_size/sizeof(int)) + k;
				output_indirent(indir_block_num, inode_num, address, k, 1,
						superblock_size);
			}
		}
	}
}


// Helper function to generate the specified output for a single indirect reference
void single_indirref(struct ext2_inode *inode, int inode_num, int superblock_size)
{
	int i, j;
	struct ext2_dir_entry dirent;
	// Loop through all block references
	for(i = 0; i < (int) (superblock_size/sizeof(int)); i++)
	{
		int block_num = get_blocknum(inode->i_block[EXT2_IND_BLOCK], i, superblock_size);
		// Exclude invalid references
		if(block_num == 0)
			continue;
		// Loop through all inodes
		for(j = 0; j < superblock_size; j += dirent.rec_len)
		{
			indirent_helper(block_num, j, &dirent, superblock_size);
			// Exclude invalid inodes
			if(dirent.inode == 0)
				continue;

			int address = (EXT2_NDIR_BLOCKS + i) * superblock_size + j;
			dirent_out(&dirent, inode_num, address);
		}
	}
}

// Helper function to generate the specified output for a double indirect reference
void double_indirref(struct ext2_inode *inode, int inode_num, int superblock_size)
{
	int i, j, k;
	struct ext2_dir_entry dirent;
	// Loop through all indirect block references
	for(i = 0; i < (int) (superblock_size/sizeof(int)); i++)
	{
		int indirect_block_num = get_blocknum(inode->i_block[EXT2_IND_BLOCK], i, superblock_size);
		//Exclude invalid blocks
		if(indirect_block_num == 0)
			continue;
		// Loop through all block references
		for(j = 0; j < (int) (superblock_size/sizeof(int)); j++)
		{
			int block_num = get_blocknum(indirect_block_num, j, superblock_size);
			// Exclude invalid blocks
			if(block_num == 0)
				continue;
			// Loop through all inodes
			for(k = 0; k < superblock_size; k += dirent.rec_len)
			{
				indirent_helper(block_num, k, &dirent, superblock_size);
				// Exclude invalid inodes
				if(dirent.inode == 0)
					continue;
				int address = (EXT2_NDIR_BLOCKS + (i + 1) * (superblock_size/sizeof(int) + j)
					   ) * superblock_size + k;
				dirent_out(&dirent, inode_num, address);
			}
		}
	}
}

// Helper function to generate the specified output for a triple indirect reference
void triple_indirref(struct ext2_inode *inode, int inode_num, int superblock_size)
{
	int i, j, k, l;
	struct ext2_dir_entry dirent;
	// Loop through all double indirect block references
	for(i = 0; i < (int) (superblock_size/sizeof(int)); i++)
	{
		int d_indirect_block_num = get_blocknum(inode->i_block[EXT2_IND_BLOCK], i, superblock_size);
		//Exclude invalid blocks
		if(d_indirect_block_num == 0)
			continue;
		// Loop through all indirect block references
		for(j = 0; j < (int) (superblock_size/sizeof(int)); j++)
		{
			int indirect_block_num = get_blocknum(d_indirect_block_num, j, superblock_size);
			// Exclude invalid blocks
			if(indirect_block_num == 0)
				continue;
			// Loop through all block references
			for(k = 0; k < (int) (superblock_size/sizeof(int)); k++)
			{
				int block_num = get_blocknum(indirect_block_num, k, superblock_size);
				// Exclude invalid blocks
				if(block_num == 0)
					continue;
				// Loop through all inodes
				for(l = 0; l < superblock_size; l += dirent.rec_len)
				{
					indirent_helper(block_num, l, &dirent, superblock_size);
					if(dirent.inode == 0)
						continue;
					int address = (EXT2_NDIR_BLOCKS + ((i + 1) * 
						      (superblock_size/sizeof(int) + 1 + j)) *
						      (superblock_size/sizeof(int) + k)) *
						       superblock_size + l;
					dirent_out(&dirent, inode_num, address);
				}
			}
		}
	}
}


int main(int argc, char *argv[])
{
	// Check number of arguments
	if(argc != 2)
	{
		fprintf(stderr, "Invalid input.\n");
		exit(1);
	}
	// Open file system image and set file descriptor
	sys_fd = open(argv[1], O_RDONLY);
	if(sys_fd == -1)
	{
		fprintf(stderr, "Failed to open specified file system image: %s\n", strerror(errno));
		exit(1);
	}
	
	// Initialize super block
	struct ext2_super_block super;
	
	// Read file system superblock
	int super_check = pread(sys_fd, &super, sizeof(struct ext2_super_block), SUPERBLOCK_OFFSET);
	if(super_check == -1)
	{
		fprintf(stderr, "Failed to read superblock: %s\n", strerror(errno));
		exit(2); 
	}
	
	// Calculate size of superblock using formula provided in ext2_fs.h
	int superblock_size = EXT2_MIN_BLOCK_SIZE << super.s_log_block_size;

	// Print specified output for superblock to stdout
	fprintf(stdout, "%s,%d,%d,%d,%d,%d,%d,%d\n",
		"SUPERBLOCK",
		super.s_blocks_count,
		super.s_inodes_count,
		superblock_size,
		super.s_inode_size,
		super.s_blocks_per_group,
		super.s_inodes_per_group,
		super.s_first_ino);

	// Initialize file system group description
        struct ext2_group_desc group;

	// Read from the group start point into the group buffer. Spec specifies there will
	// be one group.
	int group_check = pread(sys_fd, &group, sizeof(struct ext2_group_desc), GROUP_OFFSET);
	if(group_check == -1)
	{
		fprintf(stderr, "Failed to read group: %s\n", strerror(errno));
		exit(2);
	}
	
	// Print specified output for group description to stdout
	fprintf(stdout, "%s,%d,%d,%d,%d,%d,%d,%d,%d\n",
		"GROUP",
		0,
		super.s_blocks_count,
		super.s_inodes_count,
		group.bg_free_blocks_count,
		group.bg_free_inodes_count,
		group.bg_block_bitmap,
		group.bg_inode_bitmap,
		group.bg_inode_table);

	// Retrieve bitmap of file system blocks
	int block_bitmap = group.bg_block_bitmap;
	unsigned char byte;
	int i, j;

	// Iterate through the bitmap
	for(i = 0; i < superblock_size; i++)
	{
		int byte_check = pread(sys_fd, &byte, sizeof(unsigned char),
				       block_bitmap * superblock_size + i);
		if(byte_check == -1)
		{
			fprintf(stderr, "Failed to read free block entry: %s\n", strerror(errno));
			exit(2);
		}
		// Iterate through each byte to find free blocks, and print the specified output
		for(j = 0; j < 8; j++)
		{
			int jth_bit = get_jth_bit(byte, j);
			if(!jth_bit)
			{
				int index = 8 * i + (j+1);
				fprintf(stdout, "BFREE,%d\n", index);
			}
		}
	}	
	
	// Retrieve bitmap of file system inodes
	int inode_bitmap = group.bg_inode_bitmap;
	unsigned char i_byte;

	// Iterate through the bitmap
	for(i = 0; i < superblock_size; i++)
	{
		int byte_i_check = pread(sys_fd, &i_byte, sizeof(unsigned char),
					 inode_bitmap * superblock_size + i);
		if(byte_i_check == -1)
		{
			fprintf(stderr, "Failed to read free inode entry: %s\n", strerror(errno));
			exit(2);
		}

		// Iterate through each byte to find free inodes, and print the specified output
		for(j = 0; j < 8; j++)
		{
			int jth_bit = get_jth_bit(i_byte, j);
			if(!jth_bit)
			{
				int index = 8 * i + (j+1);
				fprintf(stdout, "IFREE,%d\n", index);

			}
		}
	}

	// Initialize inode buffer, and retrieve number of inodes
	struct ext2_inode inode;
	int num_inodes = (int) super.s_inodes_count;
	
	// Iterate through all inodes, compute and produce the specified output
	for(i = 0; i < num_inodes; i++)
	{
		int inode_check = pread(sys_fd, &inode, sizeof(struct ext2_inode), 
				group.bg_inode_table * superblock_size + i * sizeof(struct ext2_inode));
		if(inode_check == -1)
		{
			fprintf(stderr, "Failed to read inode entry: %s\n", strerror(errno));
			exit(2);
		}
		
		if(inode.i_links_count == 0 || inode.i_mode == 0)
			continue;
		
		char file_type;
		if((inode.i_mode & 0XF000) == 0x8000)
			file_type = 'f';	
		else if((inode.i_mode & 0xF000) == 0XA000)
			file_type = 's';
		else if((inode.i_mode & 0XF000) == 0X4000)
			file_type = 'd';
		else
			file_type = '?';
	
		//fprintf(stdout, "Got here. %d\n", num_inodes);	
		char created_timestamp[128];
		time_t time = inode.i_ctime;
		struct tm *info = gmtime(&time);
		strftime(created_timestamp, 128, "%m/%d/%y %H:%M:%S", info);
		
		char modified_timestamp[128];
		time_t m_time = inode.i_mtime;
		struct tm *m_info = gmtime(&m_time);
		strftime(modified_timestamp, 128, "%m/%d/%y %H:%M:%S", m_info);
		
		char accessed_timestamp[128];
		time_t a_time = inode.i_atime;
		struct tm *a_info = gmtime(&a_time);
		strftime(accessed_timestamp, 128, "%m/%d/%y %H:%M:%S", a_info);

		int inode_mode = (inode.i_mode & 0x0FFF);
		
		fprintf(stdout, "%s,%d,%c,%o,%d,%d,%d,%s,%s,%s,%d,%d",
			"INODE",
			i + 1,
			file_type,
			inode_mode,
			inode.i_uid,
			inode.i_gid,
			inode.i_links_count,
			created_timestamp,
			modified_timestamp,
			accessed_timestamp,
			inode.i_size,
			inode.i_blocks);
		
		// If inode is not a symbolic link, the next 15 fields are block addresses
		if(file_type != 's' || inode.i_size >= 60)
		{
			for(j = 0; j < EXT2_N_BLOCKS; j++)
				fprintf(stdout, ",%d", inode.i_block[j]);
		}
		fprintf(stdout, "\n");
	}

	// Loop through all inodes and find directory entries
	for(i = 0; i < num_inodes; i++)
	{
		int dir_check = pread(sys_fd, &inode, sizeof(struct ext2_inode),
			    group.bg_inode_table * superblock_size + i * sizeof(struct ext2_inode));
		if(dir_check == -1)
		{
			fprintf(stderr, "Failed to read inode entry: %s\n", strerror(errno));
			exit(2);
		}
		
		// Exclude free inodes
                if(inode.i_links_count == 0 || inode.i_mode == 0)
                        continue;
		
		// Exclude inodes that are not directory entries
		if((inode.i_mode & 0XF000) != 0X4000)
			continue;

		// Create outputs for direct entries
		for(j = 0; j < EXT2_NDIR_BLOCKS; j++)
		{
			// Exclude invalid blocks
			if(inode.i_block[j] == 0)
				continue;

			int address;
			struct ext2_dir_entry dirent;
			
			// Loop through each entry
			int k;
			for(k = 0; k < superblock_size; k += dirent.rec_len)
			{
				int dir_ent_check = pread(sys_fd, &dirent, sizeof(struct ext2_dir_entry),							inode.i_block[j] * superblock_size + k);
				if(dir_ent_check == -1)
				{
					fprintf(stderr, "Failed to read directory entry: %s\n", strerror(errno));
					exit(2);
				}
				
				// Exclude entries with invalid inode numbers
				if(dirent.inode == 0)
					continue;
				// Calculate address of particular directory entry
				address = j * superblock_size + k;
				
				// Output specified output to stdout
				fprintf(stdout, "DIRENT,%d,%d,%d,%d,%d,'%s'\n",
					i + 1,
					address,
					dirent.inode,
					dirent.rec_len,
					dirent.name_len,
					dirent.name);
			}
		}
		
		// Create outputs for single indirect entries
		if(inode.i_block[EXT2_IND_BLOCK] != 0)
			single_indirref(&inode, i+1, superblock_size);
		// Create outputs for double indirect entries
		if(inode.i_block[EXT2_DIND_BLOCK] != 0)
			double_indirref(&inode, i+1, superblock_size);
		// Create outputs for triple indirect entries
		if(inode.i_block[EXT2_TIND_BLOCK] != 0)
			triple_indirref(&inode, i+1, superblock_size);

	}

	// Loop through inodes and find indirect directory references
	for(i = 0; i < (int) super.s_inodes_count; i++)
	{
		int indir_ref_check = pread(sys_fd, &inode, sizeof(struct ext2_inode),
					    group.bg_inode_table * superblock_size + i * 
					    sizeof(struct ext2_inode));
		if(indir_ref_check == -1)
		{
			fprintf(stderr, "Failed to read indirect directory reference: %s\n", strerror(errno));
			exit(2);
		}
		
		// Exclude free inodes
		if(inode.i_links_count == 0 || inode.i_mode == 0)
			continue;
		// Exclude symbolic link files
		if(((inode.i_mode & 0XF000) != 0X4000) && ((inode.i_mode & 0xF000) != 0x8000))
			continue;
		
                // Create outputs for single indirect entries
                if(inode.i_block[EXT2_IND_BLOCK] != 0)
                        single_indirent(&inode, i+1, superblock_size);
                // Create outputs for double indirect entries
                if(inode.i_block[EXT2_DIND_BLOCK] != 0)
                        double_indirent(&inode, i+1, superblock_size);
                // Create outputs for triple indirect entries
                if(inode.i_block[EXT2_TIND_BLOCK] != 0)
                        triple_indirent(&inode, i+1, superblock_size);
	}
	
	close(sys_fd);
	exit(0);
}
