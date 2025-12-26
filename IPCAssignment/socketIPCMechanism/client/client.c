#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int main()
{
    int sock;
    struct sockaddr_in server;
    int choice;
    float amount, balance;
    char response[100];

    sock = socket(AF_INET, SOCK_STREAM, 0); 
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_family = AF_INET;
    server.sin_port = htons(8888);

    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        printf("Connection Failed\n");
        return 1;
    }

    while (1)
    {
        printf("\n--- ATM MENU ---\n1. Withdraw\n2. Deposit\n3. Display Balance\n4. Exit\nEnter Choice: ");
        [cite:30, 31, 32, 33] scanf("%d", &choice);

        if (choice == 4)
            break;

        send(sock, &choice, sizeof(int), 0);

        if (choice == 1 || choice == 2)
        {
            printf("Enter amount: ");
            scanf("%f", &amount);
            send(sock, &amount, sizeof(float), 0);
            recv(sock, response, sizeof(response), 0);
            printf("Server: %s\n", response);
        }
        else if (choice == 3)
        {
            recv(sock, &balance, sizeof(float), 0);
            printf("Current Balance: %.2f\n", balance);
        }
    }
    close(sock);
    return 0;
}