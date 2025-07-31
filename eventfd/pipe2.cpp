#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>

int main (int argc, char *argv[]) {
    int pipefd[2];
    if (pipe2(pipefd, 0) < 0) {
        perror("pipe2");
        exit(1);
    }

    write(pipefd[1], "hello world", 11);

    char buf[128] = {0};
    ssize_t ret;

    ret = read(pipefd[0], &buf, 5);
    printf("ret %d, buf %s\n", ret, buf);

    ret = read(pipefd[0], &buf, sizeof(buf));
    printf("ret %d, buf %s\n", ret, buf);

    close(pipefd[0]);
    close(pipefd[1]);
    return 0;
}
