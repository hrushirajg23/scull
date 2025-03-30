#include<linux/cdev.h>

#define Q_SIZE 400
#define ENTRIES_PER_SET 10

struct scull_qset{
    void** data;
    struct scull_qset* next;
};

struct scull_dev{
    struct scull_qset* head;
    struct scull_qset* tail;
    int quantum;    //keep track of current index in current qset
    int qset;   //keep track of current set in list
    unsigned long total_size; //in terms of 400 
    unsigned int access_key;
    struct semaphore sem;
    struct cdev cdev;
};

