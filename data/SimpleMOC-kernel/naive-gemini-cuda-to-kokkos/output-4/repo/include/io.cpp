#include "SimpleMOC-kernel_header.hpp"

// Prints program logo
void logo(int version) {
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
void center_print(const char *s, int width) {
    int length = strlen(s);
    int i;
    for (i = 0; i <= (width - length) / 2; i++) {
        std::cout << " ";
    }
    std::cout << s << "\n";
}

// Prints a border
void border_print(void) {
    std::cout <<
        "==================================================================="
        "=============\n";
}

// Prints comma separated integers - for ease of reading
void fancy_int(int a) {
    if (a < 1000)
        std::cout << a << "\n";
    else if (a >= 1000 && a < 1000000)
        std::cout << a / 1000 << "," << std::setw(3) << std::setfill('0') << a % 1000 << "\n";
    else if (a >= 1000000 && a < 1000000000)
        std::cout << a / 1000000 << "," << std::setw(3) << std::setfill('0') << (a % 1000000) / 1000 << "," << std::setw(3) << std::setfill('0') << a % 1000 << "\n";
    else if (a >= 1000000000)
        std::cout << a / 1000000000 << "," << std::setw(3) << std::setfill('0') << (a % 1000000000) / 1000000 << "," << std::setw(3) << std::setfill('0') << (a % 1000000) / 1000 << "," << std::setw(3) << std::setfill('0') << a % 1000 << "\n";
    else
        std::cout << a << "\n";
}

// Prints out the summary of User input
void print_input_summary(Input I) {
    center_print("INPUT SUMMARY", 79);
    border_print();

    //Kokkos specific - get device information differently
    std::cout << "%-25s" << Kokkos::DefaultExecutionSpace::name() << "\n"; //replace with appropriate Kokkos equivalent

    std::cout << "%-25s" << I.egroups << "\n";
    std::cout << "%-25s" << I.source_2D_regions << "\n";
    std::cout << "%-25s" << I.coarse_axial_intervals << "\n";
    std::cout << "%-25s" << I.fine_axial_intervals << "\n";
    std::cout << "%-25s" << I.decomp_assemblies_ax << "\n";
    std::cout << "%-25s" << I.source_3D_regions << "\n";
    std::cout << "%-25s"; fancy_int(I.segments);
    std::cout << "%-25s"; fancy_int(I.streams);
    std::cout << "%-25s" << mem_estimate(I) << "\n";
    std::cout << "%-25s" << I.seg_per_thread << "\n";
#ifdef TABLE
    std::cout << "%-25s" << "ON" << "\n";
#else
    std::cout << "%-25s" << "OFF" << "\n";
#endif
    border_print();
}

// reads command line inputs and applies options
void read_CLI(int argc, char *argv[], Input *input) {
    // Collect Raw Input
    for (int i = 1; i < argc; i++) {
        char *arg = argv[i];

        // nthreads (-t)
        if (strcmp(arg, "-t") == 0) {
            if (++i < argc)
                input->nthreads = atoi(argv[i]);
            else
                print_CLI_error();
        }

        // segments (-s)
        else if (strcmp(arg, "-s") == 0) {
            if (++i < argc)
                input->segments = atoi(argv[i]);
            else
                print_CLI_error();
        }

        // egroups (-e)
        else if (strcmp(arg, "-e") == 0) {
            if (++i < argc)
                input->egroups = atoi(argv[i]);
            else
                print_CLI_error();
        }
        // segments per thread (-p)
        else if (strcmp(arg, "-p") == 0) {
            if (++i < argc)
                input->seg_per_thread = atoi(argv[i]);
            else
                print_CLI_error();
        }
        // CUDA Device Number (-d) -  Not applicable to Kokkos, remove or handle differently
        else if (strcmp(arg, "-d") == 0) {
            std::cerr << "Warning: -d flag ignored. Kokkos handles device selection differently.\n";
            ++i; //Consume the argument, even though we ignore it.
        }
        else
            print_CLI_error();
    }
}

// print error to screen, inform program options
void print_CLI_error(void) {
    std::cerr << "Usage: ./SimpleMOC <options>\n";
    std::cerr << "Options include:\n";
    std::cerr << "  -t <threads>          Number of OpenMP threads to run\n";
    std::cerr << "  -s <segments>         Number of segments to process\n";
    std::cerr << "  -e <energy groups>    Number of energy groups\n";
    std::cerr << "  -p <segs per thread>  Number of segments per Kokkos team\n";
    std::cerr << "See readme for full description of default run values\n";
    exit(1);
}