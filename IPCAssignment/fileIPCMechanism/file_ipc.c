#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

void bubble_sort(int arr[], int n)
{
    for (int i = 0; i < n - 1; i++)
        for (int j = 0; j < n - i - 1; j++)
            if (arr[j] > arr[j + 1])
            {
                int temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
}

int main()
{
    int data[] = {45, 12, 89, 7, 33};
    int n = 5;
    FILE *fp;

    printf("First Process: Original Data: ");
    for (int i = 0; i < n; i++)
        printf("%d ", data[i]);
    printf("\n");

    if (fork() == 0)
    {   
        sleep(1);
        int child_arr[5];
        fp = fopen("ipc_file.bin", "rb");
        fread(child_arr, sizeof(int), 5, fp);
        fclose(fp);

        bubble_sort(child_arr, 5);

        fp = fopen("ipc_file.bin", "wb");
        fwrite(child_arr, sizeof(int), 5, fp);
        fclose(fp);
        exit(0);
    }
    else
    { 
        fp = fopen("ipc_file.bin", "wb");
        fwrite(data, sizeof(int), 5, fp);
        fclose(fp);

        wait(NULL);

        fp = fopen("ipc_file.bin", "rb");
        fread(data, sizeof(int), 5, fp);
        fclose(fp);

        printf("First Process: Sorted Data: ");
        for (int i = 0; i < n; i++)
            printf("%d ", data[i]);
        printf("\n");
        remove("ipc_file.bin");
    }
    return 0;
}