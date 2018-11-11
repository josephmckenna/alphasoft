
#############################
  INSTALLATION:
#############################


git clone https://jtkm@bitbucket.org/ttriumfdaq/agdaq.git
cd agdaq
. agconfig.sh
make -j


#############################
  Internal definitions:
#############################



Helix:
-1	Number of spacepoints is less than 5
0	Fitted helix
1	Good helix - NOT used for vertexing
2	Seed helix - used for vertexing 
3	Added helix - used for vertexing
-2	R fit failed
-3	Z fit failed
-4	R chi^2 cut failed
-5	Z chi^2 cut failed
-6	D cut failed
-7	duplicated

Vertex:
-2	no good helices
0	only one good helix
-1	failed to find minimum-distance-pair
1	only two good helices
2	didn't improve vertex (more than 2 helices)
3	vertex improved

