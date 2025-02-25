// Copyright (C) 2025 The Regents of the University of California
//
// Created by David W. Shattuck, Ph.D.
//
// This file is part of MouseBSE.
//
// MouseBSE is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation, version 2.1.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
//

#ifndef HPTimer_H
#define HPTimer_H

#include <chrono>
#include <iostream>
#include <iomanip>
#include <sstream>

class HPTimer {
public:
#ifdef __APPLE__
  typedef std::chrono::steady_clock ClockType;
#else
  typedef std::chrono::system_clock ClockType;
#endif
  HPTimer() : t0(ClockType::now()), t1(ClockType::now()) {}
  void start() { t0=ClockType::now(); }
  void stop()  { t1=ClockType::now(); }
  size_t ns() const
  {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(t1-t0).count();
  }
  size_t us() const
  {
    return std::chrono::duration_cast<std::chrono::microseconds>(t1-t0).count();
  }
  size_t ms() const
  {
    return std::chrono::duration_cast<std::chrono::milliseconds>(t1-t0).count();
  }
  std::string elapsedMMSS() const
  {
    auto secs=std::chrono::duration_cast<std::chrono::seconds>(t1-t0).count();
    std::ostringstream ostr;
    ostr<<secs/60<<':'<<std::setfill('0')<<std::setw(2)<<secs%60;
    return ostr.str();
  }
  std::string elapsed() const
  {
    auto microsecs=us();
    static const int64_t us_sec=1000000;
    static const int64_t us_min=60*us_sec;
    auto mins=microsecs/us_min;
    microsecs-=mins*us_min;
    auto secs=microsecs/us_sec;
    microsecs-=secs*us_sec;
    auto ms=microsecs/1000;
    std::ostringstream ostr;
    ostr<<mins<<':'<<std::setfill('0')<<std::setw(2)<<secs<<'.'<<std::setfill('0')<<std::setw(3)<<ms;
    return ostr.str();
  }
  std::string elapsedSecs() const
  {
    auto m=ms();
    auto s=m/1000;
    m-=s*1000;
    std::ostringstream ostr;
    ostr<<std::setfill('0')<<s<<'.'<<std::setw(3)<<m;
    return ostr.str();
  }
private:
  std::chrono::time_point<ClockType> t0,t1;
};

typedef HPTimer Timer;

#endif
