#!/usr/bin/python
# NAME: Steven Chu
# EMAIL: schu92620@gmail.com
# ID: 905094800

import sys
import csv
from collections import defaultdict

BLOCK_SIZE = 256
EXT2_NDIR_BLOCK = 12
EXT2_IND_BLOCK = 12
EXT2_DIND_BLOCK = 13
EXT2_TIND_BLOCK = 14
EXT2_N_BLOCK = 15

# Helper classes to store data fields for objects defined in ext2_fs.h
class group:
	def __init__(self, row):
		self.group_num = int(row[1])
		self.num_blocks = int(row[2])
		self.num_inodes = int(row[3])
		self.num_free_blocks = int(row[4])
		self.num_free_inodes = int(row[5])
		self.block_bitmap = int(row[6])
		self.inode_bitmap = int(row[7])
		self.inode_table = int(row[8])

class superblock:
	def __init__(self, row):
		self.num_blocks = int(row[1])
		self.num_inodes = int(row[2])
		self.block_size = int(row[3])
		self.inode_size = int(row[4])
		self.blocks_per_group = int(row[5])
		self.inodes_per_group = int(row[6])
		self.lead_inode = int(row[7])

class Inode:
	def __init__(self, row):
		self.inode_num = int(row[1])
		self.file_type  = row[2]
		self.mode = row[3]
		self.owner_id = int(row[4])
		self.group_id = int(row[5])
		self.link_count = int(row[6])
		self.creation_timestamp = row[7]
		self.mod_timestamp = row[8]
		self.access_timestamp = row[9]
		self.file_size = row[10]
		self.dir_blocks = list(map(int, row[12:]))[:EXT2_NDIR_BLOCK]
		self.ind_block_numbers = list(map(int, row[12:]))[EXT2_NDIR_BLOCK:EXT2_N_BLOCK]

class Bfree:
	def __init__(self, row):
		self.entry_index = int(row[1])

class Ifree:
        def __init__(self, row):
                self.entry_index = int(row[1])

class direntr:
	def __init__(self, row):
		self.parent_inode = int(row[1])
		self.offset = int(row[2])
		self.inode_num = int(row[3])
		self.entry_length = int(row[4])
		self.name_length = int(row[5])
		self.name = row[6]

class Indirect:
	def __init__(self, row):
		self.inode_num = int(row[1])
		self.level = int(row[2])
		self.offset = int(row[3])
		self.indirect_block_num = int(row[4])
		self.block_num = int(row[5])

class blockreference:
	def __init__(self, block_level, block_num, inode_num, offset):
		self.block_level = block_level
		self.block_num = block_num
		self.inode_num = inode_num
		self.offset = offset

	def get_block_level(self):
		if(self.block_level == 0): 
			return 'BLOCK'
		if(self.block_level == 1):
			return 'INDIRECT BLOCK'
		if(self.block_level == 2):
			return 'DOUBLE INDIRECT BLOCK'
		if(self.block_level == 3):
			return 'TRIPLE INDIRECT BLOCK'
		else:
			return None
		
def main():
	# Check to make sure argument is of valid length
	if len(sys.argv) != 2:
		sys.stderr.write("Unrecognized argument received.\n")
		sys.exit(1)
	# Initialize name of csv file
	file_sys = sys.argv[1]
	status = 0
	# Initialize storage structures
	super_block = None
	group_desc = None
	bfree_list = []
	ifree_list = []
	inode_list = []
	dirent_list = []
	indirect_list = []
	exit_code = 0
	# Try catch for opening file, output error message for failures
	try:
		# Open CSV file
		with open(file_sys) as csvfile:
			csv_read = csv.reader(csvfile, delimiter=',')
			# Read CSV file and store in respective structures
			for row in csv_read:
				if row[0] == "SUPERBLOCK":
					super_block = superblock(row)
				elif row[0] == "GROUP":
					group_desc = group(row)
				elif row[0] == "BFREE":
					bfree_list += [Bfree(row)]
				elif row[0] == "IFREE":
					ifree_list += [Ifree(row)]
				elif row[0] == "INODE":
					inode_list += [Inode(row)]
				elif row[0] == "DIRENT":
					dirent_list += [direntr(row)]
				elif row[0] == "INDIRECT":
					indirect_list += [Indirect(row)]
	except:
		sys.stderr.write("Failed to open csv.\n") 
		sys.exit(1)
	# Compute the start of the free list
	first_unreserved_block = group_desc.inode_table + super_block.inode_size * group_desc.num_inodes/super_block.block_size
	# Initialize tracking structure for block references
	referenced_blocks = defaultdict(list)
	# Loop through inode blocks and check for in range numbers and reserved blocks
	for inode in inode_list:
		for i in range(len(inode.dir_blocks)):
			if inode.dir_blocks[i] < 0 or inode.dir_blocks[i] >= super_block.num_blocks:
				print('INVALID BLOCK {} IN INODE {} AT OFFSET {}'.format(inode.dir_blocks[i], inode.inode_num, i))
				exit_code = 2
			elif inode.dir_blocks[i] > 0 and inode.dir_blocks[i] < first_unreserved_block:
				print('RESERVED BLOCK {} IN INODE {} AT OFFSET {}'.format(inode.dir_blocks[i], inode.inode_num, i))
				exit_code = 2
			else:
				referenced_blocks[inode.dir_blocks[i]].append(blockreference(0, inode.dir_blocks[i], inode.inode_num, i))
		for i in range(len(inode.ind_block_numbers)):
			logic_offset = None
			level_str = None
			if (i+1 == 1):
				logic_offset = EXT2_NDIR_BLOCK
				level_str = "INDIRECT"
			elif (i+1 == 2):
				logic_offset = EXT2_NDIR_BLOCK + BLOCK_SIZE
				level_str = "DOUBLE INDIRECT"
			elif (i+1 == 3):
				logic_offset = EXT2_NDIR_BLOCK + (1 + BLOCK_SIZE) * BLOCK_SIZE
				level_str = "TRIPLE INDIRECT"
			if inode.ind_block_numbers[i] < 0 or inode.ind_block_numbers[i] >= super_block.num_blocks:
				print('INVALID {} BLOCK {} IN INODE {} AT OFFSET {}'.format(level_str, inode.ind_block_numbers[i], inode.inode_num, logic_offset))
				exit_code = 2
			elif inode.ind_block_numbers[i] > 0 and inode.ind_block_numbers[i] < first_unreserved_block:
				print('RESERVED {} BLOCK {} IN INODE {} AT OFFSET {}'.format(level_str, inode.ind_block_numbers[i], inode.inode_num, logic_offset))
				exit_code = 2
			else:
				referenced_blocks[inode.ind_block_numbers[i]].append(blockreference(i+1, inode.ind_block_numbers[i], inode.inode_num, logic_offset))
	# Check indirect blocks
	for indirect in indirect_list:
		logic_offset = None
		level_str = None
		if (indirect.level == 1):
			logic_offset = EXT2_NDIR_BLOCK
			level_str = "INDIRECT"
		elif (indirect.level == 2):
			logic_offset = EXT2_NDIR_BLOCK + BLOCK_SIZE
			level_str = "DOUBLE INDIRECT"
		elif (indirect.level == 3):
			logic_offset = EXT2_NDIR_BLOCK + (1 + BLOCK_SIZE) * BLOCK_SIZE
			level_str = "TRIPLE INDIRECT"
		if indirect.block_num < 0 or indirect.block_num >= super_block.num_blocks:
			print('INVALID {} BLOCK {} IN INODE {} AT OFFSET {}'.format(level_str, indirect.inode_num, logic_offset))
			exit_code = 2
		elif indirect.block_num > 0 and indirect.block_num < first_unreserved_block:
			print('RESERVED {} BLOCK {} IN INODE {} AT OFFSET {}'.format(level_str, indirect.inode_num, logic_offset))
			exit_code = 2
		else:
			referenced_blocks[indirect.block_num].append(blockreference(indirect.level, indirect.block_num, indirect.inode_num, logic_offset))
	# Check for block errors in the free list
	free_blocks = list(map(lambda bfree: bfree.entry_index, bfree_list))
	for block_number in range(first_unreserved_block, super_block.num_blocks):
		if block_number not in referenced_blocks and block_number not in free_blocks:
			print('UNREFERENCED BLOCK {}'.format(block_number))
			exit_code = 2
		if block_number in referenced_blocks and block_number in free_blocks:
			print('ALLOCATED BLOCK {} ON FREELIST'.format(block_number))
			exit_code = 2
		block_ref_list = []
		if block_number in referenced_blocks and len(referenced_blocks[block_number]) != 1:
			block_ref_list = referenced_blocks[block_number]
		if block_ref_list:
			for block_reference in block_ref_list:
				print('DUPLICATE {} {} IN INODE {} AT OFFSET {}'.format(block_reference.get_block_level(), block_reference.block_num, block_reference.inode_num, block_reference.offset))
				exit_code = 2
	# Check for inode errors in the free list
	free_inodes = list(map(lambda ifree: ifree.entry_index, ifree_list))
	allocated_inodes = list(map(lambda inode: inode.inode_num, inode_list))
	for inode_number in allocated_inodes:
		if inode_number in free_inodes:
			print('ALLOCATED INODE {} ON FREELIST'.format(inode_number))
			exit_code = 2
	for inode_number in range(super_block.lead_inode, super_block.num_inodes + 1):
		if inode_number not in allocated_inodes and inode_number not in free_inodes:
			print('UNALLOCATED INODE {} NOT ON FREELIST'.format(inode_number))
			exit_code = 2
	# Check for directory entry errors
 	link_count = defaultdict(int)
	parents = {}
	for dirent in dirent_list:
		if dirent.inode_num < 1 or dirent.inode_num > super_block.num_inodes:
			print('DIRECTORY INODE {} NAME {} INVALID INODE {}'.format(dirent.parent_inode, dirent.name, dirent.inode_num))
			exit_code = 2
		elif dirent.inode_num not in allocated_inodes:
			print('DIRECTORY INODE {} NAME {} UNALLOCATED INODE {}'.format(dirent.parent_inode, dirent.name, dirent.inode_num))
			exit_code = 2
		else:
			link_count[dirent.inode_num] += 1
			if dirent.inode_num not in parents: 
				parents[dirent.inode_num] = dirent.parent_inode
	# Check for link count errors
	for inode in inode_list:
		if inode.link_count != link_count[inode.inode_num]:
			print('INODE {} HAS {} LINKS BUT LINKCOUNT IS {}'.format(inode.inode_num, link_count[inode.inode_num], inode.link_count))
			exit_code = 2
	# Check for special directory errors
	for dirent in dirent_list:
		if dirent.name == "'.'" and dirent.parent_inode != dirent.inode_num:
			print('DIRECTORY INODE {} NAME {} LINK TO INODE {} SHOULD BE {}'.format(dirent.parent_inode, dirent.name, dirent.inode_num, dirent.parent_inode))
			exit_code = 2
		elif dirent.name == "'..'" and parents[dirent.parent_inode] != dirent.inode_num:
			print('DIRECTORY INODE {} NAME {} LINK TO INODE {} SHOULD BE {}'.format(dirent.parent_inode, dirent.name, dirent.inode_num, parents[dirent.parent_inode]))
			exit_code = 2

	sys.exit(exit_code)

if __name__ == '__main__':
	main()
