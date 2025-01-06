#include "SimpleMOC-kernel_header.hpp"
#include <Kokkos_Core.hpp>
#include <iostream>

// Prints program logo
void logo(int version)
{
    border_print();
    std::cout <<
"   __           __        ___        __   __           ___  __        ___     \n"
"  /__` |  |\\/| |__) |    |__   |\\/| /  \\ /  ` __ |__/ |__  |__) |\\ | |__  |   \n"
"  .__/ |  |  | |    |___ |___  |  | \\__/ \\__,    |  \\ |___ |  \\ | \\| |___ |___\n" 
"\n"
"                         ������������������������������   ������������������������������  ������������������\n" 
"                        ���������������������������������   ���������������������������������������������������������\n"
"                        ���������     ���������   ������������������  ���������������������������������\n"
"                        ���������     ���������   ������������������  ���������������������������������\n"
"                        ������������������������������������������������������������������������������������  ���������\n"
"                         ��������������������� ��������������������� ��������������������� ���������  ���������\n";
    std::cout << "\n";
    border_print();
    std::cout << "\n";

    center_print("Developed at", 79);
    center_print("The Massachusetts Institute of Technology", 79);
    center_print("and", 79);
    center_print("Argonne National Laboratory", 79);
    std::cout << "\n";
    char v[100];
    sprintf(v, "Version: %d", version);
    center_print(v, 79);
    std::cout << "\n";
    border_print();
}

// Prints Section titles in center of 80 char terminal
void center_print(const char *s, int width)
{
    int length = strlen(s);
    for (int i = 0; i <= (width - length) / 2; i++) {
        std::cout << " ";
    }
    std::cout << s << std::endl;
}

// Prints a border
void border_print(void)
{
    std::cout <<
    "==================================================================="
    "=============\n";
}

// Prints comma separated integers - for ease of reading
void fancy_int(int a)
{
    if (a < 1000)
        std::cout << a << std::endl;
    else if (a >= 1000 && a < 1000000)
        std::cout << a / 1000 << "," << std::setw(3) << std::setfill('0') << a % 1000 << std::endl;
    else if (a >= 1000000 && a < 1000000000)
        std::cout << a / 1000000 << "," << std::setw(3) << std::setfill('0') << (a % 1000000) / 1000 << "," << a % 1000 << std::endl;
    else if (a >= 1000000000)
        std::cout << a / 1000000000 << "," << std::setw(3) << std::setfill('0') << (a % 1000000000) / 1000000 << "," << (a % 1000000) / 1000 << "," << a % 1000 << std::endl;
    else
        std::cout << a << std::endl;
}

// Prints out the summary of User input
void print_input_summary(Input I)
{
    center_print("INPUT SUMMARY", 79);
    border_print();

    // Assuming Kokkos is initialized and we have access to device properties
    Kokkos::Device<Kokkos::DefaultExecutionSpace, Kokkos::DefaultHostExecutionSpace> device;
    std::cout << std::setw(25) << std::left << "Kokkos Device: " << Kokkos::DefaultExecutionSpace::name() << std::endl;
    std::cout << std::setw(25) << std::left << "Energy Groups:" << I.egroups << std::endl;
    std::cout << std::setw(25) << std::left << "2D Source Regions:" << I.source_2D_regions << std::endl;
    std::cout << std::setw(25) << std::left << "Coarse Axial Intervals:" << I.coarse_axial_intervals << std::endl;
    std::cout << std::setw(25) << std::left << "Fine Axial Intervals:" << I.fine_axial_intervals << std::endl;
    std::cout << std::setw(25) << std::left << "Axial Decomposition:" << I.decomp_assemblies_ax << std::endl;
    std::cout << std::setw(25) << std::left << "3D Source Regions:" << I.source_3D_regions << std::endl;
    std::cout << std::setw(25) << std::left << "Segments:"; fancy_int(I.segments);
    std::cout << std::setw(25) << std::left << "Random Number Streams:"; fancy_int(I.streams);
    std::cout << std::setw(25) << std::left << "Memory Estimate (MB):" << mem_estimate(I) << std::endl;
    std::cout << std::setw(25) << std::left << "Segments per Thread:" << I.seg_per_thread << std::endl;
#ifdef TABLE
    std::cout << std::setw(25) << std::left << "Exponential Table:" << "ON" << std::endl;
#else
    std::cout << std::setw(25) << std::left << "Exponential Table:" << "OFF" << std::endl;
#endif
    border_print();
}

// reads command line inputs and applies options
void read_CLI(int argc, char * argv[], Input * input)
{
    // Collect Raw Input
    for (int i = 1; i < argc; i++)
    {
        char * arg = argv[i];

        // nthreads (-t)
        if (strcmp(arg, "-t") == 0)
        {
            if (++i < argc)
                input->nthreads = atoi(argv[i]);
            else
                print_CLI_error();
        }

        // segments (-s)
        else if (strcmp(arg, "-s") == 0)
        {
            if (++i < argc)
                input->segments = atoi(argv[i]);
            else
                print_CLI_error();
        }

        // egroups (-e)
        else if (strcmp(arg, "-e") == 0)
        {
            if (++i < argc)
                input->egroups = atoi(argv[i]);
            else
                print_CLI_error();
        }
        // segments per thread (-p)
        else if (strcmp(arg, "-p") == 0)
        {
            if (++i < argc)
                input->seg_per_thread = atoi(argv[i]);
            else
                print_CLI_error();
        }
        // Kokkos Device Number (-d)
        else if (strcmp(arg, "-d") == 0)
        {
            if (++i < argc)
            {
                int device_id = atoi(argv[i]);
                Kokkos::DefaultExecutionSpace::set_device(device_id);
            }
            else
                print_CLI_error();
        }
        else
            print_CLI_error();
    }
}

// print error to screen, inform program options
void print_CLI_error(void)
{
    std::cout << "Usage: ./SimpleMOC <options>" << std::endl;
    std::cout << "Options include:" << std::endl;
    std::cout << "  -t <threads>          Number of Kokkos threads to run" << std::endl;
    std::cout << "  -s <segments>         Number of segments to process" << std::endl;
    std::cout << "  -e <energy groups>    Number of energy groups" << std::endl;
    std::cout << "  -p <segs per thread>  Number of segments per thread" << std::endl;
    std::cout << "  -d <Kokkos device ID> Kokkos device ID number" << std::endl;
    std::cout << "See readme for full description of default run values" << std::endl;
    exit(1);
}