#ifndef _ARCalibrationManager_H_
#define _ARCalibrationManager_H_

// Cing
#include "graphics/Image.h"
#include "common/CommonUserAPI.h"
#include "graphics/Box.h"


// ARTKPlus
#include "ARToolKitPlus/TrackerSingleMarker.h"
#include "ARToolKitPlus/Logger.h"

// Other / AR
#include "ARMarkerObject.h"
#include "Axis.h"
#include "AROriginNode.h"

// Logger for error messages from ARTKPlus
class ARTKP_Logger : public ARToolKitPlus::Logger
{
	void artLog(const char* nStr)
	{
		Cing::println(nStr);
	}
};


class ARCalibrationManager
{
public:

	// Constructor / Destructor
	ARCalibrationManager();
	~ARCalibrationManager();

	// Init / Release
	bool init( const Cing::Image& inImage );
	void end();

	// Update
	void update( Cing::Image& inImage );
	
	// Query methods
	bool isValid () const { return m_bIsValid; }
	ARMarkerObject* getOriginMarker() const	{ return m_ObjectOrigin; }

	// Other
	void setDebugMode	( bool debugMode )		{ m_debugMode = debugMode; }
	void setOriginMarker( ARMarkerObject* originMarker );
	void saveCalibration( char* filename ) ;
	void showDebugInfo	( bool showDebugInfo );
	void setThreshold	( int threshold ) {m_threshold = threshold; }
	int	getThreshold	() const { return m_threshold; }
	// Not implementd yet:
	void setPattWidth( float pattWidth );

	Axis				m_calculatedOrigin;
	AROriginNode*		m_OriginNode;
private:

	// Constant attributes
	static const int				MAX_NUM_DETECTED_MARKERS;	///< Maximum number of detected markers in image

	// AR
	// Todo: Change data structure: from ARMarkerObject to ARMarkerObject*
	std::vector< ARMarkerObject* >	m_markerObjects;			///< Container to store ARMarkerObjects created	
	ARMarkerObject*					m_ObjectOrigin;				///< A pointer to the user selected origin (any marker)
	std::vector< ARMarkerObject* >	m_ghostObjects;				///< New ARMarkerObject ( in ghost mode) are created and
																///< attached dinamically to the m_markerObjectOrigin.
																///< They are stored in the m_childMarkerObjects container
					

	Cing::Object3D m_Cursor_AR;

	// ARToolkit specific
	ARFloat									m_pattWidth;		///<  Width of the printed markers (in mm)
	ARTKP_Logger							m_logger;			///<  The ARTKP logger member	
	ARToolKitPlus::TrackerSingleMarker*		m_tracker;			///<  The tracker
	ARToolKitPlus::TrackerSingleMarker*		m_cursor_Tracker;			///<  The tracker

	// CV
	//IplImage*			m_grayImage;	///< Image to perform grayscale conversion
	Cing::Image			m_grayImage;	///< Image to perform grayscale conversion
	int					m_threshold;	///< Control for threshold

	bool				m_showDebugInfo; ///< Indicates whether the debug information (models and threshold image)
	bool				m_debugMode;	///< Indicates whether the debug information will be drawn or not.
	bool				m_bIsValid;		///< Indicates whether the class is valid or not. If invalid none of its methods except init should be called.
};

#endif // _ARCalibrationManager_H_
