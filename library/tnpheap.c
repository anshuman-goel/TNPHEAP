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
#include <time.h>

#define TNPHEAP_IOCTL_COMMIT_LOCK  _IOWR('N', 0x50, struct tnpheap_cmd)
#define TNPHEAP_IOCTL_COMMIT_UNLOCK  _IOWR('N', 0x51, struct tnpheap_cmd)

struct user_ll
{
  struct tnpheap_cmd cmd;
  void *data;
  char buffer[8192];
  struct user_ll *next;
}*head=NULL;

struct tnpheap_cmd cmd;
__u64 txid;

__u64 tnpheap_get_version(int npheap_dev, int tnpheap_dev, __u64 offset)
{
        struct user_ll *temp=head;
        while(temp!=NULL)
        {
          if(temp->cmd.offset==offset)
          {
            return ioctl(tnpheap_dev, TNPHEAP_IOCTL_GET_VERSION, &(temp->cmd));
          }
          temp = temp->next;
        }

        return 0;
}



int tnpheap_handler(int sig, siginfo_t *si)
{
        return 0;
}


void *tnpheap_alloc(int npheap_dev, int tnpheap_dev, __u64 offset, __u64 size)
{
        struct user_ll *temp=head, *prev=NULL;
        while(temp!=NULL)
        {
          if(temp->cmd.offset==offset)
          {
            temp->cmd.version = tnpheap_get_version(npheap_dev, tnpheap_dev, offset);
            char *ptr;
            ptr = npheap_alloc(npheap_dev, offset, 8192);
            for(int i=0;i<8192;i++)
            {
              temp->buffer[i]=ptr[i];
            }
            int try=1000;
            while(try>0)
            {
              if (strcmp(npheap_alloc(npheap_dev, offset, 8192), temp->buffer)!=0)
              {
                memcpy(temp->buffer, npheap_alloc(npheap_dev, offset, 8192), 8192);
                // printf("gadbad");
                try--;
                // return 1;
              }
              else
                break;
          }
            return temp->buffer;
          }
          prev = temp;
          temp = temp->next;
        }
        if(temp==NULL)
        {
          struct user_ll *new;
          new = (struct user_ll*)malloc(sizeof(struct user_ll));
          new->cmd.offset = offset;
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
          char *ptr;
          ptr = npheap_alloc(npheap_dev, offset, 8192);
          for(int i=0;i<8192;i++)
          {
            new->buffer[i]=ptr[i];
          }
          int try=1000;
          while(try>0)
          {
            if (strcmp(npheap_alloc(npheap_dev, offset, 8192), new->buffer)!=0)
            {
              memcpy(new->buffer, npheap_alloc(npheap_dev, offset, 8192), 8192);
              try--;
               // printf("gadbad");
              // return 1;
            }
            else
              break;
        }
        }
        if (prev!=NULL)
        {
          //return prev->next->cmd.data;
          return prev->next->buffer;
        }
        else
        {
          //return head->cmd.data;
          return head->buffer;
        }
}

__u64 tnpheap_start_tx(int npheap_dev, int tnpheap_dev)
{
      ioctl(tnpheap_dev, TNPHEAP_IOCTL_COMMIT_LOCK, &(cmd));
        txid = ioctl(tnpheap_dev, TNPHEAP_IOCTL_START_TX, &cmd);
        sleep(1);
        unsigned long long msec_time;
        struct timeval current_time;
        gettimeofday(&current_time,NULL);
        msec_time = current_time.tv_usec + current_time.tv_sec*1000000;
        // printf("Tranx id %lu for pid %lu %llu\n", txid, getpid(), msec_time);
        return txid;
}

int tnpheap_commit(int npheap_dev, int tnpheap_dev)
{
        struct user_ll *temp=head, *temp2=head;
        // Assuming all process requests the offset in same order which isn't.
        // Acquire lock on all offsets using npheap_lock
       // printf("All locks acquired head %lu pid %lu\n", head->cmd.offset, getpid());
        // Do commit work
        temp=head;
        while(temp!=NULL)
        {
            // printf("inside while");
           if (strcmp(npheap_alloc(npheap_dev, temp->cmd.offset, 8192), temp->buffer)!=0)
            if (ioctl(tnpheap_dev, TNPHEAP_IOCTL_COMMIT, &(temp->cmd))==1)
            {
             // printf("All locks released pid %lu\n", getpid());
	           ioctl(tnpheap_dev, TNPHEAP_IOCTL_COMMIT_UNLOCK, &(cmd));
             return 1;
            }

            // printf("doing manual copy");
            char *ptr;
            ptr = npheap_alloc(npheap_dev, temp->cmd.offset, 8192);
            for(int i=0;i<8192;i++)
            {
              ptr[i]=temp->buffer[i];
            }
          int try=1000;
          while(try>0)
          {
            if (strcmp(npheap_alloc(npheap_dev, temp->cmd.offset, 8192), temp->buffer)!=0)
            {
               memcpy(npheap_alloc(npheap_dev, temp->cmd.offset, 8192), temp->buffer, 8192);
              // printf("Data not matched for offset %d pid %lu\n", temp->cmd.offset, getpid());
               try--;
              // printf("gadbad2");
              ioctl(tnpheap_dev, TNPHEAP_IOCTL_COMMIT_UNLOCK, &(cmd));
              return 1;
            }
            else
              break;
        }
          // saving data in
          temp = temp->next;
        }
        // Release lock

        // printf("commit %d\n",getpid());
	       ioctl(tnpheap_dev, TNPHEAP_IOCTL_COMMIT_UNLOCK, &(cmd));
        return 0;
}
