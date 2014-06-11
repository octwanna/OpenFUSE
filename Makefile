#### Makefile for the FUSE project ####

all:	cobaltToHum orderHum partMesh

cobaltToHum: ./tools/cobaltToHum.cpp
	g++ -Wall -I/opt/openmpi/include -I. -I/opt/tclap-1.2.1/include/ -I/opt/hdf5-1.8.10p1/include -I./hum/ ./hum/constants.cpp ./tools/cobaltToHum.cpp -o cobaltToHum -L/opt/hdf5-1.8.10p1/lib/ -L/opt/openmpi/lib -lhdf5 -lz -lmpi -lmpi_cxx

orderHum: ./tools/orderHum.cpp
	g++ -Wall -I/opt/openmpi/include -I. -I/opt/tclap-1.2.1/include/ -I/opt/hdf5-1.8.10p1/include -I/opt/tbb/include/ -I./hum/streamer/ -I./hum/ -L/opt/hdf5-1.8.10p1/lib/ -L/opt/openmpi/lib -L/opt/tbb/lib/ ./hum/constants.cpp ./tools/orderHum.cpp -o orderHum -ltbb -lhdf5 -lz -lmpi -lmpi_cxx

partMesh: ./tools/partMesh.cpp
	g++ -Wall -I/opt/openmpi/include -I. -I/opt/tclap-1.2.1/include/ -I/opt/hdf5-1.8.10p1/include -I/opt/tbb/include/ -I./hum/streamer/ -I./hum/ -I./dimm/ -I./dimm/dd/ -L/opt/hdf5-1.8.10p1/lib/ -L/opt/openmpi/lib -L/opt/tbb/lib/ ./hum/constants.cpp ./tools/partMesh.cpp -o partMesh -ltbb -lhdf5 -lz -lmpi -lmpi_cxx

clean:
	rm cobaltToHum orderHum partMesh


