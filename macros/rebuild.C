//optional, force build! Useful if you've been editing the libraries yourself
bool force_rebuild = true; 
if (force_rebuild)
{
   std::cout<< "Write operations of small files suprisingly slow on EOS (ie when compiling)... "<<std::endl;
   std::cout<< "This is going to take some time... go for lunch" <<std::endl;
   std::string rebuild = "bash -c 'cd alphasoft         \n";
   //rebuild          += "rm -rfv build bin             \n";
   rebuild          += "source agconfig.sh\n";
   rebuild          += "mkdir -p build                \n";
   rebuild          += "cd build                      \n";
   rebuild          += "cmake ../   | tee ../build.log && \\  \n";
   rebuild          += "make -j4    | tee ../build.log && \\  \n";
   rebuild          += "make install | tee ../build.log \n";
   rebuild          += "cd ..                         \n";
   rebuild          += "cat build.log                 '\n";
   gSystem->Exec(rebuild.c_str());
}