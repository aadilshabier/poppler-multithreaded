# Instructions

Instructions to use the modified version of `pdftoxml`. Contrary to it's name, this is now used to produce JSON files in a specific format from an PDF.

1. Build directory setup
```sh
$ mkdir build
$ cd build
```

2. Run cmake.
```sh
$ cmake ..
```

3. Compile. Replace 6 with the number of threads you want to use
```sh
$ make -j 6 pdftohtml
```
4. Make shared configuration and credential files as given in <https://docs.aws.amazon.com/sdkref/latest/guide/file-format.html>
   On Linux/MacOS, this should be in `~/.aws/config` and `~/.aws/credentials`

4. Use it like this. Replace 6 with the number of threads you want to use
```sh
$ ./utils/pdftohtml -json -j 6 -bucket my-bucket-name File.pdf
```
   To manually provide a filename to the bucket, use it like this
```sh
$ ./utils/pdftohtml -json -j 6 -bucket my-bucket-name File.pdf FileOut
```
