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

#ifndef ArgParser_H
#define ArgParser_H

#include <iostream>
#include <string>
#include <list>
#include <vol3ddatatypes.h>
#include <dspoint.h>
#include <iomanip>
#include <sstream>
#include <stdlib.h>
#include <vector>
#ifndef BUILDVERSION
#include "buildversiondef.h"
#endif
#include <commonerrors.h>
namespace SILT {
template <class T>
inline void convert(T &dst, const char *string) { dst=atof(string); }
template<>
inline void convert(std::string &dst, const char *string) { dst=string; }
template<>
inline void convert(sint8 &dst, const char *string) { dst=atoi(string); }
template<>
inline void convert(uint8 &dst, const char *string) { dst=atoi(string); }
template<>
inline void convert(sint16 &dst, const char *string) { dst=atoi(string); }
template<>
inline void convert(uint16 &dst, const char *string) { dst=atoi(string); }
template<>
inline void convert(sint32 &dst, const char *string) { dst=atoi(string); }
template<>
inline void convert(uint32 &dst, const char *string) { dst=atoi(string); }
template<>
inline void convert(float32 &dst, const char *string) { dst=(float32)atof(string); }
template<>
inline void convert(float64 &dst, const char *string) { dst=atof(string); }
template <class T> inline bool isValid(const std::vector<T> &v) { return !v.empty(); }
template <class T> inline bool isValid(const T &/*t*/) { return true; }
template<> inline bool isValid(const std::string &t) { return !t.empty(); }
};
template <class T> std::string scanTypeString(T&) { return ""; }
template<> inline std::string scanTypeString(sint16&) { return " %d"; }
template<> inline std::string scanTypeString(sint32&) { return " %d"; }
template<> inline std::string scanTypeString(uint16&) { return " %d"; }
template<> inline std::string scanTypeString(uint32&) { return " %d"; }
template<> inline std::string scanTypeString(float32&) { return " %f"; }
template<> inline std::string scanTypeString(float64&) { return " %f"; }
template<> inline std::string scanTypeString(std::string&) { return " %s"; }

class ArgParserBase {
public:
  typedef std::string TagType;

  class ParseObjectBase {
  public:
    ParseObjectBase(TagType tag, std::string argDescription, bool required, bool hidden) : tag(tag), description(argDescription), required(required), hidden(hidden)
    {
    }
    virtual ~ParseObjectBase() {}
    TagType tag;
    std::string description;
    bool required;
    bool hidden;
    virtual bool parse(int &count, int argc, char *argv[]) = 0;
    virtual std::string default_value() { return ""; }
    virtual bool valid() { return true; }
    virtual std::string arguments() = 0;
  };
  template <class T>
  class ParseObjectT : public ParseObjectBase {
  public:
    ParseObjectT(TagType tag, T &ref, std::string argDescription, bool required=false, bool hidden=false) : ParseObjectBase(tag,argDescription,required,hidden), ref(ref), defaultValue(ref) {}
    ParseObjectT(TagType tag, T &ref, std::string argdesc, std::string argDescription, bool required=false, bool hidden=false) : ParseObjectBase(tag,argDescription,required,hidden), ref(ref), defaultValue(ref), argdesc(argdesc) {}
    ~ParseObjectT() {}
    T &ref;
    const T defaultValue;
    std::string argdesc;
    std::string default_value() { std::ostringstream ostr; ostr<<defaultValue; return ostr.str(); }
    std::string arguments()
    {
      return argdesc.empty() ? "<value>" : argdesc;
    }
    virtual bool valid() { return SILT::isValid(ref); }
    virtual bool parse(int &count, int argc, char *argv[])
    {
      bool nextWord = ((count+1)<argc);
      if (!nextWord) { std::cerr<<"option "<<tag<<" requires an argument."<<std::endl; return false; }
      SILT::convert(ref,argv[++count]);
      return true;
    }
  };
  template <class T>
  class ParseObjectOptionT : public ParseObjectBase {
  public:
    ParseObjectOptionT(TagType tag, T &ref, const T& defaultOptionValue, std::string argdesc, std::string argDescription, bool required=false, bool hidden=false) : ref(ref), defaultOptionValue(defaultOptionValue), ParseObjectBase(tag,argDescription,required,hidden), defaultValue(ref), argdesc(argdesc) {}
    T &ref;
    const T defaultValue;
    const T defaultOptionValue;
    std::string argdesc;
    std::string default_value() { std::ostringstream ostr; ostr<<defaultValue<<" ("<<defaultOptionValue<<" if set)"; return ostr.str(); }
    std::string arguments()
    {
      return argdesc.empty() ? "<value>" : argdesc;
    }
    virtual bool valid() { return SILT::isValid(ref); }
    virtual bool parse(int &count, int argc, char *argv[])
    {
      bool nextWord = ((count+1)<argc);
      if ((!nextWord)||(argv[count+1][0]=='-')) { ref = defaultOptionValue; return true; }
      SILT::convert(ref,argv[++count]);
      return true;
    }
  };
  class ParseObjectFlag : public ParseObjectBase {
  public:
    ParseObjectFlag(TagType tag, bool &ref, std::string argDescription, bool hidden=false) : ParseObjectBase(tag,argDescription,false,hidden), ref(ref) {}
    bool &ref;
    std::string arguments() { return ""; }
    virtual bool parse(int &count, int argc, char * /*argv*/[])
    {
      ref = true;
      return true;
    }
  };
  template <class T>
  class ParseObjectEnum : public ParseObjectBase {
  public:
    ParseObjectEnum(TagType tag, T &ref, const T &value, std::string argDescription, bool hidden=false)
      : ParseObjectBase(tag,argDescription,false,hidden), ref(ref), value(value) {}
    T &ref;
    std::string arguments() { return ""; }
    const T value;
    virtual bool parse(int &count, int argc, char *argv[])
    {
      ref = value;
      return true;
    }
  };
  template <class T, class S>
  class ParseObjectArgState : public ParseObjectBase {
  public:
    ParseObjectArgState(TagType tag, T &stateVariable, const T &stateValue,
                        S &argVariable, std::string argDesc, std::string argDescription, bool hidden=false)
      : ParseObjectBase(tag,argDescription,false,hidden),
        stateVariableRef(stateVariable), stateValue(stateValue),
        argVariableRef(argVariable), argdesc(argDesc) {}
    T &stateVariableRef;
    const T stateValue;
    S &argVariableRef;
    std::string argdesc;
    std::string arguments()
    {
      return argdesc.empty() ? "<value>" : argdesc;
    }
    virtual bool parse(int &count, int argc, char *argv[])
    {
      bool nextWord = ((count+1)<argc);
      if (!nextWord) { std::cerr<<"option "<<tag<<" requires an argument."<<std::endl; return false; }
      SILT::convert<S>(argVariableRef,argv[++count]);
      stateVariableRef = stateValue;
      return true;
    }
  };
  template <class T>
  class ParseList : public ParseObjectBase {
  public:
    ParseList(TagType tag, std::list<T> &ref, std::string argDescription, bool required=false, bool hidden=false)
      : ParseObjectBase(tag,argDescription,required,hidden), ref(ref) {}
    std::list<T> &ref;
    std::string arguments() { return "<arguments>"; }
    virtual bool valid() { return SILT::isValid(ref); }
    virtual bool parse(int &count, int argc, char *argv[])
    {
      bool nextWord = ((count+1)<argc);
      if (!nextWord) { std::cerr<<"option "<<tag<<" requires an argument."<<std::endl; return false; }
      for (;;)
      {
        bool nextWord = ((count+1)<argc);
        if (!nextWord) break;
        if (argv[count+1][0]=='-') break;
        T value;
        SILT::convert(value,argv[++count]);
        ref.push_back(value);
      }
      return true;
    }
  };
  template <class T>
  class ParseVector : public ParseObjectBase {
  public:
    ParseVector(TagType tag, std::vector<T> &ref, std::string argDescription, bool required=false, bool hidden=false)
      : ParseObjectBase(tag,argDescription,required,hidden), ref(ref) {}
    std::vector<T> &ref;
    std::string arguments() { return "<arguments>"; }
    virtual bool valid() { return SILT::isValid(ref); }
    virtual bool parse(int &count, int argc, char *argv[])
    {
      bool nextWord = ((count+1)<argc);
      if (!nextWord || argv[count+1][0]=='-') { std::cerr<<"option "<<tag<<" requires at least one argument."<<std::endl; return false; }
      for (;;)
      {
        bool nextWord = ((count+1)<argc);
        if (!nextWord) break;
        if (argv[count+1][0]=='-') break;
        T value;
        SILT::convert(value,argv[++count]);
        ref.push_back(value);
      }
      return true;
    }
  };
  template <class T>
  class ParsePairVector : public ParseObjectBase {
  public:
    ParsePairVector(TagType tag, std::vector<T> &ref, std::string argDescription, bool required=false, bool hidden=false)
      : ParseObjectBase(tag,argDescription,required,hidden), ref(ref) {}
    std::vector<T> &ref;
    std::string arguments() { return "<arguments>"; }
    virtual bool valid() { return SILT::isValid(ref); }
    virtual bool parse(int &count, int argc, char *argv[])
    {
      bool next2Words = ((count+2)<argc);
      if (!next2Words) { std::cerr<<"option "<<tag<<" requires at least two arguments."<<std::endl; return false; }
      if (argv[count+1][0]=='-') { std::cerr<<"option "<<tag<<" requires at least two arguments."<<std::endl; return false; }
      for (;;)
      {
        bool next2Words = ((count+2)<argc);
        if (!next2Words) break;
        if (argv[count+1][0]=='-') break;
        T value;
        SILT::convert(value.first,argv[++count]);
        SILT::convert(value.second,argv[++count]);
        ref.push_back(value);
      }
      return true;
    }
  };
  std::list<ParseObjectBase *> arglist;
  ArgParserBase(std::string progname) : progname(progname), licenseFlag(false), usageDisplayWidth(30), showHidden(false), errcode(0)
  {
    version="25a";
    copyright="Copyright (C) 2025 The Regents of the University of California";
    license="This program is free software and is released under a GNU Lesser General Public"
            "License v2.1 only. For details, run this program with the argument --license.\n";
    fullLicense=
        "This program is free software; you can redistribute it and/or\n"
        "modify it under the terms of the GNU Lesser General Public License\n"
        "as published by the Free Software Foundation, version 2.1.\n"
        "\n"
        "This program is distributed in the hope that it will be useful,\n"
        "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
        "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the\n"
        "GNU Lesser General Public License for more details.\n"
        "\n"
        "You should have received a copy of the GNU Lesser General Public\n"
        "License along with this library; if not, write to the Free Software\n"
        "Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.\n";

    author= "David W. Shattuck, Ahmanson-Lovelace Brain Mapping Center\n"
            "Dept. of Neurology, David Geffen School of Medicine, UCLA.\n";
    bindFlag("-hidden",showHidden,"show hidden options",true);
    bindFlag("-license",licenseFlag,"show the license information",false);
#ifdef BUILDVERSION
    buildinfo=BUILDVERSION;
#endif
  }
  virtual ~ArgParserBase()
  {
    for (std::list<ParseObjectBase *>::iterator arg = arglist.begin(); arg != arglist.end(); arg++)
    {
      delete *arg;
    }
    arglist.clear();
  }
  template <class T>
  bool bind(TagType tag, T &variable, std::string argdesc, std::string argDescription,
            const bool required=false, const bool hidden=false)
  {
    arglist.push_back(new ParseObjectT<T>(tag,variable,argdesc,argDescription,required,hidden));
    return true;
  }
  template <class T>
  bool bindOption(TagType tag, T &variable, const T &defaultOptionValue, std::string argdesc, std::string argDescription,
                  const bool required=false, const bool hidden=false)
  {
    arglist.push_back(new ParseObjectOptionT<T>(tag,variable,defaultOptionValue,argdesc,argDescription,required,hidden));
    return true;
  }
  template <class T>
  bool bindList(TagType tag, std::list<T> &variable, std::string argDescription, const bool required=false, const bool hidden=false)
  {
    arglist.push_back(new ParseList<T>(tag,variable,argDescription,required));
    return true;
  }
  template <class T>
  bool bindVector(TagType tag, std::vector<T> &variable, std::string argDescription, const bool required=false, const bool hidden=false)
  {
    arglist.push_back(new ParseVector<T>(tag,variable,argDescription,required));
    return true;
  }
  template <class T>
  bool bindPairVector(TagType tag, std::vector<T> &variable, std::string argDescription, const bool required=false, const bool hidden=false)
  {
    arglist.push_back(new ParsePairVector<T>(tag,variable,argDescription,required));
    return true;
  }
  bool bindFlag(TagType tag, bool &variable, std::string argDescription, bool hidden=false)
  {
    arglist.push_back(new ParseObjectFlag(tag,variable,argDescription,hidden));
    return true;
  }
  template <class T>
  bool bindEnum(TagType tag, T &variable, const T &value, std::string argDescription, bool hidden=false)
  {
    arglist.push_back(new ParseObjectEnum<T>(tag,variable,value,argDescription,hidden));
    return true;
  }
  template <class T, class S>
  bool bindArgState(TagType tag, T &stateVariable, const T &stateValue, S &argVariable, std::string argDesc, std::string argDescription, bool hidden=false)
// some arguments will change a state variable and record a value
// argVariable has no default value since it is not relevant unless set
  {
    arglist.push_back(new ParseObjectArgState<T,S>(tag,stateVariable,stateValue,argVariable,argDesc,argDescription,hidden));
    return true;
  }
  std::string progname;
  std::string notes;
  std::string fullLicense;
  std::string author;
  std::string copyright;
  std::string license;
  std::string buildinfo;
  std::string version;
  std::string description;
  std::string example;
  bool licenseFlag;
  int usageDisplayWidth;
  bool showHidden;
  int errcode;
  virtual void defaults()
  {
    std::cout<<"\ndefaults:\n";
    for (std::list<ParseObjectBase *>::iterator it=arglist.begin(); it != arglist.end(); it++)
    {
      if (*it && !(*it)->hidden)
      {
        std::string def = (*it)->default_value();
        if (!def.empty())
          std::cout<<'-'<<(*it)->tag<<"\t"<<def<<'\n';
      }
    }
  }
  virtual int usage(const int n)
  {
    if (n<=0)
    {
      std::cout<<"\n"<<progname;
      if (!version.empty()) std::cout << " v." << version<<'\n';
      if (!copyright.empty()) std::cout<<copyright<<'\n';
      if (!author.empty()) std::cout<<"Created by "<<author<<"\n";
      if (!license.empty()) std::cout<<license<<"\n";
      if (!description.empty()) std::cout << description << '\n';
      else std::cout<<'\n';
      std::cout<<"Compiled at "<<__TIME__<<" on "<<__DATE__;
      if (!buildinfo.empty()) std::cout << " (#" << buildinfo<<')';
      std::cout<<"\n\n";
      std::cout<<"usage: "<<progname<<" [settings]\n\n"
                                      "required settings:\n";
      for (std::list<ParseObjectBase *>::iterator it=arglist.begin(); it != arglist.end(); it++)
      {
        if (*it && (*it)->required && !((*it)->hidden&&!showHidden))
        {
          std::cout<<std::left<<std::setw(usageDisplayWidth)
                  <<"-"+(*it)->tag + " " + (*it)->arguments()<<" "<<(*it)->description<<'\n';
        }
      }
      std::cout<<"\noptional settings:\n";
      for (std::list<ParseObjectBase *>::iterator it=arglist.begin(); it != arglist.end(); it++)
      {
        if (*it && !(*it)->required && !((*it)->hidden&&!showHidden))
        {
          std::cout<<std::left<<std::setw(usageDisplayWidth)
                  <<"-"+(*it)->tag + " " + (*it)->arguments()<<" "<<(*it)->description;
          std::string def = (*it)->default_value();
          if (!def.empty()) std::cout<<" [default: "<<def<<"]";
          std::cout<<'\n';
        }
      }
      if (!example.empty())
        std::cout<<"\nexample:\n"<<example<<'\n';
      if (!notes.empty())
        std::cout<<"\nnotes:\n"<<notes<<'\n';
      std::cout<<std::endl;
    }
    else
    {
      std::cout<<"For usage instructions, call "<<progname<<" with no arguments."<<std::endl;
    }
    return n;
  }
  virtual int showLicense()
  {
    std::cout<<"\n"<<progname;
    if (!version.empty()) std::cout << " v." << version;
		std::cout<<'\n';
    if (!copyright.empty()) std::cout<<copyright<<'\n';
    if (!author.empty()) std::cout<<"Created by "<<author<<"\n";
    if (!fullLicense.empty()) std::cout<<fullLicense<<std::endl;
    return 0;
  }
  virtual int usage()
  {
    if (!errcode && licenseFlag) return showLicense();
    return usage(errcode);
  }
  virtual bool validate()
  {
    bool code = true;
    errcode=0;
    for (std::list<ParseObjectBase *>::iterator it=arglist.begin(); it != arglist.end(); it++)
    {
      if (*it)
      {
        ParseObjectBase &obj(*(*it));
        if (obj.required)
        {
          if (!obj.valid())
          {
            errcode = 2; code = false;
            std::cout<<"Error: switch -"<<obj.tag<<" ("<<obj.description<<") is required."<<std::endl;
          }
        }
      }
    }
    return code;
  }
  virtual bool parseAndValidate(int argc, char *argv[])
  {
    if (!parse(argc,argv)) return false;
    if (!validate()) return false;
    return true;
  }
  virtual bool parse(int argc, char *argv[])
  {
    if (argc<2) { errcode=0; return false; }
    for (int i=1;i<argc;i++)
    {
      if (argv[i][0]=='-')
      {
        switch (argv[i][1])
        {
          case '+':
          default:
          {
            bool foundIt = false;
            for (std::list<ParseObjectBase *>::iterator it=arglist.begin(); it != arglist.end(); it++)
            {
              if (*it)
              {
                if (std::string(argv[i]+1)==(*it)->tag)
                {
                  foundIt = true;
                  if (!(*it)->parse(i,argc,argv))
                  {
                    errcode = 1;
                    return false;
                  }
                  break;
                }
              }
            }
            if (!foundIt)
            {
              std::cerr<<"Unrecognized option "<<argv[i]<<std::endl;
              { errcode=1; return false; }
            }
          }
            break;
        }
      }
      else
      {
        std::cerr<<"Unrecognized option "<<argv[i]<<std::endl;
        errcode=1;
        return false;
      }
    }
    errcode=0;
    if (licenseFlag) return false;
    return true;
  }
};


class ArgParser : public ArgParserBase {
public:
  ArgParser(std::string name) : ArgParserBase(name), debug(false), verbosity(0)
  {
    bind("i",ifname,"<file>","input file",true);
    bind("o",ofname,"<file>","output file",true);
    bindFlag("g",debug,"debug");
    bind("v",verbosity,"<level>","verbosity");
  }
  virtual bool validate()
  {
    bool code = true;
    if (ifname.empty())
    {
      std::cerr<<"Error: input file was not specified.\n"<<std::endl;
      code = false;
    }
    if (ofname.empty())
    {
      std::cerr<<"Error: output file was not specified.\n"<<std::endl;
      code = false;
    }
    errcode = (code) ? 0 : 1;
    return code && ArgParserBase::validate();
  }
  std::string ifname;
  std::string ofname;
  bool debug;
  int verbosity;
};

template <>
inline bool ArgParserBase::ParseObjectT<DSPoint>::parse(int &count, int argc, char *argv[])
{
  bool nextWord = ((count+1)<argc);
  if (!nextWord) { std::cerr<<"option -"<<tag<<" requires 3 arguments."<<std::endl; return false; }
  ref.x = (float)atof(argv[++count]);
  nextWord = ((count+1)<argc);
  if (!nextWord) { std::cerr<<"option -"<<tag<<" requires 3 arguments."<<std::endl; return false; }
  ref.y = (float)atof(argv[++count]);
  nextWord = ((count+1)<argc);
  if (!nextWord) { std::cerr<<"option -"<<tag<<" requires 3 arguments."<<std::endl; return false; }
  ref.z = (float)atof(argv[++count]);
  return true;
}

template <>
inline bool ArgParserBase::ParseObjectT<IPoint3D>::parse(int &count, int argc, char *argv[])
{
  bool nextWord = ((count+1)<argc);
  if (!nextWord) { std::cerr<<"option -"<<tag<<" requires 3 arguments."<<std::endl; return false; }
  ref.x = atoi(argv[++count]);
  nextWord = ((count+1)<argc);
  if (!nextWord) { std::cerr<<"option -"<<tag<<" requires 3 arguments."<<std::endl; return false; }
  ref.y = atoi(argv[++count]);
  nextWord = ((count+1)<argc);
  if (!nextWord) { std::cerr<<"option -"<<tag<<" requires 3 arguments."<<std::endl; return false; }
  ref.z = atoi(argv[++count]);
  return true;
}

#endif
