# UNIXShell</br></br>

clang shell -o shl.out</br>
./shl.out</br> </br>

issues:<br>
 - using !! more than once forces the last recorded input to revert to a single command (e.g. ls -l becomes ls).</br>
 - using cat after redirecting to a file causes the input file to become the output file.</br>
