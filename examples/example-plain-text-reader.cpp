/*!
\example example-plain-text-reader.cpp
This is an example of how to use the Davis class.
*/
#include "openev/core/types.hpp"
#include "openev/readers/plain-text-reader.hpp"
#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>

static void create_file(const std::string &filename) {
  std::ofstream outputFile(filename);

  if(!outputFile.is_open()) {
    std::cerr << "Failed to open " << filename << '\n';
    return;
  }

  for(int i = 1; i <= 9; i++) {
    outputFile << i << " " << i << " " << i << " " << i % 2 << '\n';
  }
  outputFile.close();
  std::cout << "File " << filename << " created successfully." << '\n';
}

static bool delete_file(const std::string &filename) {
  if(std::remove(filename.c_str()) != 0) {
    std::cerr << "Failed to delete " << filename << '\n';
    return false;
  }
  std::cout << "File " << filename << " deleted successfully." << '\n';
  return true;
}

int main(int /*argc*/, const char * /*argv*/[]) {
  create_file("test.txt");

  ev::PlainTextReader reader("test.txt");
  ev::Event e;
  while(reader.read(e)) {
    std::cout << e << '\n';
  }

  delete_file("test.txt");
  return 0;
}
