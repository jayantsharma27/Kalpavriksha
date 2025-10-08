#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <stdbool.h>

// Clear screen command for cross-platform compatibility
#ifdef _WIN32
#define CLEAR_SCREEN "cls"
#else
#define CLEAR_SCREEN "clear"
#endif


typedef struct
{
    int id;
    char *name;
    int age;
} User;

const char *FILENAME = "users.txt";


bool userIdExists(int id);
void clearInputBuffer();

// --- For fgets's Behavior ---
void clearInputBuffer()
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
        ;
}

bool userIdExists(int id)
{
    FILE *file = fopen(FILENAME, "r");
    if (file == NULL)
    {
        return false;
    }

    int file_id;
    char file_name[256];
    int file_age;

    while (fscanf(file, "%d,%255[^,],%d\n", &file_id, file_name, &file_age) == 3)
    {
        if (file_id == id)
        {
            fclose(file);
            return true;
        }
    }
    fclose(file);
    return false;
}


void createUser()
{
    User newUser;
    char buffer[256];

    printf("Enter User ID: ");
    while (1)
    {
        if (scanf("%d", &newUser.id) == 1)
        {
            clearInputBuffer();
            if (userIdExists(newUser.id))
            {
                printf("Error: User ID %d already exists. Please enter a different ID: ", newUser.id);
            }
            else
            {
                break;
            }
        }
        else
        {
            printf("Invalid input. Please enter a number for the ID: ");
            clearInputBuffer(); // Protects from infinite loop due to bad input
        }
    }

    printf("Enter Name: ");
    fgets(buffer, sizeof(buffer), stdin);
    buffer[strcspn(buffer, "\n")] = 0;

    newUser.name = malloc(strlen(buffer) + 1);
    if (newUser.name == NULL)
    {
        printf("ERROR: Memory allocation failed.\n");
        return;
    }

    strcpy(newUser.name, buffer);

    printf("Enter Age: ");
    
    while (scanf("%d", &newUser.age) != 1 || newUser.age <= 0)
    {
        printf("Invalid input. Please enter a positive number for the Age: ");
        clearInputBuffer();
    }

    clearInputBuffer();

    FILE *file = fopen(FILENAME, "a");
    if (file == NULL)
    {
        printf("Error opening file!\n");
        free(newUser.name);
        return;
    }

    fprintf(file, "%d,%s,%d\n", newUser.id, newUser.name, newUser.age);
    fclose(file);
    printf("User added successfully!\n");

    free(newUser.name);
}


void readUsers()
{
    FILE *file = fopen(FILENAME, "r");
    if (file == NULL)
    {
        printf("No records found or file cannot be opened.\n");
        return;
    }

    int id, age;
    char name_buffer[256];
    int count = 0;

    printf("\n--- User Records ---\n");
    printf("%-5s %-20s %s\n", "ID", "Name", "Age");
    printf("------------------------------------\n");

    while (fscanf(file, "%d,%255[^,],%d\n", &id, name_buffer, &age) == 3)
    {
        printf("%-5d %-20s %d\n", id, name_buffer, age);
        count++;
    }
    printf("------------------------------------\n");
    if (count == 0)
    {
        printf("No users found in the file.\n");
    }
    else
    {
        printf("Total users found in the file :- %d\n", count);
    }

    fclose(file);
}


void updateUser()
{
    int id_to_update;
    printf("Enter ID of user to update: ");
    while (scanf("%d", &id_to_update) != 1)
    {
        printf("Invalid input. Please enter a number: ");
        clearInputBuffer();
    }
    clearInputBuffer();

    FILE *file = fopen(FILENAME, "r");
    if (file == NULL)
    {
        printf("File cannot be opened. No users to update.\n");
        return;
    }
    FILE *tempFile = fopen("temp.txt", "w");
    if (tempFile == NULL)
    {
        printf("Error creating temporary file!\n");
        fclose(file);
        return;
    }

    int id, age;
    char name_buffer[256];
    bool found = false;

    while (fscanf(file, "%d,%255[^,],%d\n", &id, name_buffer, &age) == 3)
    {
        if (id == id_to_update)
        {
            found = true;
            char new_name_buffer[256];
            int new_age;

            printf("Updating user with ID %d.\n", id);
            printf("Enter new Name: ");
            fgets(new_name_buffer, sizeof(new_name_buffer), stdin);
            new_name_buffer[strcspn(new_name_buffer, "\n")] = 0;

            printf("Enter new Age: ");
            
            while (scanf("%d", &new_age) != 1 || new_age <= 0)
            {
                printf("Invalid input. Please enter a positive number for the Age: ");
                clearInputBuffer();
            }
            clearInputBuffer();

            fprintf(tempFile, "%d,%s,%d\n", id, new_name_buffer, new_age);
        }
        else
        {
            fprintf(tempFile, "%d,%s,%d\n", id, name_buffer, age);
        }
    }

    fclose(file);
    fclose(tempFile);

    remove(FILENAME);
    rename("temp.txt", FILENAME);

    if (found)
    {
        printf("User updated successfully!\n");
    }
    else
    {
        printf("User ID not found.\n");
    }
}


void deleteUser()
{
    int id_to_delete;
    printf("Enter ID of user to delete: ");
    while (scanf("%d", &id_to_delete) != 1)
    {
        printf("Invalid input. Please enter a number: ");
        clearInputBuffer();
    }
    clearInputBuffer();

    FILE *file = fopen(FILENAME, "r");
    if (file == NULL)
    {
        printf("File cannot be opened. No users to delete.\n");
        return;
    }
    FILE *tempFile = fopen("temp.txt", "w");
    if (tempFile == NULL)
    {
        printf("Error creating temporary file!\n");
        fclose(file);
        return;
    }

    int id, age;
    char name_buffer[256];
    bool found = false;

    while (fscanf(file, "%d,%255[^,],%d\n", &id, name_buffer, &age) == 3)
    {
        if (id == id_to_delete)
        {
            found = true;
        }
        else
        {
            fprintf(tempFile, "%d,%s,%d\n", id, name_buffer, age);
        }
    }

    fclose(file);
    fclose(tempFile);

    remove(FILENAME);
    rename("temp.txt", FILENAME);

    if (found)
    {
        printf("User deleted successfully!\n");
    }
    else
    {
        printf("User ID not found.\n");
    }
}


int main()
{
    int choice;
    do
    {
        system(CLEAR_SCREEN);

        printf("\n--- CRUD Operations in File using C Language ---\n");
        printf("1. Add User (Create)\n");
        printf("2. Display All Users (Read)\n");
        printf("3. Update User\n");
        printf("4. Delete User\n");
        printf("5. To Exit\n");
        printf("Enter your choice: ");

        if (scanf("%d", &choice) != 1)
        {
            choice = 0;
        }
        clearInputBuffer();

        switch (choice)
        {
        case 1:
            createUser();
            break;
        case 2:
            readUsers();
            break;
        case 3:
            updateUser();
            break;
        case 4:
            deleteUser();
            break;
        case 5:
            printf("Exiting program. Thank you.\n");
            break;
        default:
            printf("Invalid choice. Please try again.\n");
        }

        if (choice != 5)
        {
            printf("\nPress Enter to continue...");
            getchar();
        }

    } while (choice != 5);

    return 0;
}
