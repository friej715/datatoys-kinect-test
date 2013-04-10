//
//  Part.cpp
//  openNiSample007
//
//  Created by Jane Friedhoff on 4/3/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#include <iostream>
#include "Part.h"

void Part::draw() {
    ofSetColor(255, 255, 0);
    ofCircle(pos.x, pos.y, 10);
    
}