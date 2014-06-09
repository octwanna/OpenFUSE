#include "cobalt/cobalt.hpp"
#include <string>
#include <tclap/CmdLine.h>
#include<stdint.h>

typedef TCLAP::CmdLine CmdLineClass;
typedef TCLAP::ValueArg<std::string> StringArg;
typedef TCLAP::ValueArg<double> FloatArg;

int main(int nargs, char *args[]) {
  try {
    /// The command line object
    CmdLineClass cmd
      (
      "COBALT Mesh to hum converter - serial stream version",
      ' ', "0.1"
      );
    /// The Input COBALT file name
    StringArg cobalt_file_arg
      (
      "i", "input",
      "The cobalt mesh file name", true,
      "", "string"
      );
    cmd.add(cobalt_file_arg);
    /// The Output hum file name
    StringArg hum_file_arg
      (
      "o", "output",
      "The output mesh file name (hum)", true,
      "cobalt.hum", "string"
      );
    cmd.add(hum_file_arg);
    /// The Output hum file name
    FloatArg buf_size_arg
      (
      "s", "size",
      "The stream buffer size in GB", false,
      0.0005, "float"
      );
    cmd.add(buf_size_arg);
    /// Toggle 64 bit mode for large files
    TCLAP::SwitchArg is64_arg
      (
      "L", "large",
      "Enable 64-bit integers for large meshes",
      cmd, false
      );
    cmd.parse(nargs, args);
    std::string hum_file = hum_file_arg.getValue();
    std::string cobalt_file = cobalt_file_arg.getValue();
    bool is64 = is64_arg.getValue();
    double buf_size = buf_size_arg.getValue();
    if (is64) {
      COBALT<double, uint64_t> cobFile
        (
        cobalt_file.c_str(),
        hum_file.c_str(), buf_size
        );
      cobFile.Start();
    } else {
      COBALT<double, uint32_t> cobFile
        (
        cobalt_file.c_str(),
        hum_file.c_str(), buf_size
        );
      cobFile.Start();
    }
  }  catch (TCLAP::ArgException &e) {
    std::cerr << "error: " << e.error() << " for arg "
      << e.argId() << std::endl;
    return 1;
  }
  return 0;
}


