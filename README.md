# wib_hdf5_compress
A dunedaq package to investigate compression via HDF5.

We may want to compress TPC raw data that's sent to offline. One straightforward way to do this would be to take advantage of the compression facilities of the HDF5 file format. We can naturally express TPC raw data as a 2D array, from which we create a HDF5 dataset. The HDF5 library can be instructed to store the dataset in a compressed format, with transparent decompression when the data is accessed.

This repo contains two proof-of-concept applications:
* `write_wib_fragment_style` writes WIB frames in the same way we're currently (2022-04-28) doing in DAQ, ie by creating a 1D dataset of single-byte elements and copying the raw bytes of the frames into it
* `write_wib_hdf5` reads in a raw file of WIB1 frames and writes the data out to a 2D dataset in (channel, time) of 16-bit integers

## Compression with "fragment style"

Example usage, with a raw ProtoDUNE1 file:

```
write_wib_fragment_style -i /eos/experiment/neutplatform/protodune/rawdata/np04/protodune-sp/raw/2020/detector/test/None/02/00/00/01/felix-2020-06-02-100819.0.9.0.bin -o frames.hdf5 --max-n-frames 100000 --chunk-size 512
```

HDF5 relies on the dataset being stored in "chunks" to be able to compress: the size of the chunks has an effect on the compression ratio. For the file given in the example above, the table below shows the compression ratio for different values of `--chunk-size` (in bytes). The compression ratio is defined as the size of the raw WIB frames divided by the size of the output HDF5 file. The compression method is gzip (aka deflate) at level 3.

```
Chunk size  |  Compression ratio
--------------------------------
       1024 |              1.06
       2048 |              1.16
       4096 |              1.24
       8192 |              1.29
      16384 |              1.32
      32768 |              1.34
      65536 |              1.36
```

The compression ratio is not very high. The WIB frame format consists of 80 bytes of headers in each 464-byte frame, so just omitting those headers would give a compression ratio of 1.21.

## Compression with 2D dataset

We would expect to do better if we decode the WIB frames into a 2D array of ADC values, which is what `write_wib_hdf5` does. Example usage:

```
write_wib_hdf5 -i /eos/experiment/neutplatform/protodune/rawdata/np04/protodune-sp/raw/2020/detector/test/None/02/00/00/01/felix-2020-06-02-100819.0.9.0.bin -o frames.hdf5 --max-n-frames 100000 --frame-chunk 512 --channel-chunk 1
```

The chunks are now 2D; their size in time and channel  is controlled by the `--frame-chunk` and `--channel-chunk` arguments respectively.

For the file given in the example above, the table below shows the compression ratio for different values of `--frame-chunk` and `--channel-chunk`. The HDF5 shuffle filter and gzip (aka deflate) at level 3 have been applied.

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

Compression ratios are better, as expected. Making the chunk size larger than 1 in channel space doesn't help, presumably because adjacent-in-WIB-frame channels are not closely related/correlated.

Presumably one could do even better in terms of compression ratio with a custom compression/encoding method (eg Fibonacci encoding adjacent tick differences), at the cost of having to design and maintain the code for the custom method, rather than having HDF5 do it for us. 

This code is just a proof-of-concept: a real implementation would at least have to save the timestamps somewhere, and deal with cases where a frame is missing.
