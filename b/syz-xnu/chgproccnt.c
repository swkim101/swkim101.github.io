#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <errno.h>

#define UID1 0x1337

volatile int running = 1;

void *thread_setreuid_1(void *arg) {
    while (running) {
        if (setreuid(0, UID1) != 0) {
            perror("setreuid(0, UID1)");
            exit(1);
        }
    }
    return NULL;
}

void *thread_setreuid_2(void *arg) {
    while (running) {
        if (setreuid(UID1, 0) != 0) {
            perror("setreuid(UID1, 0)");
            exit(1);
        }
    }
    return NULL;
}

int main() {
    pthread_t t1, t2;

    if (geteuid() != 0) {
        fprintf(stderr, "This test must be run as root.\n");
        return 1;
    }

    printf("Starting setreuid race condition test (Ctrl+C to stop)...\n");

    if (pthread_create(&t1, NULL, thread_setreuid_1, NULL) != 0) {
        perror("pthread_create t1");
        return 1;
    }
    if (pthread_create(&t2, NULL, thread_setreuid_2, NULL) != 0) {
        perror("pthread_create t2");
        return 1;
    }

    sleep(10);
    running = 0;

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    printf("Test finished.\n");
    return 0;
}