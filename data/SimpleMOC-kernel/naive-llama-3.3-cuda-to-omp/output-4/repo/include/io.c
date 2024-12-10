#include "SimpleMOC-kernel_header.h"
#include <omp.h>

// Prints program logo
void logo(int version)
{
    border_print();
    printf(
"   __           __        ___        __   __           ___  __        ___     \n"
"  /__` |  |\\/| |__) |    |__   |\\/| /  \\ /  ` __ |__/ |__  |__) |\\ | |__  |   \n"
"  .__/ |  |  | |    |___ |___  |  | \\__/ \\__,    |  \\ |___ |  \\ | \\| |___ |___\n" 
"\n"
"                         %26s%43s%10s\n" 
"                        %27s%44s%9s\n"
"                        %15s     %8s   %17s%12s\n"
"                        %15s     %8s   %17s%12s\n"
"                        %40s%20s%11s  %6s\n"
"                         %24s %18s %10s  %6s\n"
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
        fputs(" ", stdout);
    }
    fputs(s, stdout);
    fputs("\n", stdout);
}

// Prints a border
void border_print(void)
{
    printf(
"==============================================================================\n");
}

// Prints comma separated integers - for ease of reading
void fancy_int( int a )
{
    if( a < 1000 )
        printf("%d\n",a);

    else if( a >= 1000 && a < 1000000 )
        printf("%d,%03d\n", a / 1000, a % 1000);

    else if( a >= 1000000 && a < 1000000000 )
        printf("%d,%03d,%03d\n", a / 1000000, (a % 1000000) / 1000, a % 1000 );

    else if( a >= 1000000000 )
        printf("%d,%03d,%03d,%03d\n",
               a / 1000000000,
               (a % 1000000000) / 1000000,
               (a % 1000000) / 1000,
               a % 1000 );
    else
        printf("%d\n",a);
}

// Prints out the summary of User input
void print_input_summary(Input I)
{
    center_print("INPUT SUMMARY", 79);
    border_print();

    #pragma omp target map(from: I.source_2D_regions, I.coarse_axial_intervals, 
                           I.fine_axial_intervals, I.decomp_assemblies_ax, 
                           I.segments, I.egroups, I.nthreads, I.streams)
    {
        printf("%-25s%d\n", "Energy Groups:", I.egroups);
        printf("%-25s%d\n", "2D Source Regions:", I.source_2D_regions);
        printf("%-25s%d\n", "Coarse Axial Intervals:", I.coarse_axial_intervals);
        printf("%-25s%d\n", "Fine Axial Intervals:", I.fine_axial_intervals);
        printf("%-25s%d\n", "Axial Decomposition:", I.decomp_assemblies_ax);
        printf("%-25s", "Segments:"); fancy_int(I.segments);
        printf("%-25s", "Random Number Streams:"); fancy_int(I.streams);
    }
    border_print();
}

// reads command line inputs and applies options
void read_CLI( int argc, char * argv[], Input * input )
{
    // Collect Raw Input
    for( int i = 1; i < argc; i++ )
    {
        char * arg = argv[i];

        // nthreads (-t)
        if( strcmp(arg, "-t") == 0 )
        {
            if( ++i < argc )
                input->nthreads = atoi(argv[i]);
            else
                print_CLI_error();
        }

        // segments (-s)
        else if( strcmp(arg, "-s") == 0 )
        {
            if( ++i < argc )
                input->segments = atoi(argv[i]);
            else
                print_CLI_error();
        }
        
        // egroups (-e)
        else if( strcmp(arg, "-e") == 0 )
        {
            if( ++i < argc )
                input->egroups = atoi(argv[i]);
            else
                print_CLI_error();
        }
        // segments per thread (-p)
        else if( strcmp(arg, "-p") == 0 )
        {
            if( ++i < argc )
                input->seg_per_thread = atoi(argv[i]);
            else
                print_CLI_error();
        }
    }
}

// print error to screen, inform program options
void print_CLI_error(void)
{
    printf("Usage: ./SimpleMOC <options>\n");
    printf("Options include:\n");
    printf("  -t <threads>          Number of OpenMP threads to run\n");
    printf("  -s <segments>         Number of segments to process\n");
    printf("  -e <energy groups>    Number of energy groups\n");
    printf("  -p <segs per thread>  Number of segments per CUDA Block\n");
    printf("See readme for full description of default run values\n");
    exit(1);
}