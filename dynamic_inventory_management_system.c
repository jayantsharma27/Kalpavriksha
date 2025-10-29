#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#define MIN_PRODUCTS 1
#define MAX_PRODUCTS 100
#define MAX_NAME_LENGTH 51

typedef struct ProductInfo
{
    int productId;
    char name[MAX_NAME_LENGTH];
    float price;
    int quantity;
} ProductInfo;

ProductInfo *Inventory = NULL;
int ProductCount = 0;
int ProductCapacity = 0;

void displayMenu();
bool addProduct();
void viewAllProducts();
void updateQuantity();
void searchProductById();
void searchProductByName();
void searchProductByPriceRange();
bool deleteProduct();
void cleanupMemory();
void clearInputBuffer();
bool readString(char *buffer, int size);
bool isIdUnique(int id);
int getIdInput(const char *prompt);
bool checkEmpty(const char *operation);
int findIndexById(int id);

int main()
{
    int noOfProducts;

    printf("Enter initial number of products (1-100): ");
    while (1)
    {
        if (scanf("%d", &noOfProducts) == 1 && noOfProducts >= MIN_PRODUCTS && noOfProducts <= MAX_PRODUCTS)
        {
            clearInputBuffer();
            break;
        }
        printf("Invalid input. Please enter a number between 1 and 100: ");
        clearInputBuffer();
    }

    Inventory = (ProductInfo *)calloc(noOfProducts, sizeof(ProductInfo));
    if (Inventory == NULL)
    {
        fprintf(stderr, "Error: Initial memory allocation failed!\n");
        return 1;
    }
    ProductCapacity = noOfProducts;
    ProductCount = 0;

    printf("\nEnter details for the %d products:\n", noOfProducts);
    for (int i = 0; i < noOfProducts; ++i)
    {
        printf("\n--- Enter details for product %d ---\n", i + 1);
        if (!addProduct())
        {
            fprintf(stderr, "Error: Failed to add 1st product %d. Exiting.\n", i + 1);
            cleanupMemory();
            return 1;
        }
    }

    int choice;
    do
    {
        displayMenu();
        printf("Enter your choice: ");
        if (scanf("%d", &choice) != 1)
        {
            clearInputBuffer();
            choice = -1;
        }
        else
        {
            clearInputBuffer();
        }

        switch (choice)
        {
        case 1:
            printf("\n--- Add New Product ---\n");
            if (addProduct())
            {
                printf("Product added successfully!\n");
            }
            break;
        case 2:
            viewAllProducts();
            break;
        case 3:
            updateQuantity();
            break;
        case 4:
            searchProductById();
            break;
        case 5:
            searchProductByName();
            break;
        case 6:
            searchProductByPriceRange();
            break;
        case 7:
            if (deleteProduct())
            {
                printf("Product deleted successfully!\n");
            }
            break;
        case 8:
            printf("Exiting program...\n");
            break;
        default:
            printf("Invalid choice. Please try again.\n");
            break;
        }
    } while (choice != 8);

    cleanupMemory();
    printf("Memory released successfully. Exiting program...\n");

    return 0;
}

void displayMenu()
{
    printf("\n========= INVENTORY MENU =========\n");
    printf("1. Add New Product\n");
    printf("2. View All Products\n");
    printf("3. Update Quantity\n");
    printf("4. Search Product by ID\n");
    printf("5. Search Product by Name\n");
    printf("6. Search Product by Price Range\n");
    printf("7. Delete Product\n");
    printf("8. Exit\n");
    printf("==================================\n");
}

int getIdInput(const char *prompt)
{
    int id;
    printf("%s", prompt);
    if (scanf("%d", &id) != 1)
    {
        clearInputBuffer();
        printf("Invalid ID format entered.\n");
        return -1;
    }
    clearInputBuffer();
    return id;
}

bool checkEmpty(const char *operation)
{
    if (ProductCount == 0)
    {
        printf("Inventory is empty. Cannot %s.\n", operation);
        return true;
    }
    return false;
}

int findIndexById(int id)
{
    for (int i = 0; i < ProductCount; i++)
    {
        if (Inventory[i].productId == id)
        {
            return i;
        }
    }
    return -1;
}

bool addProduct()
{
    if (ProductCount >= ProductCapacity)
    {
        int newCapacity = (ProductCapacity == 0) ? 1 : ProductCapacity * 2;
        ProductInfo *temp = (ProductInfo *)realloc(Inventory, newCapacity * sizeof(ProductInfo));
        if (temp == NULL)
        {
            fprintf(stderr, "Error: Memory reallocation failed when adding product!\n");
            return false;
        }
        Inventory = temp;
        ProductCapacity = newCapacity;
        printf("Inventory capacity increased to %d.\n", newCapacity);
    }

    ProductInfo newProduct;
    int tempId;
    float tempPrice;
    int tempQuantity;

    while (1)
    {
        printf("Product ID (1-10000): ");
        if (scanf("%d", &tempId) == 1 && tempId >= 1 && tempId <= 10000 && isIdUnique(tempId))
        {
            clearInputBuffer();
            newProduct.productId = tempId;
            break;
        }
        printf("Invalid or duplicate ID. Please enter a unique number between 1 and 10000.\n");
        clearInputBuffer();
    }

    printf("Product Name (max 50 chars): ");
    if (!readString(newProduct.name, MAX_NAME_LENGTH))
    {
        printf("Error reading product name.\n");
        return false;
    }
    if (strlen(newProduct.name) == 0 || strlen(newProduct.name) > 50)
    {
        printf("Invalid name. Name cannot be empty or exceed 50 characters.\n");
        return false;
    }

    while (1)
    {
        printf("Product Price (0.00-100000.00): ");
        if (scanf("%f", &tempPrice) == 1 && tempPrice >= 0.0 && tempPrice <= 100000.0)
        {
            clearInputBuffer();
            newProduct.price = tempPrice;
            break;
        }
        printf("Invalid price. Please enter a number between 0.00 and 100000.00.\n");
        clearInputBuffer();
    }

    while (1)
    {
        printf("Product Quantity (0-1000000): ");
        if (scanf("%d", &tempQuantity) == 1 && tempQuantity >= 0 && tempQuantity <= 1000000)
        {
            clearInputBuffer();
            newProduct.quantity = tempQuantity;
            break;
        }
        printf("Invalid quantity. Please enter a number between 0 and 1000000.\n");
        clearInputBuffer();
    }

    Inventory[ProductCount] = newProduct;
    ProductCount++;

    return true;
}

void viewAllProducts()
{
    printf("\n========= PRODUCT LIST =========\n");
    if (checkEmpty("view products"))
    {
    }
    else
    {
        for (int i = 0; i < ProductCount; i++)
        {
            printf("Product ID: %d | Name: %s | Price: %.2f | Quantity: %d\n",
                   Inventory[i].productId, Inventory[i].name,
                   Inventory[i].price, Inventory[i].quantity);
        }
    }
    printf("==================================\n");
}

void updateQuantity()
{
    if (checkEmpty("update quantity"))
    {
        return;
    }

    int idToUpdate;
    int newQuantity;

    idToUpdate = getIdInput("Enter Product ID to update quantity: ");
    if (idToUpdate == -1)
    {
        return;
    }

    int foundIndex = findIndexById(idToUpdate);

    if (foundIndex == -1)
    {
        printf("Product with ID %d not found.\n", idToUpdate);
    }
    else
    {
        while (1)
        {
            printf("Enter new Quantity (0-1000000): ");
            if (scanf("%d", &newQuantity) == 1 && newQuantity >= 0 && newQuantity <= 1000000)
            {
                clearInputBuffer();
                Inventory[foundIndex].quantity = newQuantity;
                printf("Quantity updated successfully!\n");
                break;
            }
            printf("Invalid quantity. Please enter a number between 0 and 1000000.\n");
            clearInputBuffer();
        }
    }
}

void searchProductById()
{
    if (checkEmpty("search by ID"))
    {
        return;
    }

    int idToSearch;

    idToSearch = getIdInput("Enter Product ID to search: ");
    if (idToSearch == -1)
    {
        return;
    }

    int foundIndex = findIndexById(idToSearch);

    if (foundIndex == -1)
    {
        printf("Product with ID %d not found.\n", idToSearch);
    }
    else
    {
        printf("Product Found: Product ID: %d | Name: %s | Price: %.2f | Quantity: %d\n",
               Inventory[foundIndex].productId, Inventory[foundIndex].name,
               Inventory[foundIndex].price, Inventory[foundIndex].quantity);
    }
}

void searchProductByName()
{
    if (checkEmpty("search by name"))
    {
        return;
    }

    char nameSubstring[MAX_NAME_LENGTH];
    char lowerSubstr[MAX_NAME_LENGTH];
    char lowerName[MAX_NAME_LENGTH];
    bool found = false;

    printf("Enter name to search (partial allowed, case-insensitive): ");
    if (!readString(nameSubstring, MAX_NAME_LENGTH) || strlen(nameSubstring) == 0)
    {
        printf("Invalid or empty search term entered.\n");
        return;
    }

    strcpy(lowerSubstr, nameSubstring);
    for (int k = 0; lowerSubstr[k]; k++)
    {
        lowerSubstr[k] = tolower((unsigned char)lowerSubstr[k]);
    }

    printf("Products Found:\n");
    for (int i = 0; i < ProductCount; i++)
    {
        strcpy(lowerName, Inventory[i].name);
        for (int k = 0; lowerName[k]; k++)
        {
            lowerName[k] = tolower((unsigned char)lowerName[k]);
        }

        if (strstr(lowerName, lowerSubstr) != NULL)
        {
            printf("Product ID: %d | Name: %s | Price: %.2f | Quantity: %d\n",
                   Inventory[i].productId, Inventory[i].name,
                   Inventory[i].price, Inventory[i].quantity);
            found = true;
        }
    }

    if (!found)
    {
        printf("No products found matching '%s'.\n", nameSubstring);
    }
}

void searchProductByPriceRange()
{
    if (checkEmpty("search by price range"))
    {
        return;
    }

    float minPrice, maxPrice;
    bool found = false;

    while (1)
    {
        printf("Enter minimum price: ");
        if (scanf("%f", &minPrice) == 1 && minPrice >= 0.0)
        {
            clearInputBuffer();
            break;
        }
        printf("Invalid minimum price. Please enter a non-negative number.\n");
        clearInputBuffer();
    }

    while (1)
    {
        printf("Enter maximum price: ");
        if (scanf("%f", &maxPrice) == 1 && maxPrice >= minPrice && maxPrice <= 100000.0)
        {
            clearInputBuffer();
            break;
        }
        printf("Invalid maximum price. Must be >= minimum price (%.2f) and <= 100000.00.\n", minPrice);
        clearInputBuffer();
    }

    printf("Products in price range [%.2f - %.2f]:\n", minPrice, maxPrice);
    for (int i = 0; i < ProductCount; i++)
    {
        if (Inventory[i].price >= minPrice && Inventory[i].price <= maxPrice)
        {
            printf("Product ID: %d | Name: %s | Price: %.2f | Quantity: %d\n",
                   Inventory[i].productId, Inventory[i].name,
                   Inventory[i].price, Inventory[i].quantity);
            found = true;
        }
    }

    if (!found)
    {
        printf("No products found in this price range.\n");
    }
}

bool deleteProduct()
{
    if (checkEmpty("delete product"))
    {
        return false;
    }

    int idToDelete;

    idToDelete = getIdInput("Enter Product ID to delete: ");
    if (idToDelete == -1)
    {
        return false;
    }

    int foundIndex = findIndexById(idToDelete);

    if (foundIndex == -1)
    {
        printf("Product with ID %d not found.\n", idToDelete);
        return false;
    }
    else
    {
        for (int i = foundIndex; i < ProductCount - 1; i++)
        {
            Inventory[i] = Inventory[i + 1];
        }
        ProductCount--;

        return true;
    }
}

void cleanupMemory()
{
    if (Inventory != NULL)
    {
        free(Inventory);
        Inventory = NULL;
    }
    ProductCount = 0;
    ProductCapacity = 0;
}

void clearInputBuffer()
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
        ;
}

bool readString(char *buffer, int size)
{
    if (fgets(buffer, size, stdin) == NULL)
    {
        buffer[0] = '\0';
        return false;
    }
    size_t len = strlen(buffer);
    if (len > 0 && buffer[len - 1] == '\n')
    {
        buffer[len - 1] = '\0';
    }
    else
    {
        clearInputBuffer();
    }
    return true;
}

bool isIdUnique(int id)
{
    for (int i = 0; i < ProductCount; i++)
    {
        if (Inventory[i].productId == id)
        {
            return false;
        }
    }
    return true;
}