echo "Setting up the directory"
echo "Copying make_Makefile.sh into this directory (from input/)"
cp input/make_Makefile.sh ./
echo "Copying a2MC.ini into this directory (from input/)"
cp input/a2MC.ini ./
echo "Creating the Makefile (this may not work out of the box"
source make_Makefile.sh
