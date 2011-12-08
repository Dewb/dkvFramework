/*
 *  KVEffect.h
 *  Created by Michael Dewberry on 7/22/11. 
 *
 */

#include "Kinect.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"

class KinectManager
{
public:
	KinectManager();
	~KinectManager(); 
	ci::gl::Texture m_DepthTexture;
	ci::gl::Texture m_VideoTexture;
	ci::Kinect m_Kinect;
	float m_fClipDistance;
	float m_fDepthFactor;
	float m_fDepthBrightness;
	bool m_bBool;
	ci::Vec2f getTextureOffset();
	ci::Vec2f getTextureScale();
	ci::Vec2f m_textureOffset;
	ci::Vec2f m_textureScale;
};

class KVEffect
{
public:
	virtual void init(KinectManager* pKinect) = 0;
	virtual void update() = 0;
	virtual void draw() = 0;
	
	virtual KinectManager* getKinect() const = 0;
	virtual void setKinect(KinectManager* pKinect) = 0;
};


KVEffect* createBasicPtCdKVEffect(KinectManager* pKinect, float* pParam1, float* pParam2);
KVEffect* createSnowflakeKVEffect(KinectManager* pKinect, float* pParam1, float* pParam2);
KVEffect* createEightBitsKVEffect(KinectManager* pKinect, float* pParam1, float* pParam2);
KVEffect* createTriDancerKVEffect(KinectManager* pKinect, float* pParam1, float* pParam2);
