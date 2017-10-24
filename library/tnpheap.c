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

#define TNPHEAP_IOCTL_COMMIT_LOCK  _IOWR('N', 0x50, struct tnpheap_cmd)
#define TNPHEAP_IOCTL_COMMIT_UNLOCK  _IOWR('N', 0x51, struct tnpheap_cmd)

struct user_ll
{
  struct tnpheap_cmd cmd;
  void *data;
  struct user_ll *next;
}*head=NULL;

struct tnpheap_cmd cmd;
__u64 txid;

__u64 tnpheap_get_version(int npheap_dev, int tnpheap_dev, __u64 offset)
{
      //  printf("Library tnpheap_get_version pid %lu\n", getpid());
        struct user_ll *temp=head;
        while(temp!=NULL)
        {
          if(temp->cmd.offset==offset)
          {
            //printf("offset found pid %lu\n", getpid());
            return ioctl(tnpheap_dev, TNPHEAP_IOCTL_GET_VERSION, &(temp->cmd));
          }
          temp = temp->next;
        }
}



int tnpheap_handler(int sig, siginfo_t *si)
{
        printf("Library tnpheap_handler pid %lu\n", getpid());
        return 0;
}


void *tnpheap_alloc(int npheap_dev, int tnpheap_dev, __u64 offset, __u64 size)
{
        //printf("Library tnpheap_alloc offset %d size %d pid %lu\n", offset, size, getpid());
        struct user_ll *temp=head, *prev=NULL;
        while(temp!=NULL)
        {
          if(temp->cmd.offset==offset)
          {
            temp->cmd.version = tnpheap_get_version(npheap_dev, tnpheap_dev, offset);
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
      //    printf("version assigned %d pid %lu\n", new->cmd.version, getpid());
          new->data = npheap_alloc(npheap_dev, offset, 8192);
          //printf("allocation done %lu\n", getpid());
        }
        if (prev!=NULL)
        return prev->next->cmd.data;
        else
        return head->cmd.data;
}

__u64 tnpheap_start_tx(int npheap_dev, int tnpheap_dev)
{
    //    printf("Library tnpheap_start_tx pid %lu\n", getpid());
        txid = ioctl(tnpheap_dev, TNPHEAP_IOCTL_START_TX, &cmd);
        printf("Tranx id %lu for pid %lu\n", txid, getpid());
        return txid;
}

int tnpheap_commit(int npheap_dev, int tnpheap_dev)
{
        // printf("Library tnpheap_commit pid %lu\n", getpid());
        struct user_ll *temp=head, *temp2=head;
        //exit(0);
        // Assuming all process requests the offset in same order which isn't.
        // Acquire lock on all offsets using npheap_lock
        // while(temp!=NULL)
        // {
        //   // printf("Lock acquire for offset %d pid %d\n", temp->cmd.offset, getpid());
        //   npheap_lock(npheap_dev, temp->cmd.offset);
        //   // printf("Lock acquired for offset %d pid %d\n", temp->cmd.offset, getpid());
        //   temp = temp->next;
        // }
        // npheap_lock(npheap_dev, head->cmd.offset);
        ioctl(tnpheap_dev, TNPHEAP_IOCTL_COMMIT_LOCK, &(cmd));
       printf("All locks acquired head %lu pid %lu\n", head->cmd.offset, getpid());
        // Do commit work
        temp=head;
        while(temp!=NULL)
        {
            if (ioctl(tnpheap_dev, TNPHEAP_IOCTL_COMMIT, &(temp->cmd))==1)
            {
              // while(temp2!=NULL)
              // {
              //   npheap_unlock(npheap_dev, temp2->cmd.offset);
              //   temp2 = temp2->next;
              // }
              // npheap_unlock(npheap_dev, head->cmd.offset);
             printf("All locks released pid %lu\n", getpid()); 
	     ioctl(tnpheap_dev, TNPHEAP_IOCTL_COMMIT_UNLOCK, &(cmd));
             return 1;
            }
          //printf("memcpy for offset %d pid %d\n", temp->cmd.offset, getpid());
          memcpy(temp->data, temp->cmd.data, 8192);
          temp = temp->next;
        }
        // Release lock
        temp2=head;
        // while(temp2!=NULL)
        // {
        //   npheap_unlock(npheap_dev, temp2->cmd.offset);
        //   temp2 = temp2->next;
        // }
        // npheap_unlock(npheap_dev, head->cmd.offset);
        printf("Commit pid %lu\n", getpid());
	ioctl(tnpheap_dev, TNPHEAP_IOCTL_COMMIT_UNLOCK, &(cmd));
        return 0;
}
