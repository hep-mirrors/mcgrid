## Change Log

### MCgrid v2.0 changes 02/03/16

- Handling of the fixed-order modifications introduced by S-MC@NLO events
- Support for fastNLO tables
- Support for APPLgrid scale logarithm grids
- Updated for SHERPA 2.2.1

### MCgrid v1.2 changes - 16/10/14

- Handling of more than one rivet analysis at once
- Fixed compatibility issue with YODA 1.3.0
- Fixed compatibility issue to APPLgrid 1.4.36

### MCgrid v1.1 changes - 08/08/14

- Updated subprocess scripts to work with SHERPA's new sqlite database
- Merged `identifyComix` and `identifyAmegic` scripts into one script
- Added reference histogram fill for APPLgrid
- Removed additional boost dependencies
- Added `BinnedGrid` class for Rivet analyses with a `BinnedHistogram`
- Updated for APPLgrid 1.4.56
- Updated for SHERPA 2.1.0
- Updated for Rivet 2.1.2

### MCgrid v1.0.1 changes - 31/01/14

- Fixed URL in `mcgrid.pc`
- Added example HepMC record for CDF Z Rapidity analysis
- Modified `applgrid-test` program to broaden compatibility with APPLgrid
versions
- Introduced epsilon for comparison of PDF $Q^2$ with APPLgrid bin edges which
enables running single-scale bins from HepMC records
- Fixed typo in example makefile cxxflags → cppflags for LHAPDF

### MCgrid v1.0 release version - 17/12/2013

