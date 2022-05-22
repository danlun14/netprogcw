#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define _BUFSIZE 64

pthread_mutex_t my_lock;

void *reciever(void *rdsocket)
{
    char *buf = (char *)malloc(_BUFSIZE * sizeof(char));
    if (buf == NULL)
    {
        perror("Unable to allocate buffer");
        exit(1);
    }
    while (recv(*(int *)rdsocket, buf, _BUFSIZE, 0) != 0)
    {
        printf("%s\n", buf);
        fflush(stdout);
    }
    printf("Сервер завершил работу\n");
    pthread_exit(0);
}

int main()
{

    int rdsocket, wrsocket;
    struct sockaddr_in addr;

    rdsocket = socket(AF_INET, SOCK_STREAM, 0);
    if (rdsocket < 0)
    {
        perror("socket");
        exit(1);
    }

    wrsocket = socket(AF_INET, SOCK_STREAM, 0);
    if (wrsocket < 0)
    {
        perror("socket");
        exit(1);
    }

    char *msg = NULL;

    // printf("Type something: ");
    // size_t characters = getline(&msg, &bufsize, stdin);
    // msg[characters] = '\0';

    addr.sin_family = AF_INET;
    addr.sin_port = htons(1234);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    pthread_t thread;
    // send(sock, &number, sizeof(int), 0);

    msg = (char *)malloc(_BUFSIZE * sizeof(char));
    if (msg == NULL)
    {
        perror("Unable to allocate buffer");
        exit(1);
    }
    size_t bufsz = _BUFSIZE;
    printf("Введите имя: ");
    int characters = getline(&msg, &bufsz, stdin);

    if (connect(rdsocket, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("connect");
        exit(2);
    }

    if (connect(wrsocket, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("connect");
        exit(2);
    }
    pthread_create(&thread, NULL, reciever, (void *)&rdsocket);
    msg[characters - 1] = 0;
    characters = send(wrsocket, msg, characters + 1, 0);
    while (1)
    {
        characters = 0;
        characters = getline(&msg, &bufsz, stdin);
        msg[characters] = 0;
        characters = send(wrsocket, msg, characters + 1, 0);
    }
    free(msg);
    close(rdsocket);
    close(wrsocket);

    return 0;
}