#ifndef _ARMarkerObject_H_
#define _ARMarkerObject_H_

#include "Axis.h"

// Cing
#include "common/CommonUserAPI.h"
#include "common/MathUtils.h"
// Ogre
#include "Ogre3d/include/OgreEntity.h"			///> To allow mouse picking

class ARMarkerObject
{
public:

	// Constructor / Destructor
	ARMarkerObject();
	~ARMarkerObject();

	// Init / Release
	bool init( int id );
	void end();

	// Update
	void update();
	
	// Control hierarchy methods
	void addChild( ARMarkerObject* child );

	// Set methods
	void setPosition	( const Cing::Vector& pos );
	void setOrientation	( const Cing::Quaternion& orientation );
	void setScale		( const Cing::Vector& scale );
	void setVisible		( bool visible ) { m_gizmoAxis.setVisible(visible); }

	void averagePosition    ( const Cing::Vector& pos );
	void averageOrientation ( const Cing::Quaternion& orientation );

	void setDetected	( bool detected )		{ m_detected = detected; }

	// Query methods
	bool isValid	() const { return m_bIsValid; }
	bool isDetected	() const { return m_detected; }
	int  getId		() const { return m_id; }

	Ogre::SceneNode*	getSceneNode () { return m_sceneNode; }



private:

	int					m_id;				///< The id of the marker
	Axis				m_gizmoAxis;		///< The visual gizmo
	Ogre::SceneNode*	m_sceneNode;		///< Node in the scene manager where the object is placed
	bool				m_detected;			///< Indicates whether the marker is tracked from last frame or not  
	Cing::Average*		m_avgx;
	Cing::Average*		m_avgy;
	Cing::Average*		m_avgz;
	bool				m_bIsValid;		///< Indicates whether the class is valid or not. If invalid none of its methods except init should be called.
};

#endif // _ARMarkerObject_H_
