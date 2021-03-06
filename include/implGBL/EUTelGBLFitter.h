#ifdef USE_GBL
#ifndef EUTELGBLFITTER_H
#define	EUTELGBLFITTER_H

#include "EUTelUtility.h"
#include "EUTelTrack.h"
#include "EUTelState.h"
#include "EUTelMillepede.h"
#include "EUTelRadCal.h"
#include "EUTelTrackCreate.h"
#include "EUTelBlock.h"
#include "EUTelExcludedPlanes.h"
#include <IMPL/TrackerHitImpl.h>
#include <EVENT/LCCollection.h>
#include <IMPL/LCCollectionVec.h>
#include <IMPL/TrackerHitImpl.h>
#include <IMPL/TrackImpl.h>
#include <IMPL/LCFlagImpl.h>
#include "lcio.h"
#include "LCIOTypes.h"
#include "TMatrixD.h"
#include "include/GblTrajectory.h"
#include "include/MilleBinary.h"
#include <map>
#include <string>
#include <algorithm>
#include <functional>
#include <iostream>

namespace eutelescope {
    //! The GBL class 
    /*!
     *
     */
	class EUTelGBLFitter{
        
    public:
			EUTelGBLFitter();
			~EUTelGBLFitter();
            inline void setBeamEnergy(double beamE) { this->_eBeam = beamE; }
            //! Use internal parameterisation or not.    
            /*! If you only have a set of hits then set to 1.     
             *   
             * @param[in] mode 1 for internal parameterisation.
             */

            inline void setMode(int mode) { this->_mode = mode; }
            //! Set if medium between detector planes should included.  
            /*!     
             *   
             * @param[in] incMed    
             */

            inline void setIncMed(int incMed) { this->_incMed = incMed; }
            //! Pass the binary file to store trajectory.   
            /*!     
             *   
             * @param[in] mille
             */

			void SetMilleBinary(gbl::MilleBinary* mille) { this->_mille = mille; }
            //! The millepede class is passed to determine the global parameters.   
            /*! Could set this up in a better way.    
             *   
             * @param[in] mille
             */

			void setMillepede( EUTelMillepede* mille ) { _MilleInterface =  mille; }
			void setParamterIdXResolutionVec( const std::vector<float>& );
			void setParamterIdYResolutionVec( const std::vector<float>& );

			void setMEstimatorType( const std::string& );
            //! This will create the needed GBL points from EUTelTracks.
            /*! This will create the GBL points from the states and the needed scatterering points.  
             *
             * @param [in]   track EUTelTrack 
             * @param [out]  pointList vector of GBL points
             */

			void getGBLPointsFromTrack(EUTelTrack&, std::vector< gbl::GblPoint >&);
            //! Get the corrections for the GBL trajectories for states.  
            /*!   
             *
             * @param [in]   GBLtraj GBL trajectory 
             * @param [in]   EUTelTrack track to update.
             * @param [out]  corrections This is a map from sensor ID to vector. 
             * @todo Must clean this up.
             */
			void getCorr(gbl::GblTrajectory* , EUTelTrack& ,std::map<int,std::vector<double> >& );
            //! This is the function which links the GBL track to millepede.cc object. 
            /*! Millepede has global labels internally stored for setup and fixed alignment constants. 
             *  Alignment Jacobain is then calculated and attached to each point with labels. 
             *
             * @param [in]   vector GBLpoints  
             * @param [out]   EUTelTrack track to update.
             */
			void getGloPar(std::vector< gbl::GblPoint >& , EUTelTrack&);
            //! This will fit the trajectory and output the chi2 and ndf 
            /*!   
             * @param [in]   GBLtraj GBL trajectory 
             * @param [out]  chi2 
             * @param [out]  ndf Number of degrees of freedom 
             * @param [out]  error code that tells you if there was a problem from GBL.
             */
			void computeTrajectoryAndFit(gbl::GblTrajectory* traj, double* chi2, int* ndf, int & ierr);
			void testUserInput();
			void testTrack(EUTelTrack& track);
    private:
           void  initNav();
           //!Create point from EUTelState 
           /*!This function will add the measurement to the GBL point. 
            * Internally a projection matrix is used to link the global frame to the local one. 
            * @param[in] point GBL point which the measurement will be added. 
            * @param[in] state  This is the EUTelState with the measurement information
            */

			void setMeasurementGBL(gbl::GblPoint& point,EUTelState& );
            //! This function will add the scattering to the GBL point. 
            /*! Track incidence of the state is used to determine local frame covariance matrix due to scattering  
             *
             * @param[in] point GBL point which the measurement will be added to 
             * @param[in] state This is the EUTelState with measurement information
             * @param[in] var The variance at that location.  
             */

			void setScattererGBL(gbl::GblPoint& point,EUTelState & state,double & );

            //! This function will add the scattering to the GBL point. 
            /*! The kink angle and expected variance must be determined before hand.   
             *  Make sure the track is consistant between points! 
             * @param[in] point GBL point which the measurement will be added to 
             * @param[in] Kinks for this point 
             * @param[in] var The variance at that location.  
             */

            void setScattererMedGBL(gbl::GblPoint& point,TVectorD& kinks, double& varMed );

            //! Will set the resolution of a state.  
            /*! This will either use the default input or the geometry of the pixel.    
             *   
             * @param[in] state  
             */

			void setMeasurementCov(EUTelState& state);
            //! This will create the point list which describes each points relation to one another.
            /*! The propagation from plane i to i+1 is saved in point i+1. 
             *  No measurement information is added only the geometric location of the points and there linking together.
             *
             * @param [in] track This is the measurement and scattering planes. These must have Local->Global relation. 
             * @param [out] pointList This is the points which describe the EUTelTrack without any measurements (Scattering and residuals) 
             */
            void getBasicList( EUTelTrack & ,std::vector< gbl::GblPoint >& pointList);   

            //! Add measurement to the trajectory. Measurement in local frame.
            /*! Note local frame is defined as the surface of the sensor. The measurement frame is the frame in which the covariance matrix is diagonal.  
             *  All uncertainties are diagonalised internally by the GBL algorithm implemented.
             * @param [in]   track EUTelTrack 
             * @param [in]  pointList vector of GBL points
             * @param [in]  linkGL Link between points with local->global transform and GBL point 
             * @param [out]  linkMeas Link between states with a measurement for position and GBL point.
             */

            void getMeas(EUTelTrack&, std::vector< gbl::GblPoint >&);

            //! Add Scattering information to GBL trajectories. 
            /*! Kinks are in local frame since they are the same as adding a residual measurement on a plane. 
             *  All planes with no global to local transform will be assumed to have normal incidence on a plane. 
             *  Note here that kink angles are always measured in the local frame and therefore the precision matrix of those measurements.
             *  The kink angle precision matrix is non diagonal in the local frame but in the frame in which the measurement of kink is on a plane perpendicular to the track
             *  it is diagonal.
             *  Internally this transform takes place. Note this transform is not between the local and global frame. It is between the frame perpendicular to the track 
             *  and plane of incidence. Transforms between frames will make no difference it is the relative incidence which counts.
             *  @param [in]   track EUTelTrack 
             *  @param [in]  pointList vector of GBL points
             */
            void getScat(EUTelTrack&, std::vector< gbl::GblPoint >&);

            //! Add local parameters to plane 271 to determine kink angles at that point. 
            /*!  
             * @param [in]   vector GBLpoints  
             * @param [in]   EUTelTrack track to update.
             */
            void getLocalKink(EUTelTrack& track, std::vector< gbl::GblPoint >& pointList);
			inline double getBeamEnergy() const { return _eBeam; }
            int _numberRadLoss;
			double _eBeam;
            int _mode;
            int _incMed;
            double _dutDistCut;
			std::string _mEstimatorType;
			gbl::MilleBinary* _mille;
			std::string _binaryname;
			std::map< int,  float> _parameterIdXResolutionVec;
			std::map< int,  float> _parameterIdYResolutionVec;
			std::map<int,int> _parameterIdXShiftsMap;
			std::map<int,int> _parameterIdYShiftsMap;
			std::map<int,int> _parameterIdZShiftsMap;
			std::map<int,int> _parameterIdXRotationsMap;
			std::map<int,int> _parameterIdYRotationsMap;
			std::map<int,int> _parameterIdZRotationsMap;
			EUTelMillepede* _MilleInterface;
			DISALLOW_COPY_AND_ASSIGN(EUTelGBLFitter)        

        
    };
}
#endif	/* EUTELGBLFITTER_H */
#endif
