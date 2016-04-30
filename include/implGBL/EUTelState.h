#ifndef EUTELSTATE_H
#define	EUTELSTATE_H

#include "EUTelUtility.h"
#include "EUTelBlock.h"

// ROOT
#if defined(USE_ROOT) || defined(MARLIN_USE_ROOT)
#include "TVector3.h"
#include "TVectorD.h"
#include "TMatrixD.h"
#include "TMatrixDSym.h"
#endif
#include "EUTelHit.h"
#include "EUTelGeometryTelescopeGeoDescription.h"
//Eigen
#include <Eigen/Core>

namespace eutelescope {
	//!   The state class
	/*!   A state holds the prediction and measurement information of specific track locations.
	*    The measurement information is defined as the hit locations and kink angles.
	*    Each measurement will have associated errors which must be estimated so these errors can be propagated onto the estimated track parameters.
	*    The radiation length is calculated as a block which contains all the information needed to approximate the thick scatterers as thin
	*/
	class  EUTelState {
	public:
		EUTelState();
		EUTelState(EUTelState *state);
		//! This stores all the information needed to approxiamte the radiation length through to the next state.
		Block block;
		//!  This is a store of the GBLLabels.
		/*!  Each state contains a GBL point for the sensor and two for scattering afterwards.
		 *   The homogeneous nature of the air in front of the sensor makes it easier to model with two different points.
		 */
		std::vector<unsigned int> GBLLabels;
		//! Get the single hit associated to the state. This will return a refererence to be manipulated.
		EUTelHit& getHit();
		//! If you want a copy of the hit.
		EUTelHit getHitCopy() const;
		//! This is used to store if the sensor is a strip or pixel sensor.
		/*! \todo Should be done by geometry.
		*/
		int getDimensionSize() const;
		//! This will the state describe by (q/p,x,y,slopeX,slopeY) in local frame
		TVectorD getStateVec();
		float getSlopeX() const;
		float getSlopeY() const;
		float getSlopeXGlobal() const;
		float getSlopeYGlobal() const;
		//! A very basic serializer. Using csv producer also exists.
		std::vector<double> getLCIOOutput();
		float getArcLengthToNextState() const { return _arcLength; }
		TVector3 getPositionGlobal() const;
		Eigen::Vector3d getPositionGlobalEig() const;
		const bool getStateHasHit() const;
		//! This will get the link between the local and global frames.
		/*! GBL Fitter expects the link to be give from global to local.
		 * MUST INVERT FOR USE!!
		 *
		 * \param [out] Projection Calculated using the incidence in the global frame and normal of the senso
		 */

		TMatrixD getProjectionMatrix() const;
		//! The scattering covariance in the local frame.
		/*! The scattering covariance matrix is only diagonal in the frame perpendicular to the track direction.
		 */
		TMatrixDSym getScatteringVarianceInLocalFrame(double const & var);
		double getRadFracAir() const;
		double getRadFracSensor() const;
		//! The direction of the track in the local frame.
		TVector3 getDirLocal() const;
		TVector3 getDirGlobal() const;
		Eigen::Vector3d getDirGlobalEig() const;
		//! The kinks on the sensors
		TVectorD getKinks() const;
		//! The kinks at the position used to model the thick scattering.
		TVectorD getKinksMedium1() const;
		//! The kinks used to model the second scattering position.
		TVectorD getKinksMedium2() const;
		const double* getPosition() const;
		int	getLocation() const;
		float getDirLocalX() const { return _dirLocalX; }
		float getDirLocalY() const { return _dirLocalY; }
		float getDirLocalZ() const { return _dirLocalZ; }
		TMatrixDSym getCov();
		//! Get the residual in the local frame.
		std::vector<double> getRes() { return std::vector<double>{this->getPosition()[0] - this->getHit().getPosition()[0], this->getPosition()[1] - this->getHit().getPosition()[1]}; }
		//! Set the hit to be stored for this state.
		void setHit(EUTelHit hit);
		//! Note will be turned to a EUTelHit internally.
		void setHit(EVENT::TrackerHit* hit);
		void setDimensionSize(int dimension);
		//! The sensor ID
		void setLocation(int location);
		void setDirLocalX(double dirX);
		void setDirLocalY(double dirY);
		void setDirLocalZ(double dirZ);
		void setDirFromGloSlope(std::vector<double> slopes);
		void setDirFromLocSlope(std::vector<double> slopes);
		void setLocalDirGlobalDir(TVector3 momentumIn);
		void setTrackFromLCIOVec(std::vector<double> input);
		void setPositionLocal(float position[]);
		void setPositionLocal(double position[]);
		void setPositionGlobal(float positionGlobal[]);
		void setPositionGlobal(double positionGlobal[]);
		void setStateUsingCorrection(TVectorD& corrections, TMatrixDSym& cov);
		void setArcLengthToNextState(float arcLength) { _arcLength = arcLength; }
		void setKinks(TVectorD kinks);
		void setKinksMedium1(TVectorD kinks);
		void setKinksMedium2(TVectorD kinks);
		void setRadFrac(double plane, double air);
		void setCov(TMatrixDSym cov);

		//print
		void print();
		//! Clear all information. This is used in pattern recognition.
		void clear();

		bool operator<(const EUTelState compareState) const;
		bool operator==(const EUTelState compareState) const;
		bool operator!=(const EUTelState compareState) const;

	private:
		EUTelHit _hit;
		int _dimension;
		int _location;
		double _position[3];
		bool _stateHasHit;
		TVectorD _kinks;
		TVectorD _kinksMedium1;
		TVectorD _kinksMedium2;
		TMatrixDSym _cov;
		double _dirLocalX;
		double _dirLocalY;
		double _dirLocalZ;
		double _radFracSensor;
		double _radFracAir;
		float _arcLength;
	};
}
#endif
