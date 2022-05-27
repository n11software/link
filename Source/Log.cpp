#include "../Includes/Log.hpp"
#include <fstream>

namespace Log {
  bool SaveLogs = false;
  bool DebugMode = false;
  bool VerboseMode = false;
  std::string LogFile = "";

  void Log(std::string level, std::string Message) {
    if (level == "Error") fprintf(stderr, "%s > %s\n", level.c_str(), Message.c_str());
    else {
      if (level == "Debug" && DebugMode) std::cout << "Debug > " << Message << std::endl;
      else if (level != "Debug") std::cout << level << " > " << Message << std::endl;
    }
    if (SaveLogs && (level != "Debug" && !DebugMode)) {
      std::ofstream Log(LogFile, std::ios::app);
      if (Log.is_open() && Log.good()) {
        Log << level << " > " << Message << std::endl;
        Log.close();
      }
    }
  }

  void Error(std::string Message) { Log("Error", Message); }
  void Info(std::string Message) { Log("Info", Message); }
  void Debug(std::string Message) { Log("Debug", Message); }

  void SetSaveLogs(bool SaveLogs) { Log::SaveLogs = SaveLogs; }
  void SetLogFile(std::string LogFile) { Log::LogFile = LogFile; }
  void SetDebugMode(bool DebugMode) { Log::DebugMode = DebugMode; }
  void SetVerboseMode(bool VerboseMode) { Log::VerboseMode = VerboseMode; }
}