#ifndef _AveragedVector_h_
#define _AveragedVector_h_

#include "graphics/GraphicsPrereqs.h"
#include "common/MathUtils.h"

/**
 * @internal
 * @brief Represents an AveragedVector.
 */
class AveragedVector
{
public:

	// Constructor / Destructor
	AveragedVector();
	virtual ~AveragedVector();

	// Init / Release
	void	init 			( int nValues );
	void	end 			();

	void addValue( float x, float y, float z );
	Cing::Vector* getValue();

	// Query  Methods
	bool	isValid		() { return m_bIsValid; }

private:

	// Attributes
	Cing::Average*					m_avgx;
	Cing::Average*					m_avgy;
	Cing::Average*					m_avgz;
	Cing::Vector*					m_vector;
	bool							m_bIsValid;	  ///< Indicates whether the class is valid or not. If invalid none of its methods except init should be called.

};


#endif // _Axis_h_