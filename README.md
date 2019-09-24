## Build

``` 
# apt install build-essential cmake libfuse2 libfuse-dev
# mkdir build; cd build;
# cmake ..
# make
```

## Usage

* 编辑 `config.txt`, 第一行是合并之后的文件名, 之后每一行为需要合并的每个文件, 例如
```
merged_filename
./CMakeCache.txt
./CombineFiles_FUSE
./Makefile
./cmake_install.cmake
```

* Create mount point
```
# mkdir mount_point
```

* Run
```
# ./CombineFiles_FUSE ./mount_point -o allow_other
```

* `./mount_point/merged_filename` 即为合并的文件

* Stop
```
# umount ./mount_point
or just
# killall CombineFiles_FUSE
``` 

## Notice

* 合并后的文件只读
* 如果修改了源文件请结束并重新运行