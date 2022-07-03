# Matrix Multiplication

## 1. matmul1.c
Naïve matrix multiplication algorithm; creates child processes to compute each element of the final matrix

## 2. matmul2.cpp
Implements a producer / worker system using shared memory, to multiply a sequence of matrices block-wise; uses mutex locks to prevent concurrent updation to shared variables