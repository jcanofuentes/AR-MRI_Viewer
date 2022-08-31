#include "PS3Camera.h"

// OpenCv
#include "opencv/cv/include/cv.h"

// Common
#include "common/Exception.h"
#include "common/Release.h"
#include "common/CommonUserAPI.h"
#include "common/MathUtils.h"
#include "graphics/GraphicsTypes.h"

void PS3Camera::PS3CaptureThread::execute()
{  
	PBYTE cameraWriteBuffer = NULL;
	PBYTE cameraBuffer		= new BYTE[m_PS3Camera.m_ringBuffer->size()];

	// Execute until the thread gets signaled
	while( !get_signaled() )
	{    
		// Get new frame from the camera
		//cvGetRawData(m_PS3Camera.m_currentCameraImage, &cameraBuffer);
		bool success = CLEyeCameraGetFrame(m_PS3Camera.m_ps3CameraInstance, cameraBuffer);
		if ( success ) 
		{
			// Get buffer to write the image
			cameraWriteBuffer = m_PS3Camera.m_ringBuffer->getNextBufferToWrite();
			if ( cameraWriteBuffer ) 
			{
				// Copy the new frame in the buffer, and notify it
				memcpy( cameraWriteBuffer, cameraBuffer, m_PS3Camera.m_ringBuffer->size() );
				m_PS3Camera.m_ringBuffer->writeFinished();			
			}
		}

	} 
	/*
	while( !get_signaled() )
	{
		// Get buffer to write the image
		cameraWriteBuffer = m_PS3Camera.m_ringBuffer->getNextBufferToWrite();

		if ( cameraWriteBuffer == NULL )
			return;

		// Get new frame from the camera
		bool success = CLEyeCameraGetFrame(m_PS3Camera.m_ps3CameraInstance, cameraWriteBuffer);
		if ( success ) 
		{

			m_PS3Camera.m_ringBuffer->writeFinished();
		}				 
	}
	*/
}

/**
 * @internal
 * @brief Constructor. Initializes class attributes.
 */
PS3Camera::PS3Camera():
m_ps3CameraInstance        ( NULL ),
m_captureThread   ( NULL  ),
m_gain (21),
m_exposure (8),
m_bIsValid        ( false )
{
}

/**
 * @internal
 * @brief Destructor. Class release.
 */
PS3Camera::~PS3Camera()
{
	// Release resources
	end();
}

/**
 * @internal
 * @brief Initializes the class so it becomes valid.
 *
 * @param[in] deviceId	Id of the device to capture from (starting at 0)
 * @param[in] width			width resolution capture
 * @param[in] height		height resolution capture
 * @param[in] fps				frames per second to capture
 * @param[in] format		Format of the image. if RGB the captured images will be color (if supported by the camera), if GRAYSCALE, they will be b/w
 */
void PS3Camera::init( int deviceId, int width, int height, int fps, Cing::GraphicsType format )
{
	// Check if the class is already initialized
	if ( isValid() )
		return;

	// Find a PS3 camera in the system
	int numCams = CLEyeGetCameraCount();
	Cing::println("Found %d PS3 cameras\n", numCams);

	if(numCams == 0)
	{
		Cing::println("No PS3Eye cameras detected\n");
		return;
	}

	GUID guid = CLEyeGetCameraUUID(deviceId);
	CLEyeCameraResolution resolutionMode;
	CLEyeCameraColorMode colorMode;

	if (width ==640 && height ==480) resolutionMode = CLEYE_VGA;
	if (width ==320 && height ==240) resolutionMode = CLEYE_QVGA;

	if (format ==Cing::GraphicsType::BGRA) colorMode = CLEYE_COLOR;
	if (format ==Cing::GraphicsType::GRAYSCALE) colorMode = CLEYE_GRAYSCALE;

	m_ps3CameraInstance = CLEyeCreateCamera(guid, colorMode, resolutionMode, fps);

	Cing::println("Camera %d created. GUID: [%08x-%04x-%04x-%02x%02x%02x%02x%02x%02x%02x%02x]\n", 
					deviceId, guid.Data1, guid.Data2, guid.Data3,
					guid.Data4[0], guid.Data4[1], guid.Data4[2],
					guid.Data4[3], guid.Data4[4], guid.Data4[5],
					guid.Data4[6], guid.Data4[7]);

	if ( m_ps3CameraInstance == NULL )
	{
		Cing::println("ERROR: Creating camera instance");
		return;
	}

	// Allocate buffers to store images
	unsigned int bufferSize = 0;

	// Create the image to store the camera frames
	if (format ==Cing::GraphicsType::BGRA)
	{
		bufferSize = width * height * 4;
		m_currentCameraImage = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 4);
	}
	if (format ==Cing::GraphicsType::GRAYSCALE)
	{
		bufferSize = width * height * 1;
		m_currentCameraImage = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 1);
	}

	m_ringBuffer  = new RingBuffer( bufferSize );

	// Set some camera parameters
	CLEyeSetCameraParameter(m_ps3CameraInstance, CLEYE_AUTO_GAIN, false);
	CLEyeSetCameraParameter(m_ps3CameraInstance, CLEYE_AUTO_EXPOSURE, false);
	CLEyeSetCameraParameter(m_ps3CameraInstance, CLEYE_AUTO_WHITEBALANCE, true);
		
	/*
	CLEyeSetCameraParameter(m_ps3CameraInstance, CLEYE_WHITEBALANCE_RED, 127);
	CLEyeSetCameraParameter(m_ps3CameraInstance, CLEYE_WHITEBALANCE_GREEN, 127);
	CLEyeSetCameraParameter(m_ps3CameraInstance, CLEYE_WHITEBALANCE_BLUE, 127);
	*/

	CLEyeSetCameraParameter(m_ps3CameraInstance, CLEYE_GAIN,		m_gain);
	CLEyeSetCameraParameter(m_ps3CameraInstance, CLEYE_EXPOSURE,	m_exposure);

	// Start capturing images from camera
	CLEyeCameraStart(m_ps3CameraInstance);

	// Create capture thread (to retrieve images from the camera in a different execution thread)
	m_captureThread = new PS3CaptureThread( *this );
	m_captureThread->start();

	// The class is now initialized
	m_bIsValid = true;
}

/**
 * @internal
 * @brief Releases the class resources. 
 * After this method is called the class is not valid anymore.
 */
void PS3Camera::end()
{
	// Check if the class is already released
	if ( !isValid() )
		return;

	// Release thread
	m_captureThread->signal();  

	// Delete thread
	Cing::Release( m_captureThread );

	// Stop capture
	CLEyeCameraStop(m_ps3CameraInstance);

	// Release resources
	//Release( sourceBuffer_ );
	Cing::Release( m_ringBuffer );

	// The class is not valid anymore
	CLEyeDestroyCamera(m_ps3CameraInstance);
	m_bIsValid = false;
}

/**
 * @internal
 * @brief Captures new image from camera
 * @note this is a blocking call -> would be good to have it in a separate thread
 */
void PS3Camera::update()
{
	// Check if we have a new frame
	PBYTE cameraReadBuffer = m_ringBuffer->getNextBufferToRead();
	if ( cameraReadBuffer == NULL )
		return;

	//memcpy(m_currentCameraImage->imageData, cameraReadBuffer, m_ringBuffer->size());
	//m_ringBuffer->readFinished();
	
	
	// Get all the new frames
	while( cameraReadBuffer != NULL )
	{
		// Copy current frame and notify it has been read to the ring buffer
		memcpy(m_currentCameraImage->imageData, cameraReadBuffer, m_ringBuffer->size());
		m_ringBuffer->readFinished();

		// Check if there are more new frames-
		cameraReadBuffer = m_ringBuffer->getNextBufferToRead();
	}
	
}

/**
 * @internal
 * @brief Set camera gain
 */
void PS3Camera::setGain( int gain )
{
	m_gain = gain; 
	CLEyeSetCameraParameter(m_ps3CameraInstance, CLEYE_GAIN, m_gain);
};

/**
 * @internal
 * @brief Set camera exposure
 */
void PS3Camera::setExposure( int exposure )
{
	m_exposure = exposure;
	CLEyeSetCameraParameter(m_ps3CameraInstance, CLEYE_EXPOSURE, m_exposure);
};
