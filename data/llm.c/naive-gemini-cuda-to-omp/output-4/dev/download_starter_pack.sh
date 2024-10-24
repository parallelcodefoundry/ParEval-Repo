#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <omp.h>

#define BASE_URL "https://huggingface.co/datasets/karpathy/llmc-starter-pack/resolve/main/"

int main() {
  // Get the directory of the script
  char script_dir[1024];
  ssize_t len = readlink("/proc/self/exe", script_dir, sizeof(script_dir) - 1);
  if (len == -1) {
    perror("readlink");
    return 1;
  }
  script_dir[len] = '\0';
  char *last_slash = strrchr(script_dir, '/');
  *last_slash = '\0';

  // Directory paths based on script location
  char save_dir_parent[1024];
  sprintf(save_dir_parent, "%s/..", script_dir);
  char save_dir_tiny[1024];
  sprintf(save_dir_tiny, "%s/data/tinyshakespeare", script_dir);
  char save_dir_hella[1024];
  sprintf(save_dir_hella, "%s/data/hellaswag", script_dir);

  // Create the directories if they don't exist
  mkdir(save_dir_tiny, 0700);
  mkdir(save_dir_hella, 0700);

  // Files to download
  char *files[] = {
      "gpt2_124M.bin",
      "gpt2_124M_bf16.bin",
      "gpt2_124M_debug_state.bin",
      "gpt2_tokenizer.bin",
      "tiny_shakespeare_train.bin",
      "tiny_shakespeare_val.bin",
      "hellaswag_val.bin",
      NULL
  };

  // Function to download files to the appropriate directory
  void download_file(char *file_name) {
    char file_url[1024];
    sprintf(file_url, "%s%s?download=true", BASE_URL, file_name);
    char file_path[1024];
    if (strstr(file_name, "tiny_shakespeare")) {
      sprintf(file_path, "%s/%s", save_dir_tiny, file_name);
    } else if (strstr(file_name, "hellaswag")) {
      sprintf(file_path, "%s/%s", save_dir_hella, file_name);
    } else {
      sprintf(file_path, "%s/%s", save_dir_parent, file_name);
    }

    // Download the file using system command
    char command[1024];
    sprintf(command, "curl -s -L -o \"%s\" \"%s\"", file_path, file_url);
    system(command);
    printf("Downloaded %s to %s\n", file_name, file_path);
  }

  #pragma omp parallel for
  for (int i = 0; files[i] != NULL; i++) {
    download_file(files[i]);
  }

  printf("All files downloaded and saved in their respective directories\n");

  return 0;
}
```

**Explanation of changes:**

1. **Headers:** Added `omp.h` for OpenMP.
2. **`readlink` for script directory:** Used `readlink` to get the script's directory since `dirname` is not portable.
3. **`sprintf` for string formatting:** Replaced string concatenation with `sprintf` for better readability and consistency.
4. **OpenMP parallelization:** Used `#pragma omp parallel for` to parallelize the download loop.
5. **System command for download:** Replaced `curl` with `system` to execute the command directly. This is done to avoid complex library integration for a simple download task.
6. **Error handling:** Added basic error handling using `perror` for `readlink`.
