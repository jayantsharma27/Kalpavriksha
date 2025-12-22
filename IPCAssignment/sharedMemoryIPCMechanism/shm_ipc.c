#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <unistd.h>

int main()
{
    int shmid = shmget(IPC_PRIVATE, 5 * sizeof(int), IPC_CREAT | 0666);
    int *arr = (int *)shmat(shmid, NULL, 0);
    int vals[] = {9, 1, 5, 2, 7};
    for (int i = 0; i < 5; i++)
        arr[i] = vals[i];

    if (fork() == 0)
    { 
        for (int i = 0; i < 4; i++)
            for (int j = 0; j < 4 - i; j++)
                if (arr[j] > arr[j + 1])
                {
                    int t = arr[j];
                    arr[j] = arr[j + 1];
                    arr[j + 1] = t;
                }
        shmdt(arr);
        return 0;
    }
    else
    {
        printf("Before Sort: 9 1 5 2 7\n");
        wait(NULL);
        printf("After Sort: ");
        for (int i = 0; i < 5; i++)
            printf("%d ", arr[i]);
        printf("\n");
        shmdt(arr);
        shmctl(shmid, IPC_RMID, NULL);
    }
    return 0;
}