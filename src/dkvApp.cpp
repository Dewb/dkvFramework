
#include "Kinect.h"
#include "Resources.h"

#include "KVEffect.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class dkvFrameworkApp : public AppBasic {
  public:
	void prepareSettings( Settings* settings );
	void setup();
	void createVbo();
	void update();
	void draw();
	
	virtual void keyDown(KeyEvent event);
	virtual void resize(ResizeEvent event);
	
	void loadMovieFile( const string &moviePath );

	
	// PARAMS
	params::InterfaceGl	mParams;
	
	// CAMERA
	CameraPersp		mCam;
	Quatf			mSceneRotation;
	Vec3f			mEye, mCenter, mUp;
	float			mCameraDistance;
	float			mKinectTilt;
	float			mClipDistance;
	float			mDepthFactor;
	float			mInstanceAngle[3];
	float			mInstanceRotation[3];
	float			mInstanceColor[3];

	float			mInstanceRotationSpeed[3];
	float			mOverallRotation;
	float			mOverallRotationSpeed;
	
	Vec2f			mTextureOffset;
	Vec2f			mTextureScale;
	
	bool			mUseInfrared;
	
	float			mDepthBrightness;
	bool			mBool;
	bool			mShowInterface;
	
	// KINECT AND TEXTURES
	vector<KinectManager*> m_vKinects;
	vector<KVEffect*> m_vEffects;
	typedef vector<KinectManager*>::iterator KinectManagerIter;
	typedef vector<KVEffect*>::iterator KVEffectIter;
	
	// BACKGROUND VIDEO
	qtime::MovieGl mMovie;
	gl::Texture	mFrameTexture;

};

void dkvFrameworkApp::prepareSettings( Settings* settings )
{
	settings->setWindowSize( 1280, 720 );
}

void dkvFrameworkApp::setup()
{	
	// SETUP PARAMS
	mParams = params::InterfaceGl( "KVFX proto @dewb", Vec2i( 200, 480 ) );
	mParams.addParam( "Scene Rotation", &mSceneRotation, "opened=1" );
	mParams.addParam( "Cam Distance", &mCameraDistance, "min=100.0 max=5000.0 step=100.0 keyIncr=z keyDecr=Z" );
	mParams.addParam( "Kinect Tilt", &mKinectTilt, "min=-31 max=31 keyIncr=T keyDecr=t" );
	mParams.addParam( "Clip distance", &mClipDistance, "min=-5 max=5 step=0.01 keyIncr=c keyDecr=C");
	mParams.addParam( "Depth factor", &mDepthFactor, "min=-10000 max=10000 step=100 keyIncr=d keyDecr=D");
	mParams.addParam( "Depth brightness", &mDepthBrightness, "min=0 max=1 step=0.05");
	mParams.addParam( "Colorize", &mBool);
	mParams.addSeparator();	
	mParams.addParam( "1 Angle", &(mInstanceAngle[0]), "min=-360 max=360 step=1 keyIncr=1 keyDecr=!");
	mParams.addParam( "2 Angle", &(mInstanceAngle[1]), "min=-360 max=360 step=1 keyIncr=2 keyDecr=@");
	mParams.addParam( "3 Angle", &(mInstanceAngle[2]), "min=-360 max=360 step=1 keyIncr=3 keyDecr=#");
	mParams.addParam( "Overall Angle", &(mOverallRotation), "min=-360 max=360 step=1 keyIncr=4 keyDecr=$");	
	mParams.addSeparator();	
	mParams.addParam( "1 Speed", &(mInstanceRotationSpeed[0]), "min=-360 max=360 step=0.1 keyIncr=q keyDecr=Q");
	mParams.addParam( "2 Speed", &(mInstanceRotationSpeed[1]), "min=-360 max=360 step=0.1 keyIncr=w keyDecr=W");
	mParams.addParam( "3 Speed", &(mInstanceRotationSpeed[2]), "min=-360 max=360 step=0.1 keyIncr=e keyDecr=E");
	mParams.addParam( "Overall Speed", &(mOverallRotationSpeed), "min=-360 max=360 step=0.1 keyIncr=r keyDecr=R");
	mParams.addSeparator();	
	mParams.addParam( "Texture offset X", &(mTextureOffset[0]), "min=-1 max=1 step=0.001");
	mParams.addParam( "Texture offset Y", &(mTextureOffset[1]), "min=-1 max=1 step=0.001");
	mParams.addParam( "Texture scale X", &(mTextureScale[0]), "min=-2 max=2 step=0.001");
	mParams.addParam( "Texture scale Y", &(mTextureScale[1]), "min=-2 max=2 step=0.001");
	
	
	// SETUP CAMERA
	mCameraDistance = 1000.0f;
	mEye			= Vec3f( 0.0f, mCameraDistance, 0.0f );
	mCenter			= Vec3f::zero();
	mUp				= Vec3f::zAxis();
	mCam.setPerspective( 75.0f, getWindowAspectRatio(), 1.0f, 8000.0f );
	mClipDistance = 0.22;
	mDepthFactor = 3500;
	mInstanceAngle[0] = 0.0;
	mInstanceAngle[1] = 90.0;
	mInstanceAngle[2] = 180.0;
	mInstanceColor[0] = 0.0;
	mInstanceColor[1] = 0.0;
	mInstanceColor[3] = 0.0;
	

	mInstanceRotationSpeed[0] = 0.0;
	mInstanceRotationSpeed[1] = 0.0;
	mInstanceRotationSpeed[2] = 0.0;
	mOverallRotationSpeed = 0.0;
	
	mTextureOffset = Vec2f(-0.043, 0.065);
	mTextureScale = Vec2f(1.0, 1.0);
	
	mUseInfrared = false;
	mKinectTilt = 0;
	mDepthBrightness = 0.1;
	
	// SETUP KINECT AND TEXTURES
	int numKinects = Kinect::getNumDevices();
	console() << "There are " << numKinects << " Kinects connected." << std::endl;
	
	for (int ii=0; ii<numKinects; ii++)
	{
		m_vKinects.push_back(new KinectManager());
		m_vEffects.push_back(createBasicPtCdKVEffect(m_vKinects[ii], &(mInstanceRotation[ii % 3]), &(mInstanceColor[ii % 3])));
	}
	
	// SETUP GL
	gl::enableDepthWrite();
	gl::enableDepthRead();
	
	mBool = false;
	mShowInterface = false;
	setFullScreen(true);

}

void dkvFrameworkApp::loadMovieFile( const string &moviePath )
{
	try {
		// load up the movie, set it to loop, and begin playing
		mMovie = qtime::MovieGl( moviePath );
		mMovie.setLoop();
		while (!mMovie.checkPlayable())
		{
		}
		mMovie.play();
	}
	catch( ... ) {
		console() << "Unable to load the movie." << std::endl;
		mMovie.reset();
		return;
	}
	
	mFrameTexture.reset();
}	

void dkvFrameworkApp::update()
{
	KinectManagerIter iterKM = m_vKinects.begin();
	while( iterKM != m_vKinects.end())
	{
		KinectManager* pKM = *iterKM;
		
		if( pKM->m_Kinect.checkNewDepthFrame() && pKM->m_Kinect.checkNewVideoFrame())
		{
			pKM->m_DepthTexture = pKM->m_Kinect.getDepthImage();
			pKM->m_VideoTexture = pKM->m_Kinect.getVideoImage();
			
			KVEffectIter iterKVE = m_vEffects.begin();
			while (iterKVE != m_vEffects.end())
			{
				KVEffect* pKVE = *iterKVE;
				if (pKVE->getKinect() == pKM)
					pKVE->update();
				
				iterKVE++;
			}
			
			iterKM++;
		}

		if( mKinectTilt != pKM->m_Kinect.getTilt() )
			pKM->m_Kinect.setTilt( mKinectTilt );
		
		pKM->m_fClipDistance = mClipDistance;
		pKM->m_fDepthFactor = mDepthFactor;
		pKM->m_textureOffset = mTextureOffset;
		pKM->m_textureScale = mTextureScale;
		pKM->m_fDepthBrightness = mDepthBrightness;
		pKM->m_bBool = mBool;
		
	}
	
	mEye = Vec3f( 0.0f, mCameraDistance, 0.0f  );
	mCam.lookAt( mEye, mCenter, mUp );
	gl::setMatrices( mCam );
	
	for (int i=0; i<2; i++)
	{
		mInstanceRotation[i] = (mInstanceRotationSpeed[i] != 0.0) ? mInstanceRotation[i] + mInstanceRotationSpeed[i] : mInstanceAngle[i];
		if (mInstanceRotation[i] > 360.0)
			mInstanceRotation[i] -= 360.0;
		if (mInstanceRotation[i] < -360.0)
			mInstanceRotation[i] += 360.0;
	}
		
	mOverallRotation += mOverallRotationSpeed;
	if (mOverallRotation > 360.0) mOverallRotation -= 360.0;
	if (mOverallRotation < -360.0) mOverallRotation += 360.0;

	if(mMovie)
	{
		mFrameTexture = mMovie.getTexture();
	}
}

void dkvFrameworkApp::draw()
{
	//gl::disableDepthRead();

	gl::clear( Color( 0.0f, 0.0f, 0.0f ), true );
	
	if( mFrameTexture ) {
		Rectf centeredRect = Rectf( mFrameTexture.getBounds() ).getCenteredFit( getWindowBounds(), true );
		gl::draw( mFrameTexture, centeredRect  );
	}
	
	gl::pushMatrices();
	gl::scale( Vec3f( 1.0f, 1.0f, 1.0f ) );
	
	gl::rotate( mSceneRotation ); // quarternion
	gl::rotate(mOverallRotation); // z-axis
	
	KVEffectIter iterKVE = m_vEffects.begin();
	while (iterKVE != m_vEffects.end())
	{
		gl::pushMatrices();
		(*iterKVE++)->draw();
		gl::popMatrices();
	}
	
	gl::popMatrices();
	
	if (mShowInterface)
		params::InterfaceGl::draw();
}

void dkvFrameworkApp::keyDown( KeyEvent event )
{
	if( event.getChar() == 'i' ) {
		mUseInfrared = !mUseInfrared;
	}
	if( event.getChar() == 'f' ) {
		setFullScreen( ! isFullScreen() );
	}
	if (event.getChar() == 'u' ) {
		mSceneRotation = Quatf(0,0,0,0);
	}
	if (event.getChar() == 'l' ) {
		string moviePath = getOpenFilePath();
		if( ! moviePath.empty() )
		{
			loadMovieFile(moviePath);
		}
		else 
		{
			mMovie.reset();
			mFrameTexture.reset();
		}
	}
	if (event.getChar() == ' ') {
		if (mMovie && mMovie.isPlaying())
			mMovie.stop();
		else if (mMovie)
			mMovie.play();
	}
	if( event.getChar() == '?' ) {
		delete m_vEffects[0];
		m_vEffects[0] = createEightBitsKVEffect(m_vKinects[0], &(mInstanceRotation[0]), &(mInstanceColor[0]));
		delete m_vEffects[1];
		m_vEffects[1] = createEightBitsKVEffect(m_vKinects[1], &(mInstanceRotation[1]), &(mInstanceColor[1]));
	}
	if( event.getChar() == '/' ) {
		delete m_vEffects[0];
		m_vEffects[0] = createBasicPtCdKVEffect(m_vKinects[0], &(mInstanceRotation[0]), &(mInstanceColor[0]));
		delete m_vEffects[1];
		m_vEffects[1] = createBasicPtCdKVEffect(m_vKinects[1], &(mInstanceRotation[1]), &(mInstanceColor[1]));
	}
	if (event.getChar() == 'v' )
	{
		mBool = !mBool;
	}
	if (event.getChar() == 'h' )
	{
		mShowInterface = !mShowInterface;
	}	
}	

void dkvFrameworkApp::resize( ResizeEvent event )
{
	mCam.setPerspective( 75.0f, getWindowAspectRatio(), 1.0f, 8000.0f );
}


CINDER_APP_BASIC( dkvFrameworkApp, RendererGl )
