#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include <limits.h>

int mallocCount = 0;
int reallocCount = 0;
int freeCount = 0;

typedef struct
{
    char* tocken;
    struct Node* left;
    struct Node* right;
} Node;

typedef struct
{
    char** items;
    int length;
} Lexems;

typedef struct
{
    char* value;
    struct Stack* next;
} Stack;

typedef struct
{
    char key;
    int value;
}Arg;

void getTreeArguments(Node* tree, Stack** args);

void Push(Stack** stack, char* str)
{
    Stack* newItem = (Stack*)malloc(sizeof(Stack));
    mallocCount++;
    newItem->value = (char*)malloc(sizeof(char) * strlen(str));
    mallocCount++;
    newItem->next = *stack;
    strcpy(newItem->value, str);
    *stack = newItem;
}

char* Pop(Stack** stack)
{
    if (*stack == NULL)
        return NULL;
    char* v = (*stack)->value;
    Stack* tmp = *stack;
    *stack = tmp->next;
    tmp->value = NULL;
    free(tmp->value);
    freeCount++;
    tmp = NULL;
    free(tmp);
    freeCount++;
    return v;
}

int getCommandIndex(char* command)
{
    if (strcmp(command, "parse") == 0)
        return 1;
    if (strcmp(command, "load_prf") == 0)
        return 2;
    if (strcmp(command, "load_pst") == 0)
        return 3;
    if (strcmp(command, "save_prf") == 0)
        return 4;
    if (strcmp(command, "save_pst") == 0)
        return 5;
    if (strcmp(command, "eval") == 0)
        return 6;
    return 0;
}

int Priority(char oper)
{
    switch (oper)
    {
        case '(':
        case ')':
            return 4;
        case '@':
            return 3;
        case '^':
            return 2;
        case '*':
        case '/':
        case '%':
            return 1;
        case '+':
        case '-':
            return 0;
        default:
            return 10;
    }
}

int isOperator(char c)
{
    int p = Priority(c);
    if (p >= 0 && p <= 3)
        return 1;
    return 0;
}

Lexems* SplitExpr(char* inputExpr)
{
    Lexems* lexems = (Lexems*)malloc(sizeof(Lexems));
    mallocCount++;
    int i = 0, j, length = 0;
    lexems->items = (char**)malloc(sizeof(char*));
    mallocCount++;
    while (inputExpr[i])
    {
        j = 0;
        if (isspace(inputExpr[i]))
        {
            i++;
            continue;
        }
        lexems->items = realloc(lexems->items, sizeof(char*)*(length + 1));
        reallocCount++;
        lexems->items[length] = (char*)malloc(sizeof(char));
        mallocCount++;
        if (!isdigit(inputExpr[i]))
        {
            lexems->items[length][j++] = inputExpr[i++];
            lexems->items[length++][j] = 0;
            continue;
        }
        while (isdigit(inputExpr[i]))
        {
            lexems->items[length] = realloc(lexems->items[length], sizeof(char)*(j + 1));
            reallocCount++;
            lexems->items[length][j] = inputExpr[i];
            j++;
            i++;
        }
        lexems->items[length][j] = 0;
        length++;
    }
    lexems->length = length;
    return lexems;
}

void FreeLexems(Lexems* lex)
{
    int i;
    for (i = 0; i < lex->length; i++)
    {
        lex->items[i] = NULL;
        free(lex->items[i]);
        freeCount++;
    }
    free(lex->items);
    freeCount++;
    free(lex);
    freeCount++;
}

void FreeTree(Node* tree)
{
    if (tree != NULL)
    {
        FreeTree(tree->right);
        tree->tocken = NULL;
        free(tree->tocken);
        freeCount++;
        FreeTree(tree->left);
        free(tree);
        freeCount++;
    }
}

Lexems* PrefixToInfix(Lexems* l)
{
    Lexems* lexems = (Lexems*)malloc(sizeof(Lexems));
    mallocCount++;
    Stack* stack = NULL;
    for (int i = l->length - 1; i >= 0; i--)
    {
        if (Priority(l->items[i][0]) < 10)
        {
            char*str1, *str2, *str;
            str1 = Pop(&stack);
            str2 = Pop(&stack);
            str1 = str1 ? str1 : "";
            str2 = str2 ? str2 : "";
            str = (char*)malloc(sizeof(char)*(strlen(str1) + strlen(str2) + 2 + strlen(l->items[i])));
            mallocCount++;
            str[0] = 0;
            strcat(str, "(");
            strcat(str, str1);
            strcat(str, l->items[i]);
            strcat(str, str2);
            strcat(str, ")");
            Push(&stack, str);
        }
        else
        {
            Push(&stack, l->items[i]);
        }
    }
    lexems = SplitExpr(Pop(&stack));
    return lexems;
}


Lexems*PostfixToInfix(Lexems*l)
{
    Lexems*lexems = (Lexems*)malloc(sizeof(Lexems));
    mallocCount++;
    Stack*stack = NULL;
    for (int i = 0; i<l->length; i++)
    {
        if (Priority(l->items[i][0]) < 10)
        {
            char*str1, *str2, *str;
            str1 = Pop(&stack);
            str2 = Pop(&stack);
            str1 = str1 ? str1 : "";
            str2 = str2 ? str2 : "";
            str = (char*)malloc(sizeof(char)*(strlen(str1) + strlen(str2) + 2 + strlen(l->items[i])));
            mallocCount++;
            str[0] = 0;
            strcat(str, "(");
            strcat(str, str2);
            strcat(str, l->items[i]);
            strcat(str, str1);
            strcat(str, ")");
            Push(&stack, str);
            
        }
        else
        {
            Push(&stack, l->items[i]);
        }
    }
    lexems = SplitExpr(Pop(&stack));
    return lexems;
}

void SavePrefix(Node* tree, FILE* OUT)
{
    if (tree == NULL)
    {
        return;
    }
    fprintf(OUT, "%s", tree->tocken);
    if (tree->left != NULL)
        fprintf(OUT, "(");
    SavePrefix(tree->left, OUT);
    if (tree->left != NULL)
        fprintf(OUT, ", ");
    SavePrefix(tree->right, OUT);
    if (tree->right != NULL)
        fprintf(OUT, ")");
}

void SavePostfix(Node* tree, FILE* OUT)
{
    if (tree == NULL)
    {
        return;
    }
    if (tree->left != NULL)
        fprintf(OUT, "(");
    SavePostfix(tree->left, OUT);
    if (tree->left != NULL)
        fprintf(OUT, ", ");
    SavePostfix(tree->right, OUT);
    if (tree->right != NULL)
        fprintf(OUT, ")");
    fprintf(OUT, "%s", tree->tocken);
}


Node* Parse(Lexems* lex, int first, int last)
{
    int p, k;
    int open = 0;
    Node* tree = (Node*)malloc(sizeof(Node));
    mallocCount++;
    if (first > last)
    {
        return NULL;
    }
    if (first == last)
    {
        tree->tocken = (char*)malloc(sizeof(char) * strlen(lex->items[first]));
        mallocCount++;
        strcpy(tree->tocken, lex->items[first]);
        tree->left = NULL;
        tree->right = NULL;
        return tree;
    }
    p = 10;
    for (int i = first; i <= last; i++)
    {
        if (strcmp(lex->items[i], "(") == 0)
        {
            open++;
            continue;
        }
        if (strcmp(lex->items[i], ")") == 0)
        {
            open--;
            continue;
        }
        if (open > 0)
            continue;
        int pr = Priority(lex->items[i][0]);
        if (pr <= p)
        {
            p = pr;
            k = i;
        }
    }
    if (p == 10 && strcmp(lex->items[first], "(") == 0 && strcmp(lex->items[last], ")") == 0)
    {
        free(tree);
        freeCount++;
        return Parse(lex, first + 1, last - 1);
    }
    tree->tocken = (char*)malloc(sizeof(char) * strlen(lex->items[k]));
    mallocCount++;
    strcpy(tree->tocken, lex->items[k]);
    tree->left = Parse(lex, first, k - 1);
    tree->right = Parse(lex, k + 1, last);
    return tree;
}

/// ÕŒƒ
int gcd(int a, int b)
{
    if (b == 0)
        return a;
    else
        return gcd(b, a % b);
}

/// ÕŒ
int lcm(int a, int b)
{
    return a / gcd(a, b) * b;
}

int GetArgValue(Arg* arguments, int size, char arg)
{
    for (int i = 0; i < size; i++)
    {
        if (arguments[i].key == arg)
            return arguments[i].value;
    }
    
    return INT_MIN;
}

int Evaluate(Node* tree, Arg* arguments, int size)
{
    int num1;
    int num2;
    if (tree == NULL)
        return 0;
    if (tree->right == NULL && tree->left == NULL)
    {
        int x;
        if (sscanf(tree->tocken, "%i", &x) < 1)
        {
            x = GetArgValue(arguments, size, *tree->tocken);
        }
        return x;
    }
    num1 = Evaluate(tree->left, arguments, size);
    num2 = Evaluate(tree->right, arguments, size);
    switch (tree->tocken[0])
    {
        case '^':
            return pow(num1, num2);
        case'@':
            return lcm(num1, num2);
        case'*':
            return num1 * num2;
        case'/':
            if (num2 == 0)
            {
                num1 = INT_MIN;
                break;
            }
            return num1 / num2;
        case'%':
            return num1 % num2;
        case'+':
            return num1 + num2;
        case'-':
            return num1 - num2;
    }
    return INT_MIN;
}

Arg*ReadArguments(FILE*IN, int* size)
{
    char c;
    int x;
    *size = 0;
    Arg*a = (Arg*)malloc(sizeof(Arg));
    mallocCount++;
    if ((c = fgetc(IN)) == EOF)
        return NULL;
    if (c == '\n' || c != ' ')
    {
        fseek(IN, -1, SEEK_CUR);
        return NULL;
    }
    while ((c = getc(IN)) != '\n' && c != EOF)
    {
        if (fscanf(IN, "=%i", &x) < 1)
        {
            a = NULL;
            free(a);
            freeCount++;
            *size = -1;
            while ((c = getc(IN)) != '\n' && c != EOF);
            return NULL;
        }
        a = (Arg*)realloc(a, sizeof(Arg)*(*size + 1));
        reallocCount++;
        a[*size].key = c;
        a[*size].value = x;
        (*size)++;
        c = getc(IN);
        if (c == '\n' || c == EOF)
            break;
        if (c != ',')
        {
            a = NULL;
            free(a);
            freeCount++;
            *size = -1;
            while ((c = getc(IN)) != '\n' && c != EOF);
            return NULL;
        }
    }
    return a;
}

int CheckTreeArgument(Node* tree, Arg* args, int size)
{
    Stack*argsInTree = NULL;
    getTreeArguments(tree, &argsInTree);
    int res = 0;
    char*a;
    if (argsInTree != NULL && args == NULL)
        return 0;
    if (size == -1)
        return 0;
    if (argsInTree == NULL)
        return 1;
    while ((a = Pop(&argsInTree)) != NULL)
    {
        res = 0;
        for (size_t i = 0; i < size; i++)
        {
            if (args[i].key == a[0])
                res = 1;
        }
        if (res == 0)
            break;
    }
    return res;
}

int CheckLexems(Lexems* l)
{
    int open = 0;
    int close = 0;
    if (l == NULL)
    {
        return 0;
    }
    for (int i = 0; i < l->length; i++)
    {
        if (strcmp(l->items[i], "(") == 0)
        {
            open++;
        }
        if (strcmp(l->items[i], ")") == 0)
        {
            close++;
        }
        if (close > open)
            return 0;
        if (!isalpha(l->items[i][0]) && !isdigit(l->items[i][0]) && Priority(l->items[i][0]) == 10)
            return 0;
    }
    if (open != close)
        return 0;
    
    for (int i = 0; i < l->length - 1; i++)
    {
        if ((isOperator(l->items[i][0]) != 0) && (strcmp(l->items[i + 1], ")") == 0))
        {
            return 0;
        }
        if (strcmp(l->items[i], "(") == 0 && isOperator(l->items[i + 1][0]))
        {
            return 0;
        }
        if (isOperator(l->items[i][0]) == 1 && isOperator(l->items[i + 1][0]) == 1)
        {
            return 0;
        }
        
        if (strcmp(l->items[i], "(") == 0 && strcmp(l->items[i + 1], ")") == 0)
            return 0;
        
        if ((isalpha(l->items[i][0]) || isdigit(l->items[i][0])) && strcmp(l->items[i + 1], "(") == 0)
            return 0;
        
        if (strcmp(l->items[i], ")") == 0 && (isalpha(l->items[i + 1][0]) || isdigit(l->items[i + 1][0])))
        {
            return 0;
        }
    }
    
    if (isOperator(l->items[0][0]) && l->items[0][0] != '-')
        return 0;
    
    return 1;
}

void getTreeArguments(Node* tree, Stack** args)
{
    if (tree == NULL)
    {
        return;
    }
    getTreeArguments(tree->left, args);
    if (Priority(tree->tocken[0]) == 10 && isalpha(tree->tocken[0]))
        Push(args, tree->tocken);
    getTreeArguments(tree->right, args);
}

void ReadFile(Node** tree)
{
    //*************************************
    FILE *InputFile, *OutputFile;
    char command[256];
    int i;
    Lexems* L;
    Lexems* tmpL;
    int evalResult;
    Arg* arguments;
    int argCount = 0;
    //*************************************
    
    InputFile = fopen("input.txt", "r");
    OutputFile = fopen("output.txt", "w");
    
    if (!InputFile)
        return;
    
    while (!feof(InputFile))
    {
        if(fscanf(InputFile, "%s", command) != 1)
            continue;
        switch (getCommandIndex(command))
        {
            case 1:
                fgets(command, 256, InputFile);
                L = SplitExpr(command);
                if (CheckLexems(L))
                {
                    if (*tree != NULL)
                    {
                        FreeTree(*tree);
                    }
                    *tree = Parse(L, 0, L->length - 1);
                    fprintf(OutputFile, "success\n");
                }
                else
                {
                    fprintf(OutputFile, "incorrect\n");
                }
                FreeLexems(L);
                break;
            case 2:
                fgets(command, 256, InputFile);
                tmpL = SplitExpr(command);
                L = PrefixToInfix(tmpL);
                if (CheckLexems(L))
                {
                    if (*tree != NULL)
                    {
                        FreeTree(*tree);
                    }
                    *tree = Parse(L, 0, L->length - 1);
                    fprintf(OutputFile, "success\n");
                }
                else
                {
                    fprintf(OutputFile, "incorrect\n");
                }
                FreeLexems(L);
                if (tmpL != NULL)
                    FreeLexems(tmpL);
                break;
            case 3:
                fgets(command, 256, InputFile);
                tmpL = SplitExpr(command);
                L = PostfixToInfix(tmpL);
                if (CheckLexems(L))
                {
                    if (*tree != NULL)
                    {
                        FreeTree(*tree);
                    }
                    *tree = Parse(L, 0, L->length - 1);
                    fprintf(OutputFile, "success\n");
                }
                else
                {
                    fprintf(OutputFile, "incorrect\n");
                }
                FreeLexems(L);
                if (tmpL != NULL)
                    FreeLexems(tmpL);
                break;
            case 4:
                if (*tree == NULL)
                {
                    fprintf(OutputFile, "not_loaded\n");
                }
                else
                {
                    SavePrefix(*tree, OutputFile);
                    fprintf(OutputFile, "\n");
                }
                break;
            case 5:
                if (*tree == NULL)
                {
                    fprintf(OutputFile, "not_loaded\n");
                }
                else
                {
                    SavePostfix(*tree, OutputFile);
                    fprintf(OutputFile, "\n");
                }
                break;
            case 6:
                arguments = ReadArguments(InputFile, &argCount);
                if (*tree == NULL)
                {
                    fprintf(OutputFile, "not_loaded\n");
                }
                else
                    if (CheckTreeArgument(*tree, arguments, argCount))
                    {
                        evalResult = Evaluate(*tree, arguments, argCount);
                        if (evalResult == INT_MIN)
                            fprintf(OutputFile, "error\n");
                        else
                            fprintf(OutputFile, "%i\n", evalResult);
                    }
                    else
                        fprintf(OutputFile, "no_var_values\n");
                arguments = NULL;
                free(arguments);
                freeCount++;
                break;
            default:
                fprintf(OutputFile, "incorrect\n");
                break;
        }
    }
    FreeTree(*tree);
    fclose(InputFile);
    fclose(OutputFile);
}

void main()
{
    FILE* StatFile;
    StatFile = fopen("memstat.txt", "w");
    Node* tree = NULL;
    ReadFile(&tree);
    fprintf(StatFile, "malloc:%i\nrealloc:%i\nfree:%i", mallocCount, reallocCount, freeCount);
    fclose(StatFile);
}

