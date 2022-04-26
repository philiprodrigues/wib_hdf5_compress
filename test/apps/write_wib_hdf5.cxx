#include "CLI/CLI.hpp"

#include <string>
#include <fstream>

#include "detdataformats/wib/WIBFrame.hpp"

#include "highfive/H5DataSet.hpp"
#include "highfive/H5DataSpace.hpp"
#include "highfive/H5File.hpp"
#include "highfive/H5Group.hpp"
#include "highfive/H5Reference.hpp"
#include "highfive/H5Utility.hpp"

int main(int argc, char** argv)
{
  CLI::App app{ "Write raw WIB data as HDF5 file" };

  std::string in_filename;
  app.add_option("-i,--input", in_filename, "Input raw WIB file");

  std::string out_filename;
  app.add_option("-o,--output", out_filename, "Output HDF5 file");

  CLI11_PARSE(app, argc, argv);

  HighFive::File file(out_filename, HighFive::File::ReadWrite | HighFive::File::Create | HighFive::File::Truncate);

  const size_t n_channels = 256;
  const size_t n_frames = 3;
  // Create the data space for the dataset.
  std::vector<size_t> dims{ n_channels, n_frames };

  HighFive::DataSpace dataspace(dims);

  // Use chunking
  HighFive::DataSetCreateProps props;
  // props.add(HighFive::Chunking(std::vector<hsize_t>{ x_chunk, y_chunk }));

  // Enable shuffle
  props.add(HighFive::Shuffle());

  // Enable deflate
  props.add(HighFive::Deflate(3));

  // Create a dataset with arbitrary type
  HighFive::DataSet dataset = file.createDataSet<uint16_t>("wib_data", dataspace, props);

  uint16_t array[n_channels][n_frames]={0};

  dataset.write(array);

  file.flush();

  return 0;
}
