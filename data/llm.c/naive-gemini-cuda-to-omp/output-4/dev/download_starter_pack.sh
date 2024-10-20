#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unistd.h>
#include <sys/stat.h>
#include <omp.h>

using namespace std;

// Function to check if a file exists
bool fileExists(const std::string& filename) {
  struct stat buffer;
  return (stat(filename.c_str(), &buffer) == 0);
}

// Function to create a directory if it doesn't exist
void createDir(const std::string& dir) {
  if (mkdir(dir.c_str(), 0700) == -1) {
    if (errno != EEXIST) {
      std::cerr << "Error creating directory: " << dir << std::endl;
      exit(1);
    }
  }
}

// Function to download a file using curl
void downloadFile(const std::string& url, const std::string& filename) {
  string command = "curl -s -L -o " + filename + " " + url;
  system(command.c_str());
  cout << "Downloaded " << filename << " from " << url << endl;
}

int main() {
  // Define the base URL and file paths
  const string baseUrl = "https://huggingface.co/datasets/karpathy/llmc-starter-pack/resolve/main/";
  const string saveDirParent = ".";
  const string saveDirTiny = "data/tinyshakespeare";
  const string saveDirHella = "data/hellaswag";

  // Create directories if they don't exist
  createDir(saveDirTiny);
  createDir(saveDirHella);

  // Files to download
  vector<string> files = {
    "gpt2_124M.bin",
    "gpt2_124M_bf16.bin",
    "gpt2_124M_debug_state.bin",
    "gpt2_tokenizer.bin",
    "tiny_shakespeare_train.bin",
    "tiny_shakespeare_val.bin",
    "hellaswag_val.bin",
  };

  // Download files in parallel using OpenMP
  #pragma omp parallel for
  for (int i = 0; i < files.size(); ++i) {
    string file = files[i];
    string url = baseUrl + file + "?download=true";
    string filename;
    if (file.find("tiny_shakespeare") != string::npos) {
      filename = saveDirTiny + "/" + file;
    } else if (file.find("hellaswag") != string::npos) {
      filename = saveDirHella + "/" + file;
    } else {
      filename = saveDirParent + "/" + file;
    }
    if (!fileExists(filename)) {
      downloadFile(url, filename);
    }
  }

  cout << "All files downloaded and saved in their respective directories." << endl;

  return 0;
}