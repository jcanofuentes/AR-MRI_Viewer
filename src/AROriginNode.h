#ifndef _AROriginNode_H_
#define _AROriginNode_H_

#include "ARToolKitPlus/TrackerSingleMarker.h"

#include "Ogre3d/include/OgreSceneNode.h"

#include "common/MathUtils.h"

#include "common/eString.h"
#include "audio/SoundFMOD.h"

#include "Axis.h"

class AROriginNode
{
public:

	// Constructor
	AROriginNode();
	~AROriginNode(){};

	//Methods
	void setActive ( bool activationFlag );
	void setTotalActivation ( bool activationFlag );
	void updateTracking( const Cing::Vector& position, const Cing::Quaternion& orientation ); // Called when the asocciated marker is founded

	void setVisible		( bool visible ) { m_gizmoAxis.setVisible(visible); }

	//Members
	Axis							m_gizmoAxis;		///< The visual gizmo
	Ogre::SceneNode*				m_sceneNode;
	Ogre::Vector3					m_position;
	Cing::Average*					m_avgx;
	Cing::Average*					m_avgy;
	Cing::Average*					m_avgz;
	Ogre::Vector3					m_scale;
	Ogre::Quaternion				m_orientation;

	float							m_energy;							// Energy grow over time
	float							m_diffEnergy;
	float							m_energyRatio;
	float							m_maxEnergy ;
	float							m_val;
	bool							m_isActive;						// is active?
	bool							m_fadeIn;

	Cing::SoundFMOD*				m_activateSound;			// To play a sound
	Cing::SoundFMOD*				m_deActivateSound;		// To play a sound


	bool										m_fadeOut;
	bool										mTotalActivation;

};

#endif // _AROriginNode_H_
