#define _POSIX_C_SOURCE 200809L

#include <stdio.h> 
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <inttypes.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <string.h>

// Function Prototypes
int get_file_type(char *path);
int acopy_file(int fd, struct stat source_st, char *source_path, char *dest_path, int args);
int copy_file(int fd, struct stat source_st, char *path);
char *get_file_content(int fd, struct stat st);
int copy_directory(const char *path, int args);

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

	// File Descriptor and stat struct for File
	int SOURCE_FD;
	struct stat st;

	// Open Source File
	SOURCE_FD = open(argv[optind], O_RDONLY);

	// Throw Error If File Does Not Exist
	if (SOURCE_FD == -1) {
		perror("open");
		return 1;
	}

	// Grab Metadata From File
	if (fstat(SOURCE_FD, &st) == -1) {
		perror("fstat");
		close(SOURCE_FD);
		return 1;
	}
	
	// Determine File Type
	int file_type;
	if ((file_type = get_file_type(argv[optind])) == -1) {
		fprintf(stderr, "Invalid File Type. Enter either a regular file, directory, or a symbolic link.\n");
		close(SOURCE_FD);
		return 1;
	}

	// Operations Based On Source File Type
	// Regular File
	if (file_type == S_IFREG) {
		// Copy File Without Arguments
		if (args == 0) {
			if (copy_file(SOURCE_FD, st, argv[optind+1]) == -1) {
				fprintf(stderr, "testing error message\n");
				return 1;
			}
		}

		// Copy File With Arguments
		if (acopy_file(SOURCE_FD, st, argv[optind], argv[optind+1], args) == -1) {
			fprintf(stderr, "testing errorr message\n");
			return 1;
		}
	}
	
	// Directory 
	else if (file_type == S_IFDIR) {
		printf("Directory\n");
	} 
	
	// Linked File
	else if (file_type == S_IFLNK) {
		printf("Symbolic Link\n");
	}

	// Finish
	close(SOURCE_FD);
	return 0;
}

int get_file_type(char *path) {
	// Exit Status 
	int status = 0;

	// Create Stat Struct
	struct stat st; 

	// Grab Metadata 
	if (lstat(path, &st) == -1) {
		perror("stat");
		status = -1;
	}
	
	// Set Flags Based on Filetype
	if (S_ISREG(st.st_mode)) {
		return S_IFREG;
	} else if (S_ISDIR(st.st_mode)) {
		return S_IFDIR;
	} else if (S_ISLNK(st.st_mode)) {
		return S_IFLNK;
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
int copy_file(int fd, struct stat source_st, char *path) {
	// Exit Status Variable
	int status = 0;

	int fd2;
	struct stat st;

	// Get File Content of Source File
	char *content = get_file_content(fd, source_st);
	if (content == NULL) {
		status = -1;
		return status;
	}
	
	// Make Copy of File
	fd2 = open(path, O_RDWR | O_CREAT | O_TRUNC, source_st.st_mode);
	if (fd2 == -1) {
		free(content);
		fprintf(stderr, "Missing DEST File\n");
		fprintf(stderr, "Usage: ./run <SOURCE-PATH> <DESTINATION-PATH>\n");
		status = -1;
		return status;
	}

	// Copy Permissions Of File 
	if (fstat(fd2, &st) == -1) {
		perror("fstat");
		close(fd2);
		status = -1;
		return status;
	}

	// Write Contents to File
	ssize_t bytes_written = write(fd2, content, source_st.st_size);
	if (bytes_written == -1) {
		perror("write");
		status = -1;
		return status;
	}

	// Free Memory & Close Files
	free(content);
	close(fd2);
	
	return status;
}

// File Function That Applies Argument Flags 
int acopy_file(int fd, struct stat source_st, char *source_path, char *dest_path, int args) {
	// Exit Status Variable
	int status = 0;
	
	// File Descriptor for Copied File
	int fd2;

	// Create File If No Flags
	if (args == 0) { 
		if (copy_file(fd, source_st, dest_path) == -1) {
			fprintf(stderr, "testing error message\n");
			status = -1;
			return status;
		}
	}

	// ---------- Check for Entered Flags -----------
	else {
		// l_FLAG
		if ((args & l_FLAG) == l_FLAG) {
			// Create a Hard Link to Source
			if (link(source_path, dest_path) == -1) {
				perror("link error"); 
				status = -1;
				return status;
			}
		}

		// p_FLAG
		if ((args & p_FLAG) == p_FLAG) {
			// Create Copy of File 
			if (copy_file(fd, source_st, dest_path) == -1) {
				fprintf(stderr, "testing error message\n");
				status = -1;
				return status;
			}

			// Re-open Copied File 
			fd2 = open(dest_path, O_RDWR | O_CREAT, source_st.st_mode);

			// Copy Ownerships
			uid_t uid = source_st.st_uid;
			gid_t gid = source_st.st_gid;

			if (chown(dest_path, uid, gid) == -1) {
				perror("chown");
				status = -1;
				return status;
			}

			// Copy Timestamps
			struct timespec new_times[2];
			new_times[0].tv_sec = source_st.st_atim.tv_sec; 
			new_times[0].tv_nsec = source_st.st_atim.tv_nsec;
			new_times[1].tv_sec = source_st.st_mtim.tv_sec;
			new_times[1].tv_nsec = source_st.st_mtim.tv_nsec; 

			// Function that Copies Timestamps
			if (futimens(fd2, new_times) == -1) {
				perror("futimens");
				close(fd2);
				status = -1;
				return status;
			}
		}
	}

	// Close Copied File
	close(fd2);
	return status;
}

// Function to Copy Directories
int copy_directory(const char *path, int args) {
	// Status Variable
	int status = 0;

	// Initialize Directory
	DIR *dp;
	struct dirent *entry;
	struct stat st;

	// Base Case When Opening Directory
	if ((dp = opendir(path)) == NULL) {
		perror("opendir");
		return -1;
	}

	// Iterate Through Directory Entries
	while ((entry = readdir(dp)) != NULL) {
		// Skip Current and Parent Directory 
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
			continue;
		}
		
		// Store Entry Data in Stat Struct
		if (stat(entry->d_name, &st) == -1) {
			status = -1;
			break;
		}

		// Check If Entry is a Directory
		
	}

	return status;
}
