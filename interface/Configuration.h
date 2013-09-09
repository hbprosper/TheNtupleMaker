#ifndef CONFIGURATION_H
#define CONFIGURATION_H
//
// Package:    PhysicsTools/TheNtupleMaker
//             Configuration.h
//
//             A singleton object to make configuration info available
//             to whoever needs it
//
// Original Author:  Harrison B. Prosper
//         Created:  Wed Sep 01, 2010
//
// $Id: Configuration.h,v 1.5 2012/05/07 04:32:42 prosper Exp $

#include <string>
#include <sstream>

namespace edm {
class ParameterSet;
}
class HLTConfigProvider;

/// A singleton class to cache global objects.
class Configuration
{
public:
  ///
  static Configuration& instance()
  {
    static Configuration singleton;
    return singleton;
  }

  ///
  void set(const edm::ParameterSet& config)
  { 
    config_ = &config;
  }

  ///
  void set(HLTConfigProvider& hltconfig) 
  { 
    hltconfig_ = &hltconfig;
  }

  ///
  void set(HLTConfigProvider* hltconfig) 
  { 
    hltconfig_ = hltconfig;
  }

  ///
  void set(std::string& blockname,
           std::string& buffername,
           std::string& labelname,
           std::map<std::string, std::string>& parameters)
  { 
    blockname_  = blockname;
    buffername_ = buffername;
    labelname_  = labelname;
    parameters_ = parameters;
  }

  ///
  const edm::ParameterSet* getConfig() const { return config_; }

  ///
  const HLTConfigProvider* getHLTconfig() const { return hltconfig_; }

  ///
  std::string getBlockname() const { return blockname_; }

  ///
  std::string getBuffername() const { return buffername_; }

  ///
  std::string getLabelname() const { return labelname_; }

  ///
  std::map<std::string, std::string> 
  getParameters() const { return parameters_; }

private:
  Configuration() {}                               // prevent explicit creation
  ~Configuration() {}                  
  Configuration(const Configuration&);             // prevent copy
  Configuration& operator=(const Configuration&);  // prevent assignment
  
  const edm::ParameterSet* config_;
  const HLTConfigProvider* hltconfig_;

  std::string blockname_;
  std::string buffername_;
  std::string labelname_;
  std::map<std::string, std::string> parameters_;
};

#endif
