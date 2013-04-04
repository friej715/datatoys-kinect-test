//
//  Point.h
//  openNiSample007
//
//  Created by Jane Friedhoff on 4/3/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#pragma once
#include "ofMain.h"

class DataPoint {
public:
    ofVec3f pos;
    
    ofColor col;
    
    void setup();
    void update();
    void draw();
    
    
    
};
