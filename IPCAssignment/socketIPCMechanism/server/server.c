#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

pthread_mutex_t file_mutex = PTHREAD_MUTEX_INITIALIZER;
const char *DB_PATH = "../resource/accountDB.txt";

float read_balance()
{
    float balance = 0.0;
    FILE *fp = fopen(DB_PATH, "r");
    if (fp)
    {
        fscanf(fp, "%f", &balance);
        fclose(fp);
    }
    return balance;
}

void write_balance(float balance)
{
    FILE *fp = fopen(DB_PATH, "w");
    if (fp)
    {
        fprintf(fp, "%.2f", balance);
        fclose(fp);
    }
}

void *client_handler(void *socket_desc)
{
    int sock = *(int *)socket_desc;
    int choice;
    float amount, current_balance;
    char message[100];

    while (recv(sock, &choice, sizeof(int), 0) > 0)
    {
        pthread_mutex_lock(&file_mutex);
        current_balance = read_balance();

        if (choice == 1)
        { 
            recv(sock, &amount, sizeof(float), 0);
            if (amount > current_balance)
            { 
                strcpy(message, "Transaction Failed: Insufficient Balance.");
            }
            else
            {
                current_balance -= amount;
                write_balance(current_balance);
                sprintf(message, "Withdrawal Successful. New Balance: %.2f", current_balance);
            }
            send(sock, message, sizeof(message), 0);
        }
        else if (choice == 2)
        { 
            recv(sock, &amount, sizeof(float), 0);
            current_balance += amount;    
            write_balance(current_balance);
            sprintf(message, "Deposit Successful. New Balance: %.2f", current_balance);
            send(sock, message, sizeof(message), 0);
        }
        else if (choice == 3)
        {
            send(sock, &current_balance, sizeof(float), 0);
        }
        pthread_mutex_unlock(&file_mutex);
    }
    close(sock);
    free(socket_desc);
    return NULL;
}

int main()
{
    int server_sock, client_sock, *new_sock;
    struct sockaddr_in server, client;
    socklen_t c = sizeof(struct sockaddr_in);

    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(8888);

    bind(server_sock, (struct sockaddr *)&server, sizeof(server));
    listen(server_sock, 5);

    printf("ATM Server Started. Waiting for connections...\n");

    while ((client_sock = accept(server_sock, (struct sockaddr *)&client, &c)))
    {
        pthread_t thread_id;
        new_sock = malloc(1);
        *new_sock = client_sock;
        pthread_create(&thread_id, NULL, client_handler, (void *)new_sock);
    }
    return 0;
}