#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sys/stat.h>
#include <unistd.h>

#ifdef _WIN32
#include <direct.h>
#define mkdir(path, mode) _mkdir(path)
#endif

// Helper function to create a directory if it doesn't exist
void create_dir_if_not_exists(const std::string& dir) {
  struct stat info;
  if (stat(dir.c_str(), &info) != 0) {
    if (mkdir(dir.c_str(), 0700) != 0) {
      std::cerr << "Error: Could not create directory: " << dir << std::endl;
      exit(EXIT_FAILURE);
    }
    std::cout << "Created directory: " << dir << std::endl;
  }
}

// Helper function to download a file using curl
void download_file(const std::string& file_name, const std::string& file_url) {
  std::string command = "curl -s -L -o " + file_name + " " + file_url;
  int result = system(command.c_str());
  if (result != 0) {
    std::cerr << "Error: Failed to download file: " << file_name << std::endl;
    exit(EXIT_FAILURE);
  }
  std::cout << "Downloaded " << file_name << " from " << file_url << std::endl;
}

int main() {
  // Get the directory of the script
  std::string script_dir = __FILE__;
  script_dir.erase(script_dir.find_last_of("/\\") + 1);

  // Base URL
  std::string base_url = "https://huggingface.co/datasets/karpathy/llmc-starter-pack/resolve/main/";

  // Directory paths based on script location
  std::string save_dir_parent = script_dir + "../";
  std::string save_dir_tiny = script_dir + "data/tinyshakespeare";
  std::string save_dir_hella = script_dir + "data/hellaswag";

  // Create the directories if they don't exist
  create_dir_if_not_exists(save_dir_tiny);
  create_dir_if_not_exists(save_dir_hella);

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

  // Download the files
  for (const auto& file : files) {
    std::string file_url = base_url + file + "?download=true";
    std::string file_path;

    // Determine the save directory based on the file name
    if (file.find("tiny_shakespeare") != std::string::npos) {
      file_path = save_dir_tiny + "/" + file;
    } else if (file.find("hellaswag") != std::string::npos) {
      file_path = save_dir_hella + "/" + file;
    } else {
      file_path = save_dir_parent + file;
    }

    download_file(file_path, file_url);
  }

  std::cout << "All files downloaded and saved in their respective directories" << std::endl;

  return 0;
}
```

This code uses the `system` function to execute `curl` commands to download the files. It also includes logic to determine the appropriate save path based on the filename. The directory creation and file download logic is encapsulated in helper functions for better code organization.  
