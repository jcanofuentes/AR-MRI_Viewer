#include "ARMarkerObject.h"

#include "common/Exception.h"
#include "common/SystemUtils.h"

#include "graphics/Box.h"

#include "Ogre3d/include/OgreSceneNode.h"


/**
* @internal
* @brief Constructor. Initializes class attributes.
*/
ARMarkerObject::ARMarkerObject() :	m_bIsValid( false ), m_detected(false)
{
}

/**
* @internal
* @brief Destructor. Class release.
*/
ARMarkerObject::~ARMarkerObject()
{
	// Release resources
	end();
}

/**
*	
* @brief Init resources
*/
bool ARMarkerObject::init( int id )
{	

	m_id = id;

	m_gizmoAxis.init(1,1,1);

	/*
	Cing::Box model;
	model.init(27,27,1);

	model.getSceneNode()->getParentSceneNode()->removeChild(model.getSceneNode());
	model.setMaterial("AR_DebugModel");

	m_gizmoAxis.getSceneNode()->addChild(model.getSceneNode());
	model.getSceneNode()->setInheritScale(false);	
	*/

	m_sceneNode = m_gizmoAxis.getSceneNode();

	m_avgx = new Cing::Average(4);
	m_avgy = new Cing::Average(4);
	m_avgz = new Cing::Average(4);

	// Allow model clicking
	m_gizmoAxis.getEntity()->setUserAny( Ogre::Any((ARMarkerObject*)this ) );
	m_gizmoAxis.getSceneNode()->setUserAny ( Ogre::Any((ARMarkerObject*)this ) );
	m_bIsValid = true;

	return true;
}

/**
* @brief Update 
*/
void ARMarkerObject::update()
{

}

/**
* @brief Sets the new position of the object.
* @param[in] pos new absolute position of the object in the scene
*/
void ARMarkerObject::setPosition( const Cing::Vector& pos )
{
	m_gizmoAxis.setPosition( pos.x, pos.y, pos.z );
}
/**
 * @brief Sets the new orientation of this object.
 *
 * @param orientation new orientation of this object. It is represented in a quaternion, this is, 
 * a rotation around an axis
 */
void ARMarkerObject::setOrientation( const Cing::Quaternion& orientation )
{
	m_gizmoAxis.setOrientation( orientation );
}

/**
 * @brief Sets the new position of the object.
 * @param[in] pos new absolute position of the object in the scene
 */
void ARMarkerObject::averagePosition( const Cing::Vector& pos )
{
	/*
	// Update and filter position
	m_avgx->addValue(pos.x);
	m_avgy->addValue(pos.y);
	m_avgz->addValue(pos.z);

	m_gizmoAxis.setPosition( m_avgx->getValue(), m_avgy->getValue(), m_avgz->getValue() );
*/
	m_gizmoAxis.setPosition(pos);
}
/**
 * @brief Sets the new orientation of this object.
 *
 * @param orientation new orientation of this object. It is represented in a quaternion, this is, 
 * a rotation around an axis
 */
void ARMarkerObject::averageOrientation( const Cing::Quaternion& orientation )
{
	Ogre::Quaternion newRot = Ogre::Quaternion::Slerp( 0.89,  orientation, m_sceneNode->getOrientation(), false);
	m_sceneNode->setOrientation(newRot);
	//m_sceneNode->setOrientation(orientation);
}

/**
* @brief Sets the new scale of the object.
* @param[in] scale vector containing the scale in the three axis of the object
*/
void ARMarkerObject::setScale( const Cing::Vector& scale )
{
	m_gizmoAxis.setScale( scale );
}

/**
* @brief Add a child to this object
* @param[in] child new child
*/
void ARMarkerObject::addChild( ARMarkerObject* child )
{
	m_sceneNode->addChild( child->getSceneNode() );
}
/**
* @internal
* @brief Releases the class resources. 
* After this method is called the class is not valid anymore.
*/
void ARMarkerObject::end()
{
	// Check if the class is already released
	if ( !isValid() )
		return;

	// The class is not valid anymore
	m_bIsValid = false;
}
