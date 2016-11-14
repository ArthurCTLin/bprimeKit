/*******************************************************************************

*  Filename    : bprimeKit.cc
*  Description : The virtual and explicit functions for the bprimeKit
*
*
*******************************************************************************/
#include "bpkFrameWork/bprimeKit/interface/bprimeKit.h"

#include <TFile.h>
#include <TTree.h>
#include <iostream>
#include <string>

#include "CommonTools/UtilAlgos/interface/TFileService.h"
#include "FWCore/Framework/interface/MakerMacros.h"// For plugin definition
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "SimGeneral/HepPDTRecord/interface/ParticleDataTable.h"
#include "TrackingTools/Records/interface/TransientTrackRecord.h"

using namespace std;
using edm::InputTag;
// ------------------------------------------------------------------------------
//   Custom enums, typedefs and macros
// ------------------------------------------------------------------------------
typedef std::vector<edm::InputTag> TagList;
typedef std::vector<std::string>   StrList;

#define GETTOKEN( PARAMSET, TYPE, TAG ) \
   ( consumes<TYPE>( PARAMSET.getParameter<edm::InputTag>( TAG ) ) )

// ------------------------------------------------------------------------------
//   bprimeKit methods: constructor and destructor
// ------------------------------------------------------------------------------
bprimeKit::bprimeKit( const edm::ParameterSet& iConfig ) :
   // Event related information
   fRhoToken( GETTOKEN( iConfig, double, "rhoLabel" ) ),
   fMETToken( GETTOKEN( iConfig, METList, "metLabel" ) ),
   fPuppiMETToken( GETTOKEN( iConfig, METList, "puppimetLabel" ) ),
   fPileupToken( GETTOKEN( iConfig, PileupList, "puInfoLabel" ) ),

   // Trigger related information
   fHLTToken( GETTOKEN( iConfig, edm::TriggerResults, "hltLabel" ) ),
   fTriggerObjToken( GETTOKEN( iConfig, pat::TriggerObjectStandAloneCollection, "trgobjLabel" ) ),
   fTrgList( GetTrgList( iConfig )),

// Vertex related
   fPrimaryVertexToken( GETTOKEN( iConfig, VertexList, "offlinePVLabel" ) ),
   fPrimVertex_withBeamSpot_Token( GETTOKEN( iConfig, VertexList,  "offlinePVBSLabel" ) ),
   fBeamspotToken( GETTOKEN( iConfig, reco::BeamSpot, "offlineBSLabel" ) ),

// Gen Info related
   fGenEventToken( GETTOKEN( iConfig, GenEventInfoProduct, "genevtLabel" ) ),
   fGenParticleToken( GETTOKEN( iConfig, GenList, "genLabel" ) ),
   fGenDigiToken( GETTOKEN( iConfig, L1GlobalTriggerReadoutRecord, "gtdigiLabel" ) ),
   fLHEToken( GETTOKEN( iConfig, LHEEventProduct, "lheLabel" ) ),
   fLHERunToken( consumes<LHERunInfoProduct, edm::InRun>( iConfig.getParameter<InputTag>( "lheRunLabel" ) ) ),

   fPhotonEffectiveArea_ChargeHadron( ( iConfig.getParameter<edm::FileInPath>( "effAreaChHadFile" ) ).fullPath() ),
   fPhotonEffectiveArea_NeutralHadron( ( iConfig.getParameter<edm::FileInPath>( "effAreaNeuHadFile" ) ).fullPath() ),
   fPhotonEffectiveArea_Photons( ( iConfig.getParameter<edm::FileInPath>( "effAreaPhoFile" ) ).fullPath() )
{
   // ----- Configuration flags  -----------------------------------------------
   fSkipfGenInfo       = iConfig.getParameter<bool>( "SkipGenInfo"     );
   fIncludeL7          = iConfig.getParameter<bool>( "IncludeL7"       );
   fDebug              = iConfig.getParameter<int>( "Debug"           );
   fRunOnB2G           = iConfig.getParameter<bool>( "runOnB2G"        );
   fRunMuonJetCleaning = iConfig.getParameter<bool>( "runMuonJetClean" );

   // ----- Jet related  -------------------------------------------------------
   for( const auto& jetsetting : iConfig.getParameter<std::vector<edm::ParameterSet> >( "JetSettings" ) ){
      fJetCollections.push_back( jetsetting.getParameter<std::string>( "jetCollection" ) );
      fJetTokens.push_back( consumes<JetList>( jetsetting.getParameter<edm::InputTag>( "jetLabel" ) ) );
      fSubjetTokens.push_back( consumes<JetList>( jetsetting.getParameter<edm::InputTag>( "subjetLabel" ) ) );
   }

   // ----- Lepton related  ----------------------------------------------------------------------------
   fLeptonCollections      = iConfig.getParameter<StrList>( "LepCollections" );  // branch names
   const auto muontag_list = iConfig.getParameter<TagList>( "muonLabel"      );
   const auto electag_list = iConfig.getParameter<TagList>( "elecLabel"      );
   const auto tautag_list  = iConfig.getParameter<TagList>( "tauLabel"       );

   for( unsigned i = 0; i < muontag_list.size(); ++i ){
      fMuonTokens.push_back( consumes<MuonList>( muontag_list[i] ) );
      fElectronTokens.push_back( consumes<ElectronList>( electag_list[i] ) );
      fTauTokens.push_back( consumes<TauList>( tautag_list[i] ) );
   }

   fPackedCandToken       = GETTOKEN( iConfig, pat::PackedCandidateCollection, "packedCand" );
   fElectronIDVetoToken   = GETTOKEN( iConfig, edm::ValueMap<bool>,  "eleVetoIdMap"    );
   fElectronIDLooseToken  = GETTOKEN( iConfig, edm::ValueMap<bool>,  "eleLooseIdMap"   );
   fElectronIDMediumToken = GETTOKEN( iConfig, edm::ValueMap<bool>,  "eleMediumIdMap"  );
   fElectronIDTightToken  = GETTOKEN( iConfig, edm::ValueMap<bool>,  "eleTightIdMap"   );
   fElectronIDHEEPToken   = GETTOKEN( iConfig, edm::ValueMap<bool>,  "eleHEEPIdMap"    );
   fConversionsTag        = GETTOKEN( iConfig, reco::ConversionCollection,  "conversionsLabel" );

   // ----- Photon related  ----------------------------------------------------
   fPhotonCollections     = iConfig.getParameter<StrList>( "PhoCollections" );   // branch names
   const auto photag_list = iConfig.getParameter<TagList>( "phoLabel"       );

   for( const auto& photag : photag_list ){
      fPhotonTokens.push_back( consumes<PhotonList>( photag ) );
   }

   fPhotonLooseIDToken            = GETTOKEN( iConfig, edm::ValueMap<bool>,  "phoLooseIdMap"             );
   fPhotonMediumIDToken           = GETTOKEN( iConfig, edm::ValueMap<bool>,  "phoMediumIdMap"            );
   fPhotonTightIDToken            = GETTOKEN( iConfig, edm::ValueMap<bool>,  "phoTightIdMap"             );
   fPhotonIsolation_Charged_Token = GETTOKEN( iConfig, edm::ValueMap<float>, "phoChargedIsolation"       );
   fPhotonIsolation_Neutral_Token = GETTOKEN( iConfig, edm::ValueMap<float>, "phoNeutralHadronIsolation" );
   fPhotonIsolation_Photon_Token  = GETTOKEN( iConfig, edm::ValueMap<float>, "phoPhotonIsolation"        );
   fPhotonSignaIEtaIEtaToken      = GETTOKEN( iConfig, edm::ValueMap<float>, "full5x5SigmaIEtaIEtaMap"   );

   for( int i = 0; i < N_TRIGGER_BOOKINGS; i++ ){
      fHighLevelTriggerMap.insert( pair<std::string, int>( TriggerBooking[i], i ) );
   }

   /***** MADITORY!! DO NOT REMOVE  *********************************************/
   edm::Service<TFileService> fs;
   TFileDirectory subDir = fs->mkdir( "mySubDirectory" );
   /******************************************************************************/
}

bprimeKit::~bprimeKit()
{
}


/*******************************************************************************
*
*  Note :
*    1. All writing objects (TTrees, TH1Fs) MUST be created here and
*       not in the analyzer constructor!!
*    2. Do not use the WRITE functions and delete operator!! These are
*       automatically handled by the TFileService instance.
*
*******************************************************************************/
void
bprimeKit::beginJob()
{
   fBaseTree = new TTree( "root", "root" );
   fRunTree  = new TTree( "run", "run"  );
   InitTree();
}
void
bprimeKit::endJob()
{
   // fBaseTree->Write();
   ClearTree();
}


// ------------------------------------------------------------------------------
//   bprimeKit event based analysis methods
// ------------------------------------------------------------------------------
void
bprimeKit::beginRun( const edm::Run& iRun, const edm::EventSetup& iSetup )
{
}
void
bprimeKit::endRun( const edm::Run& iRun, const edm::EventSetup& iSetup )
{
   GetRunObjects( iRun, iSetup );
   FillRunInfo();
   fRunTree->Fill();
}


void
bprimeKit::analyze( const edm::Event& iEvent, const edm::EventSetup& iSetup )
{
   GetEventObjects( iEvent, iSetup );
   fIsData = iEvent.isRealData();// Add by Jacky

   memset( &fGenInfo, 0x00, sizeof( fGenInfo ) );
   memset( &fEvtInfo, 0x00, sizeof( fEvtInfo ) );
   FillfGenInfo( iEvent, iSetup );
   FillEvent( iEvent, iSetup );

   FillVertex( iEvent, iSetup );
   FillLepton( iEvent, iSetup );
   FillPhoton( iEvent, iSetup );
   FillJet( iEvent, iSetup );
   FillTrgObj( iEvent, iSetup );

   fBaseTree->Fill();

}

// ------------------------------------------------------------------------------
//   Tree set-up options
// ------------------------------------------------------------------------------
void
bprimeKit::InitTree()
{
   fEvtInfo.RegisterTree( fBaseTree );
   fVertexInfo.RegisterTree( fBaseTree );
   if( !fSkipfGenInfo ){ fGenInfo.RegisterTree( fBaseTree ); }

   for( unsigned i = 0; i < fLeptonCollections.size(); ++i ){
      if( i >= MAX_LEPCOLLECTIONS ){ break; }
      fLepInfo[i].RegisterTree( fBaseTree, fLeptonCollections[i] );
   }

   for( unsigned i = 0; i < fPhotonCollections.size(); ++i ){
      if( i >= MAX_PHOCOLLECTIONS ){ break; }
      fPhotonInfo[i].RegisterTree( fBaseTree, fPhotonCollections[i] );
   }

   for( unsigned i = 0; i < fJetCollections.size(); ++i ){
      if( i >= MAX_JETCOLLECTIONS ){ break; }
      fJetInfo[i].RegisterTree( fBaseTree, fJetCollections[i] );
   }

   fTrgInfo.RegisterTree( fBaseTree );
   // ----- Setting the Run Information tree  --------------------------------------
   fRunInfo.RegisterTree( fRunTree );
}

void
bprimeKit::ClearTree()
{
   /***** DO NOT DELETE TREES!  **************************************************/
}

void
bprimeKit::GetEventObjects( const edm::Event& iEvent, const edm::EventSetup& iSetup )
{
   iEvent.getByToken( fRhoToken,                      fRho_H );
   iEvent.getByToken( fMETToken,                      fMET_H                );
   iEvent.getByToken( fPuppiMETToken,                 fPuppiMET_H           );
   iEvent.getByToken( fBeamspotToken,                 fBeamSpot_H           );
   iEvent.getByToken( fHLTToken,                      fTrigger_H            );
   iEvent.getByToken( fTriggerObjToken,               fTriggerObjList_H     );
   iEvent.getByToken( fPrimaryVertexToken,            fVertex_H             );
   iEvent.getByToken( fPrimVertex_withBeamSpot_Token, fVertexWithBeamSpot_H );

   bool changed = true;
   fHighLevelTriggerConfig.init( iEvent.getRun(), iSetup, "HLT", changed );

   if( !iEvent.isRealData() ){
      iEvent.getByToken( fPileupToken, fPileup_H );
      if( !fSkipfGenInfo ){
         iEvent.getByToken( fGenParticleToken, fGenParticle_H );
         iEvent.getByToken( fGenEventToken,    fGenEvent_H );
         iEvent.getByToken( fLHEToken,         fLHEInfo_H );
         iEvent.getByToken( fGenDigiToken,     fRecord_H  );
      }
   }

   for( unsigned i = 0; i < fJetTokens.size(); ++i ){
      fJetList_Hs.push_back( JetHandle() );
      fSubjetList_Hs.push_back( JetHandle() );
      iEvent.getByToken( fJetTokens[i],    fJetList_Hs[i] );
      iEvent.getByToken( fSubjetTokens[i], fSubjetList_Hs[i] );
   }

   for( unsigned i = 0; i < fMuonTokens.size(); ++i ){
      fMuonList_Hs.push_back( MuonHandle() );
      fElectronList_Hs.push_back( ElectronHandle() );
      fTauList_Hs.push_back( TauHandle() );
      iEvent.getByToken( fMuonTokens[i],     fMuonList_Hs[i] );
      iEvent.getByToken( fElectronTokens[i], fElectronList_Hs[i] );
      iEvent.getByToken( fTauTokens[i],      fTauList_Hs[i] );
   }

   iEvent.getByToken( fPackedCandToken,       fPackedCand_H  );
   iEvent.getByToken( fConversionsTag,        fConversions_H      );
   iEvent.getByToken( fElectronIDVetoToken,   fElectronIDVeto_H   );
   iEvent.getByToken( fElectronIDLooseToken,  fElectronIDLoose_H  );
   iEvent.getByToken( fElectronIDMediumToken, fElectronIDMedium_H );
   iEvent.getByToken( fElectronIDTightToken,  fElectronIDTight_H  );
   iEvent.getByToken( fElectronIDHEEPToken,   fElectronIDHEEP_H   );

   for( unsigned il = 0; il < fPhotonTokens.size(); il++ ){
      fPhotonList_Hs.push_back( PhotonHandle() );
      iEvent.getByToken( fPhotonTokens[il], fPhotonList_Hs[il] );
   }

   iEvent.getByToken( fPhotonLooseIDToken,            fPhotonIDLoose             );
   iEvent.getByToken( fPhotonMediumIDToken,           fPhotonIDMedium            );
   iEvent.getByToken( fPhotonTightIDToken,            fPhotonIDTight             );
   iEvent.getByToken( fPhotonIsolation_Charged_Token, fPhotonIsolation_Charged_H );
   iEvent.getByToken( fPhotonIsolation_Neutral_Token, fPhotonIsolation_Neutral_H );
   iEvent.getByToken( fPhotonIsolation_Photon_Token,  fPhotonIsolation_Photon_H  );
   iEvent.getByToken( fPhotonSignaIEtaIEtaToken,      fPhotonSigmaIEtaIEta_H     );
}

/*******************************************************************************
*   Helper functions for getting Trigger Object list
*******************************************************************************/
std::vector<std::pair<std::string, std::string>> bprimeKit::GetTrgList( const edm::ParameterSet& iConfig )
{
   std::vector<std::pair<std::string,std::string>> ans;
   for( const auto& paramset : iConfig.getParameter<std::vector<edm::ParameterSet>>("triggerlist") ){
      std::pair<std::string, std::string>  mypair;
      mypair.first = paramset.getParameter<std::string>( "HLTPath" );
      mypair.second = paramset.getParameter<std::string>( "HLTFilter" );
      ans.push_back( mypair );
   }
   return ans;
}


DEFINE_FWK_MODULE( bprimeKit );
