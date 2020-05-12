#!/usr/bin/env python
group1 = [ 0, 1, 15 ]
group2 = [ 7, 8, 9 ]

num = 0

for i in group1:
    num = num + (1<<i)

num = num << 16

for i in group2:
    num = num + (1<<i)

print "0x%x"%num
