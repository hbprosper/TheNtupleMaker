void testtreestream()
{
  TChain* chain = new TChain("Analysis");
  TChain* fchain= new TChain("GEN");
  chain->AddFriend(fchain);

  chain->Add("testtreestream.root");
  chain->Add("testtreestream.root");

  fchain->Add("testtreestream.root");
  fchain->Add("testtreestream.root");


  int nparticles=0;
  chain->SetBranchAddress("Particle_size", &nparticles);

  int njets=0;
  chain->SetBranchAddress("Jet_size", &njets);

  float jetpt[100];
  TBranch* b_jetpt;
  chain->SetBranchAddress("Jet.PT", jetpt, &b_jetpt);

  int nentries = chain->GetEntries();
  cout << endl;
  cout << "Number of entries: " << nentries << endl;

  for(int row=0; row < nentries; row++)
    {
      chain->LoadTree(row);
      chain->GetEntry(row);

      if ( row % 20 == 0  )
        {
          cout << "Event: " 
               << row << "\t"
               << "  nparticles: " << nparticles
               << "  njets: " << njets;
          if (njets > 0) cout << "  jetpt[0]: " 
                              << b_jetpt->GetLeaf("Jet.PT")->GetValue();
          cout << endl;
        }
    }
  exit(0);
}
