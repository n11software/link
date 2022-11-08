/* 
 *  _______________________________________
 * |  _   _  _  _                          |
 * | | \ | |/ |/ |     Date: 06/26/2022    |
 * | |  \| |- |- |     Author: Levi Hicks  |
 * | |     || || |                         |
 * | | |\  || || |                         |
 * | |_| \_||_||_|     File: HTTPTools.cpp |
 * |                                       |
 * |                                       |
 * | Please do not remove this header.     |
 * |_______________________________________|
 */

#ifndef decodeHTTP
#include <iostream>
#include <algorithm>
#include "../Includes/Link/HTTPTools.hpp"

/*
 * Decodes http headers
 *
 * @param request The request to decode
 * @return Decoded headers
 */
std::string decodeHTTP(std::string &src) {
  std::replace(src.begin(), src.end(), '+', ' ');
  std::string ret;
  char ch;
  int i, ii;
  for (i=0;i<src.length();i++) {
    if (int(src[i])=='%') {
      switch (src[i+1]) {
        case '0':
        case '1':
        case '2':
        case '3': 
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        case 'a':
        case 'b':
        case 'c':
        case 'd':
        case 'e':
        case 'f':
          break;
        default:
          ret += '%';
          continue;
      }
      sscanf(src.substr(i+1,2).c_str(), "%x", &ii);
      ch=static_cast<char>(ii);
      ret+=ch;
      i=i+2;
    } else ret+=src[i];
  }
  return (ret);
}

/*
 * Sanitizes a string for use in databases
 *
 * @param value The string to sanitize
 * @return Sanitized string
 */
std::string sanitize(std::string value) {
  std::string newValue = "";
  for (int i=0; i<value.length(); i++) {
    if (value[i] == '\'') newValue += "\\'";
    else if (value[i] == '\\') newValue+="\\\\";
    else newValue += value[i];
  }
  return newValue;
}

#endif