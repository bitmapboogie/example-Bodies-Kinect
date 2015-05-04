//
//  CustomContactFilter.h
//  example-Bodies-Kinect
//
//  Created by Jordi Tost on 04.05.15.
//
//

#include "ofxBox2d.h"

#ifndef example_Bodies_Kinect_CustomContactFilter_h
#define example_Bodies_Kinect_CustomContactFilter_h

class CustomContactFilter : public b2ContactFilter {
public:
    bool ShouldCollide(b2Fixture* fixtureA, b2Fixture* fixtureB) {
        
        // @TODO: Filter from collision particles inside contours
        
        return !(fixtureA->GetType() == b2Shape::e_circle && fixtureB->GetType() == b2Shape::e_circle);
    }
};

#endif
