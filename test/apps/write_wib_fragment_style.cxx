#include "CLI/CLI.hpp"

#include <fstream>
#include <iostream>
#include <string>

#include "detdataformats/wib/WIBFrame.hpp"

#include "highfive/H5DataSet.hpp"
#include "highfive/H5DataSpace.hpp"
#include "highfive/H5File.hpp"
#include "highfive/H5Group.hpp"
#include "highfive/H5Reference.hpp"
#include "highfive/H5Utility.hpp"

using dunedaq::detdataformats::wib::WIBFrame;

const size_t n_channels = 256;
const size_t frame_size = sizeof(WIBFrame);


size_t get_file_size(std::string filename)
{
  std::ifstream fin(filename, std::ifstream::binary);
  fin.seekg(0, fin.end);
  size_t length = fin.tellg();
  fin.seekg(0, fin.beg);
  return length;
}

void write_hdf5_file(std::string filename,
                     size_t chunk_size,
                     const std::vector<char>& array)
{
  const size_t n_bytes = array.size();
  HighFive::File file(filename, HighFive::File::ReadWrite | HighFive::File::Create | HighFive::File::Truncate);
  // Create the data space for the dataset.
  std::vector<size_t> dims;
  dims.push_back(n_bytes);

  HighFive::DataSpace dataspace(dims);

  // Use chunking
  HighFive::DataSetCreateProps props;
  props.add(HighFive::Chunking(std::vector<hsize_t>{ chunk_size }));

  // Enable deflate
  props.add(HighFive::Deflate(3));

  // Create a dataset with arbitrary type
  HighFive::DataSet dataset = file.createDataSet<char>("wib_data", dataspace, props);

  dataset.write(array);

  file.flush();
}

std::vector<char> fill_array_from_raw_file(std::string filename,
                                           size_t max_n_frames)
{
  std::ifstream fin(filename, std::ifstream::binary);
  size_t length = get_file_size(filename);
  if (length == 0) {
    throw std::runtime_error("Empty file");
  }
  if (length % frame_size != 0) {
    throw std::runtime_error("File does not contain an integer number of frames");
  }
  size_t n_file_frames = length / frame_size;
  size_t n_frames = std::min(n_file_frames, max_n_frames);
  
  std::cout << "There are " << n_file_frames << " frames in the file. Running on " << n_frames << std::endl;
  size_t size = n_frames * frame_size;
  size_t size_MB =  size / 1024 / 1024;
  std::cout << "Uncompressed size is " << size_MB << "MB" << std::endl;
  std::vector<char> array(size);

  fin.seekg(0);
  fin.read(array.data(), size);

  return array;
}

int
main(int argc, char** argv)
{
  CLI::App app{ "Write raw WIB data as HDF5 file in fragment style" };

  std::string in_filename;
  app.add_option("-i,--input", in_filename, "Input raw WIB file");

  std::string out_filename;
  app.add_option("-o,--output", out_filename, "Output HDF5 file");

  size_t max_n_frames;
  app.add_option("-n,--max-n-frames", max_n_frames, "Maximum number of frames to write");

  size_t chunk_size = 512;
  app.add_option("-c,--chunk-size", chunk_size, "Chunk size");

  CLI11_PARSE(app, argc, argv);

  auto array = fill_array_from_raw_file(in_filename, max_n_frames);
  write_hdf5_file(out_filename,
                  chunk_size,
                  array);
  
  size_t size = array.size();
  size_t compressed_size = get_file_size(out_filename);
  size_t compressed_size_MB = compressed_size / 1024 / 1024;
  std::cout << "Compressed size " << compressed_size_MB << "MB" << std::endl;
  std::cout << "Chunk size: " << chunk_size << ". Compression ratio: " << (static_cast<double>(size)/compressed_size) << std::endl;
  return 0;
}
