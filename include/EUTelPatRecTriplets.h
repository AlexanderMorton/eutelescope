#ifndef EUTelPatRecTriplets_H
#define	EUTelPatRecTriplets_H

// ROOT
#include "TVector3.h"
#include "TVectorD.h"
#include "TMatrixD.h"
#include "TMatrixDSym.h"

// system includes <>
#include <iostream>
#include <functional>
#include <algorithm>
#include <string>
#include <vector>
#include <cmath>

// EUTELESCOPE
#include "EUTelUtility.h"
#include "EUTelGeometryTelescopeGeoDescription.h"
#include "EUTelTrack.h"
#include "EUTelState.h"
#include "EUTelNav.h"
#include "EUTelTrackCreate.h"
#include "EUTelExcludedPlanes.h"
//LCIO
#include "lcio.h"
#include "IMPL/TrackerHitImpl.h"
#include "IMPL/TrackImpl.h"
#include <UTIL/LCTOOLS.h>

//other
#include "streamlog/streamlog.h"
#include "gear/gearimpl/Vector3D.h"

namespace eutelescope {
  //! Triplet/doublet finder class. 
  /*! The class performed pattern recognition using triplet and doublets constructed from mimosa hits 
   *  Triplets are a collection of three hits and doublets two on different planes. 
   *  These hits are associated together using geometric distances between hits. 
   *  There are two modes:Alignment and high stats. 
   *
   *  The alignment mode will construct two triplets on each arm. Attach DUT hits then return the track.
   *  This will ensure that the tracks passed to alignment are of good quality but the number of tracks found will be smaller than expected.
   *
   *  High stats mode will construct doublets from two planes and then interpolate between these planes to search for new hits.
   *  This mode will produce track of a lower quality but will find more tracks.   
   *
   *  This form of pattern recognition is the limit of what can be done without use of measurement errors. 
   *  It has been test in high occupancy situation (SLAC testbeam) and performs well. 
   *  For more complex situations a Kalman filter of some form or other can be used. 
   *
   */ 

	class EUTelPatRecTriplets {
    public: 
    EUTelPatRecTriplets();
    ~EUTelPatRecTriplets();

    //! Struct object 
    /*! This contains the informtion needed to construct a track from two hits. 
     */

    struct doublets {
        std::vector<float> pos;
        std::vector<double> slope;
        std::vector<double> diff;
        std::vector<EUTelHit> hits;

    }; 
    //! Struct object 
    /*! This contains the informtion needed to construct a track from two hits. 
     *  It also contains information in associating triplet together
     */

    struct triplets {
        /// The central plane of the triplet (1 or 4) is saved for identification
        unsigned int cenPlane;
        ///How many times this triplet has been linked to another on the other arm
        unsigned int matches;
        ///Just an ID given to link two triplets together. 
        unsigned int fitID;

        std::vector<double> pos;
        std::vector<double> slope;
        std::vector<double> diff;
        std::vector<EUTelHit> hits;
    }; 
    //! Set in the correct z order if we are dealing with a strip sensor or pixel sensor.  
    /*! 
     *  A strip sensor is 1 and pixel 2. See GBL example for use in EUTelescope.
     *  @param[in] planeDimenstions This is a vector of 1s and 2s. 1=>strip, 2=>pixel. 
     */
    ///\todo Could infer this from resolution. 

    void setPlaneDimensionsVec(EVENT::IntVec& planeDimensions);
    inline int getEventNumber()	const {
        return _eventNumber;
    }
    //! Create vector internally with ID and hits vector. 
    /*!  
     * @param[in] allHitsVec a simple vector of hits 
     */

    inline void setHitsVec(EVENT::TrackerHitVec& allHitsVec){
        _allHitsVec = allHitsVec;
    }
    void setHitsVecPerPlane();

    void setEventNumber(int eventNumber){
        _eventNumber = eventNumber;
    }
    inline void setTripletSlopeCuts(std::vector<float> cuts ){
        this->_tripletSlopeCuts = cuts;
    };

    inline void setDoubletDistCut(std::vector<float> cuts) {
        this->_doubletDistCut = cuts;
    }

    inline void setTripletConnectDistCut(std::vector<float> cuts) {
        this->_tripletConnectDistCut = cuts;
    }
    inline void setDoubletCenDistCut(std::vector<float> cuts) {
        this->_doubletCenDistCut = cuts;
    }

    inline void setBeamMomentum(double beam) {
        this->_beamE = beam;
    }
    inline void setMode(double mode) {
        this->_mode = mode;
    }
    inline void setNumHits(double hitNum) {
        this->_hitNum = hitNum;
    }

    //get public
    
    //! The function should be called in your processor to return EUTelTracks 
    /*! This function will do all the work for you and return the track reconstructed. 
     *  These are ready for use in GBL and can be saved to LCIO with EUTelReaderGenericLCIO. 
     *  See EUTelProcessorPatRecTriplets.cpp for details on how to construct and store tracks.
     *  @return tracks A vector of tracks. These tracks are ready to use for GBL fitting
     */

    std::vector<EUTelTrack> getTracks();
    private:
    //! This will place DUT hit in track. 
    /*! 
     *  @param[in] tripLeft Triplet from upstream planes. 
     *  @param[in] tripRight Triplet from downstream planes
     *  @return track EUTelTrack from triplets 
     */

    EUTelTrack getTrack(triplets tripLeft,triplets tripRight);
    //! Will create triplet from doublet and hit if passes cut.  
    /*! 
     *   Hit must come from plane in the middle of the doublet  
     *   Cut made on distance of central hit to doublet prediction
     *
     *  @param[in] doublet doublet to interpolate between hits 
     *  @param[in] hit possible hit to create triplet from.
     *  @return pass Has the cut been passed.
     */

    bool getTriplet(doublets&, EUTelHit &, triplets&  );
    //! Will create doublet from hits if passes distance cut.  
    /*! 
     * The doublet included all the information need to parameterise the track fro mthe two hits.
     *  @param[in] hit Hit on plane down stream 
     *  @param[in] cuts Pass the cuts so this can be varied at different positions in code.
     *  @param[in] hit Hit on plane upstream.
     *  @param[out] doublet doublet constructed from hits.
     *  @return pass Has the cut been passed.
     */

    bool getDoublet( EUTelHit const  &, EUTelHit const&, std::vector<float> const & doubletDistCut ,doublets&);
    //! Predict global position using triplet at certain z position.  
    /*! 
     *  
     *  @param[in] trip Triplet to extrapolate from 
     *  @return posZ Position to extrapolate to.
     */

    std::vector<float>  getTripPosAtZ(triplets trip, float posZ );
    //! Predict global position using doublet at certain z position.  
    /*! 
     *  
     *  @param[in] doub Doublet to extrapolate from 
     *  @return posZ Position to extrapolate to.
     */
    std::vector<float>  getDoubPosAtZ(doublets doub, float posZ);

    //Get functions.

    //! Create map between fitID and triplets after finding the triplets.
    /*! Hits are linked by removing the difference in position relative to curvature for doublet and triplet creation.
     *  Cuts are performed on the distance between hits on planes. See GBL examples for more information. 
     *  The fit ID is a unique number for each triplet. Two maps are created for the left and right arm
     *
     *  @param[out] mapTripLeft Map between fitID and up stream triplets.
     *  @param[out] mapTriplRight Map between fitID and down stream triplets.
     */
    void getTriplets(std::map<unsigned int, triplets >& mapTripLeft , std::map<unsigned int, triplets>& mapTripRight);
    //! This function will find the matches between left and right triplets.
    /*! Extraploate the triplets on the left and right hand side to a central point.
     *  Then if the triplets have a similar slope and are at roughly the same location then match them.
     *  The matching is done using the IDs of the triplets. So we have a map from each left hand triplet ID to a vector of all right hand triplets matched. 
     *  @param[in] mapTripLeft Map between fitID and up stream triplets.
     *  @param[in] mapTriplRight Map between fitID and down stream triplets.
     *  @return mapLeftToRight This map which takes the left triplets IDs and returns a vector of right triplets IDs matched.
     */  

     std::map<unsigned int, std::vector<unsigned int> > getMatchLeftToRight( std::map<unsigned int, triplets >&, std::map<unsigned int, triplets>&);

    //! The triplets on each plane are passed. 
    /*! If they pass a extraplolated postion/slope comparison then form a track. 
     *  If there is more than 1 match for a triplet remove triplet.  
     *  @param[in] mapTripLeft map between triplet ID and triplets of arm up stream 
     *  @param[in] mapTripRight map between triplet ID and triplet down stream
     *  @return hitList Vector of vectors of hits for each track. 
     */
    std::vector<std::vector<EUTelHit> >  getTrackHitsFromTriplets( std::map<unsigned int, triplets >& mapTripLeft , std::map<unsigned int, triplets>& mapTripRight);
    /// Will return hits in the correct z order. 
    /**
     * \param[in] hits Not correct in Z
     * \return -cZxB The correct order of hits returned. 
     */
    std::vector<EUTelHit> getCorrHitOrder(std::vector<EUTelHit> hits );
    //! This will place DUT hit in track. 
    /*! 
     *  @param[in] track track without DUT hits. 
     *  @param[in] hits DUT hits to add to hit.
     *  @return hits Hits in the correct order with DUT hits. 
     */

    std::vector<EUTelHit> getDUTHitsOrder(EUTelTrack track, std::vector<EUTelHit> dutHit );
    //! Get all hits which the predicted doublet passes  
    /*! 
     *  A predicted track from the doublet is constructed. At each sensor the closest hit is taken if included in list in arguments.
     *  A cut is made on the number of hits in the final track. If less than this the bool returned in false.
     *  @param[in] doub Doublet to parameterise track from.
     *  @param[in] sen list of sensor to collect hits from.
     *  @param[in] hitNum Number of hits as minimum needed to pass.
     *  @param[out] newHits list of hits found. WILL NOT ATTACH ORIGINAL DOUBLET HITS
     *  @return pass If we have enough hits?
     */
    bool getDoubHitOnTraj( doublets const& doub, std::vector<unsigned int> const & sen,int const & hitNum, std::vector<EUTelHit>& newHits   );


    //! Return the distance between hit position and prediction.  
    /*! 
     *  This comparision MUST be done in the local frame(WITH NO OFFSETS!). This is done since we only have x information in the local direction for strips sensors 
     *  There we must rotate out frame to line up with the local frame.
     *  @param[in] itHit Iterator of hit object 
     *  @param[in] pos Position of prediction. 
     *  @return  dist This is the distance between the prediction and measurement in the local frame.
     */

    float getDistLocal(std::vector<EUTelHit>::iterator itHit, std::vector<float>& pos);
    //! Will run the pattern recognition in alignment mode  
    /*! 
     *  @return tracks Tracks ready for use create using strict cuts for alignment. 
     */
    ///\todo DUT hits here are added using parameterisation without slope changes. This could be improved with use in magnetic fields.  

    std::vector<EUTelTrack> getMinFakeTracks();

    inline double getBeamMomentum() const {
        return _beamE;
    }
    DISALLOW_COPY_AND_ASSIGN(EUTelPatRecTriplets) // prevent users from making (default) copies of processors
	public:
    //OTHER
    void printTrackQuality( std::vector<EUTelTrack>&);
    void printHits();
    void testHitsVecPerPlane();
    void testUserInput();
    protected:
    //VARIABLES
    int _eventNumber;
    int _totalNumberOfHits;
    std::map< int, int > _planeDimensions;
    unsigned int _numberTripletsLeft;
    unsigned int _numberTripletsRight;
    ///Member variables. public for now.
    std::vector<float> _doubletDistCut;
    std::vector<float> _doubletCenDistCut;
    std::vector<float> _tripletConnectDistCut;
    std::vector<float> _tripletSlopeCuts;
    std::map<int ,std::vector<EUTelHit> > _mapHitsVecPerPlane;
    ///This will store all the hits for a single event.
    EVENT::TrackerHitVec _allHitsVec; 
    /** Beam momentum [GeV/c] */
    double _beamE;
    int _mode;
    int _hitNum;
    public:
    unsigned int _numberOfTracksTotal;

		
	};

} // namespace eutelescope

#endif	/* EUTelPatRecTriplets_H */

