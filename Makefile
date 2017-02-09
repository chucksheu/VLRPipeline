# Top-level Makefile

DEBUGFLAGS = -g -Wall 
export DEBUGFLAGS
GLOBAL_CXXFLAGS = -I/usr/local/Cellar/libmemcached/1.0.18_1/include -I/usr/local/Cellar/boost/1.63.0/include
GLOBAL_LDFLAGS = -L/usr/local/Cellar/libmemcached/1.0.18_1/lib -L/usr/local/Cellar/boost/1.63.0/lib

export GLOBAL_CXXFLAGS
export GLOBAL_LDFLAGS

all: libs programs

libs:
# Make libraries
#	cd OpenCVExtensions; $(MAKE)
	cd Common; $(MAKE)
	cd KMajorityLib; $(MAKE)
	cd IncrementalKMeansLib; $(MAKE)
	cd VocabLib; $(MAKE)

programs:
# Make programs
	cd FeatureExtract; $(MAKE)
	cd FeatureSelect; $(MAKE)
	cd SelectDescriptors; $(MAKE)
	cd VocabLearn; $(MAKE)
	cd VocabBuildDB; $(MAKE)
	cd VocabMatch; $(MAKE)
	cd GeomVerify; $(MAKE)
	cd ComputeMAP; $(MAKE)
	cd ListBuild; $(MAKE)

clean: clean-libs clean-programs

clean-libs:
# Clean libraries
	cd OpenCVExtensions; $(MAKE) clean
	cd Common; $(MAKE) clean
	cd KMajorityLib; $(MAKE) clean
	cd IncrementalKMeansLib; $(MAKE) clean
	cd VocabLib; $(MAKE) clean

clean-programs:
# Clean program
	cd FeatureExtract; $(MAKE) clean
	cd FeatureSelect; $(MAKE) clean
	cd SelectDescriptors; $(MAKE) clean
	cd VocabLearn; $(MAKE) clean
	cd VocabBuildDB; $(MAKE) clean
	cd VocabMatch; $(MAKE) clean
	cd GeomVerify; $(MAKE) clean
	cd ComputeMAP; $(MAKE) clean
	cd ListBuild; $(MAKE) clean

tests:
#	cd Common; $(MAKE)
#	cd KMajorityLib; $(MAKE)
#	cd VocabLib; $(MAKE)
	cd Common/tests; $(MAKE)
	cd KMajorityLib/tests; $(MAKE)
	cd IncrementalKMeansLib/tests; $(MAKE)
	cd VocabLib/tests; $(MAKE)

tests-clean:
#	cd Common; $(MAKE) clean
#	cd KMajorityLib; $(MAKE) clean
#	cd VocabLib; $(MAKE) clean
	cd Common/tests; $(MAKE) clean
	cd KMajorityLib/tests; $(MAKE) clean
	cd IncrementalKMeansLib/tests; $(MAKE) clean
	cd VocabLib/tests; $(MAKE) clean
