/* 
 *  _______________________________________
 * |  _   _  _  _                          |
 * | | \ | |/ |/ |     Date: 06/26/2022    |
 * | |  \| |- |- |     Author: Levi Hicks  |
 * | |     || || |                         |
 * | | |\  || || |                         |
 * | |_| \_||_||_|     File: HTTPTools.hpp |
 * |                                       |
 * |                                       |
 * | Please do not remove this header.     |
 * |_______________________________________|
 */

#pragma once
#include <iostream>

std::string decodeHTTP(std::string &src);
std::string sanitize(std::string value);