#include <stdio.h>

int main(int argc, char **argv) {
	// Test
	if (argc < 2) {
		printf("No Flags Entered\n");
		return 1;
	} else {
		for (int i = 1; i < argc; i++) { printf("%s ", argv[i]); }
		printf("\n");
	}
	return 0;
}
