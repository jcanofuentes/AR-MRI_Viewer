//#define TIXML_USE_STL

#include "Cing.h"
#include "IPS3EyeLib.h"
#include "ARCalibrationManager.h"

// Ogre
#include "Ogre3d/include/OgreCamera.h"			// To allow creating new ogre camera
#include "Ogre3d/include/OgreRenderWindow.h"	// To allow change active camera


// Flash
#include "Flash/FlashMovieManager.h"
#include "Flash/FlashVideoPlayer.h"
#include "Flash/FlashImageGallery.h"

// Visor Resonancias
#include "Visor.h"					// visor de resonancias
#include "CalibrationGUI.h"
#include "GUI/VisorGUI.h"

#include "RayQuery.h"

#include "Dumper/MiniDumper.h"

//---------------------------------------------------------------
// TODO: 
//  + Repasar la clase Cing::Transform, añadir funcionalidad 
//    que se haya usado en la demo. Transformacion entre nodos, inverse(), ...
//---------------------------------------------------------------

// App settings
int				appWidth		= 1280;
int				appHeight		= 1024;

// Video Settings		
int				captureWidth		= 640;
int				captureHeight		= 480;
float			FOVy				= 38;  // PS3Eye vertical FOV
GraphicsType	captureImageFormat	= RGB;
IPS3EyeLib*		pCam				= NULL;
PBYTE			pBuffer				= NULL;
Image			camera;

// AR
ARCalibrationManager	arManager;
Ogre::Camera*			arCam;

// Mouse picking
Ogre::RaySceneQuery*	mRaySceneQuery;
RayQuery				triRayQuery;
bool					mouseInMenu = false;

// Flash asociado a modelos
//FlashVideoPlayer	videoPlayer;
//FlashImageGallery	imageGallery;

FlashImageGallery	Hemisferio_izqGallery;
FlashImageGallery	Hemisferio_dereGallery;
FlashImageGallery	Globopalido_extGallery;
FlashImageGallery	CaudadoconmsGallery;
FlashImageGallery	Globopalido_intGallery;
FlashImageGallery	talamoGallery;
FlashVideoPlayer	PutamenconsmVideoPlayer;

bool				flashVisible = false;


// Flash global de aplicación
FlashMovie		calibController;
CalibrationGUI  calibGUI;
FlashMovie		mainMenu;

// Visor resonancias
Visor			visor;
bool			showVisorPlanos = true;
VisorGUI		visorGUI;			// GUI para controlar visor de resonancias

// Objetos 3d
PointLight	light;
Object3D	Putamenconsm, Hemisferio_izq, Hemisferio_dere, Globopalido_exterior, Caudadoconms, Globopalido_interior, talamo;

// Dumper to get crash information
MiniDumper m( MiniDumper::CALL_STACK_DUMP );

// Application states
enum ApplicationState
{
	VISOR,
	CALIBRATION,
	UNDEFINED
};

// Control general del estado de la aplicación
ApplicationState state = UNDEFINED;



CREATE_APPLICATION( "Cing Demo" );


// Funciones varias

// Poner los materiales de todos los objetos transparentes
void setTransparentMaterials()
{
	Hemisferio_izq.setMaterial		( "Hemisferio_izq_transp" );
	Hemisferio_dere.setMaterial		( "Hemisferio_dere_transp" );	
	Putamenconsm.setMaterial		( "Putamenconsm_transp" );
	Globopalido_exterior.setMaterial( "Globopalido_exterior_transp" );
	Caudadoconms.setMaterial		( "Caudadoconms_transp" );
	Globopalido_interior.setMaterial( "Globopalido_interior_transp" );
	talamo.setMaterial				( "talamo_transp" );
}

void showAll3DModels( bool show )
{
	Hemisferio_izq.setVisible( show );		
	Hemisferio_dere.setVisible( show );		
	Putamenconsm.setVisible( show );		
	Globopalido_exterior.setVisible( show );
	Caudadoconms.setVisible( show );		
	Globopalido_interior.setVisible( show );
	talamo.setVisible( show );		    		
}

////////////////////////////////// FUNCIONES GENERALES EXPORTADAS A FLASH

// Cerrar app
Hikari::FlashValue exitApp( Hikari::FlashControl* caller, const Hikari::Arguments& args )
{
	exit();
	return FLASH_VOID;
}

// Mostrar/Ocultar hemisferios+
Hikari::FlashValue showHemisferios( Hikari::FlashControl* caller, const Hikari::Arguments& args )
{
	bool show = args.at(0).getBool();
	
	Hemisferio_izq.setVisible(show);
	Hemisferio_dere.setVisible(show);

	if ( show )
	{
		Hemisferio_izq.setScale(1);
		Hemisferio_dere.setScale(1);
	}
	else
	{
		Hemisferio_izq.setScale(0);
		Hemisferio_dere.setScale(0);
	}

	return FLASH_VOID;
}

// Mostrar/Ocultar visor
Hikari::FlashValue showVisor( Hikari::FlashControl* caller, const Hikari::Arguments& args )
{
	showVisorPlanos = args.at(0).getBool();
	
	return FLASH_VOID;
}

// Cambiar plano de cada eje
Hikari::FlashValue setIndexXAxis( Hikari::FlashControl* caller, const Hikari::Arguments& args )
{
	float plano = args.at(0).getNumber();

	// Modificar eje en el plano
	visor.setIndexXAxis( plano * (float)visor.getMaxXAxis() );

	return FLASH_VOID;
}

Hikari::FlashValue setIndexYAxis( Hikari::FlashControl* caller, const Hikari::Arguments& args )
{
	float plano = args.at(0).getNumber();

	// Modificar eje en el plano
	visor.setIndexYAxis( plano * (float)visor.getMaxYAxis() );

	return FLASH_VOID;
}

Hikari::FlashValue setIndexZAxis( Hikari::FlashControl* caller, const Hikari::Arguments& args )
{
	float plano = args.at(0).getNumber();

	// Modificar eje en el plano
	visor.setIndexZAxis( plano * (float)visor.getMaxZAxis() );

	return FLASH_VOID;
}

// Trasparencia de MRI
Hikari::FlashValue setAlphaVisor( Hikari::FlashControl* caller, const Hikari::Arguments& args )
{
	float alpha = args.at(0).getNumber();

	// Modificar eje en el plano
	visor.setTransparency( alpha );

	return FLASH_VOID;
}


// Set theshold for AR
Hikari::FlashValue setThreshold( Hikari::FlashControl* caller, const Hikari::Arguments& args )
{
	float threshold = args.at(0).getNumber();

	// Modificar eje en el plano
	arManager.setThreshold( threshold * 255.0f );

	return FLASH_VOID;
}

// Set app state
Hikari::FlashValue setCalibration( Hikari::FlashControl* caller, const Hikari::Arguments& args )
{
	bool calibration = args.at(0).getBool();

	// Cambiar estaodo de aplicación
	if ( calibration )
	{
		state = CALIBRATION;
		arManager.showDebugInfo( true );
	}
	else 
	{
		state = VISOR;
		arManager.showDebugInfo( false );
		showAll3DModels(true);
		visor.saveParameters();
	}

	return FLASH_VOID;
}


// Mouse dentro/fuera del menu
Hikari::FlashValue mouseEnterMenu( Hikari::FlashControl* caller, const Hikari::Arguments& args )
{
	mouseInMenu = true;
	return FLASH_VOID;
}

Hikari::FlashValue mouseExitMenu( Hikari::FlashControl* caller, const Hikari::Arguments& args )
{
	mouseInMenu = false;
	return FLASH_VOID;
}

////////////////////////////////// APLICACIÓN
void setup()
{
	size(appWidth, appHeight);
	applyCoordinateSystemTransform(NORMAL);
	showDebugOutput( false );

	// Init video capture 
	pCam=IPS3EyeLib::Create();
	pCam->SetFormat(IPS3EyeLib::GetFormatIndex(captureWidth,captureHeight,60));
	pCam->StartCapture();
	camera.init(captureWidth,captureHeight,RGB);   
	pBuffer = new BYTE[(captureWidth*captureHeight*24)/8];

	// Init ARCalibrationManager
	arManager.init(camera);

	// Create the virtual camera
	arCam = Cing::ogreSceneManager->createCamera( "arCam" );
	arCam->setPosition(Ogre::Vector3(0,0,0));
	arCam->lookAt(Ogre::Vector3(0,0,1000));
	arCam->roll(Ogre::Degree(180));
	arCam->setFOVy(Ogre::Degree(FOVy));
	arCam->setAspectRatio( (float)width / height ); 
	arCam->setNearClipDistance(0.05f);
	arCam->setFarClipDistance(2000.0f);

	// Add to the viewport
	GraphicsManager::getSingleton().getMainWindow().getOgreWindow()->removeAllViewports( );
	GraphicsManager::getSingleton().getMainWindow().getOgreWindow()->addViewport( arCam );

	// Create RaySceneQuery
	mRaySceneQuery = ogreSceneManager->createRayQuery(Ogre::Ray());
	triRayQuery.init();

	// Visor resonancias
	visor.init( "planosBase.xml", &arManager );
	//visorGUI.init( visor );

	// Objetos 3d 
	light.init( 255, 255, 255, 50, 40, 50 );

	Ogre::SceneNode* obj3dNode = visor.getSceneNode()->createChildSceneNode("modelsNode");
	obj3dNode->setScale(0.3,0.3,0.3);
	obj3dNode->pitch(Ogre::Degree(-90));

	// Cargar objectos 3d
	Hemisferio_izq.init( "Modelos3d/Hemisferio_izq.mesh", "Hemisferio_izq",						obj3dNode );
	Hemisferio_dere.init( "Modelos3d/Hemisferio_dere.mesh", "Hemisferio_dere",					obj3dNode );	
	Globopalido_exterior.init( "Modelos3d/Globopalido_exterior.mesh", "Globopalido_exterior",	obj3dNode );
	Globopalido_interior.init( "Modelos3d/Globopalido_interior.mesh", "Globopalido_interior",	obj3dNode );
	Putamenconsm.init( "Modelos3d/Putamenconsm.mesh", "Putamenconsm",							obj3dNode );
	Caudadoconms.init( "Modelos3d/Caudadoconms.mesh", "Caudadoconms",							obj3dNode );
	talamo.init( "Modelos3d/talamo.mesh", "talamo",												obj3dNode );

	// Transparentes por defecto
	setTransparentMaterials();

	// Flash
	FlashMovieManager::getSingleton().init();

	// Load gallery and video player

	// Gallery
	float flashWidth = 840*0.72f;   
	float flashHeight = 700*0.72f;
	float marginX = width -  flashWidth;
	float marginY = height -  flashHeight;
	//imageGallery.load( flashWidth, flashHeight, marginX/2.0, marginY/2.0 );
	//imageGallery.hide();

	// Load all galleries
	Hemisferio_izqGallery.load( flashWidth, flashHeight, marginX/2.0, marginY/2.0 );
	Hemisferio_dereGallery.load( flashWidth, flashHeight, marginX/2.0, marginY/2.0 );
	Globopalido_extGallery.load( flashWidth, flashHeight, marginX/2.0, marginY/2.0 );
	CaudadoconmsGallery.load( flashWidth, flashHeight, marginX/2.0, marginY/2.0 );
	Globopalido_intGallery.load( flashWidth, flashHeight, marginX/2.0, marginY/2.0 );
	talamoGallery.load( flashWidth, flashHeight, marginX/2.0, marginY/2.0 );
	
	// and video
	flashWidth = 540;
	flashHeight = 360;
	marginX = width -  flashWidth;
	marginY = height -  flashHeight;
	PutamenconsmVideoPlayer.load( "Flash\\estriadoGPi.flv", flashWidth, flashHeight, marginX/2.0, marginY/2.0 );

	// Hide all flash movies for now
	Hemisferio_izqGallery.hide();
	Hemisferio_dereGallery.hide();
	Globopalido_extGallery.hide();
	CaudadoconmsGallery.hide();
	Globopalido_intGallery.hide();
	talamoGallery.hide();
	PutamenconsmVideoPlayer.hide();

	// Load galleries and videos
	Hemisferio_izqGallery.loadGalleryXml( "hemisferio_izq.xml" );
	Hemisferio_dereGallery.loadGalleryXml( "palidotomia.xml" );
	Globopalido_extGallery.loadGalleryXml( "globo_palido_ext.xml" );
	Globopalido_intGallery.loadGalleryXml( "globo_palido_int.xml" );
	CaudadoconmsGallery.loadGalleryXml( "caudadoconms.xml" );
	talamoGallery.loadGalleryXml( "talamo.xml" );


	// Vide player
	//flashWidth = 540;
	//flashHeight = 360;
	//marginX = width -  flashWidth;
	//marginY = height -  flashHeight;
	//videoPlayer.load( "Flash\\estriadoGPi.flv", flashWidth, flashHeight, marginX/2.0, marginY/2.0 );
	//videoPlayer.hide();


	// Main Menu
	mainMenu.load("AbadiaNet_AR_UI.swf", 512, 512, 0, 0 );
	mainMenu.getFlashControl()->setDraggable(false);

	// Regisger global functions for the main menu
	mainMenu.registerCFunction("exitApp", Hikari::FlashDelegate(&exitApp) );
	mainMenu.registerCFunction("showHemisferios",	&showHemisferios);
	mainMenu.registerCFunction("setIndexXAxis",		&setIndexXAxis);
	mainMenu.registerCFunction("setIndexYAxis",		&setIndexYAxis);
	mainMenu.registerCFunction("setIndexZAxis",		&setIndexZAxis);
	mainMenu.registerCFunction( "mouseEnterMenu",	&mouseEnterMenu );
	mainMenu.registerCFunction( "mouseExitMenu",	&mouseExitMenu );
	mainMenu.registerCFunction( "showVisor",		&showVisor );
	mainMenu.registerCFunction( "setThreshold",		&setThreshold );
	mainMenu.registerCFunction( "setCalibration",	&setCalibration );
	mainMenu.registerCFunction( "setAlphaVisor",	&setAlphaVisor );

	// Calibrator GUI
	//calibController.load( "IncrementalTransformControl.swf", 256, 256, 16, 16 );
	calibGUI.init( visor.getSceneNode(), &visor, obj3dNode, mainMenu );

	// Vincular el Visor con el nodo de ARToolkit
	ogreSceneManager->getRootSceneNode()->removeChild( visor.getSceneNode() );
	arManager.m_calculatedOrigin.getSceneNode()->setScale(1,1,1);
	visor.getSceneNode()->setInheritScale(false);
	arManager.m_OriginNode->m_sceneNode->addChild( visor.getSceneNode() );

	//arManager.m_calculatedOrigin.getSceneNode()->setScale(.5,.5,.5);
}

void draw()
{
	// Update  video and tracking here
	if(pCam->GetFrame(pBuffer, 24, false, false))
	{	
		camera.setData((char*)pBuffer,captureWidth,captureHeight,RGB);
		arManager.update(camera);
	}

	// Draw video and overlay info
	camera.setUpdateTexture(true);
	camera.drawBackground( 0, 0, width, height );

	// Flash
	FlashMovieManager::getSingleton().update();

	// Visor
	if ( showVisorPlanos )
		visor.draw();

	// Store flash visibility
	flashVisible = Hemisferio_izqGallery.isVisible() || Hemisferio_dereGallery.isVisible() ||
					Globopalido_extGallery.isVisible()|| CaudadoconmsGallery.isVisible()	||
					Globopalido_intGallery.isVisible()|| talamoGallery.isVisible()			||
					PutamenconsmVideoPlayer.isVisible();	
}

void end()
{
	// Save calibration parameters
	visor.saveParameters();

	// Stop eyetoy capture
	pCam->StopCapture();
	
	// Flash
	//videoPlayer.end();
	//imageGallery.end();

	// TODO: NOTA -> HIKARI PETA AL SALIR... DE MOMENTO memory leak
	//FlashMovieManager::getSingleton().end();

	// Destroy ray query
	ogreSceneManager->destroyQuery( mRaySceneQuery );

	// Visor
	visor.end();
}


void mousePressed()
{
	// Si es el botón derecho no hacemos nada aquí (es sólo para mover flash movies)
	if ( mouseButton != 0 )
		return;

	// Si hay algún flash asociado a modelos cargado -> no chequear colisión con modelos
	// Si el mouse está dentro del menú tampoco
	if ( /*flashVisible ||*/ mouseInMenu )
		return;


// Query para detectar clicks en objetos
	
		Ogre::Entity* entity = triRayQuery.execute( arCam );
		if ( entity )
		{
			// Get entity name
			const String& name = entity->getName();

			// Find object that was clicked
			if ( name.find( "Hemisferio_izq" ) != String::npos )
			{
				println( "Clickado: Hemisferio_izq" );

				// Abrir Imagen 1.jpg:
				Hemisferio_izqGallery.show();
				Hemisferio_izqGallery.resetPosition();
				Hemisferio_izqGallery.focus();

				// Materiales (resto transparente, y este opaco
				setTransparentMaterials();
				Hemisferio_izq.setMaterial( "Hemisferio_izq" );
			}

			else if ( name.find( "Hemisferio_dere" ) != String::npos )
			{
				println( "Clickado: Hemisferio_dere" );

				// Abrir: Imagen palidotomia.png
				Hemisferio_dereGallery.show();
				Hemisferio_dereGallery.resetPosition();
				Hemisferio_dereGallery.focus();

				// Materiales (resto transparente, y este opaco
				setTransparentMaterials();
				Hemisferio_dere.setMaterial( "Hemisferio_dere" );
			}

			else if ( name.find( "Putamenconsm" ) != String::npos )
			{
				println( "Clickado: Putamenconsm" );
				
				// Abrir video player
				//PutamenconsmVideoPlayer.callFlashFunction("rewind", FlashMovie::Args()); 
				PutamenconsmVideoPlayer.show();
				PutamenconsmVideoPlayer.resetPosition();
				PutamenconsmVideoPlayer.focus();

				// Materiales (resto transparente, y este opaco
				setTransparentMaterials();
				Putamenconsm.setMaterial( "Putamenconsm" );
			}

			else if ( name.find( "Globopalido_exte" ) != String::npos )
			{
				println( "Clickado: Globopalido_exte" );

				// Abrir -Imagen 3.jpg
				Globopalido_extGallery.show();
				Globopalido_extGallery.resetPosition();
				Globopalido_extGallery.focus();

				// Materiales (resto transparente, y este opaco
				setTransparentMaterials();
				Globopalido_exterior.setMaterial( "Globopalido_exterior" );
			}

			else if ( name.find( "Globopalido_interior" ) != String::npos )
			{
				println( "Clickado: Globopalido_interior" );
				
				// Abrir Texto + galería
				Globopalido_intGallery.show();
				Globopalido_intGallery.resetPosition();
				Globopalido_intGallery.focus();

				// Materiales (resto transparente, y este opaco
				setTransparentMaterials();
				Globopalido_interior.setMaterial( "Globopalido_interior" );
			}

			else if ( name.find( "Caudadoconms" ) != String::npos )
			{
				println( "Clickado: Caudadoconms" );

				// Abrir Imagen 2.jpg:
				CaudadoconmsGallery.show();
				CaudadoconmsGallery.resetPosition();
				CaudadoconmsGallery.focus();
				
				// Materiales (resto transparente, y este opaco
				setTransparentMaterials();
				Caudadoconms.setMaterial( "Caudadoconms" );
			}

			else if ( name.find( "talamo" ) != String::npos )
			{
				println( "Clickado: talamo" );

				// Abrir Imagen talamo.gif 
				talamoGallery.show();
				talamoGallery.resetPosition();
				talamoGallery.focus();

				// Materiales (resto transparente, y este opaco
				setTransparentMaterials();
				talamo.setMaterial( "talamo" );
			}
		}

// Query para el Ratíon y AR

	// Setup the ray scene query using the mouse position
	Ogre::Ray mouseRay = arCam->getCameraToViewportRay( mouseX/(float)width, mouseY/(float)height);
	mRaySceneQuery->setRay(mouseRay);

	// Execute query
	Ogre::RaySceneQueryResult& result = mRaySceneQuery->execute();

	// Iterate trough results
	Ogre::RaySceneQueryResult::iterator itr = result.begin( );
	bool object3DClicked = false;
	for( ; itr != result.end() ; itr++ )
	{
		// If entity has a behaviour associated...
		if ( !(itr->movable->getUserAny().isEmpty()) )
		{
			const Ogre::Any&	any		 = itr->movable->getUserAny();
			ARMarkerObject*		theMarker = Ogre::any_cast<ARMarkerObject*>(any);
			
			// Select origin marker
			if (arManager.getOriginMarker() == NULL)
			{
				arManager.setOriginMarker(theMarker);
				//itr->movable->getParentSceneNode()->showBoundingBox(true);

				Cing::Box model;
				model.init(27,27,1);
				model.getSceneNode()->getParentSceneNode()->removeChild(model.getSceneNode());
				model.setMaterial("AR_DebugModel");

				// Remove model from triangle ray queries
				model.getEntity()->removeQueryFlags( Ogre::SceneManager::WORLD_GEOMETRY_TYPE_MASK );

				itr->movable->getParentSceneNode()->addChild(model.getSceneNode());
				model.getSceneNode()->setInheritScale(false);	

				// Cambio de estado a visor, si no se ha definido todavía (para ocultar debug de AR)
				if ( state == UNDEFINED )
				{
					state = VISOR;
					arManager.showDebugInfo( false );
					showAll3DModels(true);
				}
			}
		}	
	}	
}

void mouseMoved()
{
}

void mouseReleased()
{
}

void keyPressed()
{

	switch( key )
	{
	case 's':
		//arManager.saveCalibration( "fieldMark.xml");
		visor.saveParameters();
		break;

	case '7':
			//videoPlayer.load( "Flash\\theviewer.flv", 600, 500, 0, 0 );
	break;

	};
}