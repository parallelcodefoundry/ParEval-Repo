#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
#include <omp.h>

using namespace std;
namespace fs = std::filesystem;

// Function to download files using curl
void download_file(const string& file_name, const string& file_url, const string& file_path) {
  string command = "curl -s -L -o \"" + file_path + "\" \"" + file_url + "\"";
  system(command.c_str());
  cout << "Downloaded " << file_name << " to " << file_path << endl;
}

int main() {
  // Base URL
  const string BASE_URL = "https://huggingface.co/datasets/karpathy/llmc-starter-pack/resolve/main/";

  // Directory paths
  const string SAVE_DIR_PARENT = "."; // Assuming the script is in the root directory
  const string SAVE_DIR_TINY = "data/tinyshakespeare";
  const string SAVE_DIR_HELLA = "data/hellaswag";

  // Create directories if they don't exist
  fs::create_directories(SAVE_DIR_TINY);
  fs::create_directories(SAVE_DIR_HELLA);

  // Files to download
  vector<string> FILES = {
    "gpt2_124M.bin",
    "gpt2_124M_bf16.bin",
    "gpt2_124M_debug_state.bin",
    "gpt2_tokenizer.bin",
    "tiny_shakespeare_train.bin",
    "tiny_shakespeare_val.bin",
    "hellaswag_val.bin"
  };

  #pragma omp parallel for
  for (size_t i = 0; i < FILES.size(); ++i) {
    string file_name = FILES[i];
    string file_url = BASE_URL + file_name + "?download=true";
    string file_path;

    // Determine the save directory based on the file name
    if (file_name.find("tiny_shakespeare") != string::npos) {
      file_path = SAVE_DIR_TINY + "/" + file_name;
    } else if (file_name.find("hellaswag") != string::npos) {
      file_path = SAVE_DIR_HELLA + "/" + file_name;
    } else {
      file_path = SAVE_DIR_PARENT + "/" + file_name;
    }

    // Download the file (offloaded to a device with OpenMP)
    #pragma omp target device(0)
    {
      download_file(file_name, file_url, file_path);
    }
  }

  cout << "All files downloaded and saved in their respective directories" << endl;

  return 0;
}