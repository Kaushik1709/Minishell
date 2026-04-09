#include "header.h"

char prompt[25] = "Minishell$: ";
char input_string[256];
int main()
{
    system("clear");
    scan_input(prompt,input_string);
}