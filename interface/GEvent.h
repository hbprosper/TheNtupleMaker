#ifndef GEVENT_HPP
#define GEVENT_HPP
//-----------------------------------------------------------------------------
// $Revision: 1.1.1.1 $
//-----------------------------------------------------------------------------
#include <string>
#include <iostream>
#include <vector>

///
class GEvent
{
 public:

  enum SIZE {MAXSIZE=4000};

  ///
  enum PDGID
  {
    DOWN    = 1,
    UP      = 2,
    STRANGE = 3,
    CHARM   = 4,
    BOTTOM  = 5,
    TOP     = 6,

    BPRIME  = 7,
    TPRIME  = 8,

    ELECTRON=11,
    NUE     =12,
    MUON    =13,
    NUMU    =14,
    TAU     =15,
    NUTAU   =16,

    GLUON   =21,
    GAMMA   =22,
    PHOTON  =22,
    ZBOSON  =23,
    WBOSON  =24,

    HIGGS   =25,
    H10     =25,

    HPLUS   =37,
    GRAVITON=39,

    SUSY     =1000000,
    GLUINO   =1000021,
    CHI10    =1000022,
    CHI20    =1000023,
    CHI30    =1000025,
    CHI40    =1000035,
    CHI1P    =1000024,
    CHI2P    =1000037,
    GRAVITINO=1000039
  };

  ///
  GEvent();
  
  ///
  virtual ~GEvent();

  ///
  std::vector<int> daughters(int index, int status=3);

  ///
  int         find(int pdgid, int start=0);

  std::string name(int index);

  ///
  std::string tree(int  index=-1,
                   int  printLevel=0,
                   int  maxdepth=0,   // no restriction on depth
                   int  depth=0);

  ///
  std::string table(int maxcount=100);

  ///
  void        printTree(std::ostream& stream,
                        int  index,
                        int  printLevel=0,
                        int  maxdepth=0,     // no restriction on depth
                        int  depth=0);
  ///
  void        printTable(std::ostream& stream, int maxrows=100);

  int                 nhep;
            
  std::vector<int>    firstMother;
  std::vector<int>    lastMother;

  std::vector<int>    firstDaughter;   
  std::vector<int>    lastDaughter;  

  std::vector<int>    pdgId;           
  std::vector<int>    status;          
  std::vector<float>  pt;
  std::vector<float>  eta;
  std::vector<float>  phi;
  std::vector<float>  mass;
  std::vector<float>  charge;                  
};
#endif
