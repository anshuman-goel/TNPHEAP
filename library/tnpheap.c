////////////////////////////////////////////////////////////////////////
//
//   Author:
//
//	Anshuman Goel	agoel5
//	Bhushan Thakur	bvthakur
//	Zubin Thampi	zsthampi
//
//   Description:
//     TNPHeap Pseudo Device
//
////////////////////////////////////////////////////////////////////////

#include <npheap/tnpheap_ioctl.h>
#include <npheap/npheap.h>
#include <npheap.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <signal.h>
#include <malloc.h>
#include <string.h>

void *buffer=NULL, *data=NULL;
struct tnpheap_cmd cmd;
__u64 version;

__u64 tnpheap_get_version(int npheap_dev, int tnpheap_dev, __u64 offset)
{
        printf("Library tnpheap_get_version pid %lu\n", getpid());
        return ioctl(tnpheap_dev, TNPHEAP_IOCTL_GET_VERSION, &cmd);
}



int tnpheap_handler(int sig, siginfo_t *si)
{
        printf("Library tnpheap_handler\n");
        return 0;
}


void *tnpheap_alloc(int npheap_dev, int tnpheap_dev, __u64 offset, __u64 size)
{
        printf("Library tnpheap_alloc pid %lu\n", getpid());
        cmd.data = npheap_alloc(npheap_dev, offset, 8192);
        cmd.offset = offset;
        cmd.size = size;
        cmd.version = tnpheap_get_version(npheap_dev, tnpheap_dev, cmd.offset);
        printf("Offset %lu of size %lu with version %lu pid %lu\n", cmd.offset, cmd.size, cmd.version, getpid());
        if(buffer!=NULL)
          free(buffer);
        buffer=calloc(1, 8192);
        printf("Memcopy with size %lu vs %lu pid %lu\n", npheap_getsize(npheap_dev, offset), cmd.size, getpid());
        memcpy(buffer, cmd.data, 8192);
        //printf("Copied into buffer\n");
        return buffer;
}

__u64 tnpheap_start_tx(int npheap_dev, int tnpheap_dev)
{
        printf("Library tnpheap_start_tx pid %lu\n", getpid());
        return ioctl(tnpheap_dev, TNPHEAP_IOCTL_START_TX, &cmd);
}

int tnpheap_commit(int npheap_dev, int tnpheap_dev)
{
        printf("Library tnpheap_commit pid %lu\n", getpid());
        printf("Offest for cmd %lu pid %lu\n", cmd.offset, getpid());
        if (ioctl(tnpheap_dev, TNPHEAP_IOCTL_COMMIT, &cmd)==0)
        {
          printf("Updating value pid %lu\n",getpid());
          memcpy(cmd.data, buffer, cmd.size);
          return 0;
        }
        printf("Commit failed pid %lu\n", getpid());
        return 1;
}
