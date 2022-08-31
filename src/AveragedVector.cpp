#include "AveragedVector.h"

/**
 * @internal
 * @brief Constructor. Initializes class attributes.
 */
AveragedVector::AveragedVector():
	m_bIsValid  ( false )
{
}

/**
 * @internal
 * @brief Destructor. Class release.
 */
AveragedVector::~AveragedVector()
{
	// Release resources
	end();
}

/**
 * @internal
 * @brief 
 */
void AveragedVector::init(  int nValues  )
{
	m_avgx = new Cing::Average(nValues);
	m_avgy = new Cing::Average(nValues);
	m_avgz = new Cing::Average(nValues);

	m_vector = new Cing::Vector(0,0,0);
	m_bIsValid = true;
}
/**
* @internal
* @brief 
*/
void AveragedVector::end()
{
	delete m_avgx;
	delete m_avgy;
	delete m_avgz;
	delete m_vector;
	m_bIsValid = false;
}

/**
 * @internal
 * @brief 
 */
void AveragedVector::addValue(  float x, float y, float z )
{
	m_avgx->addValue( x );
	m_avgy->addValue( y );
	m_avgz->addValue( z );

	m_vector->x =  m_avgx->getValue();
	m_vector->y =  m_avgy->getValue();
	m_vector->z =  m_avgz->getValue();

}

/**
 * @internal
 * @brief 
 */
Cing::Vector*  AveragedVector::getValue()
{
	return m_vector;
}

