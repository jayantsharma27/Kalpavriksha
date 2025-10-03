/******************************************************************************
 * Author       : Jayant Sharma
 * File         : calculator.c
 * Description  : A command-line calculator that evaluates mathematical expressions from a string, respecting the order of operations.
 *****************************************************************************/

 #include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h> // Character Type isdigit(), isspace()
#include <stdbool.h>
#include <stddef.h> // Standard Definitions size_t

#define ARENA_CAPACITY_BYTES (1024 * 4) // #define preprocessor macro  replaces every instance of the macro name with its value
#define STACK_MAX_DEPTH 256

// --- The Arena Allocator ---
// one allocation one free() call vs regular malloc() & free()
typedef struct {
    char* memory;
    size_t used; // size_t (Code works on 32-bit and 64-bit systems) vs int vs unsigned int
    size_t capacity;
} Arena;

void* arena_alloc(Arena* arena, size_t size) { // void* generic pointer, Areana * pointer to Arena struct to modify its used field

    size_t aligned_size = (size + sizeof(void *) - 1) & ~(sizeof(void *) - 1); // memory alignment, magic padding and then round down to the last full row ie multiple of sizeof(void *)

    if (arena->used + aligned_size > arena->capacity) {
        fprintf(stderr, "Error: Arena out of memory.\n"); // Standard Error, redirect the "clean" output while still seeing the errors on your screen, unbuffered
        return NULL;
    }
    
    void* ptr = arena->memory + arena->used;
    arena->used += aligned_size;
    return ptr;
}

int precedence(char op)
{
    if (op == '+' || op == '-')
        return 1;
    if (op == '*' || op == '/')
        return 2;
    return 0;
}

// --- The Operational Logic ---
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

// --- The Evaluation Logic ---
int evaluateExpression(const char* expression, Arena* arena) {
    int* values_stack = (int*)arena_alloc(arena, STACK_MAX_DEPTH * sizeof(int));
    char* ops_stack = (char*)arena_alloc(arena, STACK_MAX_DEPTH * sizeof(char));

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
    
    Arena arena = {
        .memory = malloc(ARENA_CAPACITY_BYTES),
        .capacity = ARENA_CAPACITY_BYTES,
        .used = 0
    };
    if (arena.memory == NULL) return 1;

    
    char temp_buffer[1024];
    printf("Enter a mathematical expression: ");
    if (fgets(temp_buffer, sizeof(temp_buffer), stdin) == NULL) {
        free(arena.memory);
        return 1;
    }
    temp_buffer[strcspn(temp_buffer, "\n")] = 0;

    
    size_t expr_len = strlen(temp_buffer);
    char* expression_in_arena = (char*)arena_alloc(&arena, expr_len + 1);
    strcpy(expression_in_arena, temp_buffer);

    
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
        
        int result = evaluateExpression(expression_in_arena, &arena);
        printf("Result: %d\n", result);
    }

    free(arena.memory);
    printf("\nArena freed. Program finished.\n");

    return 0;
}
