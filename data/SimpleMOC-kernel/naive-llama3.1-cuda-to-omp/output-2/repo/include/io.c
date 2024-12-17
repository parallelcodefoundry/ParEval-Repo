#include "SimpleMOC-kernel_header.h"

// Prints program logo
void logo(int version)
{
    border_print();
    printf(
"   __           __        ___        __   __           ___  __        ___     \n"
"  /__` |  |\\/| |__) |    |__   |\\/| /  \\ /  ` __ |__/ |__  |__) |\\ | |__  |   \n"
"  .__/ |  |  | |    |___ |___  |  | \\__/ \\__,    |  \\ |___ |  \\ | \\| |___ |___\n" 
"\n"
"                         ������������������������������   ������������������������������  ������������������\n" 
"                        ���������������������������������   ���������������������������������������������������������\n"
"                        ���������     ���������   ������������������  ���������������������������������\n"
"                        ���������     ���������   ������������������  ���������������������������������\n"
"                        ������������������������������������������������������������������������������������  ���������\n"
"                         ��������������������� ��������������������� ��������������������� ���������  ���������\n"
    );
    printf("\n");
    border_print();
    printf("\n");

    center_print("Developed at", 79);
    center_print("The Massachusetts Institute of Technology", 79);
    center_print("and", 79);
    center_print("Argonne National Laboratory", 79);
    printf("\n");
    char v[100];
    sprintf(v, "Version: %d", version);
    center_print(v, 79);
    printf("\n");
    border_print();
}

// Prints Section titles in center of 80 char terminal
void center_print(const char *s, int width)
{
    int length = strlen(s);
    int i;
    for (i=0; i<=(width-length)/2; i++) {
        fprintf(stderr, " ");
    }
    fprintf(stderr, "%s\n", s);
}

// Prints a border
void border_print(void)
{
    printf(
"==================================================================="
"=============\n");
}