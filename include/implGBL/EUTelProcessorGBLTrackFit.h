#ifdef USE_GBL

// C++
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <cstdio>
#include <algorithm>

// LCIO
#include <EVENT/LCCollection.h>
#include "lcio.h"
//#include "LCIOSTLTypes.h"

// MARLIN
#include "marlin/Exceptions.h"
#include "marlin/Global.h"
#include "marlin/Processor.h"
#include "marlin/VerbosityLevels.h"

//GBL
#include "include/GblTrajectory.h"
//EUTelescope
#include "EUTelUtility.h"
#include "EUTelGeometryTelescopeGeoDescription.h"
#include "EUTelRunHeaderImpl.h"
#include "EUTelGBLFitter.h"
#include "EUTelExceptions.h"
#include "EUTelEventImpl.h"
#include "EUTelHistogramManager.h"
#include "EUTelReaderGenericLCIO.h"
#include "EUTelExcludedPlanes.h"


namespace eutelescope {

	class EUTelProcessorGBLTrackFit : public marlin::Processor {

        private:
			DISALLOW_COPY_AND_ASSIGN(EUTelProcessorGBLTrackFit)
						
		public:

			virtual Processor* newProcessor() {
					return new EUTelProcessorGBLTrackFit;
			}

			EUTelProcessorGBLTrackFit();
			virtual void init();
			virtual void processRunHeader(LCRunHeader* run);
			virtual void processEvent(LCEvent * evt);
			virtual void end();

    private:
            //!Set true if you just pass unparameterised tracks.    
            /*!A sensible seed track must be created to propagate errors.
             * This can be done internally or the external input can be used.  
             */
            int _mode;
            //!Homogeneous mediums can be excluded.    
            /*!Only the thin scatterers are used in the medium estimate. Useful to exclude poorly calibrated(Guessed!) medium estimate.
             *   
             */

            int _incMed;
			int _nTrackCand;
			double _beamQ;
			double _eBeam;
            ///A silly old check.
			std::vector<float> _chi2NdfVec;
			EUTelMillepede* _Mille;
			std::string _trackCandidatesInputCollectionName;
			std::string _tracksOutputCollectionName;
            //!Apply variable weights to estimate Cauchy/Huber/Tukey influence function. Should be used with noisey data.     
            /*!Different combinations of the function can be applied. As an example, "hc" =>Cauchy then Huber applied. 
             *   
             */

			std::string _mEstimatorType;
            //!If this is applied then the resolution is not calculated internally.      
            /*!The TGeo object is used directly. 
             *   
             */
			FloatVec _SteeringxResolutions;
			FloatVec _SteeringyResolutions;
			EUTelGBLFitter *_trackFitter;
			void outputLCIO(LCEvent* evt, std::vector< EUTelTrack >& tracks);

    };

/** A global instance of the processor */
EUTelProcessorGBLTrackFit gEUTelProcessorGBLTrackFit;

}

#endif // USE_GBL

