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

struct user_ll
{
  struct tnpheap_cmd cmd;
  void *data;
  // char buffer[8192];
  // __u64 offset;
  struct user_ll *next;
}*head=NULL;

struct tnpheap_cmd cmd;
__u64 version;

__u64 tnpheap_get_version(int npheap_dev, int tnpheap_dev, __u64 offset)
{
        printf("Library tnpheap_get_version pid %lu\n", getpid());
        struct user_ll *temp=head;
        while(temp!=NULL)
        {
          if(temp->cmd.offset==offset)
          {
            return ioctl(tnpheap_dev, TNPHEAP_IOCTL_GET_VERSION, &(temp->cmd));
          }
          temp = temp->next;
        }
}



int tnpheap_handler(int sig, siginfo_t *si)
{
        printf("Library tnpheap_handler\n");
        return 0;
}


void *tnpheap_alloc(int npheap_dev, int tnpheap_dev, __u64 offset, __u64 size)
{
        printf("Library tnpheap_alloc pid %lu\n", getpid());
        struct user_ll *temp=head, *prev=NULL;
        while(temp!=NULL)
        {
          if(temp->cmd.offset==offset)
          {
            return temp->cmd.data;
          }
          prev = temp;
          temp = temp->next;
        }
        if(temp==NULL)
        {
          struct user_ll *new;
          new = (struct user_ll*)malloc(sizeof(struct user_ll));
          new->cmd.offset = offset;
          new->cmd.data = (char*)malloc(8192*sizeof(char));
          new->cmd.size = size;
          new->next = NULL;
          if(head==NULL)
          {
            head=new;
          }
          else
          {
            prev->next=new;
          }
          new->cmd.version = tnpheap_get_version(npheap_dev, tnpheap_dev, offset);
          new->data = npheap_alloc(npheap_dev, offset, 8192);
        }
        //cmd.data =
        //exit(0);
        return prev->next->cmd.data;
        // cmd.offset = offset;
        // cmd.size = size;
        // cmd.version = tnpheap_get_version(npheap_dev, tnpheap_dev, cmd.offset);
        // printf("Offset %lu of size %lu with version %lu pid %lu\n", cmd.offset, cmd.size, cmd.version, getpid());
        // if(buffer!=NULL)
        //   free(buffer);
        // buffer=calloc(1, 8192);
        // printf("Memcopy with size %lu vs %lu pid %lu\n", npheap_getsize(npheap_dev, offset), cmd.size, getpid());
        // //memcpy(buffer, cmd.data, 8192);
        // //printf("Copied into buffer\n");
        // return buffer;
}

__u64 tnpheap_start_tx(int npheap_dev, int tnpheap_dev)
{
        printf("Library tnpheap_start_tx pid %lu\n", getpid());
        __u64 id = ioctl(tnpheap_dev, TNPHEAP_IOCTL_START_TX, &cmd);
        printf("Tranx id %lu for pid %lu\n", id, getpid());
}

int tnpheap_commit(int npheap_dev, int tnpheap_dev)
{
        printf("Library tnpheap_commit pid %lu\n", getpid());
        struct user_ll *temp=head, *temp2=head;
        //exit(0);
        // Assuming all process requests the offset in same order which isn't.
        // Acquire lock on all offsets using npheap_lock
        while(temp!=NULL)
        {
          npheap_lock(npheap_dev, temp->cmd.offset);
          temp = temp->next;
        }
        // Do commit work
        temp=head;
        while(temp!=NULL)
        {
            if (ioctl(tnpheap_dev, TNPHEAP_IOCTL_COMMIT, &(temp->cmd))==1)
            {
              while(temp2!=NULL)
              {
                npheap_lock(npheap_dev, temp2->cmd.offset);
                temp2 = temp2->next;
              }
              return 1;
            }
          memcpy(temp->data, temp->cmd.data, temp->cmd.size);
          temp = temp->next;
        }
        // Release lock
        while(temp2!=NULL)
        {
          npheap_lock(npheap_dev, temp2->cmd.offset);
          temp2 = temp2->next;
        }
        return 0;
        // printf("Offest for cmd %lu pid %lu\n", cmd.offset, getpid());
        // if (ioctl(tnpheap_dev, TNPHEAP_IOCTL_COMMIT, &cmd)==0)
        // {
        //   printf("Updating value pid %lu\n",getpid());
        //   memcpy(cmd.data, buffer, cmd.size);
        //   return 0;
        // }
        // printf("Commit failed pid %lu\n", getpid());
        // return 1;
}
