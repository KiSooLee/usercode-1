// -*- C++ -*-
//
// Package:    SiStripCMNAnalyzer
// Class:      SiStripCMNAnalyzer
// 
/**\class SiStripCMNAnalyzer SiStripCMNAnalyzer.cc Appeltel/SiStripCMNAnalyzer/src/SiStripCMNAnalyzer.cc

 Description: [one line class summary]

 Implementation:
     [Notes on implementation]
*/
//
// Original Author:  Eric Appelt
//         Created:  Mon Jul 26 10:37:24 CDT 2010
// $Id: SiStripCMNAnalyzer.cc,v 1.12 2010/09/01 11:06:05 appeltel Exp $
//
//

#include "Appeltel/SiStripCMNAnalyzer/interface/SiStripCMNAnalyzer.h"

#include <memory>
#include <iostream>

SiStripCMNAnalyzer::SiStripCMNAnalyzer(const edm::ParameterSet& iConfig)
  :  algorithms(SiStripRawProcessingFactory::create(iConfig.getParameter<edm::ParameterSet>("Algorithms"))),
     clusterAlgorithm( StripClusterizerAlgorithmFactory::create(iConfig.getParameter<edm::ParameterSet>("Clusterizer")) ), 
     inputTag(iConfig.getParameter<edm::InputTag>("RawDigiProducersList")),
     galleryClusterMin_(iConfig.exists("galleryClusterMin")?iConfig.getParameter<int>("galleryClusterMin"):0)
{

  edm::Service<TFileService> fs;
  allMedians_=fs->make<TH1F>("allMedians","Median ADC count of APV25", 500, 0., 255.);
  allIterMeds_=fs->make<TH1F>("allIterMeds","Iterated Median ADC count of APV25", 500, 0., 255.);
  all25ths_=fs->make<TH1F>("all25ths","25th percentile ADC count of APV25", 500, 0., 255.);
  medvs25_=fs->make<TH2F>("medvs25","Median ADC count versus 25th percentile",500,0.,500.,500,0.,255.);

  medianOccupancy_=fs->make<TH2F>("medianOccupancy","Median ADC count of APV25 versus Reconstructed # of Unsuppressed Strips",
                                  128.,0.,128.,500,0.,500.);
  iterMedOccupancy_=fs->make<TH2F>("iterMedOccupancy","Iterated Median ADC count of APV25 versus Reconstructed # of Unsuppressed Strips",
                                  128.,0.,128.,500,0.,500.);
  per25Occupancy_=fs->make<TH2F>("per25Occupancy","25th Percentile ADC count of APV25 versus Reconstructed # of Unsuppressed Strips",
                                  128.,0.,128.,500,0.,500.);

  TIBMedians_=fs->make<TH1F>("TIBMedians","Median ADC count of APV25 for the TIB", 500, 0., 255.);
  TIBIterMeds_=fs->make<TH1F>("TIBIterMeds","Iterated Median ADC count of APV25 for the TIB", 500, 0., 255.);
  TIB25ths_=fs->make<TH1F>("TIB25ths","25th percentile ADC count of APV25 for the TIB", 500, 0., 255.);
  TIBGens_=fs->make<TH1F>("TIBGens","Expected CMN Distribution for the TIB", 500, 0., 255.);

  TOBMedians_=fs->make<TH1F>("TOBMedians","Median ADC count of APV25 for the TOB", 500, 0., 255.);
  TOBIterMeds_=fs->make<TH1F>("TOBIterMeds","Iterated Median ADC count of APV25 for the TOB", 500, 0., 255.);
  TOB25ths_=fs->make<TH1F>("TOB25ths","25th percentile ADC count of APV25 for the TOB", 500, 0., 255.);
  TOBGens_=fs->make<TH1F>("TOBGens","Expected CMN Distribution for the TOB", 500, 0., 255.);

  TIDMedians_=fs->make<TH1F>("TIDMedians","Median ADC count of APV25 for the TID", 500, 0., 255.);
  TIDIterMeds_=fs->make<TH1F>("TIDIterMeds","Iterated Median ADC count of APV25 for the TID", 500, 0., 255.);
  TID25ths_=fs->make<TH1F>("TID25ths","25th percentile ADC count of APV25 for the TID", 500, 0., 255.);
  TIDGens_=fs->make<TH1F>("TIDGens","Expected CMN Distribution for the TID", 500, 0., 255.);


  TECMedians_=fs->make<TH1F>("TECMedians","Median ADC count of APV25 for the TEC", 500, 0., 255.);
  TECIterMeds_=fs->make<TH1F>("TECIterMeds","Iterated Median ADC count of APV25 for the TEC", 500, 0., 255.);
  TEC25ths_=fs->make<TH1F>("TEC25ths","25th percentile ADC count of APV25 for the TEC", 500, 0., 255.);
  TECGens_=fs->make<TH1F>("TECGens","Expected CMN Distribution for the TEC", 500, 0., 255.);

  totalADCMedian_=fs->make<TH1F>("totalADCMedian","Integrated ADC count for each APV25 after Median CMN subtraction",500,0.,10000.);
  totalADCIterMed_=fs->make<TH1F>("totalADCIterMed","Integrated ADC count for each APV25 after Iterated Median CMN subtraction",500,0.,10000.);
  totalADCPer25_=fs->make<TH1F>("totalADCPer25","Integrated ADC count for each APV25 after 25th Percentile CMN subtraction",500,0.,10000.);
  
  medianTotalClus_=fs->make<TH1F>("medianTotalClus","Number of clusters found using median CMN subtraction",500,0,500000.);
  iterMedTotalClus_=fs->make<TH1F>("iterMedTotalClus","Number of clusters found using iterated median CMN subtraction",500,0,500000.);
  per25TotalClus_=fs->make<TH1F>("per25TotalClus","Number of clusters found using median CMN subtraction",500,0,500000.);

  medianClusWidth_ = fs->make<TH1F>("medianClusWidth","Cluster Width - Median CMN Subtraction",256,0.,256.);
  iterMedClusWidth_ = fs->make<TH1F>("iterMedClusWidth","Cluster Width - Iterated Median CMN Subtraction",256,0.,256.);
  per25ClusWidth_ = fs->make<TH1F>("per25ClusWidth","Cluster Width - 25th Percentile CMN Subtraction",256,0.,256.);

  medianAPVClusNum_ = fs->make<TH1F>("medianAPVClusNum","Number of Clusters on an APV25 - Median CMN Subtraction",128,0.,128.);
  iterMedAPVClusNum_ = fs->make<TH1F>("iterMedAPVClusNum","Number of Clusters on an APV25 - Iterated Median CMN Subtraction",128,0.,128.);
  per25APVClusNum_ = fs->make<TH1F>("per25APVClusNum","Number of Clusters on an APV25 - 25th Percentile CMN Subtraction",128,0.,128.);
  

  TString leafstring;
  leafstring = "adc[128]/I:clustersMedian[128]/I:clustersIterMed[128]/I:clustersPer25[128]/I:medianOffset/D:iterMedOffset/D:per25Offset/D:event/I:lumi/I:run/I:detID/i:apv/I";

  galleryA_ = fs->make<TTree>("galleryATree","galleryATree");
  galleryA_->Branch("apvReadouts",&tmpAPV, leafstring.Data()); 
  galleryB_ = fs->make<TTree>("galleryBTree","galleryBTree");
  galleryB_->Branch("apvReadouts",&tmpAPV, leafstring.Data()); 
  galleryC_ = fs->make<TTree>("galleryCTree","galleryCTree");
  galleryC_->Branch("apvReadouts",&tmpAPV, leafstring.Data()); 
  galleryD_ = fs->make<TTree>("galleryDTree","galleryDTree");
  galleryD_->Branch("apvReadouts",&tmpAPV, leafstring.Data()); 
  galleryE_ = fs->make<TTree>("galleryETree","galleryETree");
  galleryE_->Branch("apvReadouts",&tmpAPV, leafstring.Data()); 


  galAcount = 0;
  galBcount = 0;
  galCcount = 0;
  galDcount = 0;
  galEcount = 0;

  edm::Service<edm::RandomNumberGenerator> rng;
  if ( ! rng.isAvailable()) {
    throw cms::Exception("Configuration")
      << "SiStripCMNAnalyzer requires the RandomNumberGeneratorService\n"
      "which is not present in the configuration file.  You must add the service\n"
      "in the configuration file or remove the modules that require it.";
  }
  
  rndEngine       = &(rng->getEngine());
  genNoise = new GaussianTailNoiseGenerator(*rndEngine);
  gaussDistribution = new CLHEP::RandGaussQ(*rndEngine);

  cmnTIBRMS_ = 5.92;
  cmnTOBRMS_ = 1.08;
  cmnTIDRMS_ = 3.08;
  cmnTECRMS_ = 2.44;

}


SiStripCMNAnalyzer::~SiStripCMNAnalyzer()
{

  delete genNoise;
  delete gaussDistribution;
 
}


void
SiStripCMNAnalyzer::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{
   using namespace edm;

   // GENERAL INITIALIZATION

   algorithms->initialize(iSetup);
   clusterAlgorithm->initialize(iSetup);
   edm::Handle< edm::DetSetVector<SiStripRawDigi> > input;
   iEvent.getByLabel(inputTag,input);
   
   edm::Handle< edm::DetSetVector<SiStripRawDigi> > inputNoise;
   edm::Handle< edm::DetSetVector<SiStripRawDigi> > inputSignal;

   uint32_t n_cache_id = iSetup.get<SiStripNoisesRcd>().cacheIdentifier();
   uint32_t q_cache_id = iSetup.get<SiStripQualityRcd>().cacheIdentifier();

   if(n_cache_id != noise_cache_id) {
     iSetup.get<SiStripNoisesRcd>().get( noiseHandle );
     noise_cache_id = n_cache_id;
   }
   if(q_cache_id != quality_cache_id) {
     iSetup.get<SiStripQualityRcd>().get( qualityHandle );
     quality_cache_id = q_cache_id;
   }

   int eventNumber = iEvent.id().event();
   int lumiNumber = iEvent.id().luminosityBlock();
   int runNumber = iEvent.id().run();

   int medianTotClus =0;
   int iterMedTotClus =0;
   int per25TotClus =0;

   //----------------------------------
   //
   // MAIN LOOP OVER MODULES
   //
   //----------------------------------

   for ( edm::DetSetVector<SiStripRawDigi>::const_iterator 
         rawDigis = input->begin(); rawDigis != input->end(); rawDigis++) 
   {
     
     // ---
     // Set up some vectors to store digis at each stage of processing
     // and other variables for stuff

     std::vector<int16_t> pedestalSubDigis(rawDigis->size());

     std::vector<int16_t> medianSubtractedDigis(rawDigis->size());
     std::vector<int16_t> per25SubtractedDigis(rawDigis->size());
     std::vector<int16_t> iterMedSubtractedDigis( rawDigis->size());
     std::vector<float> medianOffset;
     std::vector<float> iterMedOffset;
     std::vector<float> per25Offset;

     edm::DetSet<SiStripDigi> medianZSDigis(rawDigis->id);
     edm::DetSet<SiStripDigi> per25ZSDigis(rawDigis->id);
     edm::DetSet<SiStripDigi> iterMedZSDigis(rawDigis->id);
     
     SiStripDetId sid( rawDigis->detId() );

     // The clusterizer wants to see entire DetSetVectors

     edm::DetSetVector<SiStripDigi> medianZSVec;
     edm::DetSetVector<SiStripDigi> per25ZSVec;
     edm::DetSetVector<SiStripDigi> iterMedZSVec;

     edmNew::DetSetVector<SiStripCluster> medianClusterVec;
     edmNew::DetSetVector<SiStripCluster> per25ClusterVec;
     edmNew::DetSetVector<SiStripCluster> iterMedClusterVec;

    // ---
    // Perform pedestal subtraction
    //

    algorithms->subtractorPed->subtract( *rawDigis, pedestalSubDigis);

    // ---
    // Perform CMN subtraction and store algorithm results by APV
    //

    subtractMedian( pedestalSubDigis, medianSubtractedDigis, medianOffset );
    subtractIterMed( pedestalSubDigis, iterMedSubtractedDigis, iterMedOffset, rawDigis->detId() );
    subtractPer25( pedestalSubDigis, per25SubtractedDigis, per25Offset );

    // ---
    // Now perform zero subtraction on the module for each CMN remover algorithm 
    //

    algorithms->suppressor->suppress( medianSubtractedDigis, medianZSDigis );
    algorithms->suppressor->suppress( iterMedSubtractedDigis, iterMedZSDigis );
    algorithms->suppressor->suppress( per25SubtractedDigis, per25ZSDigis );

    // ---
    // Run the clusterizer on the ZS digis
    //

    medianZSVec.insert(medianZSDigis);
    per25ZSVec.insert(per25ZSDigis);
    iterMedZSVec.insert(iterMedZSDigis);

    clusterAlgorithm->clusterize(medianZSVec, medianClusterVec);
    clusterAlgorithm->clusterize(iterMedZSVec, iterMedClusterVec);
    clusterAlgorithm->clusterize(per25ZSVec, per25ClusterVec);

    medianTotClus += medianClusterVec.dataSize();
    iterMedTotClus += iterMedClusterVec.dataSize();
    per25TotClus += per25ClusterVec.dataSize();

    //---------------------------------------------------
    //
    // ----- LOOP OVER EACH APV AND FILL HISTOGRAMS -----
    //
    //---------------------------------------------------

    for ( int APV = 0; APV < (int)rawDigis->size() / 128 ; APV++ ) 
    { 
      // ---
      // Get the pedestal subtracted ADCs for plotting
      //

      std::vector<int16_t> adc;  adc.reserve(128);  
      { 
        std::vector<int16_t>::const_iterator beginAPV, endAPV;
        beginAPV = pedestalSubDigis.begin(); beginAPV += 128*APV;
        endAPV = beginAPV + 128;
        adc.insert(adc.end(),beginAPV, endAPV);
      }

      // ---
      // Fill calclated baseline plots for all algorthims for all detectors and for TIB, TOB, TID, TEC
      //

      allMedians_->Fill( medianOffset[APV] );
      allIterMeds_->Fill( iterMedOffset[APV] );
      all25ths_->Fill( per25Offset[APV] );

      switch ( sid.subdetId() ) 
      {
        case SiStripDetId::TIB:
          TIBMedians_->Fill(medianOffset[APV]);
          TIBIterMeds_->Fill(iterMedOffset[APV]);
          TIB25ths_->Fill(per25Offset[APV]);
          TIBGens_->Fill( gaussDistribution->fire(128.,cmnTIBRMS_));
          break;
        case SiStripDetId::TOB:
          TOBMedians_->Fill(medianOffset[APV]);
          TOBIterMeds_->Fill(iterMedOffset[APV]);
          TOB25ths_->Fill(per25Offset[APV]);
          TOBGens_->Fill( gaussDistribution->fire(128.,cmnTOBRMS_));
          break;
        case SiStripDetId::TID:
          TIDMedians_->Fill(medianOffset[APV]);
          TIDIterMeds_->Fill(iterMedOffset[APV]);
          TID25ths_->Fill(per25Offset[APV]);
          TIDGens_->Fill( gaussDistribution->fire(128.,cmnTIDRMS_));
          break;
        case SiStripDetId::TEC:
          TECMedians_->Fill(medianOffset[APV]);
          TECIterMeds_->Fill(iterMedOffset[APV]);
          TEC25ths_->Fill(per25Offset[APV]);
          TECGens_->Fill( gaussDistribution->fire(128.,cmnTECRMS_));
          break;
      }      
      medvs25_->Fill( medianOffset[APV], per25Offset[APV]);

      // ---
      // Make occupancy histos
      //
      medianOccupancy_->Fill( digiCount(medianZSDigis, APV), medianOffset[APV] );
      iterMedOccupancy_->Fill( digiCount(iterMedZSDigis, APV), iterMedOffset[APV] );
      per25Occupancy_->Fill( digiCount(per25ZSDigis, APV), per25Offset[APV] );

      // ---
      // Make cluster width and count histos
      //

      medianAPVClusNum_->Fill( countClusters( medianClusterVec, APV));
      iterMedAPVClusNum_->Fill( countClusters( iterMedClusterVec, APV));
      per25APVClusNum_->Fill( countClusters( per25ClusterVec, APV));
 
      fillClusterWidths( medianClusterVec, medianClusWidth_, APV); 
      fillClusterWidths( iterMedClusterVec, iterMedClusWidth_, APV); 
      fillClusterWidths( per25ClusterVec, per25ClusWidth_, APV); 

      // ---
      // Integrate ADC counts and fill histos
      //

      float intMed = integrateADC(adc, medianOffset[APV] );
      float intIterMed = integrateADC(adc, iterMedOffset[APV] );
      float intPer25 = integrateADC(adc, per25Offset[APV] );

      totalADCMedian_->Fill( intMed );
      totalADCIterMed_->Fill( intIterMed);
      totalADCPer25_->Fill( intPer25 );
  
      // ---
      // Make the gallery plot
      //
   
      // gallery A
      if ( (medianOffset[APV] >= 116 && medianOffset[APV] < 140) && 
           (per25Offset[APV] >= 116 && per25Offset[APV] < 140) &&
	   (countClusters( medianClusterVec, APV) >= galleryClusterMin_))
      {
         int currentGraph = galAcount % 10;
         bool doGallery = true;
         if ( galAcount / 10 > 0 )
           if ( gaussDistribution->fire(0.,1.) < galAcount / 10 )
              doGallery = false;
       
         if ( doGallery ) {
           for ( int i = 0; i<128; i++) galA_[currentGraph].adc[i] = adc[i];
           galA_[currentGraph].medianOffset = (double)medianOffset[APV];
           galA_[currentGraph].iterMedOffset = (double)iterMedOffset[APV];
           galA_[currentGraph].per25Offset = (double)per25Offset[APV];
           fillClusterValues( medianClusterVec, galA_[currentGraph].clustersMedian, APV );
           fillClusterValues( per25ClusterVec, galA_[currentGraph].clustersPer25, APV );
           fillClusterValues( iterMedClusterVec, galA_[currentGraph].clustersIterMed, APV );
           galA_[currentGraph].event = eventNumber;
           galA_[currentGraph].lumi = lumiNumber;
           galA_[currentGraph].run = runNumber;
           galA_[currentGraph].detID = rawDigis->detId();
           galA_[currentGraph].apv = APV; 
           galAcount++;
         }
      }

      // gallery B
      if ( (medianOffset[APV] >= 2 && medianOffset[APV] < 100) && 
           (per25Offset[APV] >= 2 && per25Offset[APV] < 100) && 
	   (countClusters( medianClusterVec, APV) >= galleryClusterMin_))
      {
         int currentGraph = galBcount % 10;
         bool doGallery = true;
         if ( galBcount / 10 > 0 )
           if ( gaussDistribution->fire(0.,1.) < galBcount / 10 )
              doGallery = false;

         if ( doGallery ) {
           for ( int i = 0; i<128; i++) galB_[currentGraph].adc[i] = adc[i];
           galB_[currentGraph].medianOffset = (double)medianOffset[APV];
           galB_[currentGraph].iterMedOffset = (double)iterMedOffset[APV];
           galB_[currentGraph].per25Offset = (double)per25Offset[APV];
           fillClusterValues( medianClusterVec, galB_[currentGraph].clustersMedian, APV );
           fillClusterValues( per25ClusterVec, galB_[currentGraph].clustersPer25, APV );
           fillClusterValues( iterMedClusterVec, galB_[currentGraph].clustersIterMed, APV );
           galB_[currentGraph].event = eventNumber;
           galB_[currentGraph].lumi = lumiNumber;
           galB_[currentGraph].run = runNumber;
           galB_[currentGraph].detID = rawDigis->detId();
           galB_[currentGraph].apv = APV;
           galBcount++;
         }
      }

      // gallery C
      if ( medianOffset[APV] < 1 && 
           per25Offset[APV] < 1 && 
	   (countClusters( medianClusterVec, APV) >= galleryClusterMin_))
      {
         int currentGraph = galCcount % 10;
         bool doGallery = true;
         if ( galCcount / 10 > 0 )
           if ( gaussDistribution->fire(0.,1.) < galCcount / 10 )
              doGallery = false;

         if ( doGallery ) {
           for ( int i = 0; i<128; i++) galC_[currentGraph].adc[i] = adc[i];
           galC_[currentGraph].medianOffset = (double)medianOffset[APV];
           galC_[currentGraph].iterMedOffset = (double)iterMedOffset[APV];
           galC_[currentGraph].per25Offset = (double)per25Offset[APV];
           fillClusterValues( medianClusterVec, galC_[currentGraph].clustersMedian, APV );
           fillClusterValues( per25ClusterVec, galC_[currentGraph].clustersPer25, APV );
           fillClusterValues( iterMedClusterVec, galC_[currentGraph].clustersIterMed, APV );
           galC_[currentGraph].event = eventNumber;
           galC_[currentGraph].lumi = lumiNumber;
           galC_[currentGraph].run = runNumber;
           galC_[currentGraph].detID = rawDigis->detId();
           galC_[currentGraph].apv = APV;
           galCcount++;
         }
    
       }

      // gallery D
      if ( (medianOffset[APV] >= 160 ) &&
           (per25Offset[APV] >= 160 ) &&
	   (countClusters( medianClusterVec, APV) >= galleryClusterMin_))
      {
         int currentGraph = galDcount % 10;
         bool doGallery = true;
         if ( galDcount / 10 > 0 )
           if ( gaussDistribution->fire(0.,1.) < galDcount / 10 )
              doGallery = false;

         if ( doGallery ) {
           for ( int i = 0; i<128; i++) galD_[currentGraph].adc[i] = adc[i];
           galD_[currentGraph].medianOffset = (double)medianOffset[APV];
           galD_[currentGraph].iterMedOffset = (double)iterMedOffset[APV];
           galD_[currentGraph].per25Offset = (double)per25Offset[APV];
           fillClusterValues( medianClusterVec, galD_[currentGraph].clustersMedian, APV );
           fillClusterValues( per25ClusterVec, galD_[currentGraph].clustersPer25, APV );
           fillClusterValues( iterMedClusterVec, galD_[currentGraph].clustersIterMed, APV );
           galD_[currentGraph].event = eventNumber;
           galD_[currentGraph].lumi = lumiNumber;
           galD_[currentGraph].run = runNumber;
           galD_[currentGraph].detID = rawDigis->detId();
           galD_[currentGraph].apv = APV;
           galDcount++;
         }
      }

      // gallery E
      if ( (medianOffset[APV] >= 160) &&
           (per25Offset[APV] >= 116 && per25Offset[APV] < 140) &&
	   (countClusters( medianClusterVec, APV) >= galleryClusterMin_))
      {
         int currentGraph = galEcount % 10;
         bool doGallery = true;
         if ( galEcount / 10 > 0 )
           if ( gaussDistribution->fire(0.,1.) < galEcount / 10 )
              doGallery = false;

         if ( doGallery ) {
           for ( int i = 0; i<128; i++) galE_[currentGraph].adc[i] = adc[i];
           galE_[currentGraph].medianOffset = (double)medianOffset[APV];
           galE_[currentGraph].iterMedOffset = (double)iterMedOffset[APV];
           galE_[currentGraph].per25Offset = (double)per25Offset[APV];
           fillClusterValues( medianClusterVec, galE_[currentGraph].clustersMedian, APV );
           fillClusterValues( per25ClusterVec, galE_[currentGraph].clustersPer25, APV );
           fillClusterValues( iterMedClusterVec, galE_[currentGraph].clustersIterMed, APV );
           galE_[currentGraph].event = eventNumber;
           galE_[currentGraph].lumi = lumiNumber;
           galE_[currentGraph].run = runNumber;
           galE_[currentGraph].detID = rawDigis->detId();
           galE_[currentGraph].apv = APV;
           galEcount++;
         }
      }


    }  // END HISTOGRAMMING LOOP OVER APVS IN THE MODULE
    


  } // END LOOP OVER MODULES

    
  // ---
  // Get the total number of clusters
  //
   
  medianTotalClus_->Fill( medianTotClus );
  iterMedTotalClus_->Fill( iterMedTotClus );
  per25TotalClus_->Fill( per25TotClus );


}


void 
SiStripCMNAnalyzer::beginJob()
{
}

void 
SiStripCMNAnalyzer::endJob() {

  for( int i = 0; i< 10; i++)
  {
    copyAPVReadout( galA_[i], tmpAPV );
    galleryA_->Fill();
    copyAPVReadout( galB_[i], tmpAPV );
    galleryB_->Fill();
    copyAPVReadout( galC_[i], tmpAPV );
    galleryC_->Fill();
    copyAPVReadout( galD_[i], tmpAPV );
    galleryD_->Fill();
    copyAPVReadout( galE_[i], tmpAPV );
    galleryE_->Fill();
  }

}


void SiStripCMNAnalyzer::subtractMedian( std::vector<int16_t>& in, std::vector<int16_t>& out, std::vector<float>& results)
{
  std::vector<int16_t> tmp;  tmp.reserve(128);  
    std::vector<int16_t>::iterator  
    strip( in.begin() ),
    stripOut ( out.begin() ), 
    end(   in.end()   ),
    endAPV;
  
  while( strip < end ) {
    endAPV = strip+128; tmp.clear();
    tmp.insert(tmp.end(),strip,endAPV);
    const float offset = median(tmp);
    results.push_back( offset );

    while (strip < endAPV) {
      *stripOut = *strip-offset;
      strip++; stripOut++;
    }
  }
}    


void SiStripCMNAnalyzer::subtractIterMed( std::vector<int16_t>& in, std::vector<int16_t>& out, 
                                          std::vector<float>& results, uint32_t detId)
{
   // for now we will do 2-sigma and 3 iterations. This should be given in the configuration.
   
  SiStripNoises::Range detNoiseRange = noiseHandle->getRange(detId);
  SiStripQuality::Range detQualityRange = qualityHandle->getRange(detId);

  std::vector<int16_t>::iterator fs,ls,os;
  os = out.begin();
  float offset = 0;  
  std::vector< std::pair<float,float> > subset;
  subset.reserve(128); 


  for( int16_t APV=0; APV< (int)in.size()/128; ++APV)
  {
    subset.clear();
    // fill subset vector with all good strips and their noises
    for (int16_t istrip=APV*128; istrip<(APV+1)*128; ++istrip)
    {
      if ( !qualityHandle->IsStripBad(detQualityRange,istrip) )
      {
        std::pair<float,float> pin((float)in[istrip], (float)noiseHandle->getNoise(istrip,detNoiseRange));
        subset.push_back( pin );
      }
    }
    // caluate offset for all good strips (first iteration)
    if (subset.size() != 0)
      offset = pairMedian(subset);

    // for second, third... iterations, remove strips over threshold
    // and recalculate offset on remaining strips
    for ( int ii = 0; ii<2; ++ii )
    {
      std::vector< std::pair<float,float> >::iterator si = subset.begin();
      while(  si != subset.end() )
      {
        if( si->first-offset > 2*si->second )  
          si = subset.erase(si);
        else
          ++si;
      }

      if ( subset.size() == 0 ) break;
      offset = pairMedian(subset);
    }        

    // remove offset
    results.push_back(offset);
    fs = in.begin()+APV*128;
    ls = in.begin()+(APV+1)*128;
    while (fs < ls)
    {
      *os = *fs-offset;
      ++os;
      ++fs;
    }
  }

}

void SiStripCMNAnalyzer::subtractPer25( std::vector<int16_t>& in, std::vector<int16_t>& out, std::vector<float>& results)
{
  std::vector<int16_t> tmp;  tmp.reserve(128);
    std::vector<int16_t>::iterator 
    strip( in.begin() ),  
    stripOut( out.begin() ), 
    end(   in.end()   ),
    endAPV;
  
  while( strip < end ) {
    endAPV = strip+128; tmp.clear();
    tmp.insert(tmp.end(),strip,endAPV);
    const float offset = percentile(tmp, 25.);
    results.push_back( offset );

    while (strip < endAPV) {
      *stripOut = *strip-offset;
      strip++; stripOut++;
    }
  }
}




inline float SiStripCMNAnalyzer::median( std::vector<int16_t>& sample ) {
  std::vector<int16_t>::iterator mid = sample.begin() + sample.size()/2;
  std::nth_element(sample.begin(), mid, sample.end());
  if( sample.size() & 1 ) //odd size
    return *mid;
  return ( *std::max_element(sample.begin(), mid) + *mid ) / 2.;
}

inline float SiStripCMNAnalyzer::pairMedian( std::vector<std::pair<float,float> >& sample) {
  std::vector<std::pair<float,float> >::iterator mid = sample.begin() + sample.size()/2;
  std::nth_element(sample.begin(), mid, sample.end());
  if( sample.size() & 1 ) //odd size
    return (*mid).first;
  return ( (*std::max_element(sample.begin(), mid)).first + (*mid).first ) / 2.;
}


inline float SiStripCMNAnalyzer::percentile( std::vector<int16_t>& sample, double pct) {
  std::vector<int16_t>::iterator mid = sample.begin() + int(sample.size()*pct/100.0);
  std::nth_element(sample.begin(), mid, sample.end());
  return *mid;
} 

inline float SiStripCMNAnalyzer::integrateADC ( std::vector<int16_t>& sample, double offset ) {
  float sum = 0.;
  for( std::vector<int16_t>::iterator it = sample.begin(); it != sample.end(); ++it)
  { 
    sum += *it-offset > 0. ? *it-offset : 0. ;
  }
  return sum;
}


int SiStripCMNAnalyzer::digiCount( edm::DetSet<SiStripDigi>& module, int APV )
{
   int total = 0;
  
   for( edm::DetSet<SiStripDigi>::const_iterator di = module.begin(); di != module.end(); ++di )
   {
     if ( (int)di->strip() >= APV*128 && (int)di->strip() < (APV+1)*128 )
       total++;
   }
   return total;
}

void SiStripCMNAnalyzer::fillClusterWidths( edmNew::DetSetVector<SiStripCluster>& clusters, TH1F * hist, int APV )
{
   bool firstDet = true;
   edmNew::DetSetVector<SiStripCluster>::const_iterator it = clusters.begin();
   for ( ; it != clusters.end(); ++it )
   {
     // there should only be one DetSet
     if(firstDet == false ) break;
     firstDet = false;
     for ( edmNew::DetSet<SiStripCluster>::const_iterator clus = it->begin(); clus != it->end(); ++clus)
     {
       int firststrip = clus->firstStrip();
       int laststrip = firststrip + clus->amplitudes().size();
       // only fill clusters that begin  on the current APV
       // do not want to double count clusters stretching over multiple APVs
       if(  firststrip >= APV*128 && firststrip < (APV+1)*128 )
       {
         hist->Fill(laststrip-firststrip);
       }
     }
   }
}

int SiStripCMNAnalyzer::countClusters( edmNew::DetSetVector<SiStripCluster>& clusters, int APV)
{
   int count = 0;
   bool firstDet = true;
   edmNew::DetSetVector<SiStripCluster>::const_iterator it = clusters.begin();
   for ( ; it != clusters.end(); ++it )
   {
     // there should only be one DetSet
     if(firstDet == false ) break;
     firstDet = false;
     for ( edmNew::DetSet<SiStripCluster>::const_iterator clus = it->begin(); clus != it->end(); ++clus)
     {
       int firststrip = clus->firstStrip();
       int laststrip = firststrip + clus->amplitudes().size();
       // only fill clusters that begin  on the current APV
       // do not want to double count clusters stretching over multiple APVs
       if( ( firststrip >= APV*128 && firststrip < (APV+1)*128 ) ||
           ( laststrip >= APV*128 && laststrip < (APV+1)*128 )   )
         count++;
     }
   }
   return count;
}


void SiStripCMNAnalyzer::fillClusterValues( edmNew::DetSetVector<SiStripCluster>& clusters, int * array, int APV )
{

   // Note: Integer array should be from a apvReadout_t struct and must contain at least 128 values!

   bool firstDet = true;
   int position = 0;
 
   // zero out the old points
   for(int i=0 ; i < 128; i++) array[i] = 0;

   edmNew::DetSetVector<SiStripCluster>::const_iterator it = clusters.begin();
   for ( ; it != clusters.end(); ++it )
   {
     // there should only be one DetSet
     if(firstDet == false ) break;
     firstDet = false;
     for ( edmNew::DetSet<SiStripCluster>::const_iterator clus = it->begin(); clus != it->end(); ++clus)
     {
       if( position > 127) break;  // no more than 128 clusters on a strip!
       int firststrip = clus->firstStrip();
       int laststrip = firststrip + clus->amplitudes().size();
       // only fill clusters that are fully or partially on the current APV
       // and make strip numbers relative to the APV
       if( ( firststrip >= APV*128 && firststrip < (APV+1)*128 ) ||
           ( laststrip >= APV*128 && laststrip < (APV+1)*128 )   )
       {
         array[position] = firststrip - APV*128;
         array[position+1] = laststrip - APV*128;
         position += 2;
       }
     }

   }
}

void SiStripCMNAnalyzer::copyAPVReadout( apvReadout_t & src, apvReadout_t & dest )
{
  for( int i = 0; i < 128; i++)
  {
    dest.adc[i] = src.adc[i];
    dest.clustersMedian[i] = src.clustersMedian[i];
    dest.clustersIterMed[i] = src.clustersIterMed[i];
    dest.clustersPer25[i] = src.clustersPer25[i];
  }
  dest.medianOffset = src.medianOffset;
  dest.iterMedOffset = src.iterMedOffset;
  dest.per25Offset = src.per25Offset;
  dest.event = src.event;
  dest.lumi = src.lumi;
  dest.run = src.run;
  dest.detID = src.detID;
  dest.apv = src.apv;
}  


//define this as a plug-in
DEFINE_FWK_MODULE(SiStripCMNAnalyzer);
