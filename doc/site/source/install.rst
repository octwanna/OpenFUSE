Installation
=============

Cloning OpenFUSE source from github.com
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Make sure you have git version control system installed in your machine. Then
clone the OpenFUSE `repository <https://github.com/pavanakumar/OpenFUSE>`_ from github.com. 

  >>> git clone https://github.com/pavanakumar/OpenFUSE
  Cloning into 'OpenFUSE'...
  remote: Counting objects: 161, done.
  remote: Compressing objects: 100% (146/146), done.
  remote: Total 161 (delta 70), reused 54 (delta 12)
  Receiving objects: 100% (161/161), 957.67 KiB | 295.00 KiB/s, done.
  Resolving deltas: 100% (70/70), done.
  Checking connectivity... done.
  >>> cd OpenFUSE

Now checkout the master branch, which is the working branch.

  >>> git checkout master
 
Installing 3rd party libraries
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

OpenFUSE depends on three open-source libraries viz, HDF5, TCLAP and Intel TBB. It is mandatory to install them to compile the library.

  * HDF5 source can be downloaded from the web page http://www.hdfgroup.org/HDF5/release/obtain5.html. Extract the source and configure the source to work in parallel. Then install the sources in the desired install path $INSTALL_DIR

  >>> tar -zxvf hdf5-1.8.10-patch1.tar.gz
  >>> cd hdf5-1.8.10-patch1
  >>> ./configure --prefix=$INSTALL_DIR --enable-parallel
  >>> make install

  * TCLAP source can be downloaded from the web page http://sourceforge.net/projects/tclap/files/. This is header only library so installation is quite easy just extract the sources, configure and install.

  >>> tar -zxvf tclap-1.2.1.tar.gz
  >>> cd tclap-1.2.1
  >>> ./configure --prefix=$INSTALL_DIR
  >>> make install

  * Intel TBB can be downloaded from the web page https://www.threadingbuildingblocks.org/download. Extract the sources and make the library. But the installation is slightly tricky as TBB does not understand a install prefix. So do the following

  >>> tar -zxvf tbb_source.tgz
  >>> cd tbb_source
  >>> make
  >>> TBB_DEBUG_LIB=`find . -name libtbb_debug.so`
  >>> TBB_MALLOC_DEBUG_LIB=`find . -name libtbbmalloc_debug.so`
  >>> TBB_RELEASE_LIB=`find . -name libtbb.so`
  >>> TBB_MALLOC_RELEASE_LIB=`find . -name libtbbmalloc.so`
  >>> cp $TBB_DEBUG_LIB $INSTALL_DIR/lib/
  >>> cp $TBB_MALLOC_DEBUG_LIB $INSTALL_DIR/lib/
  >>> cp $TBB_RELEASE_LIB $INSTALL_DIR/lib/
  >>> cp $TBB_MALLOC_RELEASE_LIB $INSTALL_DIR/lib/
  >>> cp -r ./include/* $INSTALL_DIR/include

Compiling OpenFUSE
^^^^^^^^^^^^^^^^^^

Now edit the Makefile provided in the OpenFUSE repository and substitute the hdf5, tclap and tbb folders.

  >>> cd OpenFUSE
  >>> make


