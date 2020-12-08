#include <stdlib.h>
#include <string>

struct MatchPathSeparator
{
  bool operator()( char ch ) const
  {
    // posix
    return ch == '/';
    // // windows
    // return ch == '\\' || ch == '/';
  }
};

std::string basename( std::string const& pathname );
std::string removeExtension( std::string const& filename );

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
