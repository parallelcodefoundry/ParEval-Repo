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

void download_file(const char *file_name) {
  char file_url[512];
  sprintf(file_url, "%s%s?download=true", BASE_URL, file_name);
  char file_path[512];
  if (strstr(file_name, "tiny_shakespeare") != NULL) {
    sprintf(file_path, "%s/%s", SAVE_DIR_TINY, file_name);
  } else if (strstr(file_name, "hellaswag") != NULL) {
    sprintf(file_path, "%s/%s", SAVE_DIR_HELLA, file_name);
  } else {
    sprintf(file_path, "%s/%s", SAVE_DIR_PARENT, file_name);
  }

  char command[1024];
  sprintf(command, "curl -s -L -o \"%s\" \"%s\"", file_path, file_url);
  system(command);
  printf("Downloaded %s to %s\n", file_name, file_path);
}

int main(int argc, char *argv[]) {
  const char *files[] = {
    "gpt2_124M.bin",
    "gpt2_124M_bf16.bin",
    "gpt2_124M_debug_state.bin",
    "gpt2_tokenizer.bin",
    "tiny_shakespeare_train.bin",
    "tiny_shakespeare_val.bin",
    "hellaswag_val.bin",
  };

  // Create directories if they don't exist
  mkdir(SAVE_DIR_TINY, 0700);
  mkdir(SAVE_DIR_HELLA, 0700);

  // Download files in parallel
  #pragma omp parallel for
  for (int i = 0; i < sizeof(files) / sizeof(files[0]); i++) {
    download_file(files[i]);
  }

  printf("All files downloaded and saved in their respective directories\n");

  return 0;
}
```

**Explanation:**

1. **Headers:** The code includes necessary headers for file operations (`stdio.h`, `stdlib.h`, `string.h`, `unistd.h`, `sys/stat.h`) and OpenMP (`omp.h`).
2. **Constants:** Constants like `BASE_URL`, `SAVE_DIR_PARENT`, `SAVE_DIR_TINY`, and `SAVE_DIR_HELLA` are defined to store the base URL for downloads and directory paths.
3. **`download_file` Function:** The `download_file` function takes a file name as input, constructs the download URL, determines the correct save path, and executes a `curl` command to download the file.
4. **`main` Function:**
   - The `main` function defines an array of file names to download.
   - It creates the necessary directories using `mkdir`.
   - The `#pragma omp parallel for` directive parallelizes the download process using OpenMP. Each thread downloads a different file.
   - Finally, it prints a success message.

**To compile and run:**

1. **Compile:**
   ```bash
   g++ -fopenmp download_starter_pack.cpp -o download_starter_pack
   ```
2. **Run:**
   ```bash
   ./download_starter_pack
   ```

This will download the necessary files to the specified directories in parallel.