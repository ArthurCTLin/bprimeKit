/*******************************************************************************
*
*  Filename    : LeptonNtuplizer.hpp
*  Description : Class for handling Lepton ntuplizing objects and routines
*  Author      : Yi-Mu "Enoch" Chen [ ensc@hep1.phys.ntu.edu.tw ]
*
*******************************************************************************/
#ifndef BPKFRAMEWORK_BPRIMEKIT_LEPTONNTUPLIZER_HPP
#define BPKFRAMEWORK_BPRIMEKIT_LEPTONNTUPLIZER_HPP

#include "DataFormats/PatCandidates/interface/Electron.h"
#include "DataFormats/PatCandidates/interface/Muon.h"
#include "DataFormats/PatCandidates/interface/Tau.h"
#include "bpkFrameWork/bprimeKit/interface/NtuplizerBase.hpp"

class LeptonNtuplizer : public NtuplizerBase
{
public:
  LeptonNtuplizer ( const edm::ParameterSet&, bprimeKit* );
  virtual
  ~LeptonNtuplizer ();

  virtual void RegisterTree( TTree* );
  virtual void Analyze( const edm::Event&, const edm::EventSetup& );

private:
  LepInfoBranches LepInfo;

  const std::string _leptonname;
  const edm::EDGetToken _rhotoken;
  const edm::EDGetToken _muontoken;
  const edm::EDGetToken _electrontoken;
  const edm::EDGetToken _tautoken;
  const edm::EDGetToken _gentoken;
  const edm::EDGetToken _packedcandtoken;
  const edm::EDGetToken _electronID_vetotoken;
  const edm::EDGetToken _electronID_loosetoken;
  const edm::EDGetToken _electronID_mediumtoken;
  const edm::EDGetToken _electronID_tighttoken;
  const edm::EDGetToken _electronID_HEEPtoken;
  const edm::EDGetToken _conversionstoken;
  const edm::EDGetToken _vtxtoken;
  const edm::EDGetToken _beamspottoken;

  edm::Handle<double> _rhohandle;
  edm::Handle<std::vector<pat::Muon> > _muonhandle;
  edm::Handle<std::vector<pat::Electron> > _electronhandle;
  edm::Handle<std::vector<pat::Tau> > _tauhandle;
  edm::Handle<std::vector<reco::GenParticle> > _genhandle;
  edm::Handle<pat::PackedCandidateCollection> _packedhandle;
  edm::Handle<reco::ConversionCollection> _conversionhandle;
  edm::Handle<edm::ValueMap<bool> > _electronIDVeto;
  edm::Handle<edm::ValueMap<bool> > _electronIDLoose;
  edm::Handle<edm::ValueMap<bool> > _electronIDMedium;
  edm::Handle<edm::ValueMap<bool> > _electronIDTight;
  edm::Handle<edm::ValueMap<bool> > _electronIDHEEP;
  edm::Handle<std::vector<reco::Vertex> > _vtxhandle;
  edm::Handle<reco::BeamSpot> _beamspothandle;

  void FillMuon( const edm::Event&, const edm::EventSetup& );
  void FillElectron( const edm::Event&, const edm::EventSetup& );
  void FillTau( const edm::Event&, const edm::EventSetup& );


  /*******************************************************************************
  *   Helper functions for gen informatin extraction
  *******************************************************************************/
  int GetGenMCTag( double, double, double ) const;
  int GetGenMCTag( const std::vector<pat::Muon>::const_iterator&     ) const;
  int GetGenMCTag( const std::vector<pat::Electron>::const_iterator& ) const;

};

#endif/* end of include guard: BPKFRAMEWORK_BPRIMEKIT_LEPTONNTUPLIZER_HPP */
