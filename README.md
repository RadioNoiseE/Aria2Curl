# Aria2Curl

## What is this
A parser that converts the Aria2 input file into the Curl's input file.<br/>
一个让Curl能够处理Aria2的输入文件的解析器。<br/>

## Features
- Uses only the standard head files, namely `stdio.h`, `string.h` and `uchar.h` if you need to handle utf-8 characters.
- Can handle utf-8 characters elegently, which makes it really portable.
- Written in about 200 lines of `C`, fast and light-weight.
- The use of `fpos_t` enables it to read the file which is at any size in fast speed.
- Supports multi-targets in one file.

## How to use
`aria2curl [-o <output>] <input>` and `curl -K <output>`<br/>
Specify option `-#` if you want a progress bar.<br/>
The default output is `<input>.cfg`.

## Further development plans
- Support more options (currently, we support `referer`, `user-agent`, `out` and those have the same name between Aria2 and Curl).
- Directly do the actuall download (using `libcurl`)?
- Based on (ref2), support multi-thread or similar features?
### PR Welcomed!
