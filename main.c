#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <inttypes.h>
#include <fcntl.h>

int main(int argc, char *argv[]) {
	// Error Check If No File Is Entered
	if (argc < 2) {
		fprintf(stderr, "Error: No file was entered.\n");
		fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
		
		return 1;
	}

	// File Descriptor and stat struct for File
	int fd;
	struct stat st;

	// Open File
	fd = open(argv[1], O_RDONLY);

	// Throw Error If File Does Not Exist
	if (fd == -1) {
		perror("open");
		return 2;
	}

	// Get File Metadata (mainly size)
	if (fstat(fd, &st) == -1) {
		perror("stat");
		close(fd);
		return 3;
	}

	// Set Dynamic Size of File Content
	char *content = malloc(st.st_size+1);
	if (content == NULL) {
		fprintf(stderr, "Memory allocation failed\n");
		close(fd);
		return 4;
	}

	// Read File Content Onto content buffer
	ssize_t bytes_read = read(fd, content, st.st_size);
	if (bytes_read == -1) {
		perror("read");
		free(content);
		close(fd);
		return 5;
	}

	// Add Null Terminator to Content (now we can treat it like a string)
	content[bytes_read] = '\0';
	printf("File Content of %s:\n%s\n", argv[1], content);
	free(content);

	// Directory Stream (need it to open actual directory)
	DIR *dir;

	// Directory Entry (a file or subdirectory within the directory stream)
	struct dirent *dir_entry;

	// Open Directory Stream
	const char *path = "."; // . = current directory in terminal
	dir = opendir(path);

	// Error Check If Directory Doesn't Exist
	if (dir == NULL) {
		perror("opendir");
		return 4;
	}

	close(fd);
	printf("-----List Files Within Current Directory-----:\n");

	// List Entries Within Directory
	while ((dir_entry = readdir(dir)) != NULL) {
		// Access File Name since dir_entry has a member d_name
		printf("Serial Number: %ju Entry Name: %s\n", (uintmax_t)dir_entry->d_ino, dir_entry->d_name);
	}

	// Close Directory Error Check
	if (closedir(dir) == -1) {
		perror("closedir");
		return 5;
	}
	
	// Finish
	return 0;
}
