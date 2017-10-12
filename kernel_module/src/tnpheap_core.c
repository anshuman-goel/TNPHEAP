//////////////////////////////////////////////////////////////////////
//                             North Carolina State University
//
//
//
//                             Copyright 2016
//
////////////////////////////////////////////////////////////////////////
//
// This program is free software; you can redistribute it and/or modify it
// under the terms and conditions of the GNU General Public License,
// version 2, as published by the Free Software Foundation.
//
// This program is distributed in the hope it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
// more details.
//
// You should have received a copy of the GNU General Public License along with
// this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
//
////////////////////////////////////////////////////////////////////////
//
//   Author:  Hung-Wei Tseng
//
//   Description:
//     Skeleton of NPHeap Pseudo Device
//
////////////////////////////////////////////////////////////////////////

#include "tnpheap_ioctl.h"

#include <asm/uaccess.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/poll.h>
#include <linux/mutex.h>
#include <linux/time.h>

struct miscdevice tnpheap_dev;
DEFINE_MUTEX(lock);
DEFINE_MUTEX(linklist);
__u64 trxid=0;
struct ll
{
  tnpheap_cmd *node;
  ll *next;
} *head=NULL;

__u64 tnpheap_get_version(struct tnpheap_cmd __user *user_cmd)
{
    struct tnpheap_cmd cmd;
    printk(KERN_ERR "Kern tnpheap_get_version\n");
    if (copy_from_user(&cmd, user_cmd, sizeof(cmd)))
    {
        return -1 ;
    }
    mutex_lock(&linklist);
    ll *temp=head, *prev=NULL;
    while(temp!=NULL)
    {
      if(temp->node->offset == user_cmd->offset)
      {
        mutex_unlock(&linklist);
        return temp->node->version;
      }
      prev = temp;
      temp=temp->next;

    }
    if (temp==NULL)
    {
      struct ll new;
      new->node = &cmd;
      new->next = NULL;
      new->version = 0;
      if(head==NULL)
      {
        head=&new;
      }
      else
      {
        prev->next = &new;
      }
    }
    mutex_unlock(&linklist);
    return 0;
}

__u64 tnpheap_start_tx(struct tnpheap_cmd __user *user_cmd)
{
    struct tnpheap_cmd cmd;
    __u64 ret=0;
    printk(KERN_ERR "Kern tnpheap_start_tx\n");
    if (copy_from_user(&cmd, user_cmd, sizeof(cmd)))
    {
        return -1 ;
    }
    mutex_lock(&lock);
    trxid++;
    ret=trxid;
    mutex_unlock(&lock);
    printk(KERN_ERR "Kern tnpheap_start_tx id %lu\n",ret);
    return ret;
}

__u64 tnpheap_commit(struct tnpheap_cmd __user *user_cmd)
{
    struct tnpheap_cmd cmd;
    __u64 ret=0;
    printk(KERN_ERR "Kern tnpheap_commit\n");
    if (copy_from_user(&cmd, user_cmd, sizeof(cmd)))
    {
        return -1 ;
    }
    mutex_lock(&linklist);
    struct ll *temp=head;
    while(temp!=NULL)
    {
      if(temp->node->offset==cmd->offset)
      {
        if(temp->node->version==cmd->version)
        {
          temp->node->version++;
          mutex_unlock(&linklist);
          return ret;
        }
      }
    }
    mutex_unlock(&linklist);
    return 1;
}



__u64 tnpheap_ioctl(struct file *filp, unsigned int cmd,
                                unsigned long arg)
{
    //printk(KERN_ERR "Kern tnpheap_ioctl\n");
    switch (cmd) {
    case TNPHEAP_IOCTL_START_TX:
        return tnpheap_start_tx((void __user *) arg);
    case TNPHEAP_IOCTL_GET_VERSION:
        return tnpheap_get_version((void __user *) arg);
    case TNPHEAP_IOCTL_COMMIT:
        return tnpheap_commit((void __user *) arg);
    default:
        return -ENOTTY;
    }
}

static const struct file_operations tnpheap_fops = {
    .owner                = THIS_MODULE,
    .unlocked_ioctl       = tnpheap_ioctl,
};

struct miscdevice tnpheap_dev = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "tnpheap",
    .fops = &tnpheap_fops,
};

static int __init tnpheap_module_init(void)
{
    int ret = 0;
    if ((ret = misc_register(&tnpheap_dev)))
        printk(KERN_ERR "Unable to register \"tnpheap\" misc device\n");
    else
        printk(KERN_ERR "\"tnpheap\" misc device installed\n");
    return 0;
}

static void __exit tnpheap_module_exit(void)
{
    misc_deregister(&tnpheap_dev);
    return;
}

MODULE_AUTHOR("Hung-Wei Tseng <htseng3@ncsu.edu>");
MODULE_LICENSE("GPL");
MODULE_VERSION("0.1");
module_init(tnpheap_module_init);
module_exit(tnpheap_module_exit);
