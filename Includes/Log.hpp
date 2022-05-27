#ifndef Log
#include <iostream>

namespace Log {
  void Error(std::string Message);
  void Info(std::string Message);
  void Debug(std::string Message);
  void SetSaveLogs(bool SaveLogs);
  void SetLogFile(std::string LogFile);
  void SetDebugMode(bool DebugMode);
  void SetVerboseMode(bool VerboseMode);
};

#endif