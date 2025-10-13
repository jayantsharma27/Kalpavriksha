#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdbool.h>

#define MAX_STUDENTS 100
#define MIN_STUDENTS 1
#define MAX_NAME_LENGTH 100
#define NUM_SUBJECTS 3
#define LINE_BUFFER_SIZE 256

typedef struct
{
    int rollNo;
    char name[MAX_NAME_LENGTH];
    int marks[NUM_SUBJECTS];
} Student;

int calculateTotalMarks(const int marks[]);
float calculateAverageMarks(int total);
char assignGrade(float average);
void trimTrailingWhitespace(char *str);
void displayReport(int numStudents, Student students[]);
void printSortedRollNoRecursive(const Student students[], int index, int totalStudents);
int compareStudentsByRoll(const void *a, const void *b);
bool isRollNoUnique(int rollNo, const Student students[], int currentCount);

int main()
{
    int numStudents;
    printf("Enter the number of students (%d-%d): ", MIN_STUDENTS, MAX_STUDENTS);
    while (true)
    {
        if (scanf("%d", &numStudents) == 1 && numStudents >= MIN_STUDENTS && numStudents <= MAX_STUDENTS)
        {
            int c;
            while ((c = getchar()) != '\n' && c != EOF)
                ;
            break; 
        }
        printf("Invalid input. Please enter a number between %d and %d: ", MIN_STUDENTS, MAX_STUDENTS);
        int c;
        while ((c = getchar()) != '\n' && c != EOF)
            ;
    }

    Student students[numStudents];
    printf("\nEnter student details in this format \"RollNo. Name Marks1 Marks2 Marks3\":\n");

    for (int i = 0; i < numStudents; i++)
    {
        char lineBuffer[LINE_BUFFER_SIZE];
        printf("Student %d: ", i + 1);
        if (fgets(lineBuffer, sizeof(lineBuffer), stdin) == NULL)
        {
            printf("Error reading input, exiting...\n");
            return 1;
        }

        int tempRollNo;
        char tempName[MAX_NAME_LENGTH] = {0};
        int tempMarks[NUM_SUBJECTS];
        int marksCount = 0;

        int offset = 0;
        // %n stores the number of characters consumed.
        int initialParse = sscanf(lineBuffer, "%d %99[^0-9]%n", &tempRollNo, tempName, &offset);

        if (initialParse == 2)
        {
            char *remaining = lineBuffer + offset;
            int chars_in_loop = 0;
            while (marksCount < NUM_SUBJECTS && sscanf(remaining, "%d%n", &tempMarks[marksCount], &chars_in_loop) == 1)
            {
                marksCount++;
                remaining += chars_in_loop; 
            }
        }

        if (initialParse != 2 || marksCount != NUM_SUBJECTS)
        {
            printf("Invalid input format. Please try again.\n");
            i--;
            continue;
        }

        if (tempRollNo <= 0)
        {
            printf("Error: Roll number must be a positive integer. Please try again.\n");
            i--;
            continue;
        }

        if (!isRollNoUnique(tempRollNo, students, i))
        {
            printf("Error: Roll number %d already exists. Please try again.\n", tempRollNo);
            i--;
            continue;
        }

        bool marksAreValid = true;
        for (int c = 0; c < NUM_SUBJECTS; c++)
        {
            if (tempMarks[c] < 0 || tempMarks[c] > 100)
            {
                marksAreValid = false;
                break;
            }
        }
        if (!marksAreValid)
        {
            printf("Marks out of range (0-100). Please try again.\n");
            i--;
            continue;
        }

        students[i].rollNo = tempRollNo;
        strcpy(students[i].name, tempName);
        trimTrailingWhitespace(students[i].name);

        for (int c = 0; c < NUM_SUBJECTS; c++)
        {
            students[i].marks[c] = tempMarks[c];
        }
    }

    qsort(students, numStudents, sizeof(Student), compareStudentsByRoll);
    displayReport(numStudents, students);
    return 0;
}

void displayReport(int numStudents, Student students[])
{
    printf("\n--- Student Performance Analyzer Report ---\n");
    for (int i = 0; i < numStudents; i++)
    {
        int total = calculateTotalMarks(students[i].marks);
        float average = calculateAverageMarks(total);
        char grade = assignGrade(average);

        printf("Roll: %d\n", students[i].rollNo);
        printf("Name: %s\n", students[i].name);
        printf("Total: %d\n", total);
        printf("Average: %.2f\n", average);
        printf("Grade: %c\n", grade);

        if (grade != 'F')
        {
            printf("Performance: ");
            int starCount = 0;
            switch (grade)
            {
            case 'A':
                starCount = 5;
                break;
            case 'B':
                starCount = 4;
                break;
            case 'C':
                starCount = 3;
                break;
            case 'D':
                starCount = 2;
                break;
            }
            for (int j = 0; j < starCount; j++)
            {
                printf("* ");
            }
            printf("\n");
        }
        printf("------------------------------------\n");
    }
    printf("\nList of Roll Numbers (Sorted, via recursion): ");
    printSortedRollNoRecursive(students, 0, numStudents);
    printf("\n");
}

bool isRollNoUnique(int rollNo, const Student students[], int currentCount)
{
    for (int i = 0; i < currentCount; i++)
    {
        if (students[i].rollNo == rollNo)
        {
            return false;
        }
    }
    return true; 
}

int compareStudentsByRoll(const void *a, const void *b)
{
    const Student *studentA = (const Student *)a;
    const Student *studentB = (const Student *)b;
    return studentA->rollNo - studentB->rollNo;
}

void printSortedRollNoRecursive(const Student students[], int index, int totalStudents)
{
    if (index >= totalStudents)
    {
        return;
    }
    printf("%d ", students[index].rollNo);
    printSortedRollNoRecursive(students, index + 1, totalStudents);
}

int calculateTotalMarks(const int marks[])
{
    int total = 0;
    for (int i = 0; i < NUM_SUBJECTS; i++)
    {
        total += marks[i];
    }
    return total;
}

float calculateAverageMarks(int total)
{
    return (float)total / NUM_SUBJECTS;
}

char assignGrade(float average)
{
    if (average >= 85)
        return 'A';
    if (average >= 70)
        return 'B';
    if (average >= 50)
        return 'C';
    if (average >= 35)
        return 'D';
    return 'F';
}

void trimTrailingWhitespace(char *str)
{
    int index = strlen(str) - 1;
    while (index >= 0 && isspace((unsigned char)str[index]))
    {
        str[index] = '\0';
        index--;
    }
}
