#ifdef USE_GBL   
#include "EUTelProcessorGBLTrackFit.h"
using namespace eutelescope;

EUTelProcessorGBLTrackFit::EUTelProcessorGBLTrackFit() :
Processor("EUTelProcessorGBLTrackFit"),
_nTrackCand(0),
_beamQ(-1),
_eBeam(4),
_trackCandidatesInputCollectionName("Default_input"),
_tracksOutputCollectionName("Default_output"),
_mEstimatorType() 
{
        _description = "EUTelProcessorGBLTrackFit this will fit gbl tracks and output them into LCIO file. Can also convert to TBMon or ROOT TTree format.";
        registerInputCollection(LCIO::TRACK, "TrackCandidatesInputCollectionName", "Input track candidate collection name",_trackCandidatesInputCollectionName,std::string("TrackCandidatesCollection"));
        registerOutputCollection(LCIO::TRACK,"TracksOutputCollectionName","Output tracks collection name",_tracksOutputCollectionName,std::string("TrackCollection"));
        registerOptionalParameter("BeamCharge", "Beam charge [e]", _beamQ, static_cast<double> (-1));
        registerProcessorParameter("BeamEnergy", "Beam energy [GeV]", _eBeam, static_cast<double> (4.0));
        registerOptionalParameter("GBLMEstimatorType", "GBL outlier down-weighting option (t,h,c)", _mEstimatorType, std::string() );
        registerOptionalParameter("Mode", "Will this processor do the track parameterisation for you. 1 => yes 0 => no ", _mode ,int(1));
        registerOptionalParameter("IncMed", "Do you want to include the medium as addtional scattering", _incMed ,int(0));
        registerOptionalParameter("xResolutionPlane", "x resolution of planes given in Planes", _SteeringxResolutions, FloatVec());
        registerOptionalParameter("yResolutionPlane", "y resolution of planes given in Planes", _SteeringyResolutions, FloatVec());
}

void EUTelProcessorGBLTrackFit::init() {
	try{
		streamlog_out(DEBUG2) << "EUTelProcessorGBLTrackFit::init( )---------------------------------------------BEGIN" << std::endl;
		std::string name("test.root");
		geo::gGeometry().initializeTGeoDescription(name,false);
		EUTelGBLFitter* Fitter = new EUTelGBLFitter();
		Fitter->setBeamEnergy(_eBeam);
		Fitter->setMode(_mode);
        Fitter->setIncMed(_incMed); 
		Fitter->setMEstimatorType(_mEstimatorType);
		Fitter->setParamterIdXResolutionVec(_SteeringxResolutions);
		Fitter->setParamterIdYResolutionVec(_SteeringyResolutions);
		Fitter->testUserInput();
		_trackFitter = Fitter;
		if (!_trackFitter) {
			throw(lcio::Exception("Could not create instance of fitter class."));
		}
		streamlog_out(DEBUG2) << "EUTelProcessorGBLTrackFit::init( )---------------------------------------------END" << std::endl;
	}	
	catch(std::string &e){
		streamlog_out(MESSAGE9) << e << std::endl;
		throw marlin::StopProcessingException( this ) ;
	}
	catch(lcio::Exception& e){
		streamlog_out(MESSAGE9) << e.what() <<std::endl;
		throw marlin::StopProcessingException( this ) ;
	}
	catch(...){
		streamlog_out(MESSAGE9)<< "Unknown exception in init function of EUTelProcessorGBLTrackFit." << std::endl;
		throw marlin::StopProcessingException( this ) ;
	}
}

void EUTelProcessorGBLTrackFit::processRunHeader(LCRunHeader * run) {
//	std::auto_ptr<EUTelRunHeaderImpl> header(new EUTelRunHeaderImpl(run));
//	header->addProcessor(type());
	_chi2NdfVec.clear();
}

void EUTelProcessorGBLTrackFit::processEvent(LCEvent* evt){
	try{
		EUTelEventImpl * event = static_cast<EUTelEventImpl*> (evt); 
		if (event->getEventType() == kEORE) {
			streamlog_out(DEBUG4) << "EORE found: nothing else to do." << std::endl;
			return;
		}else if (event->getEventType() == kUNKNOWN) {
			streamlog_out(WARNING2) << "Event number " << event->getEventNumber() << " in run " << event->getRunNumber() << " is of unknown type. Continue considering it as a normal Data Event." << std::endl;
		}
        EUTelReaderGenericLCIO reader = EUTelReaderGenericLCIO();
        std::vector<EUTelTrack> tracks = reader.getTracks(evt, _trackCandidatesInputCollectionName );
		std::vector<EUTelTrack> allTracksForThisEvent;
		for (auto iTrack = 0; iTrack < tracks.size(); iTrack++) {
            _nTrackCand++;
			EUTelTrack track = tracks.at(iTrack); 
            streamlog_out(DEBUG1)<<"Found "<<tracks.size()<<" tracks for event " << evt->getEventNumber() << "  This is track:  " << iTrack <<std::endl;
            track.print();
			_trackFitter->testTrack(track);  
			std::vector< gbl::GblPoint > pointList;
            try{
                _trackFitter->getGBLPointsFromTrack(track, pointList);
            }catch(std::string &e){
                continue;
            }
			const gear::BField& B = geo::gGeometry().getMagneticField();//We need this to determine if we should fit a curve or a straight line.
			const double Bmag = B.at( TVector3(0.,0.,0.) ).r2();
			gbl::GblTrajectory* traj = 0;
			if ( Bmag < 1.E-6 ) {
				traj = new gbl::GblTrajectory( pointList, false ); 
			}else {
				traj = new gbl::GblTrajectory( pointList, true );
			}
			double  chi2=0; 
			int ndf=0;
			int ierr=0;
			_trackFitter->computeTrajectoryAndFit(traj, &chi2,&ndf, ierr);
			if(ierr == 0 ){
				streamlog_out(DEBUG5) << "Ierr is: " << ierr << " Entering loop to update track information " << std::endl;
				if(chi2 ==0 or ndf ==0){
					throw(std::string("Your fitted track has zero degrees of freedom or a chi2 of 0.")); 	
                }
				track.setChi2(chi2);
				track.setNdf(ndf);
				_chi2NdfVec.push_back(chi2/static_cast<float>(ndf));
				std::map<int, std::vector<double> >  mapSensorIDToCorrectionVec; 
				_trackFitter->getCorr(traj,track, mapSensorIDToCorrectionVec);
				std::map< int, std::map< float, float > >  SensorResidual; 
				std::map< int, std::map< float, float > >  SensorResidualError; 
			}else{
				streamlog_out(DEBUG5) << "Ierr is: " << ierr << " Do not update track information " << std::endl;
				continue;//We continue so we don't add an empty track
			}	
            allTracksForThisEvent.push_back(track);
        }//END OF LOOP FOR ALL TRACKS IN AN EVENT
        outputLCIO(evt, allTracksForThisEvent); 
	}
	catch (DataNotAvailableException e) {
		streamlog_out(MESSAGE0) << _trackCandidatesInputCollectionName << " collection not available" << std::endl;
		throw marlin::SkipEventException(this);
	}
	catch(std::string &e){
//		streamlog_out(MESSAGE9) << e << std::endl;
		throw marlin::SkipEventException( this ) ;
	}
	catch(lcio::Exception& e){
		streamlog_out(MESSAGE9) << e.what() <<std::endl;
		throw marlin::StopProcessingException( this ) ;
	}
	catch(...){
		streamlog_out(MESSAGE9)<<"Unknown exception in processEvent function of EUTelProcessorGBLTrackFit" <<std::endl;
		throw marlin::StopProcessingException( this ) ;
	}
	
}




void EUTelProcessorGBLTrackFit::end() {
	float total = 0;
	double sizeFittedTracks = _chi2NdfVec.size();
	for(size_t i=0; i<_chi2NdfVec.size(); ++i)
	{
		total= total + _chi2NdfVec.at(i);
	}
  float average = total/sizeFittedTracks;
	streamlog_out(MESSAGE9) << "The number of GBL tracks "<< _chi2NdfVec.size() <<" Number of original candidates "<< _nTrackCand <<std::endl;
	streamlog_out(MESSAGE9) << "This is the average chi2 -"<< average <<std::endl;



}

void EUTelProcessorGBLTrackFit::outputLCIO(LCEvent* evt, std::vector<EUTelTrack>& tracks){
    if(!tracks.empty()){
        for(unsigned int i=0 ; i< tracks.size(); i++){
            streamlog_out(DEBUG1)<<"Found "<<tracks.size()<<" track for event " << evt->getEventNumber() <<".  Track number  " << i <<std::endl;
            tracks.at(i).print();
        }
        EUTelReaderGenericLCIO reader = EUTelReaderGenericLCIO();
        reader.getColVec(tracks, evt,_tracksOutputCollectionName);
    }
}

#endif // USE_GBL

