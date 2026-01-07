#include "arena_v0.5.c"
#include <dirent.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

typedef struct {
    char*  data;
    size_t size;
} string;

typedef struct {
    pthread_t* threads;
    size_t     threads_i;

    string* files;
    size_t  szfiles;

    // Directory path queue, any file paths go to `files`, any directories
    // end up back in `dirq`
    string* dirs;
    size_t  szdirs;
    size_t  dirs_pend;

    pthread_mutex_t lock;
    pthread_cond_t  signal;

    int kill;

} thread_data_t;

void* worker_process(string* dir_path)
{

    fprintf(stdout,
            "String: s:%s, sl:%zu:, zu:%zu\n",
            dir_path->data,
            strlen(dir_path->data),
            dir_path->size);

    DIR* dir = opendir(dir_path);

    if(!dir) {
        return NULL;
    }

    struct dirent* entry;

    size_t file_count = 0;
    // char**         name_buf;
    char* name_buf[256];
    // memset(name_buf, 0, sizeof(char*)32);
    //

    char z[32][256];
    z[31][256];

    char* x[2] = { &file_count, &name_buf };

    size_t y = sizeof(name_buf);
    for(int i = 0; i < 32; ++i) {
        // name_buf[i] =
    }

    fflush(stdout);
    usleep(100000);
    return NULL;
}

void* worker_entry(void* opt)
{
    thread_data_t* td = (thread_data_t*)opt;
    printf("Spawned Thread\n");

    int kill = 0;

    while(kill == 0) {
        pthread_mutex_lock(&td->lock);
        printf("Locked Mutex\n");

        while(td->dirs_pend == 0 && td->kill == 0) {
            pthread_cond_wait(&td->signal, &td->lock);
            printf("cond signal received\n");
        }

        if(td->kill != 0) {
            printf("kill signal received\n");
            kill = 1;
        }

        if(td->dirs_pend > 0 && td->dirs->size > 0 && td->dirs->data &&
           td->kill == 0) {
            printf("PEND:%zu, szDIR:%zu, d:%p, k:%d\n",
                   td->dirs_pend,
                   td->dirs->size,
                   td->dirs->data,
                   td->kill);

            // copy data needed for op so we can release lock
            string* dir = &td->dirs[td->dirs_pend - 1];

            string clone = { .size = dir->size, .data = NULL };
            clone.data   = malloc(dir->size + 1);
            memset(clone.data, 0, dir->size + 1);
            memcpy(clone.data, dir->data, dir->size);

            printf("Cloned the data\n");

            if(!memcmp(clone.data, dir->data, dir->size)) {
                printf("Buffers match\n");
            } else {
                printf("Buffers DO NOT match\n");
            }

            free(dir->data); // allocated in main but not freed, we free
            td->dirs_pend--;
            pthread_mutex_unlock(&td->lock);
            worker_process(&clone);
            free(clone.data);
            fflush(stdout);
        } else {
            pthread_mutex_unlock(&td->lock);
        }

        //  actual work here
        //
        // for(size_t i = 0; i < td->dirs_pend; i++) {
        // }
        //
        // //

        printf("cond signal received\n");

        if(td->kill != 0) {
            kill = 1;
            break;
        }

        printf("mutex unlocked\n");
        fflush(stdout);
    }
    fflush(stdout);

    printf("exiting thread\n");
    return NULL;
}

void foo(void)
{

    thread_data_t td;
    memset(&td, 0, sizeof(thread_data_t));
    td.threads_i = 2;
    td.threads   = malloc(sizeof(pthread_t) * td.threads_i);
    td.szfiles   = 256;
    td.files     = malloc(sizeof(string) * td.szfiles);
    td.szdirs    = 32;
    td.dirs      = malloc(sizeof(string) * td.szdirs);
    td.dirs_pend = 0;

    assert(pthread_mutex_init(&td.lock, NULL) == 0);
    assert(pthread_cond_init(&td.signal, NULL) == 0);

    for(size_t i = 0; i < td.threads_i; ++i) {
        pthread_t* pt = &td.threads[i];
        pthread_create(pt, NULL, worker_entry, &td);
    }

    while(1) {
        char buf[1];
        printf("Enter..q to quit\n");
        // fscanf(stdin, "%s");
        fread(buf, 1, 1, stdin);
        if(buf[0] == 'j') {
            pthread_mutex_lock(&td.lock);

            td.dirs_pend++;
            string* s = &td.dirs[td.dirs_pend - 1];

            const char* m   = "Test String";
            size_t      szm = strlen(m) + 1;

            char* sptr = malloc(szm);
            memset(sptr, 0, szm);
            memcpy(sptr, m, szm);
            s->data = sptr;
            s->size = szm;

            pthread_mutex_unlock(&td.lock);

            pthread_cond_signal(&td.signal);
        }
        if(buf[0] == 'q')
            break;

        usleep(10000);
    }

    printf("Killing..\n");
    td.kill = 1;

    for(size_t i = 0; i < td.threads_i; ++i) {
        pthread_cond_broadcast(&td.signal);
        pthread_t* pt = &td.threads[i];
        pthread_join(*pt, NULL);
    }

    printf("Joined threads\n");

    pthread_mutex_destroy(&td.lock);
    pthread_cond_destroy(&td.signal);

    printf("Destroyed syncs\n");
    free(td.files);
    free(td.dirs);
    free(td.threads);

    printf("Memory freed\n");
}

int main(void)
{
    foo();
    return 0;
}
