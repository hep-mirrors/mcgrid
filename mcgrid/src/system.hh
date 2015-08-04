//
//  system.hh
//  MCgrid 01/04/2013.
//

#ifndef mcgrid_system_hh
#define mcgrid_system_hh

#include <string>

namespace MCgrid
{

void createPath(std::string const &);
bool boolForEnvironmentVariableForKey(std::string const & key);
std::string environmentVariableForKey(std::string const &);

}

#endif
