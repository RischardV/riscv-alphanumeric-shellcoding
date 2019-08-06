This folder contains the demos running on Linux on the HiFive Unleashed board.

How to use:
1- Build the project: `make`
2- Copy the following files on the HiFive Unleashed board:
   - `build/vuln.bin`
   - `out/hash.txt`
   - `out/slash.txt`
   - `out/tick.txt`
3- Run the demos:
   `./vuln.bin < hash.txt`
   `./vuln.bin < slash.txt`
   `./vuln.bin < tick.txt`
