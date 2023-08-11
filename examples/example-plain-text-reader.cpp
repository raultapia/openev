/*!
\example example-plain-text-reader.cpp
This is an example of how to use the Davis class.
*/
#include <iostream>
#include <openev/openev.hpp>

void create_file(const std::string &filename) {
  std::ofstream outputFile(filename);

  if(!outputFile.is_open()) {
    std::cerr << "Failed to open " << filename << std::endl;
    return;
  } else {
    for(int i = 1; i <= 9; i++) {
      outputFile << i << " " << i << " " << i << " " << i % 2 << std::endl;
    }
    outputFile.close();
    std::cout << "File " << filename << " created successfully." << std::endl;
  }
}

bool delete_file(const std::string &filename) {
  if(std::remove(filename.c_str()) != 0) {
    std::cerr << "Failed to delete " << filename << std::endl;
    return false;
  } else {
    std::cout << "File " << filename << " deleted successfully." << std::endl;
    return true;
  }
}

int main(int argc, const char *argv[]) {
  create_file("test.txt");

  ev::PlainTextReader reader("test.txt");
  ev::Event e;
  while(reader.next(e)) {
    std::cout << e << std::endl;
  }

  delete_file("test.txt");
  return 0;
}
