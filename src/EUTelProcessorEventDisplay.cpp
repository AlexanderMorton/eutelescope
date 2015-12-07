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
        //Intialise modules and sys variable.
        Py_Initialize();
//        PyRun_SimpleString("import sys");
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
            shared_ptr<EUTelTrack> trackPtr (new EUTelTrack);
            boost::python::object main_module = boost::python::import("__main__"); 
            boost::python::object global = main_module.attr("__dict__");
            boost::python::object sys = boost::python::import("sys"); 
            std::string version = extract<std::string>(sys.attr("version"));
            std::cout<< "Here1 " << version  <<std::endl;
//            boost::python::object result = boost::python::exec_file("/afs/phas.gla.ac.uk/user/a/amorton/ilcsoft/v01-17-05/Eutelescope/trunk/bin/eventDisplay.py", global, global);
      //      boost::python::object foo = global["printTest"];
  //          object ignored = exec("result = 5 ** 2", main_namespace);
//            int five_squared = extract<int>(main_namespace["result"]);
//            std::cout << "Num " << five_squared <<std::endl;
            boost::python::object main = boost::python::import("eventDisplay");
           std::cout << "here11" <<std::endl;
            boost::python::object printTest = main.attr("printTest"); 
        //    boost::shared_ptr<EUTelTrack> trackpy(new EUTelTrack);
           std::cout << "here11" <<std::endl;
   //         trackpy->print();
   
             printTest();
         //   printTest( boost::python::ptr(trackpy.get()));
       //     std::string som = extract<std::string>(classEve());

       //    std::cout << "here12" << som << std::endl;

 //           boost::python::object print = classEve.attr("printTest"); 
//            print();


            ///Execute the python object. 
  //          PyObject* pModule =PyImport_ImportModule("eventDisplay");
//            PyObject* pDict = PyModule_GetDict(pModule);
            std::cout << "here" <<std::endl;
     //       PyObject* pClassHelloPython = PyDict_GetItemString(pDict, "eventDisplay");
 //           std::cout <<" here1" <<std::endl;
   //         EUTelTrack track = tracks.at(iTrack); 

        }//for (int iTrack = 0; iTrack < tracks.size(); ++iTrack){
        Py_Finalize();
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
		streamlog_out(MESSAGE9)<<"Unknown exception in process event display" <<std::endl;
		throw marlin::StopProcessingException( this ) ;
	}

	
}

void EUTelProcessorEventDisplay::end(){


}

