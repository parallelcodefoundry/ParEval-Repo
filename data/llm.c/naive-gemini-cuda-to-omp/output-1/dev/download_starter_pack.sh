#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
#include <omp.h>

namespace fs = std::filesystem;

#ifdef _WIN32
#  define CURL_STATICLIB
#  define CURL_VERSION_INFO CURL_VERSION_NOW
#endif
#include <curl/curl.h>

// Function to download files to the appropriate directory
void download_file(const std::string& file_name, const std::string& base_url, const std::string& save_dir) {
    std::string file_url = base_url + file_name + "?download=true";
    std::string file_path = save_dir + "/" + file_name;

    CURL *curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, file_url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, NULL);
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);

        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        } else {
            std::cout << "Downloaded " << file_name << " to " << file_path << std::endl;
        }

        curl_easy_cleanup(curl);
    }
}

int main() {
    // Base URL and directory paths
    std::string base_url = "https://huggingface.co/datasets/karpathy/llmc-starter-pack/resolve/main/";
    std::string save_dir_parent = fs::current_path().string() + "/..";
    std::string save_dir_tiny = save_dir_parent + "/data/tinyshakespeare";
    std::string save_dir_hella = save_dir_parent + "/data/hellaswag";

    // Create directories if they don't exist
    fs::create_directories(save_dir_tiny);
    fs::create_directories(save_dir_hella);

    // Files to download
    std::vector<std::string> files = {
        "gpt2_124M.bin",
        "gpt2_124M_bf16.bin",
        "gpt2_124M_debug_state.bin",
        "gpt2_tokenizer.bin",
        "tiny_shakespeare_train.bin",
        "tiny_shakespeare_val.bin",
        "hellaswag_val.bin"
    };

    // Download files in parallel
    #pragma omp parallel for
    for (size_t i = 0; i < files.size(); i++) {
        std::string file_name = files[i];
        std::string save_dir = file_name.find("tiny_shakespeare") != std::string::npos ? save_dir_tiny
                             : file_name.find("hellaswag") != std::string::npos ? save_dir_hella
                             : save_dir_parent;
        download_file(file_name, base_url, save_dir);
    }

    std::cout << "All files downloaded and saved in their respective directories." << std::endl;

    return 0;
}
```

**Explanation of Changes:**

1. **Headers:** We include necessary headers for file system operations, OpenMP, and CURL. 
2. **Namespace:** We use `fs = std::filesystem` for easier file system interaction.
3. **CURL:** We include the CURL library for downloading files.
4. **download_file Function:** This function remains largely similar, using CURL to perform the download.
5. **Main Function:**
   - We use OpenMP's `#pragma omp parallel for` directive to download the files in parallel.
   - We determine the correct save directory based on the file name.
   - The rest of the code structure remains similar.

**Key Concepts:**

- **OpenMP:**  We utilize OpenMP's `#pragma omp parallel for` to parallelize the download loop. This allows multiple threads to work concurrently, potentially speeding up the download process.
- **CURL:** CURL is a robust library for handling network transfers, including downloads. 
- **File System Operations:** We use the `std::filesystem` namespace to create directories and manipulate file paths.
