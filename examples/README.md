# MCGrid examples

For any questions, please feel free to [contact the developers](mcgrid@projects.hepforge.org).

In this directory, there are examples of a typical setup for creating QCD interpolation grids for histograms of a given analysis. The example analyses are CDF_2009_S8383952, ATLAS_2011_S9131140 (both Drell-Yan) and CMS_2011_S9086218 (inclusive jets) and have been tested for Rivet version 2.2.1, Sherpa version 2.2.0 and MCgrid version 2.0. If you use older software, you can try [earlier snapshots](https://www.hepforge.org/downloads/mcgrid) of these examples.

In each analysis directory, you find a Makefile with the targets `plugin-applgrid`, `plugin-fastnlo` and (if combined grid creation is possible) `plugin-applgrid-and-fastnlo`. These compile the MCgrid enabled Rivet analysis for the respective grid implementation (APPLgrid and/or fastNLO). You can then install it and its associated data files with the target `install`. The target `install-applgrid` installs subprocess configuration where APPLgrid can find them and is only needed if you want to create APPLgrids.

A Sherpa run card (`Run.dat`) is provided for each example analysis. If you run the `Sherpa` executable inside an analysis directory, it will be read in and event generation begins. For a quick closure test, you might want to use the `-e` flag of `Sherpa` to generate fewer events, e.g. `Sherpa -e10k`. The first event generation will only determine how large the grids have to be. Only the second run will actually create a grid using the information from the first run. The entire MCgrid output is written to the `mcgrid` directory.

The provided run cards tell Sherpa to use its internal CT10 PDF set:

    PDF_LIBRARY=CT10Sherpa;
    PDF_SET=ct10;

See the [Sherpa documentation](https://sherpa.hepforge.org/trac/wiki/SherpaManual) on how to create grids using other PDFs (e.g. from the LHAPDF library). However, at least for fixed-order event generation all PDF dependencies are divided out before creating a grid, so it should not make a difference.

## The example analyses

*   `CDF_2009_S8383952`/`D0_2007_S7075677`:
          Modified versions of the CDF/D0 $Z$ rapidity analyses.
          The CDF analysis will produce two grid files,
          one for the total cross section, one for the
          rapidity distribution.
          The D0 analysis produces just one for the
          rapidity distribution.

*   `ATLAS_2011_S9131140`: 
          A modified version of an ATLAS $Z$ $p_T$ analysis.
          This analysis will produce a grid file for
          the distribution reconstructed from bare electrons.

*   `CMS_2011_S9086218`: A modification
          of a CMS inclusive jet analysis.
          This is intended as a demonstration of the
          `MCGrid::BinnedGrid` class, which complements
          Rivet's `BinnedHistogram`.


## Convolution

After creating a grid, you need to convolute it with a given set of QCD input parameters.

fastNLO comes with a binary to produce YODA files from their grids called `fnlo-tk-yodaout`, if configured with `--with-yoda`. It takes the input parameter choice as arguments.

For APPLgrid, we provide a very basic test program for scale variation, `test-applgrid`. You can compile it using the `test-applgrid` target of the root-level Makefile. For a more sophisticated script which is able to output YODA files, check out [applgrid2yoda](https://github.com/ebothmann/applgrid2yoda).

All of these scripts use LHAPDF to obtain the PDF values, so you should make sure that LHAPDF can find the specified PDF set, see the [LHAPDF website](http://lhapdf.hepforge.org) for a list of available PDF sets and how to install them correctly.

As soon as you have a bunch of YODA files from the event generation and from grid convolutions, you can use the `comparison-plot` target of the example's Makefile. This will use the `rivet-mkhtml` script provided by Rivet to make a website located at `./plots` with comparison plots between all the YODA found in the directory. This will internally use LaTeX for the plot labels, so if they are missing you should make sure that LaTeX can be found on your setup. `rivet-mkhtml` is using the Python packages of Rivet and YODA. If they are not installed to standard locations, then you might have to make sure that their `site-packages` directories are in your `PYTHONPATH` environment variable.

The `comparison-plot` target will hide the reference data from Rivet before plotting to ensure that the plots are normalized against the Monte Carlo prediction, and not against the reference data. This is appropriate for closure tests to check whether your grid predictions reproduce the Monte Carlo one. Check out [heppyplot](https://github.com/ebothmann/heppyplot) if you need a plotting tool that is developed mainly for such closure tests.


## Closure test example for the CDF_2009_S8383952 analysis

Assuming that your setup is as described in the above sections, you can do a closure test for the CDF_2009_S8383952 example using fastNLO interpolation grids created with MCgrid like this:

    cd examples/CDF_2009_S8383952
    make plugin-fastnlo
    make install
    Sherpa && Sherpa
    fnlo-tk-yodaout mcgrid/MCgrid_CDF_2009_S8383952/d02-x01-y01.tab CT10
    rivet-mkhtml *.yoda

Then open `plots/index.html` with your web browser. If everything went right, the relative deviations between the Monte Carlo and the interpolation grid predictions should be of the order of a few 0.0001, which is the interpolation accuracy for this particular setup. You can improve the result by using the LHAPDF interface of Sherpa instead of the internal CT10 library, such that both the event generation and the convolution uses the exact same PDF interpolation codes.

## Making your own MCgrid enabled analyses

Have a look into the `BASIC` analysis located in the `templates` directory.
This demonstrates the minimal modifications
required to a Rivet analysis to enable MCgrid.
Use this as a cheat sheet to produce your own
MCgrid enabled analysis.
If you want to start from an existing analysis, the [mcgridifyRivetAnalyses.py](https://github.com/ebothmann/MCgridify-Rivet-analyses) script can download and prepare the needed files for you.
