// LCIO
#include <EVENT/LCCollection.h>
#include "lcio.h"

// MARLIN
#include "marlin/Exceptions.h"
#include "marlin/Global.h"
#include "marlin/Processor.h"
#include "marlin/VerbosityLevels.h"
#include <boost/python.hpp>

//EUTelescope
#include "EUTelUtility.h"
#include "EUTelEventImpl.h"
#include "EUTelTrack.h"
#include "EUTelState.h"
#include "EUTelTrackAnalysis.h"
// AIDA
#if defined(USE_AIDA) || defined(MARLIN_USE_AIDA)
#include <AIDA/IBaseHistogram.h>
#include <AIDA/IHistogram1D.h>
#include <AIDA/IHistogram2D.h>
#include <AIDA/IProfile1D.h>
#include <marlin/AIDAProcessor.h>
#include <AIDA/IHistogramFactory.h>
#include <AIDA/IProfile2D.h>
#endif // MARLIN_USE_AIDA
#include "EUTelReaderGenericLCIO.h"

namespace eutelescope {

	class  EUTelProcessorEventDisplay : public marlin::Processor {

  	private:
  	DISALLOW_COPY_AND_ASSIGN(EUTelProcessorEventDisplay)
        
    public:

    virtual Processor* newProcessor() {
    	return new  EUTelProcessorEventDisplay;
    }

    EUTelProcessorEventDisplay();

    /** Called at the begin of the job before anything is read.
    * Use to initialize the processor, e.g. book histograms.
    */
    virtual void init();

    /** Called for every event - the working horse.*/
    virtual void processEvent(LCEvent * evt);

   	/** Called after data processing for clean up. **/
		virtual void end();
		std::string _trackInputCollectionName;
		IntVec _events;
	};

    EUTelProcessorEventDisplay gEUTelProcessorEventDisplay;

}
