#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <inttypes.h>
#include <fcntl.h>

// Function Prototypes
int list_directory(const char *path);
int file_type(int fd);

// File Type Flags
int REG_F = 0;
int DIR_F = 0;
int LNK_F = 0;

// Argument Flags

// Main Function
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
	
	// Get File Mode
	printf("mode type: %ju\n", (uintmax_t)st.st_mode);

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
	close(fd);

	// List Directories
	//const char *path = "."; // . = current directory in terminal
	
	// Finish
	return 0;
}

// Determine if File is Regular, Symbolic Link, or Directory
int file_type(int fd) {
	// Exit Status 
	int status = 0;

	// Create Stat Struct
	struct stat st; 

	// Grab Metadata Information
	if (fstat(fd, &st) == -1) {
		perror("stat");
		status = 1;
	}

	if (S_ISREG(st.st_mode)) {
		REG_F = 1;
	} else if (S_ISDIR(st.st_mode)) {
		DIR_F = 1;
	} else if (S_ISLNK(st.st_mode)) {
		LNK_F = 1;
	} else { status = 1; }
	
	return status;
}
