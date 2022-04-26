#include "CLI/CLI.hpp"

#include <string>

int main(int argc, char** argv)
{
  CLI::App app{ "Write raw WIB data as HDF5 file" };

  std::string in_filename;
  app.add_option("-i,--input", in_filename, "Input raw WIB file");

  std::string out_filename;
  app.add_option("-o,--output", out_filename, "Output HDF5 file");

  CLI11_PARSE(app, argc, argv);
  
  return 0;
}
