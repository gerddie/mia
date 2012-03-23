import mia
import numpy 

#mia.set_verbose("debug")

a = numpy.array([(1,2,3,4), (5,6,7,8)])

b = mia.run_filters(a, [])

if numpy.all(a==b):
    print "empty copy nice"
else:
    print "empty copy failed"


c = numpy.array([( 0, 1, 2, 3, 2, 3, 5),
		 ( 2, 5, 2, 3, 5, 3, 2),
		 ( 1, 2, 7, 6, 4, 2, 1),
		 ( 3, 4, 4, 3, 4, 3, 2),
		 ( 1, 3, 2, 4, 5, 6, 2)], dtype=numpy.int32);

d = mia.run_filters(c, ["median:w=1"])

dr = numpy.array([( 1, 2, 2, 2, 3, 3, 3),
		 ( 1, 2, 3, 3, 3, 3, 2),
		 ( 2, 3, 4, 4, 3, 3, 2),
		 ( 2, 3, 4, 4, 4, 3, 2),
		 ( 3, 3, 3, 4, 4, 3, 2)], dtype=numpy.int32);

if numpy.all(d==dr):
    print "filter 2d run nice"
else:
    print "filter 2d run failed"
    print d == dr
    print c
    print d
    print dr




c3d = numpy.array([((1.0, 2.0, 3), (4, 5, 6),(7, 8, 9)),
                   ((11, 12, 13), (14, 15, 16), (17, 18, 19)),
                   ((21, 22, 23), (24, 25, 26), (27, 28, 29))])

d3d = mia.run_filters(c3d, ["median:w=1"])


d3dr = numpy.array([((8, 8.5, 9),(9.5,  10, 10.5), (11, 11.5,  12)),
                    ((13,  13.5, 14), (14.5, 15, 15.5), (16, 16.5, 17)),
                    ((18, 18.5, 19), (19.5, 20, 20.5), (21, 21.5, 22))])

if numpy.all(d3d==d3dr):
    print "filter 3d run nice"
else:
    print "filter 3d run failed"


