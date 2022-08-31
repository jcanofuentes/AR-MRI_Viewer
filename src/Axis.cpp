#include "Axis.h"
#include "graphics/GraphicsManager.h"

#include "Ogre3d/include/OgreEntity.h"
#include "Ogre3d/include/OgreSceneManager.h"


// Static member init
const std::string  Axis::DEFAULT_MESH		= "axes.mesh";
const std::string  Axis::DEFAULT_MATERIAL	= "AR_AxesMaterial";

/**
 * @internal
 * @brief Constructor. Initializes class attributes.
 */
Axis::Axis():
	m_bIsValid  ( false )
{
}

/**
 * @internal
 * @brief Destructor. Class release.
 */
Axis::~Axis()
{
	// Release resources
	end();
}

/**
 * @internal
 * @brief Creates a Axis with a specific size in each axis
 * @param[in] width		Width of the Axis
 * @param[in] height	Height of the Axis
 * @param[in] depth		Depth of the Axis
 */
void Axis::init( float width, float height, float depth )
{
	// Create the object 3d of a Axis
	Object3D::init( DEFAULT_MESH, DEFAULT_MATERIAL );

	// Remove query to exclude if from triangle ray collissions
	getEntity()->removeQueryFlags( Ogre::SceneManager::WORLD_GEOMETRY_TYPE_MASK );

	/*
	// Scale correction
	width	*= OGRE_SCALE_CORRECTION;
	height	*= OGRE_SCALE_CORRECTION;
	depth	*= OGRE_SCALE_CORRECTION;
	*/

	// Set the size
	setScale( width, height, depth );

	// Store scale
	m_width		= width;
	m_height	= height;
	m_depth		= depth;

	// This is a primitive object type (Axis)
	setType( TRIANGLE_MESH );

	m_bIsValid = true;
}


/**
 * @internal
 * @brief Creates a Axis with the same size in the three axis
 * @param[in] size Defines the size of the cube (the same in the three axis: width, height, depth)
 */
void Axis::init( float size )
{
	return init( size, size, size );
}
