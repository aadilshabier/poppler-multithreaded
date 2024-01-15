# Instructions

Instructions to use the modified version of `pdftohtml`. Contrary to it's name, this is now used to produce JSON files in a specific format from an PDF.

1. Download all the dependencies(Arch Linux dependencies found in [./Dockerfile]()

2. Build directory setup
```sh
$ mkdir build
$ cd build
```

3. Run cmake.
```sh
$ cmake ..
```

4. Compile. Replace 6 with the number of threads you want to use
```sh
$ make -j 6 pdftohtml
```
5. Make shared configuration and credential files as given in <https://docs.aws.amazon.com/sdkref/latest/guide/file-format.html>
   On Linux/MacOS, this should be in `~/.aws/config` and `~/.aws/credentials`

6. Use it like this. Replace 6 with the number of threads you want to use
```sh
$ ./utils/pdftohtml -json -j 6 -bucket my-bucket-name File.pdf
```
   To manually provide a filename to the bucket, use it like this
```sh
$ ./utils/pdftohtml -json -j 6 -bucket my-bucket-name File.pdf FileOut
```
