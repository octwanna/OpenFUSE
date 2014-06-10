/*! \file h5p++.hpp
 ** A standalone C++ header only library for accessing HDF5 functions
 ** in a type independent manner using templates. It eases reading and
 ** writing to HDF5 files by providing a uniform interface regardless 
 ** of the data-type.
 **/
 
#ifndef H5PP_H

#define H5PP_H

#include<mpi.h>
#include<list>
#include<string>
#include<sstream>
#include<cassert>
#include<hdf5.h>

namespace h5pp {
  typedef std::list<std::string> listString;
  typedef std::list<std::string>::iterator listString_it;
  template <class type> hid_t GetHDF5Type();
  template <class type> H5T_class_t GetHDF5TypeClass();

  /*! \brief Converts list of strings to string by 
   **         concanating all the list elements by "/"
   **/
  std::string ListStringToString(listString &link) {
    std::stringstream cat;
    for (listString_it it = link.begin(); it != link.end(); ++it)
      cat << "/" << *it;
    return cat.str();
  }

  /*! \brief Checks if a object given by the string grp_link  
   **         exists in the hdf5 file
   **/
  bool IsValidLink(hid_t &file, const char *grp_link) {
    if (H5Lexists(file, grp_link, H5P_DEFAULT) == 0) // 0 = FALSE
      return false;
    else
      return true;
  }

  /*! \brief Checks if a group given by the string grp_link  
   **         exists in the hdf5 file
   **         (Overlaoded version )
   **/
  bool IsValidLink(hid_t &file, listString &link) {
    std::string cat = ListStringToString(link);
    return IsValidLink(file, cat.c_str());
  }

  /*! \brief Checks if dataset given by the link dataset_link 
   **         exists in the hdf5 file and is of type vtype
   **  
   **   \todo Also if passed as a vector check if also satisfies
   **          rank and dimensions
   **/
  bool IsDataset(hid_t &file, const char *dataset_link) {
    H5O_info_t object;
    if (IsValidLink(file, dataset_link) == true) {
      if (H5Oexists_by_name(file, dataset_link, H5P_DEFAULT) == 1) { // 1 = TRUE
        H5Oget_info_by_name(file, dataset_link, &object, H5P_DEFAULT);
        if (object.type == H5O_TYPE_DATASET)
          return true;
      }
    }
    return false;
  }

  /*! \brief Create the group given by link    
   **         std::list<string> link 
   **/
  void CreateGroup(hid_t &file, listString &link) {
    if (link.size() >= 1) {
      std::stringstream cat;
      hid_t grp, status;
      for (listString_it it = link.begin(); it != link.end(); ++it) {
        cat << "/" << *it;
        if (!IsValidLink(file, cat.str().c_str())) {
          grp = H5Gcreate(file, cat.str().c_str(), H5P_DEFAULT,
            H5P_DEFAULT, H5P_DEFAULT);
          status = H5Gclose(grp);
        }
      }
    }
  }

  /*! \brief Returns the legth of a 1d vector dataset from HDF5 file 
   **     
   **/
  template <class itype >
  itype GetVectorLength(hid_t &file, listString &link) {
    hsize_t len;
    itype ret_len;
    hid_t dset, dspace;
    std::stringstream cat;
    for (listString_it it = link.begin(); it != link.end(); ++it)
      cat << "/" << *it;
    assert(IsValidLink(file, cat.str().c_str()) == true);
    dset = H5Dopen2(file, cat.str().c_str(), H5P_DEFAULT);
    dspace = H5Dget_space(dset);
    int rank = H5Sget_simple_extent_ndims(dspace);
    assert(rank == 1);
    rank = H5Sget_simple_extent_dims(dspace, &len, NULL);
    ret_len = len;
    H5Dclose(dset);
    H5Sclose(dspace);
    return ret_len;
  }

  /*! \brief Makes sure the group given by link    
   **        exists in the file for writing the 
   **        dataset which is the last string in the
   **        std::list<string> link 
   **/
  void CreateGroupForDset(hid_t &file, listString &link) {
    if (link.size() > 1) {
      std::stringstream cat;
      hid_t grp, status;
      listString_it lastm1;
      lastm1 = link.end();
      lastm1--;
      for (listString_it it = link.begin(); it != lastm1; ++it) {
        cat << "/" << *it;
        if (!IsValidLink(file, cat.str().c_str())) {
          grp = H5Gcreate(file, cat.str().c_str(), H5P_DEFAULT,
            H5P_DEFAULT, H5P_DEFAULT);
          status = H5Gclose(grp);
        }
      }
    }
  }

  /*! \brief Checks if attribute given by the link attribute_link 
   **         exists in the hdf5 file and is of type vtype
   **  
   **   \todo Also if passed as a vector check if also satisfies
   **          rank and dimensions
   **/
  template <class T>
  bool IsAttribute(hid_t &file, listString &link) {
    std::stringstream cat;
    if (link.size() == 1)
      cat << "/";
    else
      for (listString_it it = link.begin(); it != --link.end(); ++it)
        cat << "/" << *it;
    herr_t status = H5Aexists_by_name(file, cat.str().c_str(), link.back().c_str(), H5P_DEFAULT);
    if (status == 0)
      return false;

    return true;
  }

  /* \brief Get the sub-group sizes 
   */
  hsize_t GetSubGroupSize(hid_t file, listString &link) {
    H5G_info_t ginfo;
    std::string cat = ListStringToString(link);
    assert(IsValidLink(file, cat.c_str()) == true);
    hid_t group = H5Gopen(file, cat.c_str(), H5P_DEFAULT);
    assert(H5Gget_info(group, &ginfo) >= 0);
    H5Gclose(group);
    return ginfo.nlinks;
  }

  /* \brief Get the names of each sub-group
   */
  std::string GetSubGroupName(hid_t file, hsize_t idx, listString &link) {
    std::string cat = ListStringToString(link);
    assert(IsValidLink(file, cat.c_str()) == true);
    hid_t group = H5Gopen(file, cat.c_str(), H5P_DEFAULT);
    /// Obtain string length by passing NULL
    ssize_t size = H5Lget_name_by_idx
      (
      group, ".", H5_INDEX_NAME, H5_ITER_INC,
      idx, NULL, 0, H5P_DEFAULT
      ) + 1;
    /// Read the string
    char *name = new char[size];
    size = H5Lget_name_by_idx
      (
      group, ".", H5_INDEX_NAME, H5_ITER_INC,
      idx, name, (size_t) size, H5P_DEFAULT
      );
    H5Gclose(group);
    return std::string(name);
  }

  /*! \brief Reads scalar attribute from HDF5 file 
   **  
   **/
  template < typename T >
  T ReadAttribute(hid_t &file, listString &link) {
    std::stringstream cat;
    hid_t attr;
    T retVal;
    switch (link.size()) {
      case 1:
        cat << "/"; // No need to check for validity as it will exist
        break;

      default:
        for (listString_it it = link.begin(); it != --link.end(); ++it)
          cat << "/" << *it;
        /// Check if the object to hold the link exists
        assert(IsValidLink(file, cat.str().c_str()));
        break;
    }
    /// Attribute does not exist
    assert(IsAttribute<T>(file, link));
    attr = H5Aopen_by_name
      (
      file, cat.str().c_str(),
      link.back().c_str(),
      H5P_DEFAULT, H5P_DEFAULT
      );
    herr_t status = H5Aread(attr, GetHDF5Type<T>(), &retVal);
    assert(status >= 0);
    H5Aclose(attr);

    return retVal;
  }

  /*! \brief Reads scalar attribute from HDF5 file 
   **   overloaded for generic user specified data-types
   **/
  template < typename T >
  void ReadAttribute(hid_t &file, listString &link, T &data, hid_t dtype) {
    std::stringstream cat;
    hid_t attr;
    switch (link.size()) {
      case 1:
        cat << "/"; // No need to check for validity as it will exist
        break;

      default:
        for (listString_it it = link.begin(); it != --link.end(); ++it)
          cat << "/" << *it;
        /// Check if the object to hold the link exists
        assert(IsValidLink(file, cat.str().c_str()));
        break;
    }

    /// Attribute does not exist
    assert(IsAttribute<T>(file, link));
    attr = H5Aopen_by_name
      (
      file, cat.str().c_str(),
      link.back().c_str(),
      H5P_DEFAULT, H5P_DEFAULT
      );
    herr_t status = H5Aread(attr, dtype, &data);
    assert(status >= 0);
    H5Aclose(attr);
  }

  /*! \brief Write the attribute data given by link    
   **         std::list<string> link 
   **/
  template<typename T>
  void WriteAttribute(hid_t &file, listString &link, T data) {
    std::stringstream cat;
    hid_t attr;
    switch (link.size()) {
      case 1:
        cat << "/"; // No need to check for validity as it will exist
        break;

      default:
        for (listString_it it = link.begin(); it != --link.end(); ++it)
          cat << "/" << *it;
        /// Check if the object to hold the link exists
        assert(IsValidLink(file, cat.str().c_str()));
        break;
    }
    hid_t dspace = H5Screate(H5S_SCALAR);
    /// Attribute does not exist
    if (IsAttribute<T>(file, link) == false) {
      attr = H5Acreate_by_name
        (
        file, cat.str().c_str(),
        link.back().c_str(), GetHDF5Type<T>(),
        dspace, H5P_DEFAULT,
        H5P_DEFAULT, H5P_DEFAULT
        );
    } else { /// Attribute exists in file
      attr = H5Aopen_by_name
        (
        file, cat.str().c_str(),
        link.back().c_str(),
        H5P_DEFAULT, H5P_DEFAULT
        );
    }
    herr_t status = H5Awrite(attr, GetHDF5Type<T>(), &data);
    H5Aclose(attr);
    H5Sclose(dspace);
    assert(status >= 0);
  }

  /*! \brief Write the attribute data given by link and data-type  
   **        specified by user (overloaded version) 
   **/
  template<typename T>
  void WriteAttribute(hid_t &file, listString &link, T &data, hid_t dtype) {
    std::stringstream cat;
    hid_t attr;
    switch (link.size()) {
      case 1:
        cat << "/"; // No need to check for validity as it will exist
        break;

      default:
        for (listString_it it = link.begin(); it != --link.end(); ++it)
          cat << "/" << *it;
        /// Check if the object to hold the link exists
        assert(IsValidLink(file, cat.str().c_str()));
        break;
    }
    hid_t dspace = H5Screate(H5S_SCALAR);
    /// Attribute does not exist
    if (IsAttribute<T>(file, link) == false) {
      attr = H5Acreate_by_name
        (
        file, cat.str().c_str(),
        link.back().c_str(), dtype,
        dspace, H5P_DEFAULT,
        H5P_DEFAULT, H5P_DEFAULT
        );
    } else { /// Attribute exists in file
      attr = H5Aopen_by_name
        (
        file, cat.str().c_str(),
        link.back().c_str(),
        H5P_DEFAULT, H5P_DEFAULT
        );
    }
    herr_t status = H5Awrite(attr, dtype, &data);
    H5Aclose(attr);
    H5Sclose(dspace);
    assert(status >= 0);
  }

  /*! \brief Creates data-space of rank one with length, 
   **         offset and stride specified
   **/
  void MakeDspaceOffsetStride(hid_t &dspace, hsize_t size, /*!< Total dataset size **/
    hsize_t slabSize, hsize_t offset, /*!< Slab size to select **/
    hsize_t stride) {
    herr_t status;
    dspace = H5Screate_simple(1, &size, NULL);
    status = H5Sselect_hyperslab(dspace, H5S_SELECT_SET, &offset,
      &stride, &slabSize, NULL);
    assert(H5Sselect_valid(dspace) != 0);
  }

  /*! \brief Creates data-space of rank one with global_length, 
   **         and select the elements using list object
   **          
   **
   **   In the context of parallel I/O one needs to specify the 
   **   the global array size and the local size of array in each
   **   MPI ranks.
   **/
  void ListSelectDspace(hid_t &dspace, hsize_t *list,
    hsize_t size, hsize_t listSize) {
    herr_t status;
    dspace = H5Screate_simple(1, &size, NULL);
    status = H5Sselect_elements(dspace, H5S_SELECT_SET, listSize, list);
    assert(H5Sselect_valid(dspace) != 0);
  }

  /*! \brief Creates data-set given the dataspace and file link 
   **         in the HDF5 file given the HDF5 type
   **/
  void CreateDset(hid_t &file, hid_t &dset, hid_t &dspace,
    hid_t &dtype, listString &link) {
    std::string cat = ListStringToString(link);
    dset = H5Dcreate(file, cat.c_str(), dtype,
      dspace, H5P_DEFAULT,
      H5P_DEFAULT, H5P_DEFAULT);
    H5Tclose(dtype);
  }

  /*! \brief Creates data-set assuming I know nothing about it in the file 
   **         so if it already exisits I just open the dset and return it
   **         it also creates a simple dataspace
   **/
  void CreateDsetDspace(hid_t &file, hid_t &dset, hid_t &dspace,
    hid_t &dtype, hsize_t len, listString &link) {
    std::string cat = ListStringToString(link);
    if (IsValidLink(file, cat.c_str()) == false) {
      dspace = H5Screate_simple(1, &len, NULL);
      dset = H5Dcreate(file, cat.c_str(), dtype,
        dspace, H5P_DEFAULT,
        H5P_DEFAULT, H5P_DEFAULT);
    } else {
      dset = H5Dopen2(file, cat.c_str(), H5P_DEFAULT);
      dspace = H5Dget_space(dset);
    }
  }

  /*! \brief Read node information from hum using a offset/stride  
   **        Assumes data has enough space of size
   **    (parallel version)
   **/
  template < typename T >
  void ReadVectorData
  (
    hid_t file, T *data, hid_t mem_dtype, h5pp::listString &link,
    hsize_t offset, hsize_t stride, hsize_t size
    ) {
    // Variables used in local function
    herr_t status;
    hid_t dspace_mem, dspace_file, dset;
    std::string cat = h5pp::ListStringToString(link);
    // Dataspace to describe Layout in Memory
    dspace_mem = H5Screate_simple(1, &size, NULL);
    // Dataspace to describe Layout in File
    assert(h5pp::IsDataset(file, cat.c_str()) == true);
    dset = H5Dopen2(file, cat.c_str(), H5P_DEFAULT);
    dspace_file = H5Dget_space(dset);
    status = H5Sselect_hyperslab(dspace_file, H5S_SELECT_SET, &offset,
      &stride, &size, NULL);
    assert(H5Sselect_valid(dspace_file) != 0);
    // Read data from file to memory 

    hid_t plist_id = H5Pcreate(H5P_DATASET_XFER);
    H5Pset_dxpl_mpio(plist_id, H5FD_MPIO_INDEPENDENT);
    status = H5Dread(dset, mem_dtype, dspace_mem, dspace_file,
      plist_id, data);
    H5Pclose(plist_id);

    assert(status <= 0);
    // Clean up 
    status = H5Dclose(dset);
    assert(status <= 0);
    status = H5Sclose(dspace_mem);
    assert(status <= 0);
    status = H5Sclose(dspace_file);
    assert(status <= 0);
  }

  /*! \brief Read node information from hum using a list 
   ** (in parallel mode)
   */
  template < typename T >
  void ReadVectorData
  (
    hid_t &file, T *data, hid_t &mem_dtype,
    listString &link, hsize_t listSize, hsize_t *list
    ) {
    // Variables used in local function
    herr_t status;
    hid_t dspace_mem, dspace_file, dset;
    std::string cat = h5pp::ListStringToString(link);
    // Dataspace to describe Layout in Memory
    dspace_mem = H5Screate_simple(1, &listSize, NULL);
    // Dataspace to describe Layout in File
    assert(IsDataset(file, cat.c_str()) == true);
    dset = H5Dopen2(file, cat.c_str(), H5P_DEFAULT);
    dspace_file = H5Dget_space(dset);
    status = H5Sselect_elements(dspace_file, H5S_SELECT_SET, listSize, list);
    assert(H5Sselect_valid(dspace_file) != 0);
    // Read data from file to memory
    hid_t plist_id = H5Pcreate(H5P_DATASET_XFER);
    H5Pset_dxpl_mpio(plist_id, H5FD_MPIO_INDEPENDENT);
    status = H5Dread(dset, mem_dtype, dspace_mem, dspace_file,
      plist_id, data);
    H5Pclose(plist_id);
    assert(status <= 0);
    // Clean up
    status = H5Dclose(dset);
    assert(status <= 0);
    status = H5Sclose(dspace_mem);
    assert(status <= 0);
    status = H5Sclose(dspace_file);
    assert(status <= 0);
  }

  /*! \brief Read node information from hum using a list 
   **        Assumes data has enough space of size listSize
   **/
  template < typename T >
  void ReadVectorDataSerial
  (
    hid_t file, T *data, hid_t mem_dtype, h5pp::listString &link,
    hsize_t offset, hsize_t stride, hsize_t size
    ) {
    // Variables used in local function
    herr_t status;
    hid_t dspace_mem, dspace_file, dset;
    std::string cat = h5pp::ListStringToString(link);
    // Dataspace to describe Layout in Memory
    dspace_mem = H5Screate_simple(1, &size, NULL);
    // Dataspace to describe Layout in File
    assert(h5pp::IsDataset(file, cat.c_str()) == true);
    dset = H5Dopen2(file, cat.c_str(), H5P_DEFAULT);
    dspace_file = H5Dget_space(dset);
    status = H5Sselect_hyperslab(dspace_file, H5S_SELECT_SET, &offset,
      &stride, &size, NULL);
    assert(H5Sselect_valid(dspace_file) != 0);
    // Read data from file to memory 
    status = H5Dread(dset, mem_dtype, dspace_mem, dspace_file,
      H5P_DEFAULT, data);
    assert(status <= 0);
    // Clean up 
    status = H5Dclose(dset);
    assert(status <= 0);
    status = H5Sclose(dspace_mem);
    assert(status <= 0);
    status = H5Sclose(dspace_file);
    assert(status <= 0);
  }

  /*! \brief Read node information from hum using a list */
  template < typename T >
  void ReadVectorDataSerial
  (
    hid_t &file, T *data, hid_t &mem_dtype,
    listString &link, hsize_t listSize, hsize_t *list
    ) {
    // Variables used in local function
    herr_t status;
    hid_t dspace_mem, dspace_file, dset;
    std::string cat = h5pp::ListStringToString(link);
    // Dataspace to describe Layout in Memory
    dspace_mem = H5Screate_simple(1, &listSize, NULL);
    // Dataspace to describe Layout in File
    assert(IsDataset(file, cat.c_str()) == true);
    dset = H5Dopen2(file, cat.c_str(), H5P_DEFAULT);
    dspace_file = H5Dget_space(dset);
    status = H5Sselect_elements(dspace_file, H5S_SELECT_SET, listSize, list);
    assert(H5Sselect_valid(dspace_file) != 0);
    // Read data from file to memory
    status = H5Dread(dset, mem_dtype, dspace_mem, dspace_file,
      H5P_DEFAULT, data);
    assert(status <= 0);
    // Clean up 
    status = H5Dclose(dset);
    assert(status <= 0);
    status = H5Sclose(dspace_mem);
    assert(status <= 0);
    status = H5Sclose(dspace_file);
    assert(status <= 0);
  }

  /*! \brief Writes a vector dataset in serial mode
   **
   **/
  template < typename T >
  void WriteVectorDataSerial
  (
    hid_t &file, T *data, hid_t &mem_dtype, hid_t &file_dtype,
    h5pp::listString &link, hsize_t offset,
    hsize_t stride, hsize_t mem_size, hsize_t file_size
    ) {
    /// Write the chunk to the file
    herr_t status;
    hid_t dspace_mem, dspace_file, dset;
    std::string cat = h5pp::ListStringToString(link);
    CreateGroupForDset(file, link);
    //std::cerr << "Creating dataset " << cat << "\n";
    /// Memory dataspace
    dspace_mem = H5Screate_simple(1, &mem_size, NULL);
    /// Dataset creation/opening
    if (IsValidLink(file, link) == false) {
      /// File dataspace 
      dspace_file = H5Screate_simple(1, &file_size, NULL);
      dset = H5Dcreate(file, cat.c_str(), file_dtype,
        dspace_file, H5P_DEFAULT,
        H5P_DEFAULT, H5P_DEFAULT);
    } else {
      dset = H5Dopen2(file, cat.c_str(), H5P_DEFAULT);
      dspace_file = H5Dget_space(dset);
    }
    status = H5Sselect_hyperslab(dspace_file, H5S_SELECT_SET, &offset,
      &stride, &mem_size, NULL);
    assert(H5Sselect_valid(dspace_file) != 0);
    status = H5Dwrite(dset, mem_dtype, dspace_mem, dspace_file,
      H5P_DEFAULT, data);
    H5Dclose(dset);
    H5Sclose(dspace_mem);
    H5Sclose(dspace_file);
  }

  /*! \brief Writes a vector dataset in parallel mode
   **
   **/
  template < typename T >
  void WriteVectorData
  (
    hid_t &file, T *data, hid_t &mem_dtype, hid_t &file_dtype,
    h5pp::listString &link, hsize_t offset,
    hsize_t stride, hsize_t mem_size, hsize_t file_size
    ) {
    /// Write the chunk to the file
    herr_t status;
    hid_t dspace_mem, dspace_file, dset;
    std::string cat = h5pp::ListStringToString(link);
    CreateGroupForDset(file, link);
    //std::cerr << "Creating dataset " << cat << "\n";
    /// Memory dataspace
    dspace_mem = H5Screate_simple(1, &mem_size, NULL);
    /// Dataset creation/opening
    if (IsValidLink(file, link) == false) {
      /// File dataspace 
      dspace_file = H5Screate_simple(1, &file_size, NULL);
      dset = H5Dcreate(file, cat.c_str(), file_dtype,
        dspace_file, H5P_DEFAULT,
        H5P_DEFAULT, H5P_DEFAULT);
    } else {
      dset = H5Dopen2(file, cat.c_str(), H5P_DEFAULT);
      dspace_file = H5Dget_space(dset);
    }
    status = H5Sselect_hyperslab(dspace_file, H5S_SELECT_SET, &offset,
      &stride, &mem_size, NULL);
    assert(H5Sselect_valid(dspace_file) != 0);

    hid_t plist_id = H5Pcreate(H5P_DATASET_XFER);
    H5Pset_dxpl_mpio(plist_id, H5FD_MPIO_INDEPENDENT);
    status = H5Dwrite(dset, mem_dtype, dspace_mem, dspace_file,
      plist_id, data);
    H5Pclose(plist_id);

    H5Dclose(dset);
    H5Sclose(dspace_mem);
    H5Sclose(dspace_file);
  }

  /*! \brief Writes data-set mpi independnet mode
   **
   **/
  template<class T>
  void WriteDset(hid_t &file, hid_t &dset, hid_t &dspace_mem,
    hid_t &dtype, hid_t &dspace_file, T *vector) {
    herr_t status;
    hid_t plist_id = H5Pcreate(H5P_DATASET_XFER);
    H5Pset_dxpl_mpio(plist_id, H5FD_MPIO_INDEPENDENT);
    status = H5Dwrite(dset, dtype, dspace_mem, dspace_file,
      plist_id, vector);
    assert(status <= 0);
    H5Pclose(plist_id);
  }

  /*! \brief Reads data-set 
   **
   **/
  template<class T>
  void ReadDset(hid_t &file, hid_t &dset, hid_t &dspace_mem,
    hid_t &dtype, hid_t &dspace_file, T *vector) {
    herr_t status;
    hid_t plist_id = H5Pcreate(H5P_DATASET_XFER);
    H5Pset_dxpl_mpio(plist_id, H5FD_MPIO_INDEPENDENT);
    status = H5Dread(dset, dtype, dspace_mem, dspace_file,
      plist_id, vector);
    assert(status <= 0);
    H5Pclose(plist_id);
  }
  /*! \brief Returns the HDF5 type of the 
   **        template type T
   **/
  template <class T> hid_t GetHDF5Type();

  /*! \brief Template specialization of GetHDF5Type()
   **        Native integer
   **/
  template<> hid_t GetHDF5Type<int>() {
    return H5T_NATIVE_INT;
  }

  /*! \brief Template specialization of GetHDF5Type()
   **        Native long integer
   **/
  template<> hid_t GetHDF5Type<long int>() {
    return H5T_NATIVE_LONG;
  }

  /*! \brief Template specialization of GetHDF5Type()
   **        Native long long integer
   **/
  template<> hid_t GetHDF5Type<long long>() {
    return H5T_NATIVE_LLONG;
  }

  /*! \brief Template specialization of GetHDF5Type()
   **        Native unsigned integer
   **/
  template<> hid_t GetHDF5Type<unsigned int>() {
    return H5T_NATIVE_UINT;
  }

  /*! \brief Template specialization of GetHDF5Type()
   **        Native unsigned long integer
   **/
  template<> hid_t GetHDF5Type<unsigned long>() {
    return H5T_NATIVE_ULONG;
  }

  /*! \brief Template specialization of GetHDF5Type()
   **        Native unsigned long long integer
   **/
  template<> hid_t GetHDF5Type<unsigned long long>() {
    return H5T_NATIVE_ULLONG;
  }

  /*! \brief Template specialization of GetHDF5Type()
   **        Native float
   **/
  template<> hid_t GetHDF5Type<float>() {
    return H5T_NATIVE_FLOAT;
  }

  /*! \brief Template specialization of GetHDF5Type()
   **        Native double
   **/
  template<> hid_t GetHDF5Type<double>() {
    return H5T_NATIVE_DOUBLE;
  }

  /*! \brief Template specialization of GetHDF5Type()
   **        Native long double
   **/
  template<> hid_t GetHDF5Type<long double>() {
    return H5T_NATIVE_LDOUBLE;
  }
  /**********************************************************/
  /**********  GetHDF5TypeClass speciailization *************/
  /**********************************************************/

  /*! \brief Template specialization of GetHDF5TypeClass()
   **        Native integer
   **/
  template<> H5T_class_t GetHDF5TypeClass<int>() {
    return H5T_INTEGER;
  }

  /*! \brief Template specialization of GetHDF5TypeClass()
   **        Native long integer
   **/
  template<> H5T_class_t GetHDF5TypeClass<long>() {
    return H5T_INTEGER;
  }

  /*! \brief Template specialization of GetHDF5TypeClass()
   **        Native long long integer
   **/
  template<> H5T_class_t GetHDF5TypeClass<long long>() {
    return H5T_INTEGER;
  }

  /*! \brief Template specialization of GetHDF5TypeClass()
   **        Native unsiged integer
   **/
  template<> H5T_class_t GetHDF5TypeClass<unsigned>() {
    return H5T_INTEGER;
  }

  /*! \brief Template specialization of GetHDF5TypeClass()
   **        Native unsiged long integer
   **/
  template<> H5T_class_t GetHDF5TypeClass<unsigned long>() {
    return H5T_INTEGER;
  }

  /*! \brief Template specialization of GetHDF5TypeClass()
   **        Native unsiged long long integer
   **/
  template<> H5T_class_t GetHDF5TypeClass<unsigned long long>() {
    return H5T_INTEGER;
  }

  /*! \brief Template specialization of GetHDF5TypeClass()
   **        Native single precision float
   **/
  template<> H5T_class_t GetHDF5TypeClass<float>() {
    return H5T_FLOAT;
  }

  /*! \brief Template specialization of GetHDF5TypeClass()
   **        Native double precision float
   **/
  template<> H5T_class_t GetHDF5TypeClass<double>() {
    return H5T_FLOAT;
  }

  /*! \brief Template specialization of GetHDF5TypeClass()
   **        Native long double precision
   **/
  template<> H5T_class_t GetHDF5TypeClass<long double>() {
    return H5T_FLOAT;
  }

  /*! \brief Get the scalar attribute data type in return
   **  
   **/
  int GetAttributeType(hid_t &file, listString &link) {
    std::stringstream cat;
    hid_t attr, attr_type;
    int ret_value;
    switch (link.size()) {
        // No need to check for validity as it will exist
      case 1:
        cat << "/";
        break;
        // Default
      default:
        for (listString_it it = link.begin(); it != --link.end(); ++it)
          cat << "/" << *it;
        /// Check if the object to hold the link exists
        assert(IsValidLink(file, cat.str().c_str()));
        break;
    }
    /// Attribute does not exist
    assert(IsAttribute<int>(file, link));
    attr = H5Aopen_by_name
      (
      file, cat.str().c_str(),
      link.back().c_str(),
      H5P_DEFAULT, H5P_DEFAULT
      );
    attr_type = H5Aget_type(attr);
    /// Get the size
    ret_value = H5Tget_size(attr_type);
    H5Aclose(attr);
    H5Tclose(attr_type);
    return ret_value;
  }

}

#endif
