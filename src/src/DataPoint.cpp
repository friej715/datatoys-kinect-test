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
    
}

void DataPoint::update() {
    
}

void DataPoint::draw() {
    ofSetColor(col);
    ofFill();
    ofCircle(pos.x, pos.y, 50);
}