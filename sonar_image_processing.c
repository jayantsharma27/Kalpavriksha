#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

void generateMatrix(int n, int matrix[][n]);
void displayMatrix(int n, int matrix[][n]);
void rotateMatrix(int n, int matrix[][n]);
void smoothingMatrix(int n, int matrix[][n]);

int main()
{
    int n;
    printf("Enter the size of the matrix (2-10): ");
    while (1)
    {
        if (scanf("%d", &n) == 1 && n >= 2 && n <= 10)
        {
            int c;
            while ((c = getchar()) != '\n' && c != EOF)
                ;
            break;
        }
        printf("Invalid input. Please enter a number between 2 and 10: ");
        int c;
        while ((c = getchar()) != '\n' && c != EOF)
            ;
    }

    int matrix[n][n];
    generateMatrix(n, matrix);
    printf("\nOriginal Randomly Generated Matrix:\n");
    displayMatrix(n, matrix);

    rotateMatrix(n, matrix);
    printf("\nMatrix after 90° Clockwise Rotation:\n");
    displayMatrix(n, matrix);

    smoothingMatrix(n, matrix);
    printf("\nMatrix after Applying 3x3 Smoothing Filter:\n");
    displayMatrix(n, matrix);

    return 0;
}

void generateMatrix(int n, int matrix[][n])
{
    srand(time(NULL));
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            *((int *)matrix + i * n + j) = rand() % 256;
        }
    }
}

void displayMatrix(int n, int matrix[][n])
{
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            printf("%-4d ", *((int *)matrix + i * n + j));
        }
        printf("\n");
    }
}

void rotateMatrix(int n, int matrix[][n])
{
    for (int i = 0; i < n; i++)
    {
        for (int j = i; j < n; j++)
        {
            int temp = *((int *)matrix + i * n + j);
            *((int *)matrix + i * n + j) = *((int *)matrix + j * n + i);
            *((int *)matrix + j * n + i) = temp;
        }
    }

    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n / 2; j++)
        {
            int temp = *((int *)matrix + i * n + j);
            *((int *)matrix + i * n + j) = *((int *)matrix + i * n + (n - 1 - j));
            *((int *)matrix + i * n + (n - 1 - j)) = temp;
        }
    }
}

void smoothingMatrix(int n, int matrix[][n])
{
    int prevRowCopy[n];
    int currentRowCopy[n];

    memcpy(prevRowCopy, matrix, n * sizeof(int));

    for (int i = 0; i < n; i++)
    {
        memcpy(currentRowCopy, (int *)matrix + i * n, n * sizeof(int));

        for (int j = 0; j < n; j++)
        {
            int sum = 0;
            int count = 0;

            // wi = row offset, wj = column offset
            for (int wi = -1; wi <= 1; wi++)
            {
                for (int wj = -1; wj <= 1; wj++)
                {
                    int ni = i + wi;
                    int nj = j + wj;

                    if (ni >= 0 && ni < n && nj >= 0 && nj < n)
                    {
                        if (ni == i)
                        {
                            sum += currentRowCopy[nj];
                        }
                        else if (ni == i - 1)
                        {
                            sum += prevRowCopy[nj];
                        }
                        else
                        {
                            sum += *((int *)matrix + ni * n + nj);
                        }
                        count++;
                    }
                }
            }
            *((int *)matrix + i * n + j) = sum / count;
        }
        memcpy(prevRowCopy, currentRowCopy, n * sizeof(int));
    }
}