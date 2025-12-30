#define _POSIX_C_SOURCE 200809L

#include <stdio.h> 
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <inttypes.h>
#include <fcntl.h>

// Function Prototypes
int get_file_type(char *path);
int copy_file(int fd, struct stat st, char *path);
char *get_file_content(int fd, struct stat st);

// File Type Flags
int REG_F, DIR_F, LNK_F;

// Argument Flags
enum ARG_FLAGS {
	 i_FLAG = 1 << 0,
	 p_FLAG = 1 << 1,
	 r_FLAG = 1 << 2,
	 b_FLAG = 1 << 3,
	 l_FLAG = 1 << 4,
	 s_FLAG = 1 << 5,
	 t_FLAG = 1 << 6
};

// Main Function
int main(int argc, char *argv[]) {
	// Default File Type Flags to 0
	REG_F = DIR_F = LNK_F = 0;

	// Store Argument Flags
	int args = 0;

	// Parse Through Argument Flags (if Any)
	int opt;
	while ((opt = getopt(argc, argv, "iprRblst")) != -1) {
		switch (opt) {
			case 'i':
				args |= i_FLAG;
				break;

			case 'p':
				args |= p_FLAG;
				break;

			case 'r':
				args |= r_FLAG;
				break;

			case 'R':
				args |= r_FLAG;
				break;

			case 'b':
				args |= b_FLAG;
				break;

			case 'l':
				args |= l_FLAG;
				break;

			case 's':
				args |= s_FLAG;
				break;

			case 't':
				args |= t_FLAG;
				break;
			default:
				fprintf(stderr, "The available argument flags are: -i -p -r -R -b -l -s -t\n");
				return 1; 
		}
	}
	
	printf("source: %s dest: %s\n", argv[optind], argv[optind+1]);	
	printf("Flags (integer form): %d\n", args); 
	// Print Every Arg After The Flags
	/* // Error Check If No File Is Entered 
	if (argc < 3) {
		fprintf(stderr, "Error: No file was entered.\n");
		fprintf(stderr, "Usage: ./run <SOURCE-PATH> <DEST-PATH>\n");
		
		return 1;
	} */
	
	// File Descriptor and stat struct for File
	int SOURCE_FD;
	struct stat st;

	// Open Source File
	SOURCE_FD = open(argv[optind], O_RDONLY);

	// Throw Error If File Does Not Exist
	if (SOURCE_FD == -1) {
		perror("open");
		return 2;
	}

	// Grab Metadata From File
	if (fstat(SOURCE_FD, &st) == -1) {
		perror("fstat");
		close(SOURCE_FD);
		return 1;
	}
	
	// Determine File Type
	if (get_file_type(argv[optind]) == -1) {
		fprintf(stderr, "Invalid File Type. Enter either a regular file, directory, or a symbolic link.\n");
		close(SOURCE_FD);
		return 1;
	}

	// Operations Based On File Type
	// Regular File
	if (REG_F) {
		// Copy File
		if (copy_file(SOURCE_FD, st, argv[optind+1]) == -1) {
			fprintf(stderr, "test");
			return 1;
		}
	}
	
	// Directory 
	else if (DIR_F) {
		printf("Directory\n");
	} 
	
	// Linked File
	else if (LNK_F) {
		printf("Symbolic Link\n");
	}

	// Finish
	return 0;
}

int get_file_type(char *path) {
	// Exit Status 
	int status = 0;

	// Create Stat Struct
	struct stat st; 

	// Grab Metadata Information
	if (lstat(path, &st) == -1) {
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
char *get_file_content(int fd, struct stat st) {
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
		free(content);
		perror("read");
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
	char *content = get_file_content(fd, st);
	if (content == NULL) {
		status = -1;
		return status;
	}
	
	// Make Copy of File
	int fd2;
	fd2 = open(path, O_RDWR | O_CREAT | O_TRUNC, st.st_mode);
	if (fd2 == -1) {
		free(content);
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
