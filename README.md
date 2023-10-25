# BruteForce Dictionary

## 1) Building

- Dependencies: openssl (`libssl-dev` package on Debian based distros), make, gcc
- Compilation: `make` command will produce an executable named `main`

## 2) Running

This program has 2 modes available:

- __G mode__ will generate a `dict` file from a list of clear password. The goal here is to hash all passwords to retrieve them efficiently in the L mode.
  __Syntax:__ `./main G <file1> <file2> ...`
  It will generate one output file for each file given in argument: here `<file1>.<digest name>.dict` and `<file2>.<digest name>.dict`
  You can specify which algorithm you want to use. By default it uses MD5.
  __Example:__ `./main Gmd5 rockyou.txt` or `./main Gsha256 rockyou.txt`.
- __L mode__ will retrieve clear passwords from hashes. It takes hashes in __stdin__ and output the clear password in __stdout__. It will display an empty line if password is not in the dictionary.
  __Example:__ `./main L rockyou.txt.dict < hashes.txt > clear.txt`
  This mode support one or more dict files in input. It will just search the first occurence of the hash in the dict files.
- __T mode__ is an additional mode for test purposes. It just translate all clear passwords to hash. It supports all openssl algorithms.
  __Example:__ `./main Tmd5 < rockyou.txt > rockyou.md5.txt` or `./main Tsha256 < rockyou.txt > rockyou.md5.txt`

## 3) Dict file format

The dict file is a binary format is designed to store key and values and retrieve them efficiently without having to load the entiere file.

Actual limitations: because I use unsigned 32 bits integer for index in files. The file cannot be bigger than 2^32 bytes ~ 4Go. Also I use unsigned 8 bits integer for storing the size of key and value so they are limited to 255 characters. It will works in 99%+ of common passwords.

Here is a basic layout:

```
--------------------------
|        HEADER          |
--------------------------
|uint32 magic            |
|uint32 elements         |
|uint32 hash_table_index |
|uint32 mask             |
--------------------------
|         DATA           |
--------------------------
| ---------------------- |
| |uint8 key_size      | |
| |uint8 padding       | |
| |uint16 value_size   | | 
| |key[]...            | |
| |value[]...          | |
| ---------------------- |
|                        |
| ---------------------- |
| |uint8 key_size      | |
| |uint8 padding       | |
| |uint16 value_size   | | 
| |key[]...            | |
| |value[]...          | |
| ---------------------- |
| ---------------------- |
--------------------------
|      HASH TABLE        |
--------------------------
| ---------------------- |
| |uint32 hash         | | 
| |uint32 index        | |
| ---------------------- |
|                        |
| ---------------------- |
| |uint32 hash         | | 
| |uint32 index        | |
| ---------------------- |
--------------------------
```

The file is divided in 3 sections:

1) The header contains properties of file (`magic` to recognize file, `elements` is the number of entry, `hash_table_index` is the position of the hash table in the file and `mask` is the mask to use in hash table)
2) The Data section contains all key and value. This section is not loaded in memory. We only load key and value when necessary.
3) The Hash Table section is loaded in memory. It contains a hash and an index for each entry in the data section. The hash function in this case is just a xor of the key bytes.
