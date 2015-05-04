#pragma once
#include "ofMain.h"
#include "ofxKinect.h"
#include "ofxCv.h"
#include "ofxBox2d.h"
#include "Styling.h"

#define CATEGORY_TRACKING   0xFFFF;
#define CATEGORY_PARTICLES  0x0001;

// ------------------------------------------------- a simple extended box2d circle
class CustomParticle : public ofxBox2dCircle {
	
public:
	CustomParticle() {
	}
	ofColor color;
	void draw() {
		float radius = getRadius();
		
		glPushMatrix();
		glTranslatef(getPosition().x, getPosition().y, 0);
		
		ofSetColor(color.r, color.g, color.b);
		ofFill();
		ofCircle(0, 0, radius);
		
		glPopMatrix();
		
	}
};


// -------------------------------------------------
class ofApp : public ofBaseApp {
	
public:
	
	void setup();
	void update();
	void draw();
    void exit();
	
	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y);
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void resized(int w, int h);
    static bool shouldRemoveOffScreen(ofPtr<ofxBox2dBaseShape> shape);
	
    // Kinect + OpenCV vars
    ofxKinect kinect;
	ofxCv::ContourFinder contourFinder;
    
    ofImage colorImg;
    ofImage grayImage;         // grayscale depth image
    ofImage grayThreshNear;    // the near thresholded image
	ofImage grayThreshFar;     // the far thresholded image
    ofImage grayPreprocImage;  // grayscale pre-processed image
    // @ToDo : Scale polyline not image
    ofImage grayImageScaled; // for getting the ize of our screen
    
    Styling style; // for storing style infos
    
    // continous particle rain
    
    const int numberOfParticles = 1000; // upper bounds particle
	
    int nearThreshold;
	int farThreshold;
    int angle;
    
    // Box2D vars
	float                                   px, py;
	bool                                    bDrawLines;
	bool                                    bMouseForce;
	
	ofxBox2d                                box2d;			  //	the box2d world
	ofPolyline                              drawing;		  //	we draw with this first
	ofxBox2dEdge                            edgeLine;		  //	the box2d edge/line shape (min 2 points)
	vector		<ofPtr<ofxBox2dCircle> >	circles;		  //	default box2d circles
	vector		<ofPtr<ofxBox2dRect> >		boxes;			  //	defalut box2d rects
	vector		<ofPtr<CustomParticle> >    customParticles;  //	this is a custom particle the extends a cirlce
};
