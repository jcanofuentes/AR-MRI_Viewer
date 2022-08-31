#include "AROriginNode.h"
#include "Ogre3d/include/OgreSceneManager.h"
#include "Ogre3d/include/OgreBillboardSet.h"
#include "Ogre3d/include/OgreBillboard.h"
#include "common/CommonUserApi.h"
//#include "MovableText.h"

AROriginNode::AROriginNode( )
{

	m_gizmoAxis.init(1,1,1);
	m_gizmoAxis.setScale(5,5,5);
	m_gizmoAxis.setPosition(-9999,0,0);

	m_sceneNode		= m_gizmoAxis.getSceneNode();

	m_orientation   = Ogre::Quaternion::IDENTITY;
	m_scale         = Ogre::Vector3::UNIT_SCALE;

	m_energy		= 0.0f;
	m_diffEnergy	= 0.05f;
	m_energyRatio   = 5.0;  // if the object is found, it adds "m_energyRatio" times more energy than the previously substracted
	m_maxEnergy		= 1.0f;

	m_val			= 0.0f;

	m_activateSound   = new Cing::SoundFMOD;
	m_activateSound->load( "sonido2.wav" );
	m_activateSound->setVolume(0.4);

	m_deActivateSound = new Cing::SoundFMOD;
	m_deActivateSound->load( "sonido4.wav" );
	m_deActivateSound->setVolume(0.4);

	m_avgx = new Cing::Average(5);
	m_avgy = new Cing::Average(5);
	m_avgz = new Cing::Average(5);

	m_fadeIn	= false;
	m_fadeOut	= false;
	m_isActive	= false;

	//m_sceneNode->setPosition( -999999, -999999, -999999 );

	mTotalActivation = true;
};

void AROriginNode::setActive ( bool activationFlag )
{
	if (activationFlag)
	{
		m_isActive = true;
		m_fadeOut  = false;
		m_fadeIn   = true;
		m_energy   = 0.0f;
		m_val      = 0.0f;
	}else{
		m_isActive = false;
		m_fadeOut  = true;
		m_fadeIn   = false;
		m_energy   = 0.0f;
		m_val      = 0.0f;
	}
};
void AROriginNode::setTotalActivation ( bool activationFlag )
{
	mTotalActivation = activationFlag;
	setActive(activationFlag);

	if (mTotalActivation == TRUE)
	{
		m_sceneNode->setVisible(true);
	}

	if (mTotalActivation == FALSE)
	{
		m_sceneNode->setVisible(false);
		m_sceneNode->setScale( .01, .01, .01 );
		m_sceneNode->setPosition( -999999, -999999, -999999 );
	}

};


void AROriginNode::updateTracking ( const Cing::Vector& position, const Cing::Quaternion& orientation )
{
	//Cing::println("x: %f   y: %f   z: %f " , position.x, position.y, position.z );

	if (orientation.w <1.0 && orientation.x <1.0 && orientation.y <1.0 && orientation.z <1.0)
	{
		if (orientation.w != 0.0 && orientation.x != 0.5 && orientation.y != 0.0 && orientation.z != 0.0)
		{
			// Add energy to founded tracked models
			float energy = m_diffEnergy * m_energyRatio;
			if ((m_energy - energy) <= (m_maxEnergy - energy)) 
				m_energy    += energy;

			// Update and filter position
			m_avgx->addValue(position.x);
			m_position.x = m_avgx->getValue();
			m_avgy->addValue(position.y);
			m_position.y = m_avgy->getValue();
			m_avgz->addValue(position.z);
			m_position.z = m_avgz->getValue();

			// Update and filter orientation
			m_orientation = Ogre::Quaternion::Slerp( 0.91, orientation, m_orientation, true);

		}
	}

	// Update position and orientation
	m_sceneNode->setPosition(m_position);
	m_sceneNode->setOrientation(m_orientation);

}

