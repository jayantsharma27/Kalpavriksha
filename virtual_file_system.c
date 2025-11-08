#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define NUM_BLOCKS 1024
#define BLOCK_SIZE 512
#define MAX_FILE_BLOCKS 32
#define MAX_NAME_LEN 50  
#define MAX_INPUT_LEN 1024
#define MAX_PATH_DEPTH 128 

char gVirtualDisk[NUM_BLOCKS][BLOCK_SIZE];

typedef struct FreeBlock
{
    int blockIndex;
    struct FreeBlock *prevBlock;
    struct FreeBlock *nextBlock;
} FreeBlock;

typedef enum
{
    FILETYPE,
    DIRECTORY
} NodeType;

typedef struct Node
{
    char name[MAX_NAME_LEN + 1];
    NodeType type;
    struct Node *nextSibling;
    struct Node *prevSibling;
    struct Node *parent;

    union
    {
        struct
        {
            struct Node *child;
        } directory;
        struct
        {
            int contentSize;
            int blockPointers[MAX_FILE_BLOCKS];
        } file;
    };

} Node;

Node *gRoot = NULL;
Node *gCwd = NULL;
FreeBlock *gFreeBlockHead = NULL;
FreeBlock *gFreeBlockTail = NULL;
int gFreeBlockCount = 0;

void initializeVfs();
void cleanupMemory();
void freeNodeRecursive(Node *node);
void returnBlockToFreeList(int blockIndex);
Node *findNodeInCwd(char *name);
bool removeFromCircularList(Node *node);
int commandIs(char *command);
int parseCommand(char *inputBuffer, char *command, char *argument);
void buildCurrentPath(char *pathBuffer, size_t bufferSize);

void callDf();
void callMkdir(char *dirName);
void callLs();
void callCd(char *dirName);
void callCreate(char *fileName);
void callWrite(char *argument);
void callRead(char *fileName);
void callDelete(char *fileName);
void callRmdir(char *dirName);
void callPwd();

void initializeVfs()
{
    memset(gVirtualDisk, 0, sizeof(gVirtualDisk));
    gRoot = (Node *)malloc(sizeof(Node));
    if (gRoot == NULL)
    {
        printf("Error: Malloc failed during VFS initialization.\n");
        exit(1);
    }

    strcpy(gRoot->name, "/");
    gRoot->type = DIRECTORY;
    gRoot->parent = NULL;
    gRoot->nextSibling = gRoot;
    gRoot->prevSibling = gRoot;
    gRoot->directory.child = NULL;
    gCwd = gRoot;

    gFreeBlockHead = NULL;
    gFreeBlockTail = NULL;
    gFreeBlockCount = 0;

    for (int i = 0; i < NUM_BLOCKS; i++)
    {
        FreeBlock *newBlock = (FreeBlock *)malloc(sizeof(FreeBlock));
        if (newBlock == NULL)
        {
            printf("Error: Malloc failed during Free Block creation.\n");
            exit(1);
        }

        newBlock->blockIndex = i;
        newBlock->nextBlock = NULL;

        if (gFreeBlockHead == NULL)
        {
            newBlock->prevBlock = NULL;
            gFreeBlockHead = newBlock;
            gFreeBlockTail = newBlock;
        }
        else
        {
            newBlock->prevBlock = gFreeBlockTail;
            gFreeBlockTail->nextBlock = newBlock;
            gFreeBlockTail = newBlock;
        }
        gFreeBlockCount++;
    }
}

int main()
{
    printf("Compact VFS - ready. Type 'exit' to quit.\n");
    initializeVfs();

    char command[MAX_NAME_LEN + 1];
    char argument[MAX_INPUT_LEN];
    char inputBuffer[MAX_INPUT_LEN];
    char path[MAX_INPUT_LEN];
    int commandCode;

    do
    {
        buildCurrentPath(path, sizeof(path));
        printf("%s> ", path);

        command[0] = '\0';
        argument[0] = '\0';

        if (fgets(inputBuffer, sizeof(inputBuffer), stdin) == NULL)
        {
            commandCode = 11;
            printf("\n");
        }
        else
        {
            if (strchr(inputBuffer, '\n') == NULL)
            {
                printf("Input too long, ignoring.\n");
                int c;
                while ((c = getchar()) != '\n' && c != EOF);
                continue;
            }
            inputBuffer[strcspn(inputBuffer, "\n")] = 0;

            if (inputBuffer[0] == '\0')
            {
                continue;
            }
            parseCommand(inputBuffer, command, argument);
            commandCode = commandIs(command);
        }
        switch (commandCode)
        {
        case 1:
            callMkdir(argument);
            break;
        case 2:
            callCreate(argument);
            break;
        case 3:
            callWrite(argument);
            break;
        case 4:
            callRead(argument);
            break;
        case 5:
            callDelete(argument);
            break;
        case 6:
            callRmdir(argument);
            break;
        case 7:
            callLs();
            break;
        case 8:
            callCd(argument);
            break;
        case 9:
            callPwd();
            break;
        case 10:
            callDf();
            break;
        case 11:
            break;
        case 0:
        default:
            printf("Invalid Command: %s\n", command);
            break;
        }

    } while (commandCode != 11);

    cleanupMemory();
    return 0;
}

void callDf()
{
    int freeCount = gFreeBlockCount;
    int usedCount = NUM_BLOCKS - freeCount;
    float diskUsage = 0.0;
    if (NUM_BLOCKS > 0)
    {
        diskUsage = (float)usedCount * 100.0 / NUM_BLOCKS;
    }

    printf("Total Blocks: %d\n", NUM_BLOCKS);
    printf("Used Blocks:  %d\n", usedCount);
    printf("Free Blocks:  %d\n", freeCount);
    printf("Disk Usage:   %.2f%%\n", diskUsage);
}

void callMkdir(char *dirName)
{
    if (dirName == NULL || dirName[0] == '\0')
    {
        printf("Error: Missing directory name.\n");
        return;
    }

    if (strcmp(dirName, ".") == 0 || strcmp(dirName, "..") == 0 || strchr(dirName, '/') != NULL)
    {
        printf("Error: Invalid name. Cannot use '.', '..', or '/'.\n");
        return;
    }

    if (findNodeInCwd(dirName) != NULL)
    {
        printf("Error: Name already exists in current directory.\n");
        return;
    }

    Node *newDir = (Node *)malloc(sizeof(Node));
    if (newDir == NULL)
    {
        printf("Error: Memory allocation failed during new directory creation.\n");
        return;
    }

    strncpy(newDir->name, dirName, MAX_NAME_LEN);
    newDir->name[MAX_NAME_LEN] = '\0';
    newDir->type = DIRECTORY;
    newDir->parent = gCwd;
    newDir->directory.child = NULL;

    Node *headChild = gCwd->directory.child;
    if (headChild == NULL)
    {
        gCwd->directory.child = newDir;
        newDir->nextSibling = newDir;
        newDir->prevSibling = newDir;
    }
    else
    {
        Node *tail = headChild->prevSibling;
        tail->nextSibling = newDir;
        newDir->prevSibling = tail;
        newDir->nextSibling = headChild;
        headChild->prevSibling = newDir;
    }
    printf("Directory '%s' created successfully.\n", dirName);
}

void callLs()
{
    Node *headChild = gCwd->directory.child;

    if (headChild == NULL)
    {
        printf("(empty)\n");
        return;
    }
    Node *current = headChild;
    do
    {
        printf("%s", current->name);
        if (current->type == DIRECTORY)
        {
            printf("/");
        }
        printf("\n");
        current = current->nextSibling;
    } while (current != headChild);
}

void callCd(char *dirName)
{
    if (dirName == NULL || dirName[0] == '\0')
    {
        printf("Error: Missing directory name.\n");
        return;
    }

    if (strcmp(dirName, "/") == 0)
    {
        gCwd = gRoot;
        printf("Moved to /\n");
        return;
    }

    if (strcmp(dirName, "..") == 0)
    {
        if (gCwd->parent == NULL)
        {
            return;
        }
        gCwd = gCwd->parent;

        char path[MAX_INPUT_LEN];
        buildCurrentPath(path, sizeof(path));
        printf("Moved to %s\n", path);
        return;
    }

    Node *target = findNodeInCwd(dirName);
    if (target == NULL)
    {
        printf("Error: Directory not found.\n");
        return;
    }
    if (target->type == FILETYPE)
    {
        printf("Error: '%s' is a file, not a directory.\n", dirName);
        return;
    }

    gCwd = target;
    char path[MAX_INPUT_LEN];
    buildCurrentPath(path, sizeof(path));
    printf("Moved to %s\n", path);
}

void callCreate(char *fileName)
{
    if (fileName == NULL || fileName[0] == '\0')
    {
        printf("Error: Missing file name.\n");
        return;
    }
    if (strcmp(fileName, ".") == 0 || strcmp(fileName, "..") == 0 || strchr(fileName, '/') != NULL)
    {
        printf("Error: Invalid name. Cannot use '.', '..', or '/'.\n");
        return;
    }

    if (findNodeInCwd(fileName) != NULL)
    {
        printf("Error: Name already exists in current directory.\n");
        return;
    }

    Node *newFile = (Node *)malloc(sizeof(Node));
    if (newFile == NULL)
    {
        printf("Error: Memory allocation failed during file creation.\n");
        return;
    }

    strncpy(newFile->name, fileName, MAX_NAME_LEN);
    newFile->name[MAX_NAME_LEN] = '\0';
    newFile->type = FILETYPE;
    newFile->parent = gCwd;
    newFile->file.contentSize = 0;
    for (int i = 0; i < MAX_FILE_BLOCKS; i++)
    {
        newFile->file.blockPointers[i] = -1;
    }

    Node *headChild = gCwd->directory.child;
    if (headChild == NULL)
    {
        gCwd->directory.child = newFile;
        newFile->nextSibling = newFile;
        newFile->prevSibling = newFile;
    }
    else
    {
        Node *tail = headChild->prevSibling;
        tail->nextSibling = newFile;
        newFile->prevSibling = tail;
        newFile->nextSibling = headChild;
        headChild->prevSibling = newFile;
    }
    printf("File '%s' created successfully.\n", fileName);
}

void callWrite(char *argument)
{
    char fileName[MAX_NAME_LEN + 1];
    char content[MAX_INPUT_LEN];
    const char *contentPtr = NULL;
    const char *argPtr = argument;

    if (argPtr[0] == '"')
    {
        const char *endQuote = strchr(argPtr + 1, '"');
        if (endQuote == NULL)
        {
            printf("Error: Invalid format. Unclosed quote in filename.\n");
            return;
        }
        int len = endQuote - (argPtr + 1);
        if (len > MAX_NAME_LEN)
        {
            printf("Error: Filename is too long.\n");
            return;
        }
        strncpy(fileName, argPtr + 1, len);
        fileName[len] = '\0';
        contentPtr = endQuote + 1;
    }
    else
    {
        const char *firstSpace = strchr(argPtr, ' ');
        if (firstSpace == NULL)
        {
            printf("Error: Invalid format. Missing content. Use: write <file> 'content'\n");
            return;
        }
        int len = firstSpace - argPtr;
        if (len > MAX_NAME_LEN)
        {
            printf("Error: Filename is too long.\n");
            return;
        }
        strncpy(fileName, argPtr, len);
        fileName[len] = '\0';
        contentPtr = firstSpace;
    }

    while (*contentPtr == ' ')
    {
        contentPtr++;
    }

    if (*contentPtr != '\'' && *contentPtr != '"')
    {
        printf("Error: Invalid format. Content must be in 'quotes' or \"quotes\".\n");
        return;
    }

    char quoteChar = *contentPtr;
    const char *endContent = strchr(contentPtr + 1, quoteChar);
    if (endContent == NULL)
    {
        printf("Error: Invalid format. Unclosed quote in content.\n");
        return;
    }

    int contentLen = endContent - (contentPtr + 1);
    if (contentLen >= MAX_INPUT_LEN)
    {
        printf("Error: Content is too long (max %d bytes).\n", MAX_INPUT_LEN - 1);
        return;
    }
    strncpy(content, contentPtr + 1, contentLen);
    content[contentLen] = '\0';

    Node *node = findNodeInCwd(fileName);
    if (node == NULL)
    {
        printf("Error: File not found.\n");
        return;
    }
    if (node->type == DIRECTORY)
    {
        printf("Error: '%s' is a directory.\n", fileName);
        return;
    }

    for (int i = 0; i < MAX_FILE_BLOCKS; i++)
    {
        if (node->file.blockPointers[i] != -1)
        {
            returnBlockToFreeList(node->file.blockPointers[i]);
            node->file.blockPointers[i] = -1;
        }
        else
        {
            break;
        }
    }

    int dataSize = strlen(content);
    int blocksNeeded = 0;
    if (dataSize > 0)
    {
        blocksNeeded = (dataSize + BLOCK_SIZE - 1) / BLOCK_SIZE;
    }
    if (blocksNeeded > MAX_FILE_BLOCKS)
    {
        printf("Error: File content is too large (max %d blocks).\n", MAX_FILE_BLOCKS);
        return;
    }
    if (gFreeBlockCount < blocksNeeded)
    {
        printf("Error: Not enough free space on disk.\n");
        return;
    }
    int dataRemaining = dataSize;
    int contentOffset = 0;
    for (int i = 0; i < blocksNeeded; i++)
    {
        FreeBlock *blockToUse = gFreeBlockHead;
        gFreeBlockHead = blockToUse->nextBlock;
        if (gFreeBlockHead)
        {
            gFreeBlockHead->prevBlock = NULL;
        }
        else
        {
            gFreeBlockTail = NULL;
        }
        int blockIdx = blockToUse->blockIndex;
        node->file.blockPointers[i] = blockIdx;
        int bytesToWrite = (dataRemaining > BLOCK_SIZE) ? BLOCK_SIZE : dataRemaining;
        memcpy(gVirtualDisk[blockIdx], content + contentOffset, bytesToWrite);
        contentOffset += bytesToWrite;
        dataRemaining -= bytesToWrite;
        free(blockToUse);
        gFreeBlockCount--;
    }
    node->file.contentSize = dataSize;
    printf("Data written successfully (size=%d bytes).\n", dataSize);
}

void callRead(char *fileName)
{
    Node *node = findNodeInCwd(fileName);
    if (node == NULL)
    {
        printf("Error: File not found.\n");
        return;
    }
    if (node->type == DIRECTORY)
    {
        printf("Error: '%s' is a directory.\n", fileName);
        return;
    }
    if (node->file.contentSize == 0)
    {
        printf("(empty)\n");
        return;
    }
    int dataRemaining = node->file.contentSize;
    for (int i = 0; i < MAX_FILE_BLOCKS; i++)
    {
        int blockIdx = node->file.blockPointers[i];
        if (blockIdx == -1)
            break;
        int bytesToRead = (dataRemaining > BLOCK_SIZE) ? BLOCK_SIZE : dataRemaining;
        for (int j = 0; j < bytesToRead; j++)
        {
            putchar(gVirtualDisk[blockIdx][j]);
        }
        dataRemaining -= bytesToRead;
        if (dataRemaining <= 0)
            break;
    }
    printf("\n");
}

void callDelete(char *fileName)
{
    Node *node = findNodeInCwd(fileName);
    if (node == NULL)
    {
        printf("Error: File not found.\n");
        return;
    }
    if (node->type == DIRECTORY)
    {
        printf("Error: '%s' is a directory. Use 'rmdir'.\n", fileName);
        return;
    }
    for (int i = 0; i < MAX_FILE_BLOCKS; i++)
    {
        if (node->file.blockPointers[i] != -1)
        {
            returnBlockToFreeList(node->file.blockPointers[i]);
        }
        else
        {
            break;
        }
    }
    removeFromCircularList(node);
    free(node);
    printf("File deleted successfully.\n");
}

void callRmdir(char *dirName)
{
    Node *node = findNodeInCwd(dirName);
    if (node == NULL)
    {
        printf("Error: Directory not found.\n");
        return;
    }
    if (node->type == FILETYPE)
    {
        printf("Error: '%s' is a file. Use 'delete'.\n", dirName);
        return;
    }
    if (node->directory.child != NULL)
    {
        printf("Error: Directory not empty. Remove files first.\n");
        return;
    }
    removeFromCircularList(node);
    free(node);
    printf("Directory removed successfully.\n");
}

void callPwd()
{
    char path[MAX_INPUT_LEN];
    buildCurrentPath(path, sizeof(path));
    printf("%s\n", path);
}

void cleanupMemory()
{
    freeNodeRecursive(gRoot);
    FreeBlock *current = gFreeBlockHead;
    while (current != NULL)
    {
        FreeBlock *next = current->nextBlock;
        free(current);
        current = next;
    }
    printf("Memory released. Exiting program...\n");
}

void freeNodeRecursive(Node *node)
{
    if (node == NULL)
        return;
    if (node->type == DIRECTORY && node->directory.child != NULL)
    {
        Node *current = node->directory.child;
        current->prevSibling->nextSibling = NULL;

        while (current != NULL)
        {
            Node *next = current->nextSibling;
            freeNodeRecursive(current);
            current = next;
        }
    }
    free(node);
}

bool removeFromCircularList(Node *node)
{
    if (node->parent == NULL)
        return false;
    Node *parent = node->parent;
    Node *prev = node->prevSibling;
    Node *next = node->nextSibling;
    if (node == next)
    {
        parent->directory.child = NULL;
        return true;
    }
    prev->nextSibling = next;
    next->prevSibling = prev;
    if (parent->directory.child == node)
    {
        parent->directory.child = next;
    }
    return false;
}

void returnBlockToFreeList(int blockIndex)
{
    FreeBlock *newFreeNode = (FreeBlock *)malloc(sizeof(FreeBlock));
    if (newFreeNode == NULL)
    {
        printf("Error: Malloc failed during freeing block.\n");
        return;
    }
    newFreeNode->blockIndex = blockIndex;
    newFreeNode->nextBlock = NULL;
    if (gFreeBlockTail == NULL)
    {
        newFreeNode->prevBlock = NULL;
        gFreeBlockHead = newFreeNode;
        gFreeBlockTail = newFreeNode;
    }
    else
    {
        newFreeNode->prevBlock = gFreeBlockTail;
        gFreeBlockTail->nextBlock = newFreeNode;
        gFreeBlockTail = newFreeNode;
    }
    gFreeBlockCount++;
}

int parseCommand(char *inputBuffer, char *command, char *argument)
{
    int index = strcspn(inputBuffer, " ");
    int cmdLen = (index > MAX_NAME_LEN) ? MAX_NAME_LEN : index;
    strncpy(command, inputBuffer, cmdLen);
    command[cmdLen] = '\0';

    char *arg_ptr = inputBuffer + index;
    while (*arg_ptr == ' ')
    {
        arg_ptr++;
    }
    strcpy(argument, arg_ptr);
    return index;
}

int commandIs(char *command)
{
    if (strcmp(command, "mkdir") == 0)
        return 1;
    if (strcmp(command, "create") == 0)
        return 2;
    if (strcmp(command, "write") == 0)
        return 3;
    if (strcmp(command, "read") == 0)
        return 4;
    if (strcmp(command, "delete") == 0)
        return 5;
    if (strcmp(command, "rmdir") == 0)
        return 6;
    if (strcmp(command, "ls") == 0)
        return 7;
    if (strcmp(command, "cd") == 0)
        return 8;
    if (strcmp(command, "pwd") == 0)
        return 9;
    if (strcmp(command, "df") == 0)
        return 10;
    if (strcmp(command, "exit") == 0)
        return 11;
    return 0;
}

Node *findNodeInCwd(char *name)
{
    Node *headChild = gCwd->directory.child;
    if (headChild == NULL)
    {
        return NULL;
    }
    Node *current = headChild;
    do
    {
        if (strcmp(current->name, name) == 0)
        {
            return current;
        }
        current = current->nextSibling;
    } while (current != headChild);
    return NULL;
}

void buildCurrentPath(char *pathBuffer, size_t bufferSize)
{
    if (gCwd == gRoot)
    {
        snprintf(pathBuffer, bufferSize, "/");
        return;
    }
    Node *path[MAX_PATH_DEPTH];
    int depth = 0;
    Node *current = gCwd;
    while (current != gRoot && depth < MAX_PATH_DEPTH)
    {
        path[depth] = current;
        current = current->parent;
        depth++;
    }
    char *bufferPtr = pathBuffer;
    char *bufferEnd = pathBuffer + bufferSize;
    for (int i = depth - 1; i >= 0; i--)
    {
        int remaining = bufferEnd - bufferPtr;
        if (remaining <= 1)
            break; 
        int written = snprintf(bufferPtr, remaining, "/%s", path[i]->name);

        if (written < 0 || written >= remaining)
        {
            break;
        }
        bufferPtr += written;
    }
    if (bufferPtr == pathBuffer)
    {
        snprintf(pathBuffer, bufferSize, "/");
    }
}