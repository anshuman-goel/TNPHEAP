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

void *buffer, *data;
struct tnpheap_cmd cmd;
__u64 version;

__u64 tnpheap_get_version(int npheap_dev, int tnpheap_dev, __u64 offset)
{
        printf("Library tnpheap_get_version\n");
        return ioctl(tnpheap_dev, TNPHEAP_IOCTL_GET_VERSION, &cmd);
        return 0;
}



int tnpheap_handler(int sig, siginfo_t *si)
{
        printf("Library tnpheap_handler\n");
        return 0;
}


void *tnpheap_alloc(int npheap_dev, int tnpheap_dev, __u64 offset, __u64 size)
{
        printf("Library tnpheap_alloc\n");
        cmd.data = npheap_alloc(npheap_dev, offset, size);;
        cmd.offset = offset;
        cmd.size = size;
        cmd.version = tnpheap_get_version(npheap_dev, tnpheap_dev, offset);
        printf("Offset %lu of size %lu with version %lu\n", cmd.offset, cmd.size, cmd.version);
        buffer=calloc(1, size);
        memcpy(buffer, cmd.data, size);
        printf("Copied into buffer\n");
        return buffer;
}

__u64 tnpheap_start_tx(int npheap_dev, int tnpheap_dev)
{
        printf("Library tnpheap_start_tx\n");
        return ioctl(tnpheap_dev, TNPHEAP_IOCTL_START_TX, &cmd);
}

int tnpheap_commit(int npheap_dev, int tnpheap_dev)
{
        printf("Library tnpheap_commit\n");
        if (ioctl(tnpheap_dev, TNPHEAP_IOCTL_COMMIT, &cmd)==0)
        {
          memcpy(cmd.data, buffer, cmd.size);
          return 0;
        }
        return 1;
}
