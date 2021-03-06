#include "ofApp.h"

using namespace ofxCv;
using namespace cv;

static int pts[] = {257,219,257,258,259,274,263,325,266,345,266,352,269,369,276,387,286,415,291,425,302,451,308,462,316,472,321,480,328,488,333,495,339,501,345,505,350,507,365,515,370,519,377,522,382,525,388,527,405,534,426,538,439,539,452,539,468,540,485,540,496,541,607,541,618,539,625,537,641,530,666,513,682,500,710,476,723,463,727,457,729,453,732,450,734,447,738,440,746,423,756,404,772,363,779,343,781,339,784,327,789,301,792,278,794,267,794,257,795,250,795,232,796,222,796,197,797,195,797,188,796,188};
static int nPts  = 61*2;

//--------------------------------------------------------------
void ofApp::setup() {
    
    ofSetLogLevel(OF_LOG_VERBOSE);
    
	ofSetVerticalSync(true);
    //ofHideCursor();
	ofBackground(style.getBackgroundColor());
    
    
    // Kinect setup
	kinect.init();
	//kinect.init(true);                // shows infrared instead of RGB video image
	//kinect.init(false, false);        // disable video image (faster fps)
	
    // Open Kinect stream
	kinect.open();                      // opens first available kinect
	//kinect.open(1);                   // open a kinect by id, starting with 0 (sorted by serial # lexicographically))
	//kinect.open("A00362A08602047A");	// open a kinect using it's unique serial #
	
	// Print the intrinsic IR sensor values
	if (kinect.isConnected()) {
		ofLogNotice() << "sensor-emitter dist: " << kinect.getSensorEmitterDistance() << "cm";
		ofLogNotice() << "sensor-camera dist:  " << kinect.getSensorCameraDistance() << "cm";
		ofLogNotice() << "zero plane pixel size: " << kinect.getZeroPlanePixelSize() << "mm";
		ofLogNotice() << "zero plane dist: " << kinect.getZeroPlaneDistance() << "mm";
	}
	
    // Thresholds
    nearThreshold = 188; //230;
	farThreshold = 102; //70;
    
	// Zero the tilt on startup
	angle = 14;
	kinect.setCameraTiltAngle(angle);
    
    // Allocate images
    colorImg.allocate(kinect.width, kinect.height, OF_IMAGE_COLOR);
	grayImage.allocate(kinect.width, kinect.height, OF_IMAGE_GRAYSCALE);
    grayThreshNear.allocate(kinect.width, kinect.height, OF_IMAGE_GRAYSCALE);
	grayThreshFar.allocate(kinect.width, kinect.height, OF_IMAGE_GRAYSCALE);
    grayPreprocImage.allocate(kinect.width, kinect.height, OF_IMAGE_GRAYSCALE);
    grayImageScaled.allocate(1920, 1080, OF_IMAGE_GRAYSCALE);
    
    // Configure contour finder
	contourFinder.setMinAreaRadius(10);
	contourFinder.setMaxAreaRadius(200);
    contourFinder.setFindHoles(false);
    contourFinder.setSortBySize(true);
    
    // Box2D setup
	bMouseForce = false;
	
	box2d.init();
	box2d.setGravity(0, 1);
	box2d.setFPS(60.0);
	box2d.registerGrabbing();
	
	// lets add a contour to start
    //	for (int i=0; i<nPts; i+=2) {
    //		float x = pts[i];
    //		float y = pts[i+1];
    //		edgeLine.addVertex(x, y);
    //	}
    
    // World Collision Filtering
    b2World *world = box2d.getWorld();
    CustomContactFilter *filter = new CustomContactFilter();
    world->SetContactFilter(filter);
    
    // make the shape
    //    b2FixtureDef edgeFixture = edgeLine.fixture;
    //    edgeFixture.filter.groupIndex = 8;
    //    edgeFixture.filter.categoryBits = CATEGORY_TRACKING;
    //    edgeFixture.filter.maskBits = CATEGORY_PARTICLES;
    
    edgeLine.setPhysics(1.0, 0.5, 0.5);
	edgeLine.create(box2d.getWorld());
}

//--------------------------------------------------------------
void ofApp::update() {
	
    if (circles.size() < numberOfParticles) {
        int diff = numberOfParticles - circles.size();
        int num = (int) ofRandom(10);
        for (int i = 0; i < num;i++){
            float r = ofRandom(2, 6);		// a random radius 4px - 20px
            
            // now add a circle to the vector
            ofPtr<ofxBox2dCircle> circle = ofPtr<ofxBox2dCircle>(new ofxBox2dCircle);
            
            //            b2FixtureDef circleFixture = circle.get()->fixture;
            //            circleFixture.filter.groupIndex = -8;
            //            circleFixture.filter.categoryBits = CATEGORY_PARTICLES;
            //            circleFixture.filter.maskBits = CATEGORY_TRACKING;
            
            circle.get()->setPhysics(3.0, 0.1/*0.53*/, 0.3);
            circle.get()->setup(box2d.getWorld(), ofRandom(ofGetWidth()), -ofRandom(r, 50), r);
            
            circles.push_back(circle);
        }
    }
    
    kinect.update();
    
	// There is a new frame and we are connected
	if (kinect.isFrameNew()) {
        
        // Load grayscale depth image from the kinect source
        
		grayImage.setFromPixels(kinect.getDepthPixels(), kinect.width, kinect.height, OF_IMAGE_GRAYSCALE);
        // THIS IS DIRTY
        // WE NEED TO DI THIS THE RIGHT WAY
        // @Todo
        //        grayImage.resize(1920, 1080);
        
        // Threshold image
        threshold(grayImage, grayThreshNear, nearThreshold, true);
        threshold(grayImage, grayThreshFar, farThreshold);
        
        // Convert to CV to perform AND operation
        Mat grayThreshNearMat = toCv(grayThreshNear);
        Mat grayThreshFarMat = toCv(grayThreshFar);
        Mat grayImageMat = toCv(grayImage);
        
        // cvAnd to get the pixels which are a union of the two thresholds
        bitwise_and(grayThreshNearMat, grayThreshFarMat, grayImageMat);
        
        // Save pre-processed image for drawing it
        grayPreprocImage = grayImage;
		// Process image
        
        grayImage.mirror(false, true);
        dilate(grayImage);
        dilate(grayImage);
        blur(grayImage, 5);
        
        
        // Mark image as changed
        grayImage.update();
        
        // Find contours
        //contourFinder.setThreshold(ofMap(mouseX, 0, ofGetWidth(), 0, 255));
        //        grayImageScaled = grayImage;
        //        grayImageScaled.resize(1920, 1080);
        contourFinder.findContours(grayImage);
        
        
        // Update biggest contour -------------------------------------------------------------
        
        if (edgeLine.isBody()) {
            drawing.clear();
            edgeLine.destroy();
        }
        
        int n = contourFinder.size();
        if (n > 0) {
            
            ofPolyline biggestPolyline = contourFinder.getPolyline(0); // The biggest one
            
            // Scale polyline and add vertices
            for(int i=0;i< (int)biggestPolyline.size();i++){
                biggestPolyline[i].x*= (ofGetWidth()/ 640);
                biggestPolyline[i].y*= (ofGetHeight()/360);
                drawing.addVertex(biggestPolyline[i]);
            }
            //drawing.addVertices(biggestPolyline.getVertices());
            
            // Close shape
            drawing.close();
            
            // Smooth shape
            drawing = drawing.getSmoothed(8);
            //int smoothSize = ofMap(mouseX, 0, ofGetWidth(), 0, 32, true);
            //drawing = drawing.getSmoothed(smoothSize, 0.0f);
            
            // Simplify shape (remove some points)
            drawing.simplify(2.0f);
            float simplifyAmount = 2.5f;
            while (drawing.size() > 100){
                drawing.simplify(simplifyAmount);
                simplifyAmount += .5f;
            }
            
            // Add vertices to box2d edge
            edgeLine.addVertexes(drawing);
            //polyLine.simplifyToMaxVerts(); // this is based on the max box2d verts
            edgeLine.setPhysics(0.0, 0.5, 0.5);
            edgeLine.create(box2d.getWorld());
            
            drawing.clear();
        }
	}
    
    // Update Box2D world
    
	box2d.update();
	
	if (bMouseForce) {
        
		float strength = 8.0f;
		float damping  = 0.7f;
		float minDis   = 100;
		for(int i=0; i<circles.size(); i++) {
			circles[i].get()->addAttractionPoint(mouseX, mouseY, strength);
			circles[i].get()->setDamping(damping, damping);
		}
		for(int i=0; i<customParticles.size(); i++) {
			customParticles[i].get()->addAttractionPoint(mouseX, mouseY, strength);
			customParticles[i].get()->setDamping(damping, damping);
		}
	}
	
    // remove shapes offscreen
    ofRemove(boxes, ofxBox2dBaseShape::shouldRemoveOffScreen);
    ofRemove(circles, ofApp::shouldRemoveOffScreen);
    ofRemove(customParticles, ofxBox2dBaseShape::shouldRemoveOffScreen);
}


//--------------------------------------------------------------

void ofApp::draw() {
	
	
	for(int i=0; i<circles.size(); i++) {
		ofFill();
        ofSetColor(style.getForgroundColor());
        //		ofSetHexColor(0x90d4e3);
        ofxBox2dCircle *circ = circles[i].get();
        
        
        if(!circ->isBody()) return;
        
        ofPushMatrix();
        ofTranslate(circ->getPosition().x,circ->getPosition().y, 0);
        ofRotate(circ->getRotation(), 0, 0, 1);
        ofCircle(0, 0, circ->getRadius());
        
        //        ofPushStyle();
        //        ofEnableAlphaBlending();
        //        ofSetColor(0);
        //        ofLine(0, 0, radius, 0);
        //        if(circ->isSleeping()) {
        //            ofSetColor(255, 100);
        //            ofCircle(0, 0, radius);
        //        }
        //        ofPopStyle();
        ofPopMatrix();
	}
	
	for(int i=0; i<boxes.size(); i++) {
		ofFill();
        ofSetColor(style.getForgroundColor()) ;
        //		ofSetHexColor(0xe63b8b);
		boxes[i].get()->draw();
        
	}
	
	for(int i=0; i<customParticles.size(); i++) {
		customParticles[i].get()->draw();
	}
	
	ofNoFill();
	ofSetColor(style.getForgroundColor());
	if(drawing.size()==0) {
        edgeLine.updateShape();
        //edgeLine.update();
        edgeLine.draw();
    }
	else drawing.draw();
	
	
	string info = "";
	info += "Press [s] to draw a line strip ["+ofToString(bDrawLines)+"]\n";
	info += "Press [f] to toggle Mouse Force ["+ofToString(bMouseForce)+"]\n";
	info += "Press [c] for circles\n";
	info += "Press [b] for blocks\n";
	info += "Press [z] for custom particle\n";
	info += "Total Bodies: "+ofToString(box2d.getBodyCount())+"\n";
	info += "Total Joints: "+ofToString(box2d.getJointCount())+"\n\n";
	info += "FPS: "+ofToString(ofGetFrameRate())+"\n";
	ofSetColor(250, 250, 250);
	ofDrawBitmapString(info, 30, 30);
    
    // MAKE THE LOGO BIGGER !!!!!!!!!!!
    
    ofImage img = style.getLogo();
    ofSetColor(255,255,255,200);
    float w = img.getWidth();
    float h = img.getHeight();
    img.resize(w/5, h/5);
    ofEnableAlphaBlending();
    img.draw(25,ofGetHeight() - (img.getHeight() + 25));
    ofDisableAlphaBlending();
}

//--------------------------------------------------------------

void ofApp::exit() {
	kinect.setCameraTiltAngle(0); // zero the tilt on exit
	kinect.close();
}

//--------------------------------------------------------------

void ofApp::keyPressed(int key) {
	
	if(key == 'c') {
		float r = ofRandom(4, 20);		// a random radius 4px - 20px
		circles.push_back(ofPtr<ofxBox2dCircle>(new ofxBox2dCircle));
		circles.back().get()->setPhysics(3.0, 0.53, 0.1);
		circles.back().get()->setup(box2d.getWorld(), mouseX, mouseY, r);
		
	}
	
	if(key == 'b') {
		float w = ofRandom(4, 20);
		float h = ofRandom(4, 20);
		boxes.push_back(ofPtr<ofxBox2dRect>(new ofxBox2dRect));
		boxes.back().get()->setPhysics(3.0, 0.53, 0.1);
		boxes.back().get()->setup(box2d.getWorld(), mouseX, mouseY, w, h);
	}
	
	if(key == 'z') {
        
		customParticles.push_back(ofPtr<CustomParticle>(new CustomParticle));
        CustomParticle * p = customParticles.back().get();
		float r = ofRandom(3, 10);		// a random radius 4px - 20px
		p->setPhysics(0.4, 0.53, 0.31);
		p->setup(box2d.getWorld(), mouseX, mouseY, r);
		p->color.r = ofRandom(20, 100);
		p->color.g = 0;
		p->color.b = ofRandom(150, 255);
	}
    
	if(key == 'f') bMouseForce = !bMouseForce;
	if(key == 't') ofToggleFullscreen();
	
	
	
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key) {
	
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ) {
	
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {
    //	drawing.addVertex(x, y);
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {
	
    //	if(edgeLine.isBody()) {
    //		drawing.clear();
    //		edgeLine.destroy();
    //	}
    //
    //	drawing.addVertex(x, y);
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {
	
    //	drawing.setClosed(false);
    //	drawing.simplify();
    //
    //	edgeLine.addVertexes(drawing);
    //	//polyLine.simplifyToMaxVerts(); // this is based on the max box2d verts
    //	edgeLine.setPhysics(0.0, 0.5, 0.5);
    //	edgeLine.create(box2d.getWorld());
    //    
    //	drawing.clear();
    
}

//--------------------------------------------------------------
void ofApp::resized(int w, int h){
	
}

bool ofApp::shouldRemoveOffScreen(ofPtr<ofxBox2dBaseShape> shape) {
    return !ofRectangle(0, -100, ofGetWidth(), ofGetHeight() + 100).inside(shape.get()->getPosition());
}



