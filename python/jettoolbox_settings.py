#*******************************************************************************
 #
 #  Filename    : jettoolbox_settings.py
 #  Description : Configuration settings for jet settings
 #  Author      : Yi-Mu 'Enoch' Chen [ ensc@hep1.phys.ntu.edu.tw ]
 #
#*******************************************************************************
import FWCore.ParameterSet.Config     as cms
from   JMEAnalysis.JetToolbox.jetToolbox_cff import jetToolbox

listBtagDiscriminators = [
    #Standard CSVv2 b-tagging discriminator
    'pfCombinedInclusiveSecondaryVertexV2BJetTags',
    #AK8 double b-tagging discriminator
    'pfBoostedDoubleSecondaryVertexAK8BJetTags',
    #Deep CSV
    'pfDeepCSVJetTags:probb',
    'pfDeepCSVJetTags:probc',
    'pfDeepCSVJetTags:probudsg',
    'pfDeepCSVJetTags:probbb',
#    'pfDeepCSVJetTags:probcc',
    'pfCombinedSecondaryVertexV2BJetTags',
    'pfCombinedMVAV2BJetTags'
]

ak4Cut='pt > 25 && abs(eta) < 5.'
ak8Cut='pt > 100 && abs(eta) < 5.'

def jettoolbox_settings( process , runMC ):
    process.load('CommonTools/PileupAlgos/Puppi_cff')
    process.puppi.candName = cms.InputTag('packedPFCandidates')
    process.puppi.vertexName = cms.InputTag('offlineSlimmedPrimaryVertices')
    process.puppi.useExistingWeights = cms.bool(True)

    jetToolbox( process, 'ak4', 'ak4chs', 'edmOut',
        runOnMC            = runMC,
        addQGTagger        = True,
        bTagDiscriminators = listBtagDiscriminators,
        Cut                = ak4Cut
    )

    jetToolbox( process, 'ak4', 'ak4puppi', 'edmOut',
        runOnMC            =runMC,
        PUMethod           ='Puppi',
        newPFCollection    =True,
        nameNewPFCollection='puppi',
        bTagDiscriminators =listBtagDiscriminators,
        Cut                =ak4Cut
    )

    jetToolbox( process, 'ak8', 'ak8chs', 'edmOut',
        runOnMC            = runMC,
        addSoftDropSubjets = True,
        addTrimming        = True,
        rFiltTrim          = 0.1,
        addPruning         = True,
        addFiltering       = True,
        addSoftDrop        = True,
        addNsub            = True,
        bTagDiscriminators = listBtagDiscriminators,
        addCMSTopTagger    = True,
        Cut                = ak8Cut ,
        addNsubSubjets     = True,
        subjetMaxTau       = 4
    )

    jetToolbox( process, 'ak8', 'ak8puppi', 'edmOut',
        runOnMC            =runMC,
        PUMethod           ='Puppi',
        newPFCollection    =True,
        nameNewPFCollection='puppi',
        addSoftDropSubjets =True,
        addTrimming        =True,
        addPruning         =True,
        addFiltering       =True,
        addSoftDrop        =True,
        addNsub            =True,
        bTagDiscriminators =listBtagDiscriminators,
        addCMSTopTagger    =True,
        Cut                =ak8Cut,
        addNsubSubjets     =True,
        subjetMaxTau       =4
    )

    jetToolbox( process, 'ca8', 'ca8chs', 'edmOut',
        runOnMC            = runMC ,
        addMassDrop        = True,
        addCMSTopTagger    = True,
        GetJetMCFlavour    = True,
        GetSubjetMCFlavour = True,
        bTagDiscriminators = listBtagDiscriminators ,
        Cut                = ''
    )

    jetToolbox( process, 'ca8', 'ca8puppi', 'edmOut',
        runOnMC            = runMC ,
        PUMethod           = 'Puppi',
        newPFCollection    = True,
        nameNewPFCollection= 'puppi',
        addMassDrop        = True,
        addCMSTopTagger    = True,
        GetJetMCFlavour    = True,
        GetSubjetMCFlavour = True,
        bTagDiscriminators = listBtagDiscriminators ,
        Cut                = ''
    )

    #To avoid producing additional JetToolBox format files, kick out the "EndPath" attribute of process.
    delattr(process, 'endpath')

    #Additional QGTagger Information
    #How to include QGL database : https://twiki.cern.ch/twiki/bin/view/CMS/QuarkGluonLikelihood#Step_0_only_for_MiniAOD_CMSSW_7
    from CondCore.DBCommon.CondDBSetup_cfi import CondDBSetup

    #Latest QGL database : https://github.com/cms-jet/QGLDatabase/tree/master/SQLiteFiles
    qgDatabaseVersion = 'cmssw8020_v2'
    process.QGPoolDBESSource = cms.ESSource("PoolDBESSource",
        CondDBSetup,
        toGet = cms.VPSet(
            cms.PSet(
                record = cms.string('QGLikelihoodRcd'),
                tag    = cms.string('QGLikelihoodObject_'+qgDatabaseVersion+'_AK4PFchs'),
                label  = cms.untracked.string('QGL_AK4PFchs')
            ),
        ),
        connect = cms.string('sqlite_fip:bpkFrameWork/bprimeKit/data/QGL_'+qgDatabaseVersion+'.db')
    )
    process.es_prefer_qg = cms.ESPrefer('PoolDBESSource','QGPoolDBESSource')

    process.QGTaggerAK4PFCHS.srcVertexCollection = cms.InputTag('offlineSlimmedPrimaryVertices')

    from RecoJets.JetProducers.QGTagger_cfi import QGTagger
    setattr( process, 'QGTaggerAK4PFPuppi',
                    QGTagger.clone(
                        srcJets              = cms.InputTag('ak4PFJetsPuppi'),
                        jetsLabel            = cms.string('QGL_AK4PFchs'), # Other options : see https://twiki.cern.ch/twiki/bin/viewauth/CMS/QGDataBaseVersion
                        srcVertexCollection  = cms.InputTag('offlineSlimmedPrimaryVertices')
                    )
    )
    getattr( process, 'patJetsAK4PFPuppi').userData.userFloats.src += ['QGTaggerAK4PFPuppi:qgLikelihood']

    #Use new Task() attribute of python
    process.myTask.add( process.QGTaggerAK4PFPuppi )
    JetToolBoxSequence = cms.Sequence( process.myTask )
    setattr( process, 'JetToolBoxSequence', JetToolBoxSequence)
