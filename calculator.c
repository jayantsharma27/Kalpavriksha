#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <stddef.h> // For size_t and alignment

#define ARENA_CAPACITY_BYTES (1024 * 4) // 4KB total memory for everything
#define STACK_MAX_DEPTH 256            // Max depth for our stacks

// --- The Arena Allocator ---
typedef struct {
    char* memory;
    size_t used;
    size_t capacity;
} Arena;

void* arena_alloc(Arena* arena, size_t size) {
    // Align the size to the next multiple of a pointer size for safety
    size_t aligned_size = (size + sizeof(void*) - 1) & ~(sizeof(void*) - 1);

    if (arena->used + aligned_size > arena->capacity) {
        fprintf(stderr, "Error: Arena out of memory.\n");
        return NULL; // Out of memory
    }
    
    void* ptr = arena->memory + arena->used;
    arena->used += aligned_size;
    return ptr;
}

// --- Helper Functions (No changes) ---
int precedence(char op);

// --- The Core Logic (Now uses memory from the arena) ---
void applyOp(int* values_top, char* ops_top) {
    char op = *(--ops_top);
    int val2 = *(--values_top);
    int val1 = *(--values_top);

    switch (op) {
        case '+': *values_top = val1 + val2; break;
        case '-': *values_top = val1 - val2; break;
        case '*': *values_top = val1 * val2; break;
        case '/':
            if (val2 == 0) {
                fprintf(stderr, "Error: Division by zero.\n");
                exit(EXIT_FAILURE);
            }
            *values_top = val1 / val2;
            break;
    }
}

int evaluateExpression(const char* expression, Arena* arena) {
    // Allocate our stacks directly from the arena. No more dynamic structs.
    int* values_stack = (int*)arena_alloc(arena, STACK_MAX_DEPTH * sizeof(int));
    char* ops_stack = (char*)arena_alloc(arena, STACK_MAX_DEPTH * sizeof(char));

    // Pointers to the current top of each stack
    int* values_top = values_stack;
    char* ops_top = ops_stack;

    for (int i = 0; expression[i]; i++) {
        if (isspace(expression[i])) continue;
        
        if (isdigit(expression[i])) {
            int num = 0;
            while (isdigit(expression[i])) {
                num = (num * 10) + (expression[i] - '0');
                i++;
            }
            *(values_top++) = num;
            i--;
        } else {
            while (ops_top > ops_stack && precedence(*(ops_top - 1)) >= precedence(expression[i])) {
                applyOp(values_top--, ops_top--);
            }
            *(ops_top++) = expression[i];
        }
    }

    while (ops_top > ops_stack) {
        applyOp(values_top--, ops_top--);
    }
    
    return values_stack[0];
}

int main() {
    // --- Step 1: Create the Arena ---
    Arena arena = {
        .memory = malloc(ARENA_CAPACITY_BYTES),
        .capacity = ARENA_CAPACITY_BYTES,
        .used = 0
    };
    if (arena.memory == NULL) return 1;

    // --- Step 2: Read expression into a temporary buffer ---
    char temp_buffer[1024];
    printf("Enter a mathematical expression: ");
    if (fgets(temp_buffer, sizeof(temp_buffer), stdin) == NULL) {
        free(arena.memory);
        return 1;
    }
    temp_buffer[strcspn(temp_buffer, "\n")] = 0;

    // --- Step 3: Copy the expression into the arena ---
    size_t expr_len = strlen(temp_buffer);
    char* expression_in_arena = (char*)arena_alloc(&arena, expr_len + 1);
    strcpy(expression_in_arena, temp_buffer);

    // --- Step 4: Validation (on the arena copy) ---
    bool is_valid = true;
    for (int i = 0; expression_in_arena[i]; i++) {
        if (!isdigit(expression_in_arena[i]) && !isspace(expression_in_arena[i]) &&
            strchr("+-*/", expression_in_arena[i]) == NULL) {
            is_valid = false;
            break;
        }
    }

    if (!is_valid) {
        printf("Error: Invalid expression.\n");
    } else {
        // --- Step 5: Evaluate ---
        int result = evaluateExpression(expression_in_arena, &arena);
        printf("Result: %d\n", result);
    }

    // --- Step 6: Free the entire arena in one go ---
    free(arena.memory);
    printf("\nArena freed. Program finished.\n");

    return 0;
}

// --- Collapsed Precedence Function ---
int precedence(char op) {
    if (op == '+' || op == '-') return 1;
    if (op == '*' || op == '/') return 2;
    return 0;
}