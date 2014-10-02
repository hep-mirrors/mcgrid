## Change Log

### MCgrid upcoming release

- Handling of more than one analysis at once (as long as there is memory left)

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
- Introduced epsilon for comparison of PDF Q^2 with APPLgrid bin edges which
enables running single-scale bins from HepMC records
- Fixed typo in example makefile cxxflags → cppflags for LHAPDF

### MCgrid v1.0 release version - 17/12/2013

