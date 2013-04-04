//
//  Part.h
//  openNiSample007
//
//  Created by Jane Friedhoff on 4/3/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#pragma once
#include "ofMain.h"
#include "ofxOpenNI.h"

class Part {
public:
    string nameOfPart; // so we can check it if we're not sure
    ofVec3f pos;
    
    void draw();
    
    
    
    
};