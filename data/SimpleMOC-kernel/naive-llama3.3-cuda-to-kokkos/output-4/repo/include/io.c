#include "SimpleMOC-kernel_header.hpp"
#include <iostream>
#include <iomanip>
#include <string>
#include <cmath>
#include <random>
#include <Kokkos_Core.hpp>

// Prints program logo
void logo(int version)
{
    std::cout << "===============================================================================" << std::endl;
    std::cout << "   __           __        ___        __   __           ___  __        ___     " << std::endl;
    std::cout << "  /__` |  |\\/| |__) |    |__   |\\/| /  \\ /  ` __ |__/ |__  |__) |\\ | |__  |   " << std::endl;
    std::cout << "  .__/ |  |  | |    |___ |___  |  | \\__/ \\__,    |  \\ |___ |  \\ | \\| |___ |___" << std::endl;
    std::cout << "                         Version: " << version << std::endl;
    std::cout << "===============================================================================" << std::endl;
}

// Prints Section titles in center of 80 char terminal
void center_print(const std::string& s, int width)
{
    int length = s.length();
    int i;
    for (i = 0; i <= (width - length) / 2; i++) {
        std::cout << " ";
    }
    std::cout << s << std::endl;
}

// Prints a border
void border_print(void)
{
    std::cout << "===============================================================================" << std::endl;
}

// Prints comma separated integers - for ease of reading
void fancy_int(int a)
{
    if (a < 1000) {
        std::cout << a << std::endl;
    } else if (a >= 1000 && a < 1000000) {
        std::cout << a / 1000 << "," << std::setw(3) << std::setfill('0') << a % 1000 << std::endl;
    } else if (a >= 1000000 && a < 1000000000) {
        std::cout << a / 1000000 << "," << std::setw(3) << std::setfill('0') << (a % 1000000) / 1000 << "," << std::setw(3) << std::setfill('0') << a % 1000 << std::endl;
    } else if (a >= 1000000000) {
        std::cout << a / 1000000000 << "," << std::setw(3) << std::setfill('0') << (a % 1000000000) / 1000000 << "," << std::setw(3) << std::setfill('0') << (a % 1000000) / 1000 << "," << std::setw(3) << std::setfill('0') << a % 1000 << std::endl;
    } else {
        std::cout << a << std::endl;
    }
}

// Prints out the summary of User input
void print_input_summary(Input I)
{
    center_print("INPUT SUMMARY", 79);
    border_print();

    std::cout << std::setw(25) << std::left << "Number of Energy Groups:" << I.egroups << std::endl;
    std::cout << std::setw(25) << std::left << "Number of 2D Source Regions:" << I.source_2D_regions << std::endl;
    std::cout << std::setw(25) << std::left << "Number of Coarse Axial Intervals:" << I.coarse_axial_intervals << std::endl;
    std::cout << std::setw(25) << std::left << "Number of Fine Axial Intervals:" << I.fine_axial_intervals << std::endl;
    std::cout << std::setw(25) << std::left << "Number of 3D Source Regions:" << I.source_3D_regions << std::endl;
    std::cout << std::setw(25) << std::left << "Number of Segments:"; fancy_int(I.segments);
    std::cout << std::setw(25) << std::left << "Number of Random Number Streams:"; fancy_int(I.streams);
    std::cout << std::setw(25) << std::left << "Memory Estimate (MB):" << I.nbytes / 1024.0 / 1024.0 << std::endl;
    std::cout << std::setw(25) << std::left << "Segments per Thread:" << I.seg_per_thread << std::endl;
    border_print();
}

// reads command line inputs and applies options
void read_CLI(int argc, char* argv[], Input* input)
{
    // Collect Raw Input
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];

        // nthreads (-t)
        if (arg == "-t") {
            if (++i < argc) {
                input->nthreads = std::stoi(argv[i]);
            } else {
                print_CLI_error();
            }
        }

        // segments (-s)
        else if (arg == "-s") {
            if (++i < argc) {
                input->segments = std::stol(argv[i]);
            } else {
                print_CLI_error();
            }
        }

        // egroups (-e)
        else if (arg == "-e") {
            if (++i < argc) {
                input->egroups = std::stoi(argv[i]);
            } else {
                print_CLI_error();
            }
        }

        // segments per thread (-p)
        else if (arg == "-p") {
            if (++i < argc) {
                input->seg_per_thread = std::stoi(argv[i]);
            } else {
                print_CLI_error();
            }
        }

        else {
            print_CLI_error();
        }
    }
}

// print error to screen, inform program options
void print_CLI_error(void)
{
    std::cout << "Usage: ./SimpleMOC <options>" << std::endl;
    std::cout << "Options include:" << std::endl;
    std::cout << "  -t <threads>          Number of threads to run" << std::endl;
    std::cout << "  -s <segments>         Number of segments to process" << std::endl;
    std::cout << "  -e <energy groups>    Number of energy groups" << std::endl;
    std::cout << "  -p <segs per thread>  Number of segments per thread" << std::endl;
    std::cout << "See readme for full description of default run values" << std::endl;
    exit(1);
}