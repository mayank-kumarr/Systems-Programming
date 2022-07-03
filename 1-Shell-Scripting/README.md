# Familiarization with shell scripting and shell commands

Instructions for executing the scripts

## a.  Computation - Prime Factorisation

```shell
$ chmod +x ./Assgn1_1a.sh
$ ./Assgn1_1a.sh <num>

Sample input:
$ ./Assgn1_1a.sh 280
```

A number is given as command line argument and its prime factors (including repetitions) are printed in ascending order.

## b. File Manipulation

```shell
$ chmod +x ./Assgn1_1b.sh
$ ./Assgn1_1b.sh
```

The folder "1.b.files.out" is created in root directory and the txt files in "1.b.files" is put there after each of them is sorted. A file "1.b.out.txt" is created in root directory in which the individual sorted files are merged. This file has the frequency of each number, and is sorted in increasing order.

## c. File/Directory Handling

```shell
$ chmod +x ./Assgn1_1c.sh
$ ./Assgn1_1c.sh
```

In the directory "data1c", new folders are created for each extension present in the folder hierarchy, with the name as that of the extension, and a Nil folder is created for files having no extension. The files in the folder hierarchy are now segregated based on their extensions, and all other folders are removed from the directory.

## d. File Handling

```shell
$ chmod +x ./Assgn1_1d.sh

For one input file:
$ ./Assgn1_1d.sh <input_filepath1>

For multiple input files:
$ ./Assgn1_1d.sh <input_filepath1> <input_filepath2> <input_filepath3> ...
```

The folder "files_mod" is created in the root directory and for each filepath in input, a new file is created in "files_mod", in which, each line starts with its line number and the spaces are now replaced by commas.

## e. Using Curl, Environment Variable and More

```shell
$ chmod +x ./Assgn1_1e.sh
$ ./Assgn1_1e.sh

For debugging:
$ ./Assgn1_1e.sh -v
```

This part of the assignment involver various tasks, the outputs for which are printed in the console, except for the part where the JSON syntax of each of the downloaded files in folder "data1e" is checked. After checking JSON syntax, the names of valid files are put in "valid.txt" and those of invalid ones are put in "invalid.txt", and the two files are now sorted in ascending order.

## f. Create a frequency distribution from a large file

```shell
$ chmod +x ./Assgn1_1f.sh
$ ./Assgn1_1f.sh 1c_input.txt/1d_input.txt <num>

Sample input:
$ ./Assgn1_1f.sh 1c_input.txt/1d_input.txt 4
```

The input file and a column number (num, varies from 1 to 4) are given as command line arguments, and a file "1c_output _\<num\>_column.freq" is created, which has the frequency of the elements in given column, and is sorted in decreasing order of frequency.

## g. Pattern matching

```shell
$ chmod +x ./Assgn1_1g.sh
$ ./Assgn1_1g.sh <filename> <column_number> <regular_expression>

Sample input:
$ ./Assgn1_1g.sh Data.csv 5 5$
```

A csv file with the given name is created in root directory with 150 rows and 10 columns, and filled with random integers. Then, it determines if the given column contains a value that matches the given regular expression.