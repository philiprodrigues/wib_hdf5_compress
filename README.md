# wib_hdf5_compress
A dunedaq package to investigate compression via HDF5.

We may want to compress TPC raw data that's sent to offline. One straightforward way to do this would be to take advantage of the compression facilities of the HDF5 file format. We can naturally express TPC raw data as a 2D array, from which we create a HDF5 dataset. The HDF5 library can be instructed to store the dataset in a compressed format, with transparent decompression when the data is accessed.

This repo contains one proof-of-concept application, `write_wib_hdf5`, which reads in a raw file of WIB1 frames and writes the data out to a dataset in a compressed HDF5 file. Example usage, with a raw ProtoDUNE1 file:

```
write_wib_hdf5 -i /eos/experiment/neutplatform/protodune/rawdata/np04/protodune-sp/raw/2020/detector/test/None/02/00/00/01/felix-2020-06-02-100819.0.9.0.bin -o frames.hdf5 --max-n-frames 100000 --frame-chunk 512 --channel-chunk 1
```

HDF5 relies on the dataset being stored in "chunks" to be able to compress: the dimensions of the chunks have a strong effect on the compression ratio. The size of the chunk is controlled by the `--frame-chunk` and `--channel-chunk` arguments.

For the file given in the example above, the table below shows the compression ratio for different values of `--frame-chunk` and `--channel-chunk`. The compression ratio is defined as the size of the raw WIB frames divided by the size of the output HDF5 file. The HDF5 shuffle filter and gzip (aka deflate) at level 3 have been applied.

```
               |        --channel-chunk
 --frame-chunk |   1       2       4       8
--------------------------------------------
  512          | 2.5     2.4     2.2     2.1
 1024          | 2.8     2.6     2.3     2.2
 2048          | 3.0     2.7     2.4     2.2
 4096          | 3.1     2.8     2.4     2.3
 8192          | 3.1     2.8     2.5     2.3
16384          | 3.2     2.8     2.5     2.4
```

This code is just a proof-of-concept: a real implementation would at least have to save the timestamps somewhere, and deal with cases where a frame is missing.
