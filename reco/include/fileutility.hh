#include <stdlib.h>
#include <string>
#include <algorithm>


struct MatchPathSeparator
{
   bool islinux() const
   {
      //std::string ostype(getenv("OSTYPE"));
#ifdef OSTYPE
      std::string ostype(OSTYPE);
#else
      std::string ostype("linux"); // default to linux
#endif
      //  std::transform(ostype.begin(),ostype.end(),ostype.begin(),std::tolower);
      std::string str("linux");
      return ostype.compare(0,str.length(),str)==0?true:false; 
   }

   bool operator()( char ch ) const
   {
      // posix
      if(islinux()) return ch == '/';
      // windows
      else return ch == '\\' || ch == '/';
   }
};

std::string
basename( std::string const& pathname )
{
   return std::string( 
                      std::find_if( pathname.rbegin(), pathname.rend(),
                                    MatchPathSeparator() ).base(),
                      pathname.end() );
}

std::string
removeExtension( std::string const& filename )
{
   std::string::const_reverse_iterator
      pivot
      = std::find( filename.rbegin(), filename.rend(), '.' );
   return pivot == filename.rend()
      ? filename
      : std::string( filename.begin(), pivot.base() - 1 );
}


/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
