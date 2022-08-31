#include "ARCalibrationManager.h"

#include "common/Exception.h"
#include "common/SystemUtils.h"
#include "common/MathUtils.h"

#include "graphics/GraphicsUserApi.h"

// ARToolkitPlus
#include "ARToolKitPlus/TrackerSingleMarkerImpl.h"

// Open CV
#include "OpenCV/cv/include/cv.h"

// Ogre
#include "Ogre3d/include/OgreSceneNode.h"

// XML
#include "TinyXML\include\tinyxml.h"

// Static member init
const int ARCalibrationManager::MAX_NUM_DETECTED_MARKERS = 12;

// Define to debug ARToolkig with higui windows (it cannot work together with flash (hikari)
//#define DEBUG_ARTOOLKIT

using namespace Cing;

#include <float.h>

/**
* @internal
* @brief Constructor. Initializes class attributes.
*/
ARCalibrationManager::ARCalibrationManager() :	m_bIsValid( false ),
m_debugMode( true ),
//m_grayImage(NULL),
m_threshold(168),
m_pattWidth(40/*in mm*/),
m_tracker(NULL),
m_ObjectOrigin(NULL),
m_showDebugInfo(false)
{
}

/**
* @internal
* @brief Destructor. Class release.
*/
ARCalibrationManager::~ARCalibrationManager()
{
	// Release resources
	end();
}

/**
*	
* @brief Init resources
*/
bool ARCalibrationManager::init( const Cing::Image& inImage )
{	
	// Create the image to convert input image to grayscale
	//m_grayImage = cvCreateImage( cvSize(inImage.getWidth(), inImage.getHeight()), 8, 1); 
	m_grayImage.init( inImage.getWidth(), inImage.getHeight(), GRAYSCALE );

#ifdef DEBUG_ARTOOLKIT

	// If debug mode is active, show thresholded image and print information to console
	if ( m_debugMode )
	{
		// Create UI controls 
		cvNamedWindow( "ARCalibrationManager Controller", 1);
		cvCreateTrackbar( "threshold:", "ARCalibrationManager Controller", &m_threshold, 255, 0 );
	}
#endif

	// Now the ARTracker, that does:
	//  - 6x6 sized marker images
	//  - samples at a maximum of 6x6
	//  - can load a maximum of 1 pattern
	//  - can detect a maximum of 6 patterns in one image
	m_tracker  = new ARToolKitPlus::TrackerSingleMarkerImpl<6,6,6*6, 1, 6>(m_grayImage.getWidth(), m_grayImage.getHeight());

	// Set properties
	m_tracker->setLogger(&m_logger);
	m_tracker->setPixelFormat(ARToolKitPlus::PIXEL_FORMAT_LUM);
	m_tracker->setUndistortionMode( ARToolKitPlus::UNDIST_NONE );
	m_tracker->setPoseEstimator(ARToolKitPlus::POSE_ESTIMATOR_ORIGINAL);
	//m_tracker->setPoseEstimator(ARToolKitPlus::POSE_ESTIMATOR_ORIGINAL_CONT);
	//m_tracker->setPoseEstimator(ARToolKitPlus::POSE_ESTIMATOR_RPP);

	m_tracker->setImageProcessingMode(ARToolKitPlus::IMAGE_FULL_RES);

	m_tracker->setThreshold(m_threshold);
	//m_tracker->activateAutoThreshold(true);
	//m_tracker->setNumAutoThresholdRetries(5);
	m_tracker->setBorderWidth(0.125);
	m_tracker->setMarkerMode(ARToolKitPlus::MARKER_ID_SIMPLE);
	m_tracker->setPatternWidth(m_pattWidth);

	// Load a camera calib file
	if(!m_tracker->init("../data/Camera.cal", 1.0f, 1000.0f))
	{
		// TODO: Check this
		Cing::println("ERROR: init() failed\n");
	}

	// Calculated global origin from markers
	m_calculatedOrigin.init(5,5,5);
	m_calculatedOrigin.setPosition(-9999,-9999,-9999);

	// Debug info
	const char* description = m_tracker->getDescription();
	Cing::println("ARToolKitPlus compile-time information:\n%s\n\n", description);

	// Reserve memory
	m_markerObjects.reserve(MAX_NUM_DETECTED_MARKERS);
	m_ghostObjects.reserve(MAX_NUM_DETECTED_MARKERS ); //  It must be (MAX_NUM_DETECTED_MARKERS -1 )

	m_OriginNode = new AROriginNode();

	// TODO: This is another
	m_cursor_Tracker = new ARToolKitPlus::TrackerSingleMarkerImpl<16,16,64, 1, 16>(m_grayImage.getWidth(), m_grayImage.getHeight() );
	// Set properties
	m_cursor_Tracker->setLogger(&m_logger);
	m_cursor_Tracker->setPixelFormat(ARToolKitPlus::PIXEL_FORMAT_LUM);
	m_cursor_Tracker->setUndistortionMode( ARToolKitPlus::UNDIST_NONE );
	m_cursor_Tracker->setPoseEstimator(ARToolKitPlus::POSE_ESTIMATOR_ORIGINAL);
	//m_cursor_Tracker->setPoseEstimator(ARToolKitPlus::POSE_ESTIMATOR_ORIGINAL_CONT);
	//m_cursor_Tracker->setPoseEstimator(ARToolKitPlus::POSE_ESTIMATOR_RPP);

	//m_cursor_Tracker->setImageProcessingMode(ARToolKitPlus::IMAGE_FULL_RES);

	m_cursor_Tracker->setThreshold(m_threshold);
	//m_cursor_Tracker->activateAutoThreshold(true);
	//m_cursor_Tracker->setNumAutoThresholdRetries(5);

	m_cursor_Tracker->setBorderWidth(0.25);
	m_cursor_Tracker->setPatternWidth(80);


	// Load a camera calib file
	if(!m_cursor_Tracker->init("../data/Camera.cal", 1.0f, 1000.0f))
	{
		// TODO: Check this
		Cing::println("ERROR: init() failed\n");
	}

	m_cursor_Tracker->addPattern("../data/cursor_AR.mkr");

	m_Cursor_AR.init( "../data/Modelos3d/cursor_AR.mesh", "cursor_AR/01_-_Default" );


	m_bIsValid = true;

	return true;
}

/**
* @brief Update tracking of markers
* @param[in] inImage image where the markers will be searched
*/
void ARCalibrationManager::update( Cing::Image& inImage )
{
	// Convert color input image to gray and update tracker
	cvCvtColor( &inImage.getCVImage(), &m_grayImage.getCVImage(), CV_RGB2GRAY  );

	// Detect AR cursor
	ARToolKitPlus::ARMarkerInfo* marker_info;
	int	marker_num;
	ARFloat	patt_center[2];
	patt_center[0] = 0;
	patt_center[1] = 0;

	float	minConf			= 0.7f;
	int		success			= m_cursor_Tracker->arDetectMarker( (unsigned char*)m_grayImage.getData(), m_threshold,	&marker_info, &marker_num );

	m_Cursor_AR.setPosition( Vector(-10000, -10000, -10000) );

	for ( unsigned int i = 0; i < (unsigned int)marker_num; i++)
	{
		int markerId =  marker_info[i].id;

		if (markerId == 0 )
		{
			if ( marker_info[i].cf > minConf )
			{	
				// Calculate matrix from markers
				float* mv = new float[16];
				m_cursor_Tracker->calcOpenGLMatrixFromMarker( &marker_info[i], patt_center, m_pattWidth , mv);

				// Extract axis from modelView matrix
				Ogre::Vector3 x			= Ogre::Vector3(*(mv+0), *(mv+1), *(mv+2));		
				Ogre::Vector3 y			= Ogre::Vector3(*(mv+4), *(mv+5), *(mv+6));
				Ogre::Vector3 z			= Ogre::Vector3(*(mv+8), *(mv+9), *(mv+10));
				//Ogre::Vector3 scale = Ogre::Vector3(x.length(), y.length(), z.length());
				x.normalise();	y.normalise();	z.normalise();

				// Update cursor
				Ogre::Vector3		newPosition    = Vector( *(mv+12), *(mv+13), *(mv+14) );
				Ogre::Quaternion	newOrientation = Ogre::Quaternion(x, y, z);
				m_Cursor_AR.setPosition( newPosition );
				if (newOrientation.w <1.0 && newOrientation.x <1.0 && newOrientation.y <1.0 && newOrientation.z <1.0)
				{
					if (newOrientation.w != 0.0 && newOrientation.x != 0.5 && newOrientation.y != 0.0 && newOrientation.z != 0.0)
					{
						// Update and filter orientation
						Ogre::Quaternion newRot = Ogre::Quaternion::Slerp( 0.9,  newOrientation, m_Cursor_AR.getSceneNode()->getOrientation(), true);
						m_Cursor_AR.getSceneNode()->setOrientation(newRot);
					}
				}
			}
		}else{

		}

	}

	// Detect AR markers			
	success	= m_tracker->arDetectMarker( (unsigned char*)m_grayImage.getData(), m_threshold, &marker_info, &marker_num );

	// Reset detection state for each ARMarkerObject and calculate averaged origin
	Ogre::Vector3		avgPos = m_OriginNode->m_sceneNode->getPosition();	
	Ogre::Quaternion	avgRot = m_OriginNode->m_sceneNode->getOrientation();	

	// Take the initial pose of origin marker
	if ( m_ObjectOrigin != NULL && m_ObjectOrigin->isDetected() )
	{
		avgPos = m_ObjectOrigin->getSceneNode()->getPosition();
		avgRot = m_ObjectOrigin->getSceneNode()->getOrientation();

		m_OriginNode->updateTracking(avgPos,avgRot);
	}else{


		bool isAveraged = false;
		for( unsigned int i = 0; i < m_markerObjects.size(); i++ )
		{
			ARMarkerObject* tempObject = m_markerObjects[i];
			if ( tempObject->isDetected() && tempObject!= m_ObjectOrigin && m_ObjectOrigin!= NULL)
			{
				// Get origin ( ghost - object )from this marker
				ARMarkerObject* ghostOrigin = NULL;
				for( unsigned int j = 0; j < m_ghostObjects.size(); j++ )
					if ( tempObject->getId() == m_ghostObjects[j]->getId() )	ghostOrigin = m_ghostObjects[j];

				// If the ghost object exists, average origin
				if (ghostOrigin!=NULL && !isAveraged )
				{
					Ogre::Matrix4 originTrans = ghostOrigin->getSceneNode()->_getFullTransform();

					// Update origin here	
					m_OriginNode->updateTracking(originTrans.getTrans(),originTrans.extractQuaternion());
					//isAveraged = true;
				}

			}
			tempObject->setDetected( false );
		}
	}

    if (m_ObjectOrigin!=NULL)	m_ObjectOrigin->setDetected(false);



	// Loop through all the markers founded in the image
	for ( unsigned int i = 0; i < (unsigned int)marker_num; i++)
	{
		if ( marker_info[i].cf > 0.5 ) // Confidende of ID recognition
		{		
			// Check if the marker is founded previously
			int markerId =  marker_info[i].id;

			// Iterate through ARMarkerObjects and ckeck ids
			ARMarkerObject* tempMarkerObject	= NULL;

			for( unsigned int j = 0; j < m_markerObjects.size(); j++ )
			{
				// If the id is founded, store in tempMarkerObject a pointer to
				// the ARMarkerObject assocciated
				if ( markerId == m_markerObjects[j]->getId() )
				{
					tempMarkerObject = m_markerObjects[j];
					break;
				}
			}

			// If the marker is not founded, create a new one
			if ( tempMarkerObject == NULL )
			{
				tempMarkerObject = new ARMarkerObject();
				tempMarkerObject->init( markerId );
				tempMarkerObject->setScale( Cing::Vector(2.7,2.7,2.7) );

				// Store the reference in the container m_markerObjects
				m_markerObjects.push_back(tempMarkerObject);
			}

			// Calculate the modelview matrix from marker
			float* mv = new float[16];
			m_tracker->calcOpenGLMatrixFromMarker( &marker_info[i], patt_center, m_pattWidth , mv);

			// Extract Position , scale and orientation from modelView matrix 
			Ogre::Vector3 x	= Ogre::Vector3(*(mv+0), *(mv+1), *(mv+2));		
			Ogre::Vector3 y	= Ogre::Vector3(*(mv+4), *(mv+5), *(mv+6));
			Ogre::Vector3 z	= Ogre::Vector3(*(mv+8), *(mv+9), *(mv+10));
			Ogre::Vector3 scale = Ogre::Vector3(x.length(), y.length(), z.length());

			x.normalise();
			y.normalise();
			z.normalise();

			Ogre::Vector3	 newPosition    = Ogre::Vector3( *(mv+12), *(mv+13), *(mv+14) );
			Ogre::Quaternion newOrientation = Ogre::Quaternion(x, y, z);

			// TODO: Check this bug when using RPP estimator
			if ( newPosition.x != 0.0 ) 
			{
				// Update pose to tempMarkerObject
				tempMarkerObject->setPosition( newPosition );
				tempMarkerObject->setOrientation( newOrientation );
				//tempMarkerObject->averagePosition( newPosition );
				//tempMarkerObject->averageOrientation( newOrientation );

				tempMarkerObject->setDetected( true );
			}

			if ( m_debugMode )
			{
				// Paint marker ids on inputImage
				std::ostringstream debugStr;
				debugStr << "id:";
				debugStr << markerId;
				inImage.text(marker_info[i].vertex[0][0],marker_info[i].vertex[0][1], debugStr.str().c_str() );
			}

		}
	}

	// Update tracker and visualization
	for( unsigned int i = 0; i < m_markerObjects.size(); i++ )
	{
		int markerId = m_markerObjects[i]->getId();

		// Creamos ghostObjects que  marcan el origen si:
		//      1. Existe una marca origen seleccionada
		//		2. La marca origen está siendo detectada
		//		3. Si la marca destino es distinta de la marca origen
		//      4. se ha detectado en el ultimo fotograma la marca destino

		if (   ( m_ObjectOrigin != NULL ) && ( m_ObjectOrigin->isDetected() )
			&& ( m_ObjectOrigin->getId() != m_markerObjects[i]->getId() ) && ( m_markerObjects[i]->isDetected() ))
		{
			ARMarkerObject* tempGhostObject	= NULL;

			// Comprobamos si existe el ghostObjet
			for( unsigned int j = 0; j < m_ghostObjects.size(); j++ )
			{
				// If the id is founded, store it
				if ( markerId == m_ghostObjects[j]->getId() )
				{
					tempGhostObject = m_ghostObjects[j];
					break;				
				}
			}

			// If the ghost object is not founded, create a new one
			if ( tempGhostObject == NULL )
			{
				tempGhostObject = new ARMarkerObject();
				tempGhostObject->init( markerId );
				tempGhostObject->setScale( Cing::Vector(1,1,1) );

				// Attach the calculated origin to the marker
				tempGhostObject->getSceneNode()->getParentSceneNode()->removeChild(tempGhostObject->getSceneNode());
				
				
				m_markerObjects[i]->addChild(tempGhostObject);

				// Get SceneNodes and transforms
				Ogre::SceneNode* orig  = m_ObjectOrigin->getSceneNode();
				orig->setScale(1,1,1); // TODO: Check this. Uncomment setScale line for view scale - position problem
				Ogre::SceneNode* other = m_markerObjects[i]->getSceneNode();
				other->setScale(1,1,1);// TODO: Check this. Uncomment setScale line for view scale - position problem
				
				Ogre::Matrix4 origMat = Ogre::Matrix4(orig->getOrientation());
				origMat.setTrans(orig->getPosition());
				
				Ogre::Matrix4 otherMat = Ogre::Matrix4(other->getOrientation());
				otherMat.setTrans(other->getPosition());

				// Calculate the relative transform matrix between markers
				Ogre::Matrix4 invertedMat = otherMat.inverse();
				Ogre::Matrix4 resultMat   = invertedMat*origMat;

				// Init ghost object
				//tempGhostObject->averagePosition( resultMat.getTrans() );
				//tempGhostObject->averageOrientation( resultMat.extractQuaternion() );
				tempGhostObject->getSceneNode()->setPosition( resultMat.getTrans() );
				tempGhostObject->getSceneNode()->setOrientation( resultMat.extractQuaternion() );	

				// Store the pointer
				m_ghostObjects.push_back(tempGhostObject);
			}

			

			// Get SceneNodes and transforms
			Ogre::SceneNode* orig  = m_ObjectOrigin->getSceneNode();
			orig->setScale(1,1,1); 
			Ogre::SceneNode* other = m_markerObjects[i]->getSceneNode();
			other->setScale(1,1,1);
			
			Ogre::Matrix4 origMat = Ogre::Matrix4(orig->getOrientation());
			origMat.setTrans(orig->getPosition());
			Ogre::Matrix4 otherMat = Ogre::Matrix4(other->getOrientation());
			otherMat.setTrans(other->getPosition());

			// Calculate the relative transform matrix between markers
			Ogre::Matrix4 invertedMat = otherMat.inverse();
			Ogre::Matrix4 resultMat   = invertedMat*origMat;


			// Update averaged pose
			Ogre::Quaternion destRot = resultMat.extractQuaternion();
			Ogre::Vector3 deltaPos;
			deltaPos = tempGhostObject->getSceneNode()->getPosition() - resultMat.getTrans();
			//float minDelta = 100.0;

			//if ( deltaPos.length() < minDelta ) 
			//{
				tempGhostObject->averagePosition( resultMat.getTrans() );	
				if (destRot.w <1.0 && destRot.x <1.0 && destRot.y <1.0 && destRot.z <1.0)
				{
					if (destRot.w != 0.0 && destRot.x != 0.5 && destRot.y != 0.0 && destRot.z != 0.0)
					{
						// Update and filter orientation
						//Ogre::Quaternion newRot = Ogre::Quaternion::Slerp( 0.97,  destRot, tempGhostObject->getSceneNode()->getOrientation(), false);
						tempGhostObject->averageOrientation( destRot );

					}
				}
			//}
			

		}
		// If not detected, make it invisible
		if ( m_markerObjects[i]->isDetected() == false )	m_markerObjects[i]->setPosition(Cing::Vector(-99999,-99999,-99999));
	}

#ifdef DEBUG_ARTOOLKIT

	// Paint thresholded image
	if ( m_debugMode )
	{
		cvThreshold( m_grayImage, m_grayImage, m_threshold, 255, 1);
		cvShowImage( "ARCalibrationManager Controller", m_grayImage );
	}
#endif

	// Show debug info
	if ( m_showDebugInfo )
	{
		cvThreshold( &m_grayImage.getCVImage(), &m_grayImage.getCVImage(), m_threshold, 255, 1);
		m_grayImage.updateTexture();
		m_grayImage.draw2d( 0, height - m_grayImage.getHeight()/2.0, m_grayImage.getWidth()/2.0, m_grayImage.getHeight()/2.0 );
	}
}

/**
* @brief Store a pointer to the Origin marker
* @param[in] originMarker: the origin calibration marker
*/
void ARCalibrationManager::setOriginMarker( ARMarkerObject* originMarker )
{
	/*if (m_markerObjectOrigin==NULL)*/	
	m_ObjectOrigin = originMarker; 
}

/** 
* @brief Save markers transforms to file
*/
void ARCalibrationManager::saveCalibration( char* filename ) 
{
	if ( m_ObjectOrigin != NULL )
	{
		// Create file
		TiXmlDocument XMLDoc;
		XMLDoc.InsertEndChild(TiXmlElement("MultiMarker"));

		// Write data
		TiXmlElement* rootNode = XMLDoc.RootElement();

		// Iterate trough markers attached to the origin 
		Ogre::SceneNode::ChildNodeIterator it = m_ObjectOrigin->getSceneNode()->getChildIterator();

		rootNode->SetAttribute( "OriginMarkerId", m_ObjectOrigin->getId() );

		while(it.hasMoreElements())
		{	
			Ogre::SceneNode* tmpSceneNode = dynamic_cast<Ogre::SceneNode*>(it.getNext());

			// Get marker
			const Ogre::Any&	any		  = tmpSceneNode->getUserAny();
			ARMarkerObject*		theMarker = Ogre::any_cast< ARMarkerObject* >(any);

			Ogre::Vector3 x, y, z;
			Ogre::Vector3 pos;

			pos = theMarker->getSceneNode()->getPosition();
			theMarker->getSceneNode()->getOrientation().ToAxes( x, y, z );


			// Invertimos la matrix...
			Ogre::Matrix4 tempTransform = Ogre::Matrix4::IDENTITY ;
			tempTransform.makeTransform( pos, Ogre::Vector3::UNIT_SCALE, theMarker->getSceneNode()->getOrientation());
			tempTransform = tempTransform.inverse();
			Ogre::Quaternion destRot = tempTransform.extractQuaternion();
			destRot.ToAxes( x, y, z );
			pos = tempTransform.getTrans();


			TiXmlElement* markerObjectXML = rootNode->InsertEndChild(TiXmlElement( "ARMarkerObject" ))->ToElement();
			markerObjectXML->SetAttribute( "id", theMarker->getId() );

			TiXmlElement* position = markerObjectXML->InsertEndChild(TiXmlElement( "Position" ))->ToElement();
			position->SetDoubleAttribute( "x", pos.x );
			position->SetDoubleAttribute( "y", pos.y );
			position->SetDoubleAttribute( "z", pos.z );

			TiXmlElement* xAxis = markerObjectXML->InsertEndChild(TiXmlElement( "xAxis" ))->ToElement();
			xAxis->SetDoubleAttribute( "x", x.x );
			xAxis->SetDoubleAttribute( "y", x.y );
			xAxis->SetDoubleAttribute( "z", x.z );
			TiXmlElement* yAxis = markerObjectXML->InsertEndChild(TiXmlElement( "yAxis" ))->ToElement();
			yAxis->SetDoubleAttribute( "x", y.x );
			yAxis->SetDoubleAttribute( "y", y.y );
			yAxis->SetDoubleAttribute( "z", y.z );
			TiXmlElement* zAxis = markerObjectXML->InsertEndChild(TiXmlElement( "zAxis" ))->ToElement();
			zAxis->SetDoubleAttribute( "x", z.x );
			zAxis->SetDoubleAttribute( "y", z.y );
			zAxis->SetDoubleAttribute( "z", z.z );

			//tmpSceneNode->getD		
		}
		// Save file
		XMLDoc.SaveFile( filename );

	}
}


/**
 * @internal
 * @brief Shows debug info (axis and threshold image)
*/
void ARCalibrationManager::showDebugInfo( bool showDebugInfo )
{
	m_showDebugInfo = showDebugInfo;
	m_debugMode		= showDebugInfo;

	// Make models visible/invisible
	m_OriginNode->setVisible( showDebugInfo );
	std::for_each( m_markerObjects.begin(), m_markerObjects.end(), std::bind2nd( std::mem_fun( &ARMarkerObject::setVisible ), showDebugInfo ) );
	std::for_each( m_ghostObjects.begin(), m_ghostObjects.end(), std::bind2nd( std::mem_fun( &ARMarkerObject::setVisible ), showDebugInfo ) );
}

/**
* @internal
* @brief Releases the class resources. 
* After this method is called the class is not valid anymore.
*/
void ARCalibrationManager::end()
{
	// TODO: Clean all resources!!

	// Check if the class is already released
	if ( !isValid() )
		return;

	// Release resources -> now is Cing Image, not necessary the release.
	//cvReleaseImage(&m_grayImage);

	// The class is not valid anymore
	m_bIsValid = false;
}
