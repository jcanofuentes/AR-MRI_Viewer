#ifndef _Axis_h_
#define _Axis_h_

#include "graphics/GraphicsPrereqs.h"
#include "graphics/Object3D.h"

/**
 * @internal
 * @brief Represents an Axis.
 */
class Axis: public Cing::Object3D
{
public:

	// Constructor / Destructor
	Axis();
	virtual ~Axis();

	// Init / Release
	void	init 			( float width, float height, float depth );
	void	init 			( float size );

	// Query  Methods
	bool	isValid		() { return m_bIsValid; }
	float	getWidth	() { return m_width;		}
	float	getHeight	() { return m_height;		}
	float	getDepth	() { return m_depth;		}

private:

	// Constant attributes
	static const std::string  DEFAULT_MESH;			///< Name of the default mesh used by this object
	static const std::string  DEFAULT_MATERIAL; ///< Name of the default material used by this object

	// Attributes
	float		m_width;			///< Width of the Axis
	float		m_height;			///< Height of the Axis
	float		m_depth;			///< Depth of the Axis
	bool		m_bIsValid;	  ///< Indicates whether the class is valid or not. If invalid none of its methods except init should be called.

};


#endif // _Axis_h_