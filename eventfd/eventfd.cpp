#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>

int main (int argc, char *argv[]) {
    uint64_t buf = 3;
    ssize_t ret;

    int efd = eventfd(2, EFD_NONBLOCK|EFD_SEMAPHORE);
    ret = write(efd, &buf, sizeof(buf));

    ret = read(efd, &buf, sizeof(uint64_t));
    printf("ret %d, buf %llu\n", ret, buf);

    ret = read(efd, &buf, sizeof(uint64_t));
    printf("ret %d, buf %llu\n", ret, buf);

    ret = read(efd, &buf, sizeof(uint64_t));
    printf("ret %d, buf %llu\n", ret, buf);
    ret = read(efd, &buf, sizeof(uint64_t));
    printf("ret %d, buf %llu\n", ret, buf);

    ret = read(efd, &buf, sizeof(uint64_t));
    printf("ret %d, buf %llu\n", ret, buf);
    ret = read(efd, &buf, sizeof(uint64_t));
    printf("ret %d, buf %llu\n", ret, buf);
    ret = read(efd, &buf, sizeof(uint64_t));
    printf("ret %d, buf %llu\n", ret, buf);
    close(efd);
    return 0;
}
