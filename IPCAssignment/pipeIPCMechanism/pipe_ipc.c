#include <stdio.h>
#include <unistd.h>

int main()
{
    int p1[2], p2[2];
    int data[] = {10, 2, 8, 4, 6};
    pipe(p1);
    pipe(p2);

    if (fork() == 0)
    { 
        int received[5];
        read(p1[0], received, sizeof(received));
        for (int i = 0; i < 4; i++)
            for (int j = 0; j < 4 - i; j++)
                if (received[j] > received[j + 1])
                {
                    int t = received[j];
                    received[j] = received[j + 1];
                    received[j + 1] = t;
                }
        write(p2[1], received, sizeof(received));
    }
    else
    { 
        printf("Original Data: 10 2 8 4 6\n");
        write(p1[1], data, sizeof(data));
        read(p2[0], data, sizeof(data));
        printf("Sorted Data: ");
        for (int i = 0; i < 5; i++)
            printf("%d ", data[i]);
        printf("\n");
    }
    return 0;
}