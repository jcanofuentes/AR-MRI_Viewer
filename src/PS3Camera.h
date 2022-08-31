#ifndef _PS3Camera_H_
#define _PS3Camera_H_

#include "camerainput/CameraInputPrereqs.h"
#include "camerainput/BaseCameraInput.h"
#include "graphics/GraphicsTypes.h"
#include "graphics/Image.h"

// PortVideo
#include "RingBuffer.h"

// PS3Eye SDK
#include "CLEyeMulticam.h"

// STL
#include <string>

// OpenCV
#include "OpenCV/cxcore/include/cxtypes.h"

// PTypes
#include "PTypes/include/pasync.h"

/**
 * @internal
 * Class to capture camera input based on PS3 SDK by Code Laboratories
 */
class PS3Camera
{
public:

  // Structure to capture the image frames from the camera in a different thread
  struct PS3CaptureThread: public pt::thread
  {
    PS3CaptureThread ( PS3Camera& pS3Camera ):pt::thread( false ), m_PS3Camera( pS3Camera ) {};
    void execute     ();
    PS3Camera&       m_PS3Camera; ///< Camera that will retrieve the images captured in this thread
  };

	// Constructor / Destructor
	PS3Camera();
	~PS3Camera();

	// BaseCameraInput like interface
	void  init   ( int deviceId, int width, int height, int fps, Cing::GraphicsType format );
	void  end    ();
	void  update ();

	// Get methods
	const IplImage&	getCVImage	() const	{ return *m_currentCameraImage; }
	int				getGain()				{ return m_gain; };
	int				getExposure()			{ return m_exposure; };

	// Set methods
	void	setGain		( int gain );
	void	setExposure	( int exposure );

	bool	isValid() const				{ return m_bIsValid; }
	
private:

	// Attributes
	int							m_gain;					///< Gain		 [0, 79]
	int							m_exposure;				///< Exposure	 [0, 511]
	IplImage*					m_currentCameraImage; 	///< Image captured from the camera 
	CLEyeCameraInstance			m_ps3CameraInstance;	///< Object that allows access to the camera through PS3Eye SDK video library
	RingBuffer*					m_ringBuffer;			///< Circular buffer to store camera frames. Shared usage between this class and the PS3CaptureThread class.
	PS3CaptureThread*			m_captureThread;		///< Thread to capture images from the camera
	bool						m_bIsValid;				///< Indicates whether the class is valid or not. If invalid none of its methods except init should be called.

};


#endif // _PVCamera_H_
