#ifndef ROOT_H
#define ROOT_H
// -*- C++ -*-
//
// Package:    PhysicsTools
// Class:      root
// 
/**\class kit kit.cc 
   PhysicsTools/Mntuple/src/kit.cc

 Description: A class of Root utilities. These functions are placed in a class 
              so that Reflex can handle overloading automatically. This is
	      just a collection of simple boilerplate code to lessen
	      clutter that I've written over the years.
 
 Implementation:
     As simple as possible
*/
//
// Original Author:  Harrison B. Prosper
//         Created:  Fri Apr 04 2008
// $Id: root.h,v 1.1.1.1 2011/05/04 13:04:28 prosper Exp $
//
//
//-----------------------------------------------------------------------------
#include <iostream>
#include <vector>
#include <string>
//-----------------------------------------------------------------------------
#include <TGClient.h>
#include <TGWindow.h>
#include <TGListBox.h>
#include <TGFileDialog.h>
//-----------------------------------------------------------------------------

struct root
{
  ///
  static
  Pixel_t Color(std::string name);

  ///
  static
  TGLBEntry* LBEntry(TGListBox* listbox, std::string str, int id, 
                     std::string font="helvetica-medium-r", int fontsize=14);
};


/// Simple wrapper around TGFileDialog.
class TFileDialog
{
public:

  TFileDialog();

  ///
  TFileDialog(const TGWindow* window, 
              const TGWindow* main,
              EFileDialogMode dlg_type=kFDOpen, 
              std::string inidir=".",
              std::string inifilename="");

  ///
  ~TFileDialog();

  ///
  std::string Filename();

  ///
  std::string IniDir();

 private:
  std::string _filename;
  std::string _inidir;
};


/// Simple wrapper around TRootHelpDialog.
class THelpDialog
{
public:
  THelpDialog();

  ///
  THelpDialog(const TGWindow* main, 
              std::string title, 
              std::string text, 
              UInt_t w=600, 
              UInt_t h=300);
 
  ///
  ~THelpDialog();
};

/*
class ToolBar : TGHorizontalFrame
{
 public:
  ///
  ToolBar();

  ToolBar(const TGWindow *p = 0, 
	  UInt_t w = 1, UInt_t h = 1,
	  UInt_t options = kChildFrame,
	  Pixel_t back = GetDefaultFrameBackground());


  TGPictureButton* Add(std::string picture,
		       std::string tooltiptext);

  ~ToolBar();
}
*/

#endif
