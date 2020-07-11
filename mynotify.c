#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <string.h>

#define BUFSIZE 1024 * 1024

/* Display information about watch descriptors and events of files
   and directories being watched.  
*/
static void
displayHandledEvents(struct inotify_event *i){
    printf("  *** Watch Descriptor = %2d ***\n", i->wd);
    if (i->cookie > 0){
        printf("Use this cookie for related items. Cookie=%4d; ", i->cookie);
    }

    if (i->mask & IN_ISDIR){
        printf("The directory ");
    } else{
        printf("The file ");
    }
    if (i->len > 0){
        printf("%s ", i->name);
    }

    if (i->mask & IN_ACCESS)          printf("has been opened and read ");
    if (i->mask & IN_ATTRIB)          printf("has it's metadata changed ");
    if (i->mask & IN_CLOSE_WRITE)     printf("opened for editing has been closed ");
    if (i->mask & IN_CLOSE_NOWRITE)   printf("opened for reading has been closed ");
    if (i->mask & IN_CREATE)          printf("was created inside watched directory ");
    if (i->mask & IN_DELETE)          printf("was deleted from watched directory ");
    if (i->mask & IN_DELETE_SELF)     printf("was deleted ");
    if (i->mask & IN_IGNORED)         printf("has been removed from watched by kernel or app ");
    if (i->mask & IN_ISDIR)           printf("is a directory and not a file ");
    if (i->mask & IN_MODIFY)          printf("was modified ");
    if (i->mask & IN_MOVE_SELF)       printf("was itself moved ");
    if (i->mask & IN_MOVED_FROM)      printf("was moved from the directory being watched ");
    if (i->mask & IN_MOVED_TO)        printf("was moved into the watched directory ");
    if (i->mask & IN_OPEN)            printf(" was opened ");
    if (i->mask & IN_Q_OVERFLOW)      printf("has caused an overflow on the event queue ");
    if (i->mask & IN_UNMOUNT)         printf("has it's file system unmounted ");
    printf("\n");
}

int
main(int argc, char *argv[])
{
    // Declare variables to hold file and watch descriptors
    int fd, wd, i;
    char buf[BUFSIZE] __attribute__ ((aligned(__alignof__(struct inotify_event))));
    char *ptr;
    ssize_t eventRead;

    /* A struct to hold the events returned from a watch descriptor for a particular file
       or directory declared on a file descriptor.
    */   
    struct inotify_event *event; 

    if (argc < 2){
        printf("Usage: %s PATH [PATH ...]\n");
        exit(EXIT_FAILURE);
    }

    printf("Press the ENTER key to terminate.\n");
    // Create an inotify instance and store the file descriptor returned for the instance.
    // The file descriptor is used to refer to an inotify instance.
    fd = inotify_init();
    if (fd == -1){
        perror("inotify_init");
        exit(EXIT_FAILURE);
    }
    // For each argument(file or directory), add a watch descriptor to the watch list.
    for (i = 0; i < argc; i++){
        wd = inotify_add_watch(fd, argv[i], IN_ALL_EVENTS);
        if (wd == -1){
            fprintf(stderr, "Cannot watch '%s': %s\n", argv[i], strerror(errno));
            exit(EXIT_FAILURE);
        } else{
            printf("Watch descriptior %2d is watching events for %s \n", wd, argv[i]);
        }
    }
    // Read events indefinitely
    for (;;){
        eventRead = read(fd, buf, BUFSIZE);
        if (eventRead == 0){
            printf("read() returned 0 events");
            break;
        }
        if (eventRead == -1 && errno != EAGAIN){
            perror("read");
            exit(EXIT_FAILURE);
        }
        printf("Read %ld from inotify file descriptor \n", (long) eventRead);

        for (ptr = buf; ptr < buf + eventRead; ){
            event = (struct inotify_event *) ptr;
            displayHandledEvents(event);

            ptr += sizeof(struct inotify_event) + event->len;
        }
    }
    exit(EXIT_SUCCESS);
}