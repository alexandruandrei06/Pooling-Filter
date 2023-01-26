# Pooling Filter

Pooling filters are often used in neural networks to reduce the size of feature matrices, thus reducing the complexity of calculations.

More about Pooling Filter : https://shorturl.at/adglZ

# Description

The project aims to parallelize the calculations for the application of a Pooling filter, starting from the sequential implementation, with the aim of observing the differences between the different parallel programming methods and their performances.

-   Pthread
-   MPI
-   OpenMP
-   CUDA

# Input file format

The input is a photo in format .bmp

# Output file format

The result is written as a picture in .bmp format

# MPI installation

```bash
sudo apt install openmpi-bin openmpi-common openmpi-doc libopenmpi-dev
```

# Usage

How to run specific implementation:

```bash
cd /Pooling_<Method>
make
```

-   Serial

```bash
./Pooling input_file output_file
```

-   Pthreads

```bash
./Pooling threads_no input_file output_file
```

-   MPI

```bash
mpirun --oversubscribe -np process_no ./Pooling input_file output_file
```

-   OpenMP

```bash
./Pooling input_file output_file
```

-   CUDA

For CUDA you will need CUDA Toolkit: https://developer.nvidia.com/cuda-toolkit

```bash
./Pooling input_file output_file
```
