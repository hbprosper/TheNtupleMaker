#ifndef ADDVARS_H
#define ADDVARS_H
//-----------------------------------------------------------------------------
// File:        addvars.h
// Description: user macro called by TheNtupleMaker
// Created:     Tue May  8 03:47:22 2012 by mkmacro.py
// Author:      Harry Prosper
//-----------------------------------------------------------------------------
#include <map>
#include <string>
#include <vector>
#include <cmath>
#include <iostream>
#include <algorithm>
#include <cassert>
#include <stdlib.h>

#include "TROOT.h"
#include "TTree.h"
//-----------------------------------------------------------------------------
struct countvalue
{
  int*    count;
  double* value;
};
typedef std::map<std::string, countvalue> VarMap;
typedef std::map<std::string, std::vector<int> > IndexMap;

// -----------------------------------------------------------------------
// This struct is defined by the user in addvars.cc
class addvarsInternal;

class addvars
{
public:
  addvars(TTree* tree_, VarMap* varmap_, IndexMap* indexmap_)
    : tree(tree_), varmap(varmap_), indexmap(indexmap_)
    , jet_chargedEmEnergyFraction	(std::vector<float>(200,0))
    , jet_chargedHadronEnergyFraction	(std::vector<float>(200,0))
    , jet_chargedMultiplicity	(std::vector<int>(200,0))
    , jet_combinedSecondaryVertexBJetTags	(std::vector<float>(200,0))
    , jet_combinedSecondaryVertexMVABJetTags	(std::vector<float>(200,0))
    , jet_energy	(std::vector<double>(200,0))
    , jet_et	(std::vector<double>(200,0))
    , jet_eta	(std::vector<double>(200,0))
    , jet_genJet_energy	(std::vector<double>(200,0))
    , jet_genJet_eta	(std::vector<double>(200,0))
    , jet_genJet_phi	(std::vector<double>(200,0))
    , jet_genJet_pt	(std::vector<double>(200,0))
    , jet_genParton_pdgId	(std::vector<int>(200,0))
    , jet_jetArea	(std::vector<float>(200,0))
    , jet_jetBProbabilityBJetTags	(std::vector<float>(200,0))
    , jet_jetProbabilityBJetTags	(std::vector<float>(200,0))
    , jet_mass	(std::vector<double>(200,0))
    , jet_muonEnergyFraction	(std::vector<float>(200,0))
    , jet_nConstituents	(std::vector<int>(200,0))
    , jet_neutralEmEnergyFraction	(std::vector<float>(200,0))
    , jet_neutralHadronEnergyFraction	(std::vector<float>(200,0))
    , jet_partonFlavour	(std::vector<int>(200,0))
    , jet_phi	(std::vector<double>(200,0))
    , jet_photonEnergyFraction	(std::vector<float>(200,0))
    , jet_pt	(std::vector<double>(200,0))
    , jet_simpleSecondaryVertexHighEffBJetTags	(std::vector<float>(200,0))
    , jet_simpleSecondaryVertexHighPurBJetTags	(std::vector<float>(200,0))
    , jet_trackCountingHighEffBJetTags	(std::vector<float>(200,0))
    , jet_trackCountingHighPurBJetTags	(std::vector<float>(200,0))
    , jet_uncor_energy	(std::vector<double>(200,0))
    , jet_uncor_et	(std::vector<double>(200,0))
    , jet_uncor_pt	(std::vector<double>(200,0))
    , jethelper_chargedEmEnergyFraction	(std::vector<float>(200,0))
    , jethelper_chargedHadronEnergyFraction	(std::vector<float>(200,0))
    , jethelper_chargedMultiplicity	(std::vector<int>(200,0))
    , jethelper_combinedSecondaryVertexBJetTags	(std::vector<float>(200,0))
    , jethelper_combinedSecondaryVertexMVABJetTags	(std::vector<float>(200,0))
    , jethelper_energy	(std::vector<double>(200,0))
    , jethelper_et	(std::vector<double>(200,0))
    , jethelper_eta	(std::vector<double>(200,0))
    , jethelper_genJet_energy	(std::vector<double>(200,0))
    , jethelper_genJet_eta	(std::vector<double>(200,0))
    , jethelper_genJet_phi	(std::vector<double>(200,0))
    , jethelper_genJet_pt	(std::vector<double>(200,0))
    , jethelper_genParton_pdgId	(std::vector<int>(200,0))
    , jethelper_jetArea	(std::vector<float>(200,0))
    , jethelper_jetBProbabilityBJetTags	(std::vector<float>(200,0))
    , jethelper_jetProbabilityBJetTags	(std::vector<float>(200,0))
    , jethelper_mass	(std::vector<double>(200,0))
    , jethelper_muonEnergyFraction	(std::vector<float>(200,0))
    , jethelper_nConstituents	(std::vector<int>(200,0))
    , jethelper_neutralEmEnergyFraction	(std::vector<float>(200,0))
    , jethelper_neutralHadronEnergyFraction	(std::vector<float>(200,0))
    , jethelper_partonFlavour	(std::vector<int>(200,0))
    , jethelper_phi	(std::vector<double>(200,0))
    , jethelper_photonEnergyFraction	(std::vector<float>(200,0))
    , jethelper_pt	(std::vector<double>(200,0))
    , jethelper_simpleSecondaryVertexHighEffBJetTags	(std::vector<float>(200,0))
    , jethelper_simpleSecondaryVertexHighPurBJetTags	(std::vector<float>(200,0))
    , jethelper_trackCountingHighEffBJetTags	(std::vector<float>(200,0))
    , jethelper_trackCountingHighPurBJetTags	(std::vector<float>(200,0))
    , jethelper_uncor_energy	(std::vector<double>(200,0))
    , jethelper_uncor_et	(std::vector<double>(200,0))
    , jethelper_uncor_pt	(std::vector<double>(200,0))

	{}

  ~addvars() {}

  void beginJob();
  void initialize() { initialize_(); }
  void endJob();
  
  bool analyze();

  // call these functions to select the specified objects
  // example:
  //
  //   select("jet");    which is to be called once from beginJob()
  //
  // and
  //
  //   select("jet", i); which is to be called in analyze() for every object
  //
  // to be kept
  
  void select(std::string name)
  {
    (*indexmap)[name] = std::vector<int>();
  }
  
  void select(std::string name, int index)
  {
    if ( indexmap->find(name) == indexmap->end() ) return;
    (*indexmap)[name].push_back(index);
  }

private:
  TTree*    tree;
  VarMap*   varmap;
  IndexMap* indexmap;
  addvarsInternal* local;

  // ------------------------------------------------------------------------
  // --- Initialize variables
  // ------------------------------------------------------------------------
  std::vector<float>	jet_chargedEmEnergyFraction;
  std::vector<float>	jet_chargedHadronEnergyFraction;
  std::vector<int>	jet_chargedMultiplicity;
  std::vector<float>	jet_combinedSecondaryVertexBJetTags;
  std::vector<float>	jet_combinedSecondaryVertexMVABJetTags;
  std::vector<double>	jet_energy;
  std::vector<double>	jet_et;
  std::vector<double>	jet_eta;
  std::vector<double>	jet_genJet_energy;
  std::vector<double>	jet_genJet_eta;
  std::vector<double>	jet_genJet_phi;
  std::vector<double>	jet_genJet_pt;
  std::vector<int>	jet_genParton_pdgId;
  std::vector<float>	jet_jetArea;
  std::vector<float>	jet_jetBProbabilityBJetTags;
  std::vector<float>	jet_jetProbabilityBJetTags;
  std::vector<double>	jet_mass;
  std::vector<float>	jet_muonEnergyFraction;
  std::vector<int>	jet_nConstituents;
  std::vector<float>	jet_neutralEmEnergyFraction;
  std::vector<float>	jet_neutralHadronEnergyFraction;
  std::vector<int>	jet_partonFlavour;
  std::vector<double>	jet_phi;
  std::vector<float>	jet_photonEnergyFraction;
  std::vector<double>	jet_pt;
  std::vector<float>	jet_simpleSecondaryVertexHighEffBJetTags;
  std::vector<float>	jet_simpleSecondaryVertexHighPurBJetTags;
  std::vector<float>	jet_trackCountingHighEffBJetTags;
  std::vector<float>	jet_trackCountingHighPurBJetTags;
  std::vector<double>	jet_uncor_energy;
  std::vector<double>	jet_uncor_et;
  std::vector<double>	jet_uncor_pt;
  std::vector<float>	jethelper_chargedEmEnergyFraction;
  std::vector<float>	jethelper_chargedHadronEnergyFraction;
  std::vector<int>	jethelper_chargedMultiplicity;
  std::vector<float>	jethelper_combinedSecondaryVertexBJetTags;
  std::vector<float>	jethelper_combinedSecondaryVertexMVABJetTags;
  std::vector<double>	jethelper_energy;
  std::vector<double>	jethelper_et;
  std::vector<double>	jethelper_eta;
  std::vector<double>	jethelper_genJet_energy;
  std::vector<double>	jethelper_genJet_eta;
  std::vector<double>	jethelper_genJet_phi;
  std::vector<double>	jethelper_genJet_pt;
  std::vector<int>	jethelper_genParton_pdgId;
  std::vector<float>	jethelper_jetArea;
  std::vector<float>	jethelper_jetBProbabilityBJetTags;
  std::vector<float>	jethelper_jetProbabilityBJetTags;
  std::vector<double>	jethelper_mass;
  std::vector<float>	jethelper_muonEnergyFraction;
  std::vector<int>	jethelper_nConstituents;
  std::vector<float>	jethelper_neutralEmEnergyFraction;
  std::vector<float>	jethelper_neutralHadronEnergyFraction;
  std::vector<int>	jethelper_partonFlavour;
  std::vector<double>	jethelper_phi;
  std::vector<float>	jethelper_photonEnergyFraction;
  std::vector<double>	jethelper_pt;
  std::vector<float>	jethelper_simpleSecondaryVertexHighEffBJetTags;
  std::vector<float>	jethelper_simpleSecondaryVertexHighPurBJetTags;
  std::vector<float>	jethelper_trackCountingHighEffBJetTags;
  std::vector<float>	jethelper_trackCountingHighPurBJetTags;
  std::vector<double>	jethelper_uncor_energy;
  std::vector<double>	jethelper_uncor_et;
  std::vector<double>	jethelper_uncor_pt;
  unsigned int	triggerresultshelper2_HLT_PFJet320_v5;
  int	triggerresultshelper_HLT_PFJet320_v5;


  void initialize_()
  {
    // clear object selection map every event
	
    for(IndexMap::iterator item=indexmap->begin(); 
        item != indexmap->end();
        ++item)
      item->second.clear();	
	
    countvalue& v0 = (*varmap)["patJet_selectedPatJetsAK5.chargedEmEnergyFraction"];
    if ( v0.value )
      {
        jet_chargedEmEnergyFraction.resize(*v0.count);
        copy(v0.value, v0.value+*v0.count, jet_chargedEmEnergyFraction.begin());
      }
    else
      jet_chargedEmEnergyFraction.clear();

    countvalue& v1 = (*varmap)["patJet_selectedPatJetsAK5.chargedHadronEnergyFraction"];
    if ( v1.value )
      {
        jet_chargedHadronEnergyFraction.resize(*v1.count);
        copy(v1.value, v1.value+*v1.count, jet_chargedHadronEnergyFraction.begin());
      }
    else
      jet_chargedHadronEnergyFraction.clear();

    countvalue& v2 = (*varmap)["patJet_selectedPatJetsAK5.chargedMultiplicity"];
    if ( v2.value )
      {
        jet_chargedMultiplicity.resize(*v2.count);
        copy(v2.value, v2.value+*v2.count, jet_chargedMultiplicity.begin());
      }
    else
      jet_chargedMultiplicity.clear();

    countvalue& v3 = (*varmap)["patJet_selectedPatJetsAK5.combinedSecondaryVertexBJetTags"];
    if ( v3.value )
      {
        jet_combinedSecondaryVertexBJetTags.resize(*v3.count);
        copy(v3.value, v3.value+*v3.count, jet_combinedSecondaryVertexBJetTags.begin());
      }
    else
      jet_combinedSecondaryVertexBJetTags.clear();

    countvalue& v4 = (*varmap)["patJet_selectedPatJetsAK5.combinedSecondaryVertexMVABJetTags"];
    if ( v4.value )
      {
        jet_combinedSecondaryVertexMVABJetTags.resize(*v4.count);
        copy(v4.value, v4.value+*v4.count, jet_combinedSecondaryVertexMVABJetTags.begin());
      }
    else
      jet_combinedSecondaryVertexMVABJetTags.clear();

    countvalue& v5 = (*varmap)["patJet_selectedPatJetsAK5.energy"];
    if ( v5.value )
      {
        jet_energy.resize(*v5.count);
        copy(v5.value, v5.value+*v5.count, jet_energy.begin());
      }
    else
      jet_energy.clear();

    countvalue& v6 = (*varmap)["patJet_selectedPatJetsAK5.et"];
    if ( v6.value )
      {
        jet_et.resize(*v6.count);
        copy(v6.value, v6.value+*v6.count, jet_et.begin());
      }
    else
      jet_et.clear();

    countvalue& v7 = (*varmap)["patJet_selectedPatJetsAK5.eta"];
    if ( v7.value )
      {
        jet_eta.resize(*v7.count);
        copy(v7.value, v7.value+*v7.count, jet_eta.begin());
      }
    else
      jet_eta.clear();

    countvalue& v8 = (*varmap)["patJet_selectedPatJetsAK5.genJet_energy"];
    if ( v8.value )
      {
        jet_genJet_energy.resize(*v8.count);
        copy(v8.value, v8.value+*v8.count, jet_genJet_energy.begin());
      }
    else
      jet_genJet_energy.clear();

    countvalue& v9 = (*varmap)["patJet_selectedPatJetsAK5.genJet_eta"];
    if ( v9.value )
      {
        jet_genJet_eta.resize(*v9.count);
        copy(v9.value, v9.value+*v9.count, jet_genJet_eta.begin());
      }
    else
      jet_genJet_eta.clear();

    countvalue& v10 = (*varmap)["patJet_selectedPatJetsAK5.genJet_phi"];
    if ( v10.value )
      {
        jet_genJet_phi.resize(*v10.count);
        copy(v10.value, v10.value+*v10.count, jet_genJet_phi.begin());
      }
    else
      jet_genJet_phi.clear();

    countvalue& v11 = (*varmap)["patJet_selectedPatJetsAK5.genJet_pt"];
    if ( v11.value )
      {
        jet_genJet_pt.resize(*v11.count);
        copy(v11.value, v11.value+*v11.count, jet_genJet_pt.begin());
      }
    else
      jet_genJet_pt.clear();

    countvalue& v12 = (*varmap)["patJet_selectedPatJetsAK5.genParton_pdgId"];
    if ( v12.value )
      {
        jet_genParton_pdgId.resize(*v12.count);
        copy(v12.value, v12.value+*v12.count, jet_genParton_pdgId.begin());
      }
    else
      jet_genParton_pdgId.clear();

    countvalue& v13 = (*varmap)["patJet_selectedPatJetsAK5.jetArea"];
    if ( v13.value )
      {
        jet_jetArea.resize(*v13.count);
        copy(v13.value, v13.value+*v13.count, jet_jetArea.begin());
      }
    else
      jet_jetArea.clear();

    countvalue& v14 = (*varmap)["patJet_selectedPatJetsAK5.jetBProbabilityBJetTags"];
    if ( v14.value )
      {
        jet_jetBProbabilityBJetTags.resize(*v14.count);
        copy(v14.value, v14.value+*v14.count, jet_jetBProbabilityBJetTags.begin());
      }
    else
      jet_jetBProbabilityBJetTags.clear();

    countvalue& v15 = (*varmap)["patJet_selectedPatJetsAK5.jetProbabilityBJetTags"];
    if ( v15.value )
      {
        jet_jetProbabilityBJetTags.resize(*v15.count);
        copy(v15.value, v15.value+*v15.count, jet_jetProbabilityBJetTags.begin());
      }
    else
      jet_jetProbabilityBJetTags.clear();

    countvalue& v16 = (*varmap)["patJet_selectedPatJetsAK5.mass"];
    if ( v16.value )
      {
        jet_mass.resize(*v16.count);
        copy(v16.value, v16.value+*v16.count, jet_mass.begin());
      }
    else
      jet_mass.clear();

    countvalue& v17 = (*varmap)["patJet_selectedPatJetsAK5.muonEnergyFraction"];
    if ( v17.value )
      {
        jet_muonEnergyFraction.resize(*v17.count);
        copy(v17.value, v17.value+*v17.count, jet_muonEnergyFraction.begin());
      }
    else
      jet_muonEnergyFraction.clear();

    countvalue& v18 = (*varmap)["patJet_selectedPatJetsAK5.nConstituents"];
    if ( v18.value )
      {
        jet_nConstituents.resize(*v18.count);
        copy(v18.value, v18.value+*v18.count, jet_nConstituents.begin());
      }
    else
      jet_nConstituents.clear();

    countvalue& v19 = (*varmap)["patJet_selectedPatJetsAK5.neutralEmEnergyFraction"];
    if ( v19.value )
      {
        jet_neutralEmEnergyFraction.resize(*v19.count);
        copy(v19.value, v19.value+*v19.count, jet_neutralEmEnergyFraction.begin());
      }
    else
      jet_neutralEmEnergyFraction.clear();

    countvalue& v20 = (*varmap)["patJet_selectedPatJetsAK5.neutralHadronEnergyFraction"];
    if ( v20.value )
      {
        jet_neutralHadronEnergyFraction.resize(*v20.count);
        copy(v20.value, v20.value+*v20.count, jet_neutralHadronEnergyFraction.begin());
      }
    else
      jet_neutralHadronEnergyFraction.clear();

    countvalue& v21 = (*varmap)["patJet_selectedPatJetsAK5.partonFlavour"];
    if ( v21.value )
      {
        jet_partonFlavour.resize(*v21.count);
        copy(v21.value, v21.value+*v21.count, jet_partonFlavour.begin());
      }
    else
      jet_partonFlavour.clear();

    countvalue& v22 = (*varmap)["patJet_selectedPatJetsAK5.phi"];
    if ( v22.value )
      {
        jet_phi.resize(*v22.count);
        copy(v22.value, v22.value+*v22.count, jet_phi.begin());
      }
    else
      jet_phi.clear();

    countvalue& v23 = (*varmap)["patJet_selectedPatJetsAK5.photonEnergyFraction"];
    if ( v23.value )
      {
        jet_photonEnergyFraction.resize(*v23.count);
        copy(v23.value, v23.value+*v23.count, jet_photonEnergyFraction.begin());
      }
    else
      jet_photonEnergyFraction.clear();

    countvalue& v24 = (*varmap)["patJet_selectedPatJetsAK5.pt"];
    if ( v24.value )
      {
        jet_pt.resize(*v24.count);
        copy(v24.value, v24.value+*v24.count, jet_pt.begin());
      }
    else
      jet_pt.clear();

    countvalue& v25 = (*varmap)["patJet_selectedPatJetsAK5.simpleSecondaryVertexHighEffBJetTags"];
    if ( v25.value )
      {
        jet_simpleSecondaryVertexHighEffBJetTags.resize(*v25.count);
        copy(v25.value, v25.value+*v25.count, jet_simpleSecondaryVertexHighEffBJetTags.begin());
      }
    else
      jet_simpleSecondaryVertexHighEffBJetTags.clear();

    countvalue& v26 = (*varmap)["patJet_selectedPatJetsAK5.simpleSecondaryVertexHighPurBJetTags"];
    if ( v26.value )
      {
        jet_simpleSecondaryVertexHighPurBJetTags.resize(*v26.count);
        copy(v26.value, v26.value+*v26.count, jet_simpleSecondaryVertexHighPurBJetTags.begin());
      }
    else
      jet_simpleSecondaryVertexHighPurBJetTags.clear();

    countvalue& v27 = (*varmap)["patJet_selectedPatJetsAK5.trackCountingHighEffBJetTags"];
    if ( v27.value )
      {
        jet_trackCountingHighEffBJetTags.resize(*v27.count);
        copy(v27.value, v27.value+*v27.count, jet_trackCountingHighEffBJetTags.begin());
      }
    else
      jet_trackCountingHighEffBJetTags.clear();

    countvalue& v28 = (*varmap)["patJet_selectedPatJetsAK5.trackCountingHighPurBJetTags"];
    if ( v28.value )
      {
        jet_trackCountingHighPurBJetTags.resize(*v28.count);
        copy(v28.value, v28.value+*v28.count, jet_trackCountingHighPurBJetTags.begin());
      }
    else
      jet_trackCountingHighPurBJetTags.clear();

    countvalue& v29 = (*varmap)["patJet_selectedPatJetsAK5.uncor_energy"];
    if ( v29.value )
      {
        jet_uncor_energy.resize(*v29.count);
        copy(v29.value, v29.value+*v29.count, jet_uncor_energy.begin());
      }
    else
      jet_uncor_energy.clear();

    countvalue& v30 = (*varmap)["patJet_selectedPatJetsAK5.uncor_et"];
    if ( v30.value )
      {
        jet_uncor_et.resize(*v30.count);
        copy(v30.value, v30.value+*v30.count, jet_uncor_et.begin());
      }
    else
      jet_uncor_et.clear();

    countvalue& v31 = (*varmap)["patJet_selectedPatJetsAK5.uncor_pt"];
    if ( v31.value )
      {
        jet_uncor_pt.resize(*v31.count);
        copy(v31.value, v31.value+*v31.count, jet_uncor_pt.begin());
      }
    else
      jet_uncor_pt.clear();

    countvalue& v32 = (*varmap)["patJetHelper_selectedPatJetsAK5.chargedEmEnergyFraction"];
    if ( v32.value )
      {
        jethelper_chargedEmEnergyFraction.resize(*v32.count);
        copy(v32.value, v32.value+*v32.count, jethelper_chargedEmEnergyFraction.begin());
      }
    else
      jethelper_chargedEmEnergyFraction.clear();

    countvalue& v33 = (*varmap)["patJetHelper_selectedPatJetsAK5.chargedHadronEnergyFraction"];
    if ( v33.value )
      {
        jethelper_chargedHadronEnergyFraction.resize(*v33.count);
        copy(v33.value, v33.value+*v33.count, jethelper_chargedHadronEnergyFraction.begin());
      }
    else
      jethelper_chargedHadronEnergyFraction.clear();

    countvalue& v34 = (*varmap)["patJetHelper_selectedPatJetsAK5.chargedMultiplicity"];
    if ( v34.value )
      {
        jethelper_chargedMultiplicity.resize(*v34.count);
        copy(v34.value, v34.value+*v34.count, jethelper_chargedMultiplicity.begin());
      }
    else
      jethelper_chargedMultiplicity.clear();

    countvalue& v35 = (*varmap)["patJetHelper_selectedPatJetsAK5.combinedSecondaryVertexBJetTags"];
    if ( v35.value )
      {
        jethelper_combinedSecondaryVertexBJetTags.resize(*v35.count);
        copy(v35.value, v35.value+*v35.count, jethelper_combinedSecondaryVertexBJetTags.begin());
      }
    else
      jethelper_combinedSecondaryVertexBJetTags.clear();

    countvalue& v36 = (*varmap)["patJetHelper_selectedPatJetsAK5.combinedSecondaryVertexMVABJetTags"];
    if ( v36.value )
      {
        jethelper_combinedSecondaryVertexMVABJetTags.resize(*v36.count);
        copy(v36.value, v36.value+*v36.count, jethelper_combinedSecondaryVertexMVABJetTags.begin());
      }
    else
      jethelper_combinedSecondaryVertexMVABJetTags.clear();

    countvalue& v37 = (*varmap)["patJetHelper_selectedPatJetsAK5.energy"];
    if ( v37.value )
      {
        jethelper_energy.resize(*v37.count);
        copy(v37.value, v37.value+*v37.count, jethelper_energy.begin());
      }
    else
      jethelper_energy.clear();

    countvalue& v38 = (*varmap)["patJetHelper_selectedPatJetsAK5.et"];
    if ( v38.value )
      {
        jethelper_et.resize(*v38.count);
        copy(v38.value, v38.value+*v38.count, jethelper_et.begin());
      }
    else
      jethelper_et.clear();

    countvalue& v39 = (*varmap)["patJetHelper_selectedPatJetsAK5.eta"];
    if ( v39.value )
      {
        jethelper_eta.resize(*v39.count);
        copy(v39.value, v39.value+*v39.count, jethelper_eta.begin());
      }
    else
      jethelper_eta.clear();

    countvalue& v40 = (*varmap)["patJetHelper_selectedPatJetsAK5.genJet_energy"];
    if ( v40.value )
      {
        jethelper_genJet_energy.resize(*v40.count);
        copy(v40.value, v40.value+*v40.count, jethelper_genJet_energy.begin());
      }
    else
      jethelper_genJet_energy.clear();

    countvalue& v41 = (*varmap)["patJetHelper_selectedPatJetsAK5.genJet_eta"];
    if ( v41.value )
      {
        jethelper_genJet_eta.resize(*v41.count);
        copy(v41.value, v41.value+*v41.count, jethelper_genJet_eta.begin());
      }
    else
      jethelper_genJet_eta.clear();

    countvalue& v42 = (*varmap)["patJetHelper_selectedPatJetsAK5.genJet_phi"];
    if ( v42.value )
      {
        jethelper_genJet_phi.resize(*v42.count);
        copy(v42.value, v42.value+*v42.count, jethelper_genJet_phi.begin());
      }
    else
      jethelper_genJet_phi.clear();

    countvalue& v43 = (*varmap)["patJetHelper_selectedPatJetsAK5.genJet_pt"];
    if ( v43.value )
      {
        jethelper_genJet_pt.resize(*v43.count);
        copy(v43.value, v43.value+*v43.count, jethelper_genJet_pt.begin());
      }
    else
      jethelper_genJet_pt.clear();

    countvalue& v44 = (*varmap)["patJetHelper_selectedPatJetsAK5.genParton_pdgId"];
    if ( v44.value )
      {
        jethelper_genParton_pdgId.resize(*v44.count);
        copy(v44.value, v44.value+*v44.count, jethelper_genParton_pdgId.begin());
      }
    else
      jethelper_genParton_pdgId.clear();

    countvalue& v45 = (*varmap)["patJetHelper_selectedPatJetsAK5.jetArea"];
    if ( v45.value )
      {
        jethelper_jetArea.resize(*v45.count);
        copy(v45.value, v45.value+*v45.count, jethelper_jetArea.begin());
      }
    else
      jethelper_jetArea.clear();

    countvalue& v46 = (*varmap)["patJetHelper_selectedPatJetsAK5.jetBProbabilityBJetTags"];
    if ( v46.value )
      {
        jethelper_jetBProbabilityBJetTags.resize(*v46.count);
        copy(v46.value, v46.value+*v46.count, jethelper_jetBProbabilityBJetTags.begin());
      }
    else
      jethelper_jetBProbabilityBJetTags.clear();

    countvalue& v47 = (*varmap)["patJetHelper_selectedPatJetsAK5.jetProbabilityBJetTags"];
    if ( v47.value )
      {
        jethelper_jetProbabilityBJetTags.resize(*v47.count);
        copy(v47.value, v47.value+*v47.count, jethelper_jetProbabilityBJetTags.begin());
      }
    else
      jethelper_jetProbabilityBJetTags.clear();

    countvalue& v48 = (*varmap)["patJetHelper_selectedPatJetsAK5.mass"];
    if ( v48.value )
      {
        jethelper_mass.resize(*v48.count);
        copy(v48.value, v48.value+*v48.count, jethelper_mass.begin());
      }
    else
      jethelper_mass.clear();

    countvalue& v49 = (*varmap)["patJetHelper_selectedPatJetsAK5.muonEnergyFraction"];
    if ( v49.value )
      {
        jethelper_muonEnergyFraction.resize(*v49.count);
        copy(v49.value, v49.value+*v49.count, jethelper_muonEnergyFraction.begin());
      }
    else
      jethelper_muonEnergyFraction.clear();

    countvalue& v50 = (*varmap)["patJetHelper_selectedPatJetsAK5.nConstituents"];
    if ( v50.value )
      {
        jethelper_nConstituents.resize(*v50.count);
        copy(v50.value, v50.value+*v50.count, jethelper_nConstituents.begin());
      }
    else
      jethelper_nConstituents.clear();

    countvalue& v51 = (*varmap)["patJetHelper_selectedPatJetsAK5.neutralEmEnergyFraction"];
    if ( v51.value )
      {
        jethelper_neutralEmEnergyFraction.resize(*v51.count);
        copy(v51.value, v51.value+*v51.count, jethelper_neutralEmEnergyFraction.begin());
      }
    else
      jethelper_neutralEmEnergyFraction.clear();

    countvalue& v52 = (*varmap)["patJetHelper_selectedPatJetsAK5.neutralHadronEnergyFraction"];
    if ( v52.value )
      {
        jethelper_neutralHadronEnergyFraction.resize(*v52.count);
        copy(v52.value, v52.value+*v52.count, jethelper_neutralHadronEnergyFraction.begin());
      }
    else
      jethelper_neutralHadronEnergyFraction.clear();

    countvalue& v53 = (*varmap)["patJetHelper_selectedPatJetsAK5.partonFlavour"];
    if ( v53.value )
      {
        jethelper_partonFlavour.resize(*v53.count);
        copy(v53.value, v53.value+*v53.count, jethelper_partonFlavour.begin());
      }
    else
      jethelper_partonFlavour.clear();

    countvalue& v54 = (*varmap)["patJetHelper_selectedPatJetsAK5.phi"];
    if ( v54.value )
      {
        jethelper_phi.resize(*v54.count);
        copy(v54.value, v54.value+*v54.count, jethelper_phi.begin());
      }
    else
      jethelper_phi.clear();

    countvalue& v55 = (*varmap)["patJetHelper_selectedPatJetsAK5.photonEnergyFraction"];
    if ( v55.value )
      {
        jethelper_photonEnergyFraction.resize(*v55.count);
        copy(v55.value, v55.value+*v55.count, jethelper_photonEnergyFraction.begin());
      }
    else
      jethelper_photonEnergyFraction.clear();

    countvalue& v56 = (*varmap)["patJetHelper_selectedPatJetsAK5.pt"];
    if ( v56.value )
      {
        jethelper_pt.resize(*v56.count);
        copy(v56.value, v56.value+*v56.count, jethelper_pt.begin());
      }
    else
      jethelper_pt.clear();

    countvalue& v57 = (*varmap)["patJetHelper_selectedPatJetsAK5.simpleSecondaryVertexHighEffBJetTags"];
    if ( v57.value )
      {
        jethelper_simpleSecondaryVertexHighEffBJetTags.resize(*v57.count);
        copy(v57.value, v57.value+*v57.count, jethelper_simpleSecondaryVertexHighEffBJetTags.begin());
      }
    else
      jethelper_simpleSecondaryVertexHighEffBJetTags.clear();

    countvalue& v58 = (*varmap)["patJetHelper_selectedPatJetsAK5.simpleSecondaryVertexHighPurBJetTags"];
    if ( v58.value )
      {
        jethelper_simpleSecondaryVertexHighPurBJetTags.resize(*v58.count);
        copy(v58.value, v58.value+*v58.count, jethelper_simpleSecondaryVertexHighPurBJetTags.begin());
      }
    else
      jethelper_simpleSecondaryVertexHighPurBJetTags.clear();

    countvalue& v59 = (*varmap)["patJetHelper_selectedPatJetsAK5.trackCountingHighEffBJetTags"];
    if ( v59.value )
      {
        jethelper_trackCountingHighEffBJetTags.resize(*v59.count);
        copy(v59.value, v59.value+*v59.count, jethelper_trackCountingHighEffBJetTags.begin());
      }
    else
      jethelper_trackCountingHighEffBJetTags.clear();

    countvalue& v60 = (*varmap)["patJetHelper_selectedPatJetsAK5.trackCountingHighPurBJetTags"];
    if ( v60.value )
      {
        jethelper_trackCountingHighPurBJetTags.resize(*v60.count);
        copy(v60.value, v60.value+*v60.count, jethelper_trackCountingHighPurBJetTags.begin());
      }
    else
      jethelper_trackCountingHighPurBJetTags.clear();

    countvalue& v61 = (*varmap)["patJetHelper_selectedPatJetsAK5.uncor_energy"];
    if ( v61.value )
      {
        jethelper_uncor_energy.resize(*v61.count);
        copy(v61.value, v61.value+*v61.count, jethelper_uncor_energy.begin());
      }
    else
      jethelper_uncor_energy.clear();

    countvalue& v62 = (*varmap)["patJetHelper_selectedPatJetsAK5.uncor_et"];
    if ( v62.value )
      {
        jethelper_uncor_et.resize(*v62.count);
        copy(v62.value, v62.value+*v62.count, jethelper_uncor_et.begin());
      }
    else
      jethelper_uncor_et.clear();

    countvalue& v63 = (*varmap)["patJetHelper_selectedPatJetsAK5.uncor_pt"];
    if ( v63.value )
      {
        jethelper_uncor_pt.resize(*v63.count);
        copy(v63.value, v63.value+*v63.count, jethelper_uncor_pt.begin());
      }
    else
      jethelper_uncor_pt.clear();

    countvalue& v66 = (*varmap)["edmTriggerResultsHelper_TriggerResults_HLT.HLT_PFJet320_v5"];
    if ( v66.value )
      triggerresultshelper2_HLT_PFJet320_v5 = *v66.value;
    else
      triggerresultshelper2_HLT_PFJet320_v5 = 0;

    countvalue& v67 = (*varmap)["edmTriggerResultsHelper_TriggerResults_HLT.HLT_PFJet320_v5"];
    if ( v67.value )
      triggerresultshelper_HLT_PFJet320_v5 = *v67.value;
    else
      triggerresultshelper_HLT_PFJet320_v5 = 0;
  }
  
public:
  ClassDef(addvars,1)
};
ClassImp(addvars)

#endif
