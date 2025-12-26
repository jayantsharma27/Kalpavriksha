#include <stdio.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <sys/wait.h>

struct msg
{
    long type;
    int a[5];
} m;

int main()
{
    int qid = msgget(IPC_PRIVATE, 0666 | IPC_CREAT);
    if (fork() == 0)
    {
        msgrcv(qid, &m, sizeof(m.a), 1, 0);
        for (int i = 0; i < 4; i++)
            for (int j = 0; j < 4 - i; j++)
                if (m.a[j] > m.a[j + 1])
                {
                    int t = m.a[j];
                    m.a[j] = m.a[j + 1];
                    m.a[j + 1] = t;
                }
        m.type = 2;
        msgsnd(qid, &m, sizeof(m.a), 0);
    }
    else
    {
        m.type = 1;
        int initial[] = {5, 3, 8, 1, 2};
        for (int i = 0; i < 5; i++)
            m.a[i] = initial[i];
        printf("Before Sort: 5 3 8 1 2\n");
        msgsnd(qid, &m, sizeof(m.a), 0);
        wait(NULL);
        msgrcv(qid, &m, sizeof(m.a), 2, 0);
        printf("After Sort: ");
        for (int i = 0; i < 5; i++)
            printf("%d ", m.a[i]);
        printf("\n");
        msgctl(qid, IPC_RMID, NULL);
    }
    return 0;
}