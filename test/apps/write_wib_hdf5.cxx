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
                     size_t frame_chunk,
                     size_t channel_chunk,
                     const std::vector<std::vector<int16_t>>& array)
{
  const size_t n_frames = array.size();
  HighFive::File file(filename, HighFive::File::ReadWrite | HighFive::File::Create | HighFive::File::Truncate);
  // Create the data space for the dataset.
  std::vector<size_t> dims{ n_frames, n_channels };

  HighFive::DataSpace dataspace(dims);

  // Use chunking
  HighFive::DataSetCreateProps props;
  props.add(HighFive::Chunking(std::vector<hsize_t>{ frame_chunk, channel_chunk }));

  // Enable shuffle
  props.add(HighFive::Shuffle());

  // Enable deflate
  props.add(HighFive::Deflate(3));

  // Create a dataset with arbitrary type
  HighFive::DataSet dataset = file.createDataSet<int16_t>("wib_data", dataspace, props);

  dataset.write(array);

  file.flush();
}

std::vector<std::vector<int16_t>>
fill_array_from_raw_file(std::string filename,
                         size_t max_n_frames,
                         bool diff)
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
  std::vector<std::vector<int16_t>> array;
  for (size_t i = 0; i < n_frames; ++i) {
    array.push_back(std::vector<int16_t>(n_channels));
  }

  char buffer[frame_size];
  std::vector<int16_t> prev_row(n_channels);
  
  for (size_t i = 0; i < n_frames; ++i) {
    fin.seekg(i * frame_size);
    // Check we didn't go past the end
    if (fin.bad() || fin.eof())
      throw std::runtime_error("Error reading file");
    // Actually read the fragment into the buffer
    fin.read(buffer, frame_size);
    WIBFrame* frame = reinterpret_cast<WIBFrame*>(buffer);
    for (size_t j = 0; j < n_channels; ++j) {
      if (diff && i>0) {
        array[i][j] = frame->get_channel(j) - prev_row[j];
      } else {
        array[i][j] = frame->get_channel(j);
      }
      prev_row[j] = frame->get_channel(j);
    }
  }
  return array;
}

int
main(int argc, char** argv)
{
  CLI::App app{ "Write raw WIB data as HDF5 file" };

  std::string in_filename;
  app.add_option("-i,--input", in_filename, "Input raw WIB file");

  std::string out_filename;
  app.add_option("-o,--output", out_filename, "Output HDF5 file");

  size_t max_n_frames;
  app.add_option("-n,--max-n-frames", max_n_frames, "Maximum number of frames to write");

  size_t frame_chunk = 512;
  app.add_option("-f,--frame-chunk", frame_chunk, "Chunk size in frame dimension");

  size_t channel_chunk = 16;
  app.add_option("-c,--channel-chunk", channel_chunk, "Chunk size in channel dimension");

  bool diff = false;
  app.add_flag("-d,--diff", diff, "Do frame-to-frame difference");
  
  CLI11_PARSE(app, argc, argv);

  auto array = fill_array_from_raw_file(in_filename, max_n_frames, diff);
  write_hdf5_file(out_filename,
                  frame_chunk,
                  channel_chunk,
                  array);
  
  size_t size = array.size() * frame_size;
  size_t compressed_size = get_file_size(out_filename);
  size_t compressed_size_MB = compressed_size / 1024 / 1024;
  std::cout << "Compressed size " << compressed_size_MB << "MB" << std::endl;
  std::cout << "Compression ratio: " << (static_cast<double>(size)/compressed_size) << std::endl;
  return 0;
}
