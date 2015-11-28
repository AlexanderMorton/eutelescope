///This is the event display that begins to use the python code avaliable. Note another event display exists see EUTelEventViewer based on MarlinCED
#include "EUTelProcessorEventDisplay.h"
#include <AIDA/IAxis.h>      
using namespace eutelescope;

EUTelProcessorEventDisplay::EUTelProcessorEventDisplay() :
Processor("EUTelProcessorEventDisplay"){
	
	registerInputCollection(LCIO::TRACK, "TrackInputCollectionName", "Input track collection name",_trackInputCollectionName,std::string("TrackCandidatesCollection"));
	registerOptionalParameter("events", "Events to display",_events,IntVec());
}


void EUTelProcessorEventDisplay::init(){
	try{
	}catch(...){	
		streamlog_out(MESSAGE9)<<"There is an unknown error in EUTelProcessorEventDisplay-init()" <<std::endl;
		throw marlin::StopProcessingException( this ) ;
	}

}

void EUTelProcessorEventDisplay::processEvent(LCEvent * evt){
	try{
		EUTelEventImpl * event = static_cast<EUTelEventImpl*> (evt); ///We change the class so we can use EUTelescope functions

		if (event->getEventType() == kEORE) {
			streamlog_out(DEBUG4) << "EORE found: nothing else to do." << std::endl;
			return;
		}else if (event->getEventType() == kUNKNOWN) {
			streamlog_out(WARNING2) << "Event number " << event->getEventNumber() << " in run " << event->getRunNumber() << " is of unknown type. Continue considering it as a normal Data Event." << std::endl;
		}
        streamlog_out(DEBUG2) << "Collection contains data! Continue!" << std::endl;
        EUTelReaderGenericLCIO reader = EUTelReaderGenericLCIO(); streamlog_out(DEBUG2) << "Collection contains data! Continue! line 53" << std::endl;
        streamlog_out(DEBUG2) << "_trackInputCollectionName = " <<_trackInputCollectionName<<std::endl;
        std::vector<EUTelTrack> tracks = reader.getTracks(evt, _trackInputCollectionName);
        streamlog_out(DEBUG2) << "Collection contains data! Continue! line 54: tracks.size() = " << tracks.size()<<std::endl;
        for (int iTrack = 0; iTrack < tracks.size(); ++iTrack){
   //         track.print();
            EUTelTrack track = tracks.at(iTrack); 

        }//for (int iTrack = 0; iTrack < tracks.size(); ++iTrack){
        }catch (DataNotAvailableException e) {
//		streamlog_out(WARNING2) << " Collection not available" << std::endl;
		throw marlin::SkipEventException(this);
	}
	catch(std::string &e){
		streamlog_out(MESSAGE9) << e << std::endl;
		throw marlin::SkipEventException( this ) ;
	}
	catch(lcio::Exception& e){
		streamlog_out(MESSAGE9) << e.what() <<std::endl;
		throw marlin::StopProcessingException( this ) ;
	}
	catch(...){
		streamlog_out(MESSAGE9)<<"Unknown exception in process function of track analysis" <<std::endl;
		throw marlin::StopProcessingException( this ) ;
	}

	
}

void EUTelProcessorEventDisplay::end(){


}

