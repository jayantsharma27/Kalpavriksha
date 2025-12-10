#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_PROCESS 50
#define MAX_KILL_EVENTS 50

typedef enum
{
    NEW,
    READY,
    RUNNING,
    WAITING,
    TERMINATED
} ProcessState;

typedef struct PCB
{
    char name[25];
    int pid;
    int burst;
    int ioStart;
    int ioDuration;
    ProcessState state;
    int cpuExecuted;
    int cpuRemaining;
    int ioRemaining;
    int completionTime;
    bool killedFlag;
    struct PCB *nextQueue;
    struct PCB *nextHash;
} PCB;

typedef struct Queue
{
    PCB *front;
    PCB *rear;
} Queue;

typedef struct Kill
{
    int pid;
    int killTime;
} Kill;

PCB *hashMap[MAX_PROCESS];
Kill killSchedule[MAX_KILL_EVENTS];
int killCount = 0;
Queue ReadyQ = {NULL, NULL};
Queue WaitingQ = {NULL, NULL};
Queue TerminatedQ = {NULL, NULL};
int clockTime = 0;    
int totalProcess = 0;
int terminatedProcessCount = 0;
PCB *running = NULL;

int key(PCB *p)
{
    return (p->pid) % MAX_PROCESS;
}

PCB *HashSearch(int pid)
{
    int k = pid % MAX_PROCESS;
    PCB *p = hashMap[k];
    while (p != NULL && p->pid != pid)
    {
        p = p->nextHash;
    }
    return p;
}

void enqueue(Queue *q, PCB *p)
{
    if (p == NULL)
        return;
    p->nextQueue = NULL;
    if (q->front == NULL)
    {
        q->front = p;
        q->rear = p;
    }
    else
    {
        q->rear->nextQueue = p;
        q->rear = p;
    }
}

PCB *dequeue(Queue *q)
{
    if (q->front == NULL)
    {
        return NULL;
    }
    PCB *p = q->front;
    q->front = q->front->nextQueue;
    if (q->front == NULL)
    {
        q->rear = NULL;
    }
    p->nextQueue = NULL;
    return p;
}

void removeNode(Queue *q, PCB *p)
{
    if (p == NULL || q->front == NULL)
        return;

    if (q->front == p)
    {
        dequeue(q);
        return;
    }

    PCB *curr = q->front;
    while (curr != NULL && curr->nextQueue != p)
    {
        curr = curr->nextQueue;
    }

    if (curr != NULL)
    {
        curr->nextQueue = p->nextQueue;
        if (q->rear == p)
        {
            q->rear = curr;
        }
        p->nextQueue = NULL; 
    }
}

void removeFromQueue(PCB *p)
{
    if (p->state == READY)
    {
        removeNode(&ReadyQ, p);
    }
    else if (p->state == WAITING)
    {
        removeNode(&WaitingQ, p);
    }
}

void header1(void)
{
    printf("\n| PID | Name | CPU | I/O | Turnaround | Waiting |\n");
}

void header2(void)
{
    printf("\n| PID | Name | CPU | I/O | Status | Turnaround | Waiting |\n");
}

int calculateWaitingTime(PCB *p)
{
    return p->completionTime - p->burst;
}

void print_results(void)
{
    if (killCount == 0)
    {
        header1();
        printf("|-----|------|-----|-----|------------|---------|\n");
    }
    else
    {
        header2();
        printf("|-----|------|-----|-----|----------|------------|---------|\n");
    }

    PCB *current = TerminatedQ.front;
    while (current != NULL)
    {
        PCB *curr = current;
        int ioTime = curr->ioDuration;

        if (killCount > 0)
        {
            if (curr->killedFlag)
            {
                printf("| %-3d | %-4s | %-5d | %-5d | KILLED at %-7d | - | - |\n",
                       curr->pid, curr->name, curr->burst, ioTime, curr->completionTime);
            }
            else
            {
                int wt = calculateWaitingTime(curr);
                printf("| %-3d | %-4s | %-5d | %-5d | OK       | %-10d | %-7d |\n",
                       curr->pid, curr->name, curr->burst, ioTime, curr->completionTime, wt);
            }
        }
        else
        {
            int wt = calculateWaitingTime(curr);
            printf("| %-3d | %-4s | %-5d | %-5d | %-10d | %-7d |\n",
                   curr->pid, curr->name, curr->burst, ioTime, curr->completionTime, wt);
        }
        current = current->nextQueue;
    }
}

void freeMemory(void)
{
    for (int i = 0; i < MAX_PROCESS; i++)
    {
        PCB *current = hashMap[i];
        PCB *nextNode = NULL;

        while (current != NULL)
        {
            nextNode = current->nextHash;
            free(current);
            current = nextNode;
        }
        hashMap[i] = NULL;
    }
}

int main()
{
    printf("---FCFS Implementation---\n");
    printf("Enter processes (Name PID Burst IOStart IODuration) or KILL events (KILL PID Time).\n");
    printf("Press Ctrl+D (or Ctrl+Z on Windows) to start simulation.\n");
    char input[100];
    char cmd[25];
    while (fgets(input, 100, stdin) != NULL)
    {
        if (sscanf(input, "%s", cmd) != 1)
            continue;
        if (strcmp(cmd, "KILL") == 0)
        {
            if (sscanf(input, "%s %d %d", cmd, &(killSchedule[killCount].pid), &(killSchedule[killCount].killTime)) == 3)
            {
                if (killCount < MAX_KILL_EVENTS)
                    killCount++;
            }
        }
        else
        {
            PCB *p = (PCB *)malloc(sizeof(PCB));
            if (!p)
            {
                fprintf(stderr, "Memory allocation failed.\n");
                break;
            }
            if (sscanf(input, "%s %d %d %d %d", p->name, &(p->pid), &(p->burst), &(p->ioStart), &(p->ioDuration)) == 5)
            {
                p->state = READY;
                p->cpuExecuted = 0;
                p->cpuRemaining = p->burst;
                p->ioRemaining = p->ioDuration;
                p->completionTime = -1;
                p->killedFlag = false;
                p->nextQueue = NULL;
                p->nextHash = NULL;
                if (p->ioStart == 0 || p->ioDuration == 0)
                {
                    p->ioStart = -1;
                    p->ioDuration = 0;
                    p->ioRemaining = 0;
                }
                int k = key(p);
                if (hashMap[k] != NULL)
                {
                    p->nextHash = hashMap[k];
                }
                hashMap[k] = p;
                enqueue(&ReadyQ, p);
                totalProcess++;
            }
            else
            {
                free(p);
            }
        }
    }
    while (terminatedProcessCount < totalProcess)
    {
        for (int i = 0; i < killCount; i++)
        {
            if (killSchedule[i].killTime == clockTime)
            {
                int target_pid = killSchedule[i].pid;
                PCB *p = HashSearch(target_pid);
                if (p != NULL && p->state != TERMINATED)
                {
                    if (p == running)
                    {
                        running = NULL;
                    }
                    else
                    {
                        removeFromQueue(p);
                    }
                    p->killedFlag = true;
                    p->completionTime = clockTime;
                    p->state = TERMINATED;
                    enqueue(&TerminatedQ, p);
                    terminatedProcessCount++;
                }
            }
        }
        if (running == NULL)
        {
            running = dequeue(&ReadyQ);
            if (running != NULL)
            {
                running->state = RUNNING;
            }
        }
        if (running != NULL)
        {
            PCB *p = running;
            p->cpuRemaining--;
            p->cpuExecuted++;
            if (p->ioStart > 0 && p->cpuExecuted == p->ioStart && p->cpuRemaining > 0)
            {
                p->state = WAITING;
                enqueue(&WaitingQ, p);
                running = NULL;
            }
            else if (p->cpuRemaining == 0)
            {
                p->completionTime = clockTime + 1;
                p->state = TERMINATED;
                enqueue(&TerminatedQ, p);
                terminatedProcessCount++;
                running = NULL;
            }
        }
        {
            PCB *current = WaitingQ.front;
            PCB *next_to_check = NULL;
            while (current != NULL)
            {
                next_to_check = current->nextQueue;

                if (current->state == WAITING && current->ioRemaining > 0)
                {
                    current->ioRemaining--;
                }

                if (current->ioRemaining == 0 && current->state == WAITING)
                {
                    removeNode(&WaitingQ, current);
                    current->state = READY;
                    enqueue(&ReadyQ, current);
                }
                current = next_to_check;
            }
        }
        clockTime++;
    }
    print_results();
    freeMemory();
    return 0;
}