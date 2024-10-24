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

// Function to check if a file exists
int file_exists(const char* filename) {
  struct stat buffer;
  return (stat(filename, &buffer) == 0);
}

// Function to download a file using curl
int download_file(const char* file_name, const char* file_path) {
  char file_url[512];
  sprintf(file_url, "%s%s?download=true", BASE_URL, file_name);
  char command[1024];
  sprintf(command, "curl -s -L -o %s %s", file_path, file_url);
  return system(command);
}

int main() {
  // Create the directories if they don't exist
  mkdir(SAVE_DIR_TINY, 0777);
  mkdir(SAVE_DIR_HELLA, 0777);

  // Files to download
  const char* files[] = {
    "gpt2_124M.bin",
    "gpt2_124M_bf16.bin",
    "gpt2_124M_debug_state.bin",
    "gpt2_tokenizer.bin",
    "tiny_shakespeare_train.bin",
    "tiny_shakespeare_val.bin",
    "hellaswag_val.bin"
  };

  // Download the files in parallel
  #pragma omp parallel for
  for (int i = 0; i < sizeof(files) / sizeof(files[0]); i++) {
    char file_path[256];
    if (strstr(files[i], "tiny_shakespeare") != NULL) {
      sprintf(file_path, "%s/%s", SAVE_DIR_TINY, files[i]);
    } else if (strstr(files[i], "hellaswag") != NULL) {
      sprintf(file_path, "%s/%s", SAVE_DIR_HELLA, files[i]);
    } else {
      sprintf(file_path, "%s/%s", SAVE_DIR_PARENT, files[i]);
    }
    if (!file_exists(file_path)) {
      if (download_file(files[i], file_path) == 0) {
        printf("Downloaded %s to %s\n", files[i], file_path);
      } else {
        printf("Error downloading %s\n", files[i]);
      }
    }
  }

  printf("All files downloaded and saved in their respective directories\n");

  return 0;
}
```

**Explanation:**

1. **Headers:** Include necessary headers for file operations, system calls, and OpenMP.
2. **Constants:** Define constants for the base URL and directory paths.
3. **`file_exists` function:** Uses `stat` to check if a file exists.
4. **`download_file` function:** Uses `sprintf` to construct the download command for `curl`, then calls `system` to execute it.
5. **Main function:**
    - Creates the directories using `mkdir`.
    - Defines an array of file names to download.
    - Uses `#pragma omp parallel for` to download files in parallel. Inside the loop:
        - Determines the file path based on the file name.
        - Checks if the file exists.
        - Downloads the file if it doesn't exist using `download_file`.
    - Prints a success message after all downloads are complete.
