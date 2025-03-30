
#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/fs.h>
#include<linux/init.h>
#include<linux/cdev.h>
#include"scull.h"

dev_t dev;

static struct scull_fops fops={
    .open=scull_open
};

static struct scull_dev sc_dev;

static void scull_setup_cdev(struct scull_dev* dev,int minor){
    int result=0;
    result=alloc_chrdev_region(&dev,minor,1,"scull");
    if(result < 0){
        printk(KERN_WARNING "Can't get major number for device\n");
        return result;
    }
    
    /*
        initialised the character driver structure,
        file table madhe file opsration add hotat. 
        Means file table madhla fops* will point to out fops
    */
    cdev_init(&dev->cdev,&scull_fops);
    dev->cdev.owner=THIS_MODULE;
    dev->cdev.ops=&scull_fops;

    /*
        Here our device (dev) becomes assscoiciated 
        with our character driver (char_dev)
        count = number of device numbers that should be associated with the device 
    */
    result=cdev_add(&dev->cdev,dev,1);
    if(result)
    printk(KERN_NOTICE "Error %d addding scull %d ",result,minor);
}
/*
    Open call is used in drivers for following thins:
        1) check device specific errors , initialisation problem etc
        2) Initialise the device if being opened first ime
        3) update the f_op pointer(i.e file operations) , if necesary
        4) allocate and fill any neccessary data struictues in filp->private_data



*/


void scull_trim(struct scull_dev* dev){
    struct scull_qset* curr=NULL,*ahead=NULL;
    curr=dev->head;
    while(curr!=NULL){
        if(curr->data!=NULL){
            for(int i=0;i<Q_SIZE;i++){
                kfree(curr->data[i]);
            }
            kfree(curr->data);
            curr->data=NULL;
        }
        ahead=curr->next;
        kfree(curr);
    }
}



int scull_open(struct inode* inode,struct file* filp){
    struct scull_dev* dev;
    dev=container_of(inode->i_cdev,struct scull_dev,cdev);
    filp->private_data=dev;

    if((filp->flags & O_ACCMODE)==O_WRONLY){
        scull_trim(dev);
    }
    return 0;
}


ssize_t scull_read(struct file* filp,char __user* buf,size_t count,loff_t* fpos){
    struct scull_dev* dev=filp->private_data;
    struct scull_qset* dptr;
    int quantum=dev->quantum,qset=dev->qset;
    ssize_t retval=0;

    if(down_interr)

}


void allocate_qset(struct scull_dev* dev){
    struct scull_qset* qset=kmalloc(sizeof(struct scull_qset));
    qset->next=NULL;
    qset->data=kmalloc(sizeof(char*)*ENTRIES_PER_SET,GFP_KERNEL);
    if(dev->head==NULL && dev->tail==NULL){
        dev->head=qset;
        dev->tail=qset;
    }
    else{
        dev->tail->next=qset;
        dev->tail=qset;
    }
}
/*
    given a set number reach till it element wise in list
*/
struct scull_qset* follow_qset(struct scull_dev* dev){
    int num=dev->qset;
    struct scull_qset* dptr=(struct scull_qset*)dev->head;
    while(num--){
        dptr=dptr->next;
    }
    return dptr;
}

ssize_t scull_write(struct file* filp,const char __user* buf,size_t count,loff_t* f_pos){
    struct scull_dev* dev=filp->private_data;
    struct scull_qset* dptr=NULL;
    int quantum=dev->quantum,qset=dev->qset;
    int index=0;
    ssize_t user_wants=count;
    ssize_t retval=-ENOMEM;

    if(down_interruptible(&dev->sem))
        return -ERESTARTSYS;
    
    /*
        Scenario 1: count given by user fits within the block of preallocated quantum
    */    
    if(count<=0){
        return 0;
    }
    else if((Q_SIZE)-(long)(*f_pos)%Q_SIZE>= count){
        *fpos+=count;
        count=0;
    }
    else{
        *fpos+=(Q_SIZE)-(long)(*f_pos)%Q_SIZE;
        count-=(Q_SIZE)-(long)(*f_pos)%Q_SIZE;
        
    }
    dptr=follow_qset(dev); 
    quantum++;

    /*
        scenario 2: count doesn't fit in prev allocated block , need more blocks
    */
    while(count > 0){
        index=((long)(*fpos)/Q_SIZE)%ENTRIES_PER_SET;
        
        /* scenario 3: no room in previous qset, hence new set needs to be allocated */
        if(quantum > ENTRIES_PER_SET){
            allocate_qset(dev);
            dev->qset++;
            dptr=follow_qset(dev);
            dev->quantum=0;
            quantum=0;
        }
        if(count >= Q_SIZE){
            *fpos += (*fpos)+Q_SIZE;
        }
        else{
            *fpos+=count;
        }
        count=count-Q_SIZE;
        dptr[index]=kmalloc(Q_SIZE,GFP_KERNEL);
        quantum++;
    }
    
    up(&dev->sem);
    return user_wants;


}

static void scull_disable(struct scull_dev* dev){
    unregister_chrdev_region(dev->dev,1);
    cdev_del(&dev->cdev);
}
static int __init scull_init(void){
    printk(KERN_INFO "initialised driver\n");
   scull_setup_cdev()

    return 0;
}



static void __exit scull_exit(void){
    printk(KERN_INFO "removing driver\n");
    scull_disable(&scull_dev);
}

module_init(scull_init);
module_exit(scull_exit);

MODULE_AUTHOR("hrushiraj gandhi");
MODULE_LICENSE("GPL");

