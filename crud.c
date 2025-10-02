#include <stdio.h>

#include <stdlib.h>

#include <string.h>



// Define the structure for a user record

typedef struct {

    int id;

    char name[50];

    int age;

} User;



const char* FILENAME = "users.txt";



// Function to add a new user

void createUser() {

    User newUser;

    printf("Enter User ID: ");

    scanf("%d", &newUser.id);

    printf("Enter Name: ");

    scanf(" %[^\n]", newUser.name); // Read string with spaces

    printf("Enter Age: ");

    scanf("%d", &newUser.age);



    FILE *file = fopen(FILENAME, "a"); // "a" for append mode

    if (file == NULL) {

        printf("Error opening file!\n");

        return;

    }



    fprintf(file, "%d,%s,%d\n", newUser.id, newUser.name, newUser.age);

    fclose(file);

    printf("User added successfully!\n");

}



// Function to display all users

void readUsers() {

    FILE *file = fopen(FILENAME, "r"); // "r" for read mode

    if (file == NULL) {

        printf("No records found or file cannot be opened.\n");

        return;

    }



    User user;

    printf("\n--- User Records ---\n");

    printf("ID\tName\t\tAge\n");

    printf("--------------------\n");



    // Read file line by line

    while (fscanf(file, "%d,%49[^,],%d\n", &user.id, user.name, &user.age) == 3) {

        printf("%d\t%-15s\t%d\n", user.id, user.name, user.age);

    }



    fclose(file);

}



// Function to update a user by ID

void updateUser() {

    int id_to_update;

    printf("Enter ID of user to update: ");

    scanf("%d", &id_to_update);



    FILE *file = fopen(FILENAME, "r");

    FILE *tempFile = fopen("temp.txt", "w");



    if (file == NULL || tempFile == NULL) {

        printf("Error opening files!\n");

        return;

    }



    User user;

    int found = 0;

    while (fscanf(file, "%d,%49[^,],%d\n", &user.id, user.name, &user.age) == 3) {

        if (user.id == id_to_update) {

            found = 1;

            printf("Enter new Name: ");

            scanf(" %[^\n]", user.name);

            printf("Enter new Age: ");

            scanf("%d", &user.age);

        }

        fprintf(tempFile, "%d,%s,%d\n", user.id, user.name, user.age);

    }



    fclose(file);

    fclose(tempFile);



    remove(FILENAME); // Delete original file

    rename("temp.txt", FILENAME); // Rename temp file to original



    if (found) {

        printf("User updated successfully!\n");

    } else {

        printf("User ID not found.\n");

    }

}



// Function to delete a user by ID

void deleteUser() {

    int id_to_delete;

    printf("Enter ID of user to delete: ");

    scanf("%d", &id_to_delete);



    FILE *file = fopen(FILENAME, "r");

    FILE *tempFile = fopen("temp.txt", "w");



    if (file == NULL || tempFile == NULL) {

        printf("Error opening files!\n");

        return;

    }



    User user;

    int found = 0;

    while (fscanf(file, "%d,%49[^,],%d\n", &user.id, user.name, &user.age) == 3) {

        if (user.id == id_to_delete) {

            found = 1; // Mark as found and skip writing to temp file

        } else {

            fprintf(tempFile, "%d,%s,%d\n", user.id, user.name, user.age);

        }

    }

    

    fclose(file);

    fclose(tempFile);



    remove(FILENAME);

    rename("temp.txt", FILENAME);



    if (found) {

        printf("User deleted successfully!\n");

    } else {

        printf("User ID not found.\n");

    }

}





int main() {

    int choice;

    do {

        printf("\n--- File CRUD Operations ---\n");

        printf("1. Add User\n");

        printf("2. Display All Users\n");

        printf("3. Update User\n");

        printf("4. Delete User\n");

        printf("5. Exit\n");

        printf("Enter your choice: ");

        scanf("%d", &choice);



        switch (choice) {

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

                printf("Exiting program.\n");

                break;

            default:

                printf("Invalid choice. Please try again.\n");

        }

    } while (choice != 5);



    return 0;

}