#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <inttypes.h>
#include <fcntl.h>

// Function Prototypes
int set_flag(int fd);
int copy_file(int fd, struct stat st, char *path);
char *file_content(int fd, struct stat st);
void print_permissions(struct stat st);

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
		fprintf(stderr, "Usage: ./run <SOURCE-PATH> <DEST-PATH>\n");
		
		return 1;
	}
	
	// File Descriptor and stat struct for File
	int fd;
	struct stat st;

	// Open Source File
	fd = open(argv[1], O_RDONLY);

	// Throw Error If File Does Not Exist
	if (fd == -1) {
		perror("open");
		return 2;
	}

	// Grab Metadata From File
	if (fstat(fd, &st) == -1) {
		perror("fstat");
		close(fd);
		return 1;
	}
	if (copy_file(fd, st, argv[2]) == -1) {
		fprintf(stderr, "test");
		return 1;
	}
/*
	char *content = file_content(fd, st);
	if (content == NULL) {
		return 1;
	}

	printf("file-content: %s\n", content);
	free(content); */

	// Set File Type Flag
	/*file_type(fd);

	// Operations Based on File Type
	if (REG_F) {
		// Get File Metadata (mainly size)
		if (fstat(fd, &st) == -1) {
			perror("stat");
			close(fd);
			return 3;
		}
		copy(fd, st, argv[2]);	
	}*/

	// Finish
	return 0;
}

int set_flag(int fd) {
	// Exit Status 
	int status = 0;

	// Create Stat Struct
	struct stat st; 

	// Grab Metadata Information
	if (fstat(fd, &st) == -1) {
		perror("stat");
		status = -1;
	}
	
	// Set Flags Based on Filetype
	if (S_ISREG(st.st_mode)) {
		REG_F = 1;
	} else if (S_ISDIR(st.st_mode)) {
		DIR_F = 1;
	} else if (S_ISLNK(st.st_mode)) {
		LNK_F = 1;
	} else { status = -1; }
	
	return status;
}

// Get File Content
char *file_content(int fd, struct stat st) {
	// Set Dynamic Size of File Content
	char *content = malloc(st.st_size+1);
	if (content == NULL) {
		perror("malloc");
		close(fd);
		return NULL;
	}

	// Read File Content Onto content buffer
	ssize_t bytes_read = read(fd, content, st.st_size);
	if (bytes_read == -1) {
		perror("read");
		free(content);
		close(fd);
		return NULL;
	}

	// Add Null Terminator to Content (now we can treat it like a string)
	content[bytes_read] = '\0';
	return content;
}

// Copy File Function - Assumes Given File Descriptor is for a Regular File
int copy_file(int fd, struct stat st, char *path) {
	// Exit Status Variable
	int status = 0;
	
	// Get File Content of Source File
	char *content = file_content(fd, st);
	if (content == NULL) {
		status = -1;
		return status;
	}
	
	// Make Copy of File
	int fd2;
	fd2 = open(path, O_RDWR | O_CREAT, st.st_mode);
	if (fd2 == -1) {
		fprintf(stderr, "Missing DEST File\n");
		fprintf(stderr, "Usage: ./run <SOURCE-PATH> <DESTINATION-PATH>\n");
		status = -1;
		return status;
	}

	// Copy Permissions Of File 
	struct stat st2; 
	if (fstat(fd2, &st2) == -1) {
		perror("fstat");
		close(fd2);
		status = -1;
		return status;
	}

	//st2.st_mode |= (ow_perms | gr_perms | ot_perms);

	// Write Contents to File
	ssize_t bytes_written = write(fd2, content, st.st_size);
	if (bytes_written == -1) {
		perror("write");
		status = -1;
	}
	// Free Memory & Close Files
	free(content);
	close(fd);
	close(fd2);

	return status;
}
