//
//  Point.cpp
//  openNiSample007
//
//  Created by Jane Friedhoff on 4/3/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#include <iostream>
#include "DataPoint.h"

void DataPoint::setup() {
    pos.set(ofRandom(170, 950), ofRandom(200,700), ofRandom(1900, 2300));
    
    ofCircle(pos.x, pos.y, pos.z, 30);
    
}

void DataPoint::update() {
    
}

void DataPoint::draw() {
    ofSetColor(col);
    ofFill();
    ofCircle(pos.x, pos.y, 50);
}