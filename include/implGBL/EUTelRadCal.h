#ifndef EUTELRADCAL_H
#define	EUTELRADCAL_H

#include "EUTelUtility.h"
#include <iostream>
#include "EUTelTrack.h"
#include "EUTelState.h"
#include "EUTelGeometryTelescopeGeoDescription.h"
#include "EUTelGBLFitter.h"
#include "TMath.h"
#include "EUTelBlock.h"

#include <map>

namespace eutelescope {
  //! This class performs all radiation length calculations. 
  /*! This class will use EUTelTrack objects directly. 
   *  Radiation length is stored in terms of blocks. All sensors included in fit are assumed a thin scatterer.
   *  The radiation length in front of each sensor is assumed thick. Integration over the propagated trajectory for thick scatterers is needed.
   *  After this the weighted position with radiation length is calculated to model the thick scatterer by two thin scattering points.
   *  The "integration" is done by getting the needed radiation length and distance from each homogeneous volume.  
   *  These volumes are combined if there is dead material to describe the full scattering by only two scattering points.
   *
   *  Note the total radiation length must be used to calculate the total variance. The the total variance must be split between different points of tracks. 
   */ 


	class  EUTelRadCal {
        public:
            
            //! This will add the scattering information to each state on a track. This comes in the form of blocks. 
            /*!  
             *  @param [in] track
			 *  @param [in] Mode 1/0. If 1 then model all scattering. If 0 then only use thin sensors.
             */

            void setRad(EUTelTrack& track, int& mode);
		private:
			//! Will update the track with the total radiation length and each block with their own rad length.
            void setIncSenBlocks(EUTelTrack & track);
			//! Collect the thickness and radiation length of each homogeneous block
            void setThicknessAndRad(EUTelTrack & track);
			//! Determine the position of the scattering points using the estimated radiation length
            void setRelativePosOfScatterers(EUTelState & state);
			//!  Set the position and variance of the scattering points. This uses the radiation length calculated before.
            void setPosVar(EUTelTrack & track); 
			//! set the variance for each scattering point.  
            void getVarForAllScatters(EUTelTrack & track );
			//! This is used if we only want to estimate the thin scatterers(sensors) only and not the air/dead material
            void getVarForSensorScatterersOnly(EUTelTrack & track );
			void setMeanWeight(EUTelTrack &);
			void setVarWeight(EUTelTrack &);




	};
}
#endif	/* EUTELRADCAL */
