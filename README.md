# MDPI-Applied-Sciences-2021
Sources and data of the SystemC simulation used in our journal article for MDPI Applied Sciences Special Issue "New Trends in Real-Time Embedded Systems"


[![DOI](https://zenodo.org/badge/369064108.svg)](https://zenodo.org/badge/latestdoi/369064108)


## Dependencies

* A Linux distribution with up-to-date libraries

### SystemC Simulation

* LLVM (clang, clang++) with C++14 support
* SystemC 2.3.3
* Python3 3.6+
  * numpy
  * sklearn
* Python3 Embed
* TinyXML2
* GNU Scientific Library (GSL)

### Results/Evaluation.sh

* GNU/octave

### Tools/pdfcompare2

* cairo 1.16+
* cairomm
* freetype2
* fontconfig
* libconfig
* LLVM (clang++) with C++17 support
* Adobe Source Sans Pro font

The SystemC path can be set in the build.sh scripts first lines.

## Usage

This section describes the content of this repository and how to get everything to work.
All input data, models and evaluation tools used for the related publication are included in this repository.

### SystemC Model

This directory contains the code and the configuration of the simulation as well as a set of scripts to run the simulation.

#### Build the actors code for the simulation

Inside the *experiments* sub directory, the description of the individual experiments are stored.
Since the simulation does also simulate the functional behavior, the Use-Case applications and their input data must be made available to the simulation.
This is done by dynamically loaded shared objects inside the *apps* directory.
So build these shared objects, the build scripts related to the Use-Cases must be executed:
```bash
./build-libjpeg.sh # Build apps/jpeg.so and apps/jpegdata.so
./build-libsobel2.sh # Build apps/sobel2.so and several sobel2data shared objects
```

#### Build the simulation

To build the simulation, the `build.sh` script must be executed.
This script compiles the whole source code of the simulation.
Before the compilation starts, the script checks if all dependencies needed to compile the code are available.
In case a library cannot be found you may want to check the code of the script.
Especially the SystemC installation path may be added at the beginning of the script where an array of possible paths is defined.

#### Run the test script

To test if everything works fine, you can execute the `demorun.sh` script after compiling the JPEG application and the SystemC simulation.
The script runs a functional simulation of 256 iterations of the JPEG decoder.
The results gets printed to the screen in form of colored characters:

![Demorun](JPEG Functional Simulation.png)

To see the results you need to have a terminal emulator that supports 24bit colors and a is configured with an Unicode capable font that comes with the '█' (FULL BLOCK, U+2588) character.
You may have to reduce the font size to be able to print 128 rows and 257 columns of characters.

#### Run a single simulation

To run a simulation you have to call the `model` executable with the following parameters:

* --experiment "Name of the experiment to run"
* --iterations "Number of iterations to simulate"

The experiment name is the file name of the experiment description inside the experiments directory without the file extension.
The number of iterations is an integer.

The following line would run the first 100 iterations of the JPEG-CA1-AVG experiment from the article:
```bash
./model --experiment "mdpi-JPEG-CA1Average" --iterations 100
```

Note that it will not print the decoded image into the terminal because this experiment is configured to do no functional simulation.
Instead only the simulated execution time of each iteration is printed to *stdout*.
Furthermore some meta information are printed to *stderr*.

#### Batch Runs

To automate running several experiments, and to make better use of multi-processor systems, you can use the `bathrun.sh` script.
To configure which experiments shall be executed, you need to edit the script file.
By default, it runs all experiments as it was done for the evaluation described in the article.



### Use-Cases

This directory contains the source code of the Use-Cases used for the evaluation and their dependencies.
The build-scripts from the SystemC simulation accesses these directories.



### PlatformV2

In this directory, the data of the characterization of the actors is stored.
The simulation loads the characterization from the related files, depending on the configuration of the experiment.

Furthermore inside the sub directories for the individual Use-Cases, there is another sub directory called *Iterations*.
This directory contains the observed execution times of 1000000 iterations of the whole application as it is described in the article.
These files are used to compare and evaluate the results of the simulation.



### Results

This directory contains all simulations results from the experiments described in the article.
Furthermore there is a script `Evaluation.sh` that compared the simulation results with the observed behavior and calculates the error and Bhattacharyya Distance for each experiment.



### Tools

In this directory some additional tools are provided to be able to fully reproduce the experiments.

#### jpeg2c

The *jpeg2c* tool creates a C file out of a JPEG file.
The JPEG decoder Use-Cases is not able to parse a jpg-file.
It expects the raw data of the JPEG in form of a set of C arrays.

For details, read the README inside corresponding directory.

#### pdfcompare2

This tool is used to perform the Bhattacharyya distance calculations and to plot the histograms shown in the article.

For details, read the README inside corresponding directory.

