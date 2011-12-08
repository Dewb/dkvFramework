/*
 *  KVEffect.cpp
 *  Created by Michael Dewberry on 7/22/11.
 *
 */

#include "KVEffect.h"
#include "cinder/gl/Vbo.h"
#include "cinder/gl/GlslProg.h"
#include "Resources.h"
#include "cinder/app/AppBasic.h"

using namespace ci;


typedef gl::VboMesh VboType;
typedef gl::GlslProg ShaderType;

static int s_nActiveKinects = 0;

KinectManager::KinectManager()
: m_Kinect(s_nActiveKinects++)
{
	m_DepthTexture = gl::Texture(640, 480);
	m_VideoTexture = gl::Texture(640, 480);
	m_textureOffset = Vec2f(0.052, 0.101);
	m_textureScale = Vec2f(1,1);
}

KinectManager::~KinectManager()
{
}

Vec2f KinectManager::getTextureOffset()
{
	return (m_Kinect.isVideoInfrared()) ? Vec2f(0,0) : m_textureOffset;
}

Vec2f KinectManager::getTextureScale()
{
	return (m_Kinect.isVideoInfrared()) ? Vec2f(1,1) : m_textureScale;
}

class KVEffectVboImpl : public KVEffect
{

public:
	KVEffectVboImpl() : m_pKinect(NULL), m_pVbo(NULL), m_pShader(NULL) {};
	
	virtual void init(KinectManager* pKinect) 
	{ 
		setKinect(pKinect); 
		
		if (m_pVbo)
			delete m_pVbo;
		m_pVbo = initVbo(); 

		if (m_pShader)
			delete m_pShader;
		m_pShader = initShader();
	}
	
	virtual ~KVEffectVboImpl() { delete m_pVbo; delete m_pShader; }

public:
	virtual void update() = 0;
	virtual void draw() = 0;

protected:
	virtual VboType* initVbo() = 0;
	virtual ShaderType* initShader() = 0;
protected:	
	virtual KinectManager* getKinect() const { return m_pKinect; }
	virtual void setKinect(KinectManager* pKinect) { m_pKinect = pKinect; }
	virtual VboType* getVbo() const { return m_pVbo; }
	virtual ShaderType* getShader() const { return m_pShader; }

private:
	KinectManager* m_pKinect;
	VboType* m_pVbo;
	ShaderType* m_pShader;
};

class KVEBasicPointCloud : public KVEffectVboImpl
{
public:
	KVEBasicPointCloud(float* pAngle, float* pColor) 
	: m_pAngle(pAngle)
	, m_pColor(pColor) 
	{
	}
	
	virtual void update()
	{
	}
	
	virtual void draw()
	{
		gl::rotate(*m_pAngle);
		getKinect()->m_DepthTexture.bind(0);
		getShader()->bind();
		getShader()->uniform("depthTex", 0);
		getShader()->uniform("clipDistance", getKinect()->m_fClipDistance);
		getShader()->uniform("depthFactor", getKinect()->m_fDepthFactor);
		gl::draw( *getVbo() );
		getShader()->unbind();	
	}
	
protected:
	virtual VboType* initVbo()
	{
		int VBO_X_RES  = 640;
		int VBO_Y_RES  = 480;
		gl::VboMesh::Layout layout;
		
		layout.setStaticPositions();
		layout.setStaticTexCoords2d();
		layout.setStaticIndices();
		
		std::vector<Vec3f> positions;
		std::vector<Vec2f> texCoords;
		std::vector<uint32_t> indices; 
		
		int numVertices = VBO_X_RES * VBO_Y_RES;
		int numShapes	= ( VBO_X_RES - 1 ) * ( VBO_Y_RES - 1 );
				
		for( int x=0; x<VBO_X_RES; ++x ){
			for( int y=0; y<VBO_Y_RES; ++y ){
				indices.push_back( x * VBO_Y_RES + y );
				
				float xPer	= x / (float)(VBO_X_RES-1);
				float yPer	= y / (float)(VBO_Y_RES-1);
				
				positions.push_back( Vec3f( ( xPer * 2.0f - 1.0f ) * VBO_X_RES, 0.0f, ( yPer * 2.0f - 1.0f ) * VBO_Y_RES ) );
				texCoords.push_back( Vec2f( (1-xPer), (1-yPer) ) );			
			}
		}
		
		VboType *pVbo = new VboType( numVertices, numShapes, layout, GL_POINTS );
	
		pVbo->bufferPositions( positions );
		pVbo->bufferIndices( indices );
		pVbo->bufferTexCoords2d( 0, texCoords );		
	
		return pVbo;
	}
	
	virtual ShaderType* initShader()
	{
		ShaderType* pShader	= new ShaderType( app::loadResource( RES_BASIC_VERT_ID ), app::loadResource( RES_BASIC_FRAG_ID ) );
		return pShader;
	}
	
	float* m_pAngle;
	float* m_pColor;
};	

class KVESnowflake : public KVEffectVboImpl
{
public:
	KVESnowflake(float* pAngle, float* pColor) 
	: m_pAngle(pAngle)
	, m_pColor(pColor) 
	{
	}
	
	virtual void update()
	{
	}
	virtual void draw()
	{
	}
	
protected:
	virtual VboType* initVbo()
	{
		return NULL;
	}
	virtual ShaderType* initShader()
	{
		return NULL;
	}
	
	float* m_pAngle;
	float* m_pColor;
};


class KVE8Bit : public KVEffectVboImpl
{
public:
	KVE8Bit(float* pAngle, float* pColor) 
	: m_pAngle(pAngle)
	, m_pColor(pColor) 
	{
	}
	
	virtual void update()
	{
	}
	virtual void draw()
	{
		gl::rotate(*m_pAngle);
		getKinect()->m_DepthTexture.bind(0);
		getKinect()->m_VideoTexture.bind(1);
		getShader()->bind();
		getShader()->uniform("depthTex", 0);
		getShader()->uniform("colorTex", 1);
		getShader()->uniform("clipDistance", getKinect()->m_fClipDistance);
		getShader()->uniform("depthFactor", getKinect()->m_fDepthFactor);
		getShader()->uniform("depthBrightness", 0.2f);
		getShader()->uniform( "offset", getKinect()->getTextureOffset());
		getShader()->uniform( "scale", getKinect()->getTextureScale());
		getShader()->uniform("depthBrightness", getKinect()->m_fDepthBrightness);
		getShader()->uniform("colorize", getKinect()->m_bBool ? 1.0f : 0.0f);
		gl::draw( *getVbo() );
		getShader()->unbind();			
	}
	
protected:
	virtual VboType* initVbo()
	{		
		int PIXELGRID_X_RES  = 48;
		int PIXELGRID_Y_RES  = 32;
		int PIXEL_X = 20;
		int PIXEL_Y = 24;
		int PIXEL_MARGIN_X = 4;
		int PIXEL_MARGIN_Y = 4;
	
		int VBO_X_RES  = (PIXEL_X+PIXEL_MARGIN_X)*PIXELGRID_X_RES;
		int VBO_Y_RES  = (PIXEL_Y+PIXEL_MARGIN_Y)*PIXELGRID_Y_RES;

		gl::VboMesh::Layout layout;
		
		layout.setStaticPositions();
		layout.setStaticTexCoords2d();
		layout.setStaticIndices();
		
		std::vector<Vec3f> positions;
		std::vector<Vec2f> texCoords;
		std::vector<uint32_t> indices; 
		
		int numVertices = PIXELGRID_X_RES * PIXELGRID_Y_RES * 8;
		int numQuads	= PIXELGRID_X_RES * PIXELGRID_Y_RES * 5;
		
		for( int x=0; x<PIXELGRID_X_RES; ++x ){
			for( int y=0; y<PIXELGRID_Y_RES; ++y ){
				
				Vec2f pixelStart;
				pixelStart.x = 0 + (PIXEL_X+PIXEL_MARGIN_X)*x;
				pixelStart.y = 0 + (PIXEL_Y+PIXEL_MARGIN_Y)*y;
				float z = PIXEL_X;
				
				int idx = x*PIXELGRID_Y_RES*8 + y*8;
				
				float px1 = ((pixelStart.x/(float)VBO_X_RES)*2.0f - 1.0f) * 640.0;
				float px2 = (((pixelStart.x + PIXEL_X)/(float)VBO_X_RES)*2.0f - 1.0f) * 640.0;
				float py1 = ((pixelStart.y/(float)VBO_Y_RES)*2.0f - 1.0f) * 480.0;
				float py2 = (((pixelStart.y + PIXEL_Y)/(float)VBO_Y_RES)*2.0f - 1.0f) * 480.0;
				
				positions.push_back( Vec3d(px1, 0.0f, py1) );
				positions.push_back( Vec3d(px2, 0.0f, py1) );
				positions.push_back( Vec3d(px2, 0.0f, py2) );
				positions.push_back( Vec3d(px1, 0.0f, py2) );
				positions.push_back( Vec3d(px1,   -z, py1) );
				positions.push_back( Vec3d(px2,   -z, py1) );
				positions.push_back( Vec3d(px2,   -z, py2) );
				positions.push_back( Vec3d(px1,   -z, py2) );
				
				indices.push_back( idx+0 );
				indices.push_back( idx+1 );
				indices.push_back( idx+2 );
				indices.push_back( idx+3 );
				
				indices.push_back( idx+0 );
				indices.push_back( idx+1 );
				indices.push_back( idx+5 );
				indices.push_back( idx+4 );
				
				indices.push_back( idx+0 );
				indices.push_back( idx+4 );
				indices.push_back( idx+7 );
				indices.push_back( idx+3 );
				
				indices.push_back( idx+3 );
				indices.push_back( idx+2 );
				indices.push_back( idx+6 );
				indices.push_back( idx+7 );
				
				indices.push_back( idx+1 );
				indices.push_back( idx+2 );
				indices.push_back( idx+6 );
				indices.push_back( idx+5 );
				
				float xPer	= x / (float)(PIXELGRID_X_RES);
				float yPer	= y / (float)(PIXELGRID_Y_RES);
				
				float xPer1	= (x+0) / (float)(PIXELGRID_X_RES);
				float yPer1	= (y+0) / (float)(PIXELGRID_Y_RES);
				
				texCoords.push_back( Vec2f( 1-xPer,  1-yPer ) );			
				texCoords.push_back( Vec2f( 1-xPer1, 1-yPer ) );			
				texCoords.push_back( Vec2f( 1-xPer1, 1-yPer1 ) );			
				texCoords.push_back( Vec2f( 1-xPer,  1-yPer1 ) );			
				texCoords.push_back( Vec2f( 1-xPer,  1-yPer ) );			
				texCoords.push_back( Vec2f( 1-xPer1, 1-yPer ) );			
				texCoords.push_back( Vec2f( 1-xPer1, 1-yPer1 ) );			
				texCoords.push_back( Vec2f( 1-xPer,  1-yPer1 ) );			
			}
		}
		
		VboType *pVbo = new VboType( numVertices, numQuads*4, layout, GL_QUADS );
	
		pVbo->bufferIndices( indices );
		pVbo->bufferTexCoords2d( 0, texCoords );
		pVbo->bufferPositions( positions );
		return pVbo;
	}
	virtual ShaderType* initShader()
	{
		ShaderType* pShader	= new ShaderType( app::loadResource( RES_8BIT_VERT_ID ), app::loadResource( RES_8BIT_FRAG_ID ) );
		return pShader;
	}
	
	float* m_pAngle;
	float* m_pColor;
};


class KVETriDancer : public KVEffectVboImpl
{
public:
	KVETriDancer(float* pAngle, float* pColor) 
	: m_pAngle(pAngle)
	, m_pColor(pColor) 
	{
	}
	
	virtual void update()
	{
	}
	virtual void draw()
	{
	}
	
protected:
	virtual VboType* initVbo()
	{
		return NULL;
	}
	virtual ShaderType* initShader()
	{
		return NULL;
	}
	
	float* m_pAngle;
	float* m_pColor;
};

KVEffect* createBasicPtCdKVEffect(KinectManager* pKinect, float* pParam1, float* pParam2) 
{
	KVEffect* pKVE = new KVEBasicPointCloud(pParam1, pParam2);
	pKVE->init(pKinect);
	return pKVE;
}

KVEffect* createSnowflakeKVEffect(KinectManager* pKinect, float* pParam1, float* pParam2) 
{
	return NULL;
}

KVEffect* createEightBitsKVEffect(KinectManager* pKinect, float* pParam1, float* pParam2)
{
	KVEffect* pKVE = new KVE8Bit(pParam1, pParam2);
	pKVE->init(pKinect);
	return pKVE;
}

KVEffect* createTriDancerKVEffect(KinectManager* pKinect, float* pParam1, float* pParam2)
{
	KVEffect* pKVE = new KVETriDancer(pParam1, pParam2);
	pKVE->init(pKinect);
	return pKVE;
}
