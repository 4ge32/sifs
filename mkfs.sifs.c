#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>

#include "sifs.h"

static void write_superblock(int fd)
{
	ssize_t ret;

	struct sifs_sb sb = {
		.version = 0,
		.magic = SIFS_MAGIC,
		.block_size = BLOCK_DEFAULT_SIZE,
	};

	ret = write(fd, &sb, sizeof(sb));
	if (ret != BLOCK_DEFAULT_SIZE) {
		printf("bytes written [%ld]\n", ret);
	}
}

int main(int argc, char *argv[])
{
	int fd;

	if (argc != 2) {
		printf("Usage: mkfs.sifs <device>\n");
		return -1;
	}

	fd = open(argv[1], O_RDWR);
	if (fd == -1) {
		perror("unable to open the device");
		return -1;
	}

	write_superblock(fd);

	return 0;
}
