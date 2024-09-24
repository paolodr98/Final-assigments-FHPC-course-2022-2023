# Exercise 1

There are several directories:

* `bash_file/` : There are the bash files used to run the experiments. There are also those one for EPYC node even if they are not used.
* `data/`: Contain three folders, one for each scalability test. Each of these folders contain the `.csv` files with the raw data for both static and ordered dynamic, the plots and the python notebook used for analyze the data and make the plot.
* `headers/`: headers directory.
* `images/`: Folder to store the output images of the game.
* `Makefile`: File for compiling the source code and make the executable file.
* `main.x`: Executable file.
* `src/`: It contains all the source code files
  * `main.c` : contains the main. Reads arguments and calls one of the modes.
  * `initialize.c`: code for the initialize mode.
  * `read_write.c`: code for reading and writing pbm images provided by the professor
  * `static_evolution.c`: code with the functions for the static evolution
  * `ordered_evolution.c`: code with the functions for the ordered evolution

More detail about the game, how is implement and the result obtained can be found in the report.
