#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define HASH_TABLE_SIZE 2048
#define MAX_VALUE_LENGTH 64

typedef struct QueueNode
{
    int key;
    char value[MAX_VALUE_LENGTH];
    struct QueueNode *prev;
    struct QueueNode *next;
    struct QueueNode *hashNext;
} QueueNode;

typedef struct LruCache
{
    int capacity;
    int count;
    QueueNode *head;
    QueueNode *tail;
    QueueNode *hashTable[HASH_TABLE_SIZE];
} LruCache;

unsigned int getHashIndex(int key)
{
    return key % HASH_TABLE_SIZE;
}

void detachNode(LruCache *cache, QueueNode *node)
{
    if (node->prev != NULL)
    {
        node->prev->next = node->next;
    }
    else
    {
        cache->head = node->next;
    }

    if (node->next != NULL)
    {
        node->next->prev = node->prev;
    }
    else
    {
        cache->tail = node->prev;
    }
}

void insertAtHead(LruCache *cache, QueueNode *node)
{
    node->next = cache->head;
    node->prev = NULL;

    if (cache->head != NULL)
    {
        cache->head->prev = node;
    }

    cache->head = node;

    if (cache->tail == NULL)
    {
        cache->tail = node;
    }
}

void removeFromHashTable(LruCache *cache, QueueNode *targetNode)
{
    unsigned int hashIndex = getHashIndex(targetNode->key);
    QueueNode *current = cache->hashTable[hashIndex];
    QueueNode *prev = NULL;

    while (current != NULL)
    {
        if (current == targetNode)
        {
            if (prev == NULL)
            {
                cache->hashTable[hashIndex] = current->hashNext;
            }
            else
            {
                prev->hashNext = current->hashNext;
            }
            return;
        }
        prev = current;
        current = current->hashNext;
    }
}

void removeLruNode(LruCache *cache)
{
    if (cache->tail == NULL)
    {
        return;
    }
    QueueNode *lruNode = cache->tail;
    detachNode(cache, lruNode);
    removeFromHashTable(cache, lruNode);
    free(lruNode);
    cache->count--;
}

void freeCache(LruCache *cache)
{
    QueueNode *current = cache->head;
    while (current != NULL)
    {
        QueueNode *next = current->next;
        free(current);
        current = next;
    }
    free(cache);
}

LruCache *createCache(int capacity)
{
    if (capacity <= 0)
    {
        return NULL;
    }
    LruCache *newCache = (LruCache *)malloc(sizeof(LruCache));
    if (newCache == NULL)
    {
        fprintf(stderr, "Error: Memory allocation failed for LruCache.\n");
        exit(EXIT_FAILURE);
    }
    newCache->capacity = capacity;
    newCache->count = 0;
    newCache->head = NULL;
    newCache->tail = NULL;
    for (int i = 0; i < HASH_TABLE_SIZE; i++)
    {
        newCache->hashTable[i] = NULL;
    }
    return newCache;
}

char *get(LruCache *cache, int key)
{
    unsigned int hashIndex = getHashIndex(key);
    QueueNode *node = cache->hashTable[hashIndex];
    while (node != NULL)
    {
        if (node->key == key)
        {
            detachNode(cache, node);
            insertAtHead(cache, node);
            return node->value;
        }
        node = node->hashNext;
    }
    return NULL;
}

void put(LruCache *cache, int key, char *value)
{
    unsigned int hashIndex = getHashIndex(key);
    QueueNode *node = cache->hashTable[hashIndex];

    while (node != NULL)
    {
        if (node->key == key)
        {
            strcpy(node->value, value);
            detachNode(cache, node);
            insertAtHead(cache, node);
            return;
        }
        node = node->hashNext;
    }

    QueueNode *newNode = (QueueNode *)malloc(sizeof(QueueNode));
    if (newNode == NULL)
    {
        fprintf(stderr, "Error: Memory allocation failed for QueueNode.\n");
        exit(EXIT_FAILURE);
    }
    newNode->key = key;
    strcpy(newNode->value, value);
    newNode->prev = NULL;
    newNode->next = NULL;
    newNode->hashNext = NULL;

    if (cache->count >= cache->capacity)
    {
        removeLruNode(cache);
    }
    insertAtHead(cache, newNode);
    newNode->hashNext = cache->hashTable[hashIndex];
    cache->hashTable[hashIndex] = newNode;

    cache->count++;
}

int main()
{
    char command[32];
    int key;
    char value[MAX_VALUE_LENGTH];
    int capacity;
    LruCache *cache = NULL;
    printf("Input:\n");
    while (scanf("%s", command) != EOF)
    {
        if (strcmp(command, "createCache") == 0)
        {
            scanf("%d", &capacity);
            if (cache != NULL)
            {
                freeCache(cache);
            }
            cache = createCache(capacity);
        }
        else if (strcmp(command, "put") == 0)
        {
            if (cache == NULL)
                continue;
            scanf("%d %s", &key, value);
            put(cache, key, value);
        }
        else if (strcmp(command, "get") == 0)
        {
            if (cache == NULL)
                continue;
            scanf("%d", &key);
            char *result = get(cache, key);

            if (result != NULL)
            {
                printf("%s\n", result);
            }
            else
            {
                printf("NULL\n");
            }
        }
        else if (strcmp(command, "exit") == 0)
        {
            break;
        }
        else
        {
            printf("Please enter a valid command\n");
        }
    }
    if (cache != NULL)
    {
        freeCache(cache);
    }

    return 0;
}