//-----------------------------------------------------------------------------
/** PhysicsTools/TheNtupleMaker/src/root.cc

 Description: A collection of simple Root utilities. Functions are placed in 
              a class so that Reflex can handle overloading automatically. 
 
 Implementation:
     As simple as possible
*/
// Created: Summer-2008 Harrison B. Prosper
//-----------------------------------------------------------------------------
//$Revision: 1.1.1.1 $

#include "PhysicsTools/TheNtupleMaker/interface/root.h"
#include <TRootHelpDialog.h>
#include <TGFont.h>
#include <TGListBox.h>
#include <TGResourcePool.h>
#include <algorithm>

using namespace std;

Pixel_t 
root::Color(std::string name)
{
  Pixel_t pixel;
  gClient->GetColorByName(name.c_str(), pixel);
  return pixel;
}

TGLBEntry*
root::LBEntry(TGListBox* listbox, 
              std::string str, int id, 
              std::string font, int fontsize)
{
  // Create font object

  char fontstr[256];
  sprintf(fontstr,"-adobe-%s-*-*-%d-*-*-*-*-*-iso8859-1", 
          font.c_str(), fontsize);

  const TGFont* ufont = gClient->GetFont(fontstr);
   if (!ufont)
     ufont = gClient->GetResourcePool()->GetDefaultFont();

   // Create graphics context object

   GCValues_t val;
   val.fMask = kGCFont;
   val.fFont = ufont->GetFontHandle();
   TGGC* uGC = gClient->GetGC(&val, kTRUE);

   TGTextLBEntry* entry = new TGTextLBEntry(listbox->GetContainer(), 
                                            new TGString(str.c_str()), 
                                            id, 
                                            uGC->GetGC(), 
                                            ufont->GetFontStruct());
   return (TGLBEntry*)entry;
}

TFileDialog::TFileDialog() {}

TFileDialog::TFileDialog(const TGWindow* window, 
                         const TGWindow* main,
                         EFileDialogMode dlg_type,
                         string IniDir,
                         string IniFilename)
  {
    TGFileInfo file_info;

    char* inidir = new char[IniDir.size()+1];
    copy(IniDir.begin(), IniDir.end(), inidir);
    inidir[IniDir.size()]=0;
    file_info.fIniDir = inidir;

    if ( IniFilename != "" )
      {
        char* inifilename = new char[IniFilename.size()+1];
        copy(IniFilename.begin(), IniFilename.end(), inifilename);
        inifilename[IniFilename.size()]=0;
        file_info.fFilename = inifilename;
      }

    vector<string> ftypes;
    if ( dlg_type == kFDOpen )
      {
        ftypes.push_back("Root files");
        ftypes.push_back("*.root");
        ftypes.push_back("All files");
        ftypes.push_back("*");
      }
    else
      {
        ftypes.push_back("All files");
        ftypes.push_back("*");
        ftypes.push_back("Root files");
        ftypes.push_back("*.root");
      }
    const char* filetypes[] = {ftypes[0].c_str(), ftypes[1].c_str(),
                               ftypes[2].c_str(), ftypes[3].c_str(),
                               0,            0};
    
    file_info.fFileTypes = filetypes;

    new TGFileDialog(window, main, dlg_type, &file_info);
    _filename = string(file_info.fFilename);
    _inidir   = string(file_info.fIniDir);
  }

TFileDialog::~TFileDialog(){}

string
TFileDialog::Filename() {return _filename;}

string
TFileDialog::IniDir()   {return _inidir;}

THelpDialog::THelpDialog() {}

THelpDialog::THelpDialog(const TGWindow* main, 
                         string title, 
                         string text, 
                         UInt_t w, 
                         UInt_t h)
{ 
  TRootHelpDialog* d = new TRootHelpDialog(main, 
                                           title.c_str(), 
                                           w, h); 
  d->SetText(text.c_str());
  d->Popup();
}
 
THelpDialog::~THelpDialog() {}

