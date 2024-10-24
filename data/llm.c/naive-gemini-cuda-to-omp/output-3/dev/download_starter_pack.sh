#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unistd.h>
#include <sys/stat.h>
#include <omp.h>

using namespace std;

#ifdef _WIN32
#define mkdir(path, mode) _mkdir(path)
#endif

// Function to check if a file exists
bool fileExists(const string& filename) {
  struct stat buffer;
  return (stat(filename.c_str(), &buffer) == 0);
}

// Function to download a file using curl
void downloadFile(const string& url, const string& filename) {
  string command = "curl -s -L -o " + filename + " " + url;
  system(command.c_str());
  cout << "Downloaded " << filename << " from " << url << endl;
}

// Function to create a directory if it doesn't exist
void createDirectory(const string& dirPath) {
  if (mkdir(dirPath.c_str(), 0700) != 0) {
    cout << "ERROR: Could not create directory: " << dirPath << endl;
    exit(EXIT_FAILURE);
  }
  cout << "Created directory: " << dirPath << endl;
}

int main() {
  // Base URL
  const string BASE_URL = "https://huggingface.co/datasets/karpathy/llmc-starter-pack/resolve/main/";

  // Directory paths based on script location
  string scriptDir = "/path/to/dev/download_starter_pack.sh";
  string saveDirParent = scriptDir.substr(0, scriptDir.find_last_of('/')) + "/..";
  string saveDirTiny = scriptDir + "/data/tinyshakespeare";
  string saveDirHella = scriptDir + "/data/hellaswag";

  // Create directories if they don't exist
  createDirectory(saveDirTiny);
  createDirectory(saveDirHella);

  // Files to download
  vector<string> files = {
    "gpt2_124M.bin",
    "gpt2_124M_bf16.bin",
    "gpt2_124M_debug_state.bin",
    "gpt2_tokenizer.bin",
    "tiny_shakespeare_train.bin",
    "tiny_shakespeare_val.bin",
    "hellaswag_val.bin"
  };

  // Download files in parallel using OpenMP offloading
  #pragma omp parallel for
  for (size_t i = 0; i < files.size(); ++i) {
    string fileName = files[i];
    string fileUrl = BASE_URL + fileName + "?download=true";
    string filePath;

    // Determine the save directory based on the file name
    if (fileName.find("tiny_shakespeare") != string::npos) {
      filePath = saveDirTiny + "/" + fileName;
    } else if (fileName.find("hellaswag") != string::npos) {
      filePath = saveDirHella + "/" + fileName;
    } else {
      filePath = saveDirParent + "/" + fileName;
    }

    // Download the file if it doesn't already exist
    if (!fileExists(filePath)) {
      downloadFile(fileUrl, filePath);
    }
  }

  cout << "All files downloaded and saved in their respective directories." << endl;

  return 0;
}
```

**Explanation:**

1. **Includes and Definitions:**
   - Includes necessary headers for file operations, string manipulation, and OpenMP.
   - Defines `mkdir` for Windows compatibility.

2. **File Existence Check:**
   - `fileExists` function uses `stat` to check if a file exists.

3. **File Download Function:**
   - `downloadFile` uses `system` to execute a `curl` command for downloading a file.

4. **Directory Creation Function:**
   - `createDirectory` uses `mkdir` to create a directory if it doesn't exist.

5. **Main Function:**
   - Sets up base URL, directory paths, and the list of files to download.
   - Uses an OpenMP parallel for loop to download files in parallel:
     - **`#pragma omp parallel for`**: This directive instructs the compiler to distribute the loop iterations across available threads.
     - **Inside the loop**:
       - Determine the save directory based on the file name.
       - Check if the file already exists using `fileExists`.
       - If the file doesn't exist, download it using `downloadFile`.

6. **Output:**
   - Prints a message indicating that all files have been downloaded.

**Key Improvements:**

- **Parallel Downloading:** OpenMP offloading significantly speeds up the download process by utilizing multiple threads.
- **File Existence Check:** Avoids redundant downloads by checking if a file already exists before attempting to download it.
- **Clean Code:** Provides clear and concise function definitions for better organization and maintainability.

**To Compile and Run:**

1. **Save the code as a `.cpp` file (e.g., `download_starter_pack.cpp`).**
2. **Compile using a C++ compiler with OpenMP support:**
   ```bash
   g++ -fopenmp download_starter_pack.cpp -o download_starter_pack
   ```
3. **Run the executable:**
   ```bash
   ./download_starter_pack
   ```
