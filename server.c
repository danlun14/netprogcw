#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <arpa/inet.h>

#define _BUFSIZE 64

struct linked_listROOT
{
    struct linked_list *start;
};

struct linked_list
{
    struct linked_list *next;
    int socket;
};

struct linked_list *create_node(int socket)
{
    struct linked_list *node = malloc(sizeof(struct linked_list));
    if (node)
    {
        node->next = NULL;
        node->socket = socket;
        return node;
    }
    return NULL;
}

struct linked_listROOT *add_node(struct linked_list *start, struct linked_listROOT *root)
{

    start->next = root->start;
    root->start = start;
    return root;
}

struct linked_listROOT *delete_node(struct linked_listROOT *root, int socket_to_delete)
{
    struct linked_list *prev = root->start;
    struct linked_list *next = prev->next;

    if (prev->socket == socket_to_delete)
    {
        free(prev);
        root->start = next;
    }
    while (next)
    {
        if (next->socket == socket_to_delete)
        {
            prev->next = next->next;
            free(next);
        }
        prev = next;
        next = next->next;
    }
    return root;
}

void show_linked_node(struct linked_listROOT *root)
{
    struct linked_list *list = root->start;
    while (list)
    {
        printf("%d  ", list->socket);
        list = list->next;
    }
    printf("\n");
}

struct arguments_list
{
    int rdsocketcontrol;
    int nowrsocket;
    char *nick;
    struct linked_listROOT *root;
};

void *worker(void *args)
{
    int bytes_read = 0, bytes_send = 0;
    struct arguments_list *data = *(struct arguments_list **)args;
    int sock = data->rdsocketcontrol;
    int nosock = data->nowrsocket;
    struct linked_listROOT *root = data->root;
    show_linked_node(root);
    char *fullmsg = malloc(_BUFSIZE * sizeof(char));

    char *msg = (char *)malloc(_BUFSIZE * sizeof(char));
    if (msg == NULL)
    {
        perror("Unable to allocate buffer");
        exit(1);
    }
    while (1)
    {
        bytes_read = recv(sock, msg, _BUFSIZE, 0);

        struct linked_list *list = data->root->start;
        if (bytes_read == 0)
        {
            while (list)
            {
                if (list->socket != nosock)
                {
                    sprintf(fullmsg, "Пользователь %s завершил работу", data->nick);
                    bytes_send = send(list->socket, fullmsg, _BUFSIZE, 0);
                    printf("Отправлено: %d байт в сокет %d\n", bytes_send, list->socket);
                }

                list = list->next;
            }
            delete_node(root, nosock);
            free(msg);
            free(data->nick);
            free(fullmsg);
            close(sock);
            pthread_exit("thread is exit\n");
        }
        msg[bytes_read] = 0;
        printf("Принято %d байт на сокете %d от %s\n", bytes_read, sock, data->nick);
        printf("Принято сообщение: %s\n", msg);
        sprintf(fullmsg, "%s: %s", data->nick, msg);
        while (list)
        {
            if (list->socket != nosock)
            {

                bytes_send = send(list->socket, fullmsg, _BUFSIZE, 0);
                printf("Отправлено: %d байт в сокет %d\n", bytes_send, list->socket);
            }

            list = list->next;
        }
    }
}

int main()
{
    int rdsocket, wrsocket, listener;
    struct sockaddr_in addr, clientaddr;
    // struct sockaddr_in in_addr;
    // u_int32_t in_addr_size;
    // char buf[1024];
    // int bytes_read;

    listener = socket(AF_INET, SOCK_STREAM, 0);

    uint32_t addrlen = sizeof(clientaddr);
    if (listener < 0)
    {
        perror("socket");
        exit(1);
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(1234);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(listener, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("bind");
        exit(2);
    }

    listen(listener, 1);
    int *arg;
    struct linked_listROOT *wrlist = malloc(sizeof(struct linked_listROOT));

    while (1)
    {

        wrsocket = accept(listener, (struct sockaddr *)&clientaddr, &addrlen);
        if (wrsocket < 0)
        {
            perror("accept");
            exit(3);
        }
        printf("New connection from %s on socket %d\n", inet_ntoa(clientaddr.sin_addr), wrsocket);

        rdsocket = accept(listener, (struct sockaddr *)&clientaddr, &addrlen);
        if (rdsocket < 0)
        {
            perror("accept");
            exit(3);
        }
        printf("New connection from %s on socket %d\n", inet_ntoa(clientaddr.sin_addr), rdsocket);
        char *nick = malloc(_BUFSIZE * sizeof(char));
        int nick_lenght = recv(rdsocket, nick, _BUFSIZE, 0);
        nick[nick_lenght] = 0;
        struct linked_list *node = create_node(wrsocket);
        add_node(node, wrlist);

        pthread_t thread;
        struct arguments_list *args = malloc(sizeof(struct arguments_list));
        args->rdsocketcontrol = rdsocket;
        args->nowrsocket = wrsocket;
        args->nick = nick;
        args->root = wrlist;
        if (pthread_create(&thread, NULL, worker, (void *)&args) != 0)
        {
            return EXIT_FAILURE;
        }
    }
    free(arg);
    return 0;
}