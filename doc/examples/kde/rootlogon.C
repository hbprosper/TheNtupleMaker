{
string path("");
path += "-I. ";
path += "-O3 ";
gSystem->SetIncludePath(path.c_str());

cout << "enabling AutoLibraryLoader...";
if ( gSystem.Load( "libFWCoreFWLite" ) != 0 )
{
  cout << "unable to load FWCoreFWLite" << endl;
  exit(0);
}

AutoLibraryLoader::enable();
cout << "done!" << endl;
}
