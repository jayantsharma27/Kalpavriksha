#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h> 
#include <stdbool.h>
#include <stddef.h>
#include <limits.h>

#define ARENA_CAPACITY_BYTES (1024 * 4)
#define STACK_MAX_DEPTH 256

// --- The Arena Allocator ---
typedef struct
{
    char *memory;
    size_t used;
    size_t capacity;
} Arena;


bool isExpressionSyntaxValid(const char *expr);
long long evaluateExpression(const char *expression, Arena *arena);

void *arena_alloc(Arena *arena, size_t size)
{

    size_t aligned_size = (size + sizeof(void *) - 1) & ~(sizeof(void *) - 1);

    if (arena->used + aligned_size > arena->capacity)
    {
        return NULL;
    }

    void *ptr = arena->memory + arena->used;
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
void applyOp(long long *values_top, char *ops_top)
{
    char op = *(--ops_top);
    long long val2 = *(--values_top);
    long long val1 = *(--values_top);

    // Overflow checks
    if (op == '+' && ((val2 > 0 && val1 > LLONG_MAX - val2) || (val2 < 0 && val1 < LLONG_MIN - val2)))
    {
        fprintf(stderr, "Error: Arithmetic overflow during addition.\n");
        exit(EXIT_FAILURE);
    }
    if (op == '-' && ((val2 > 0 && val1 < LLONG_MIN + val2) || (val2 < 0 && val1 > LLONG_MAX + val2)))
    {
        fprintf(stderr, "Error: Arithmetic overflow during subtraction.\n");
        exit(EXIT_FAILURE);
    }
    if (op == '*' && val1 != 0 && val2 != 0)
    {
        if ((val1 > 0 && val2 > 0 && val1 > LLONG_MAX / val2) ||
            (val1 < 0 && val2 < 0 && val1 < LLONG_MAX / val2) ||
            (val1 > 0 && val2 < 0 && val2 < LLONG_MIN / val1) ||
            (val1 < 0 && val2 > 0 && val1 < LLONG_MIN / val2))
        {
            fprintf(stderr, "Error: Arithmetic overflow during multiplication.\n");
            exit(EXIT_FAILURE);
        }
    }

    switch (op)
    {
    case '+':
        *values_top = val1 + val2;
        break;
    case '-':
        *values_top = val1 - val2;
        break;
    case '*':
        *values_top = val1 * val2;
        break;
    case '/':
        if (val2 == 0)
        {
            fprintf(stderr, "Error: Division by zero.\n");
            exit(EXIT_FAILURE);
        }
        if (val1 == LLONG_MIN && val2 == -1)
        {
            fprintf(stderr, "Error: Arithmetic overflow during division.\n");
            exit(EXIT_FAILURE);
        }
        *values_top = val1 / val2;
        break;
    }
}


long long evaluateExpression(const char *expression, Arena *arena)
{
    long long *values_stack = (long long *)arena_alloc(arena, STACK_MAX_DEPTH * sizeof(long long));
    char *ops_stack = (char *)arena_alloc(arena, STACK_MAX_DEPTH * sizeof(char));

    if (values_stack == NULL || ops_stack == NULL)
    {
        fprintf(stderr, "Error: Failed to allocate memory for stacks from arena.\n");
        exit(EXIT_FAILURE);
    }

    long long *values_top = values_stack;
    char *ops_top = ops_stack;

    enum
    {
        EXPECT_OPERAND,
        EXPECT_OPERATOR
    } state = EXPECT_OPERAND;

    for (int i = 0; expression[i]; i++)
    {
        if (isspace(expression[i]))
        {
            continue;
        }

        if (state == EXPECT_OPERAND)
        {
            long long sign = 1;
            // Handle chains of unary operators like --5 or + - 5
            while (expression[i] == '+' || expression[i] == '-')
            {
                if (expression[i] == '-')
                    sign *= -1;
                i++;
                while (isspace(expression[i]))
                    i++;
            }

            
            if (!isdigit(expression[i]))
            {
                fprintf(stderr, "Error: Invalid expression syntax - expected number.\n");
                exit(EXIT_FAILURE);
            }

            long long num = 0;
            while (isdigit(expression[i]))
            {
                
                if (num > (LLONG_MAX - (expression[i] - '0')) / 10)
                {
                    fprintf(stderr, "Error: Number in expression exceeds maximum value.\n");
                    exit(EXIT_FAILURE);
                }
                num = (num * 10) + (expression[i] - '0');
                i++;
            }
            *(values_top++) = num * sign;
            i--;
            state = EXPECT_OPERATOR;
        }
        else // state == EXPECT_OPERATOR
        {
            // Handle binary operators
            while (ops_top > ops_stack && precedence(*(ops_top - 1)) >= precedence(expression[i]))
            {
                applyOp(values_top--, ops_top--);
            }
            *(ops_top++) = expression[i];
            state = EXPECT_OPERAND;
        }
    }

    while (ops_top > ops_stack)
    {
        applyOp(values_top--, ops_top--);
    }

    return values_stack[0];
}


bool isExpressionSyntaxValid(const char *expr)
{
    enum
    {
        EXPECT_OPERAND,
        EXPECT_OPERATOR
    } state = EXPECT_OPERAND;
    int i = 0;

    bool found_non_space = false;
    for (int k = 0; expr[k]; k++)
    {
        if (!isspace(expr[k]))
        {
            found_non_space = true;
            break;
        }
    }
    if (!found_non_space)
        return false; // Expression is empty or only whitespace

    while (expr[i])
    {
        if (isspace(expr[i]))
        {
            i++;
            continue;
        }

        if (isdigit(expr[i]))
        {
            if (state != EXPECT_OPERAND)
                return false;
            state = EXPECT_OPERATOR;
            while (isdigit(expr[i]))
                i++;
        }
        else if (expr[i] == '+' || expr[i] == '-')
        {
            if (state == EXPECT_OPERAND)
            { // Unary operator
              // State remains EXPECT_OPERAND
            }
            else
            { // Binary operator
                state = EXPECT_OPERAND;
            }
            i++;
        }
        else if (expr[i] == '*' || expr[i] == '/')
        {
            if (state != EXPECT_OPERATOR)
                return false;
            state = EXPECT_OPERAND;
            i++;
        }
        else
        {
            return false;
        }
    }

    return state == EXPECT_OPERATOR;
}

int main()
{
    Arena arena = {
        .memory = malloc(ARENA_CAPACITY_BYTES),
        .capacity = ARENA_CAPACITY_BYTES,
        .used = 0};
    if (arena.memory == NULL)
    {
        fprintf(stderr, "Error: Failed to initialize memory arena.\n");
        return 1;
    }

    char choice;
    do
    {
        arena.used = 0; // Reset arena for each new calculation

        char temp_buffer[1024];
        printf("\nEnter a mathematical expression: ");
        if (fgets(temp_buffer, sizeof(temp_buffer), stdin) == NULL)
        {
            break;
        }
        temp_buffer[strcspn(temp_buffer, "\n")] = 0;

        size_t expr_len = strlen(temp_buffer);
        char *expression_in_arena = (char *)arena_alloc(&arena, expr_len + 1);

        strcpy(expression_in_arena, temp_buffer);

        if (!isExpressionSyntaxValid(expression_in_arena))
        {
            printf("Error: Invalid expression syntax.\n");
        }
        else
        {
            long long result = evaluateExpression(expression_in_arena, &arena);
            printf("Result: %lld\n", result);
        }

        printf("\nPress 'c' to continue : ");
        scanf(" %c", &choice);
        while (getchar() != '\n')
            ;

    } while (choice == 'c' || choice == 'C');

    free(arena.memory);
    printf("\nArena freed. Program finished.\n");

    return 0;
}
