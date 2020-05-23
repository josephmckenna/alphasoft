mkdir $AGRELEASE/build
cd $AGRELEASE/build
#
cmake3 .. -DBUILD_AG_SIM=ON -DBUILD_A2=OFF 
time cmake3 --build . -- -j
time cmake3 --build . --target install -- -j
#
cmake3 --build . --target clean
rm -rf $AGRELEASE/build $AGRELEASE/bin


