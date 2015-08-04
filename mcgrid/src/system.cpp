//
//  system.cpp
//  MCgrid 01/04/2013.
//

#include <stdlib.h>
#include <sys/stat.h>

#include "system.hh"

namespace MCgrid
{

void createPath(std::string const & path)
{
  // Now just use a system call to avoid linking to boost
  mkdir(path.c_str(),0777);
}

bool boolForEnvironmentVariableForKey(std::string const & key)
{
  std::string val(environmentVariableForKey(key));
  if (val == "" || val == "0" || val == "false") {
    return false;
  } else {
    return true;
  }
}

std::string environmentVariableForKey(std::string const & key)
{
  char * val;
  val = getenv( key.c_str() );
  std::string retval = "";
  if (val != NULL) {
    retval = val;
  }
  return retval;
}

}
