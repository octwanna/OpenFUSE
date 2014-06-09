#### Makefile for the FUSE project ####

all:	cobaltToHum orderHum

cobaltToHum: ./tools/cobaltToHum.cpp
	g++ -I/opt/openmpi/include -I. -I/opt/tclap-1.2.1/include/ -I/opt/hdf5-1.8.10p1/include -I./hum/ ./hum/constants.cpp ./tools/cobaltToHum.cpp -o cobaltToHum -L/opt/hdf5-1.8.10p1/lib/ -L/opt/openmpi/lib -lhdf5 -lz -lmpi -lmpi_cxx

orderHum: ./tools/orderHum.cpp
	g++ -I/opt/openmpi/include -I. -I/opt/tclap-1.2.1/include/ -I/opt/hdf5-1.8.10p1/include -I/opt/tbb/include/ -I./hum/streamer/ -I./hum/ -L/opt/hdf5-1.8.10p1/lib/ -L/opt/openmpi/lib -L/opt/tbb/lib/ ./hum/constants.cpp ./tools/orderHum.cpp -o orderHum -ltbb -lhdf5 -lz -lmpi -lmpi_cxx

clean:
	rm cobaltToHum orderHum


