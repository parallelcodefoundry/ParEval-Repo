```c++
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <omp.h>

#define BASE_URL "https://huggingface.co/datasets/karpathy/llmc-starter-pack/resolve/main/"

#define SAVE_DIR_PARENT "."
#define SAVE_DIR_TINY "data/tinyshakespeare"
#define SAVE_DIR_HELLA "data/hellaswag"

#define HEADER_SIZE 256

// Function to download files to the appropriate directory
void download_file(const char *file_name) {
    char file_url[512];
    char file_path[512];
    sprintf(file_url, "%s%s?download=true", BASE_URL, file_name);

    if (strncmp(file_name, "tiny_shakespeare", 16) == 0) {
        sprintf(file_path, "%s/%s", SAVE_DIR_TINY, file_name);
    } else if (strncmp(file_name, "hellaswag", 9) == 0) {
        sprintf(file_path, "%s/%s", SAVE_DIR_HELLA, file_name);
    } else {
        sprintf(file_path, "%s/%s", SAVE_DIR_PARENT, file_name);
    }

    // Create the directories if they don't exist
    #pragma omp target teams distribute parallel for
    for (int i = 0; i < 2; i++) {
        if (i == 0) {
            mkdir(SAVE_DIR_TINY, 0700);
        } else {
            mkdir(SAVE_DIR_HELLA, 0700);
        }
    }

    // Download the file
    char command[1024];
    sprintf(command, "curl -s -L -o \"%s\" \"%s\"", file_path, file_url);
    system(command);
    printf("Downloaded %s to %s\n", file_name, file_path);
}

int main() {
    const char *files[] = {
        "gpt2_124M.bin",
        "gpt2_124M_bf16.bin",
        "gpt2_124M_debug_state.bin",
        "gpt2_tokenizer.bin",
        "tiny_shakespeare_train.bin",
        "tiny_shakespeare_val.bin",
        "hellaswag_val.bin"
    };
    int num_files = sizeof(files) / sizeof(files[0]);

    #pragma omp target teams distribute parallel for
    for (int i = 0; i < num_files; i++) {
        download_file(files[i]);
    }

    printf("All files downloaded and saved in their respective directories\n");
    return 0;
}
```

**Explanation:**

1. **Headers:** We include necessary headers for standard input/output, file operations, and OpenMP.
2. **Constants:** We define constants for the base URL, directory paths, and the header size.
3. **`download_file` function:**
   - Takes the file name as input.
   - Constructs the download URL.
   - Determines the save path based on the file name.
   - Creates the necessary directories using OpenMP offload.
   - Constructs a `curl` command string.
   - Executes the command using `system()`.
   - Prints a success message.
4. **`main` function:**
   - Defines an array of file names to download.
   - Uses OpenMP offload to download each file in parallel.
   - Prints a final success message.

**Key Changes:**

- **OpenMP Offload:**  The `#pragma omp target teams distribute parallel for` directive is used to offload the file downloads to the target device. This allows for parallel execution.
- **`mkdir`:** The `mkdir` function is called within the OpenMP loop to create the necessary directories on the target device.
- **`system`:** The `system` function is used to execute the `curl` command directly on the target device. This avoids data transfer back to the host for the download operation.

This code is a basic translation of the script to use OpenMP offload. You can further refine it based on the capabilities of your target device and optimize for performance. For example, you could use the OpenMP `teams` construct to distribute the downloads across multiple teams on the target device.
