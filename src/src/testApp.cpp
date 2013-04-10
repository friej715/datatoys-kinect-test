#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup() {
    
	isLive			= true;
	isTracking		= true;
	isTrackingHands	= true;
	isFiltering		= false;
	isRecording		= false;
	isCloud			= false;
	isCPBkgnd		= true;
	isMasking		= true;
    
	nearThreshold = 500;
	farThreshold  = 1000;
    
	filterFactor = 0.1f;
    
	setupRecording();
    
	ofBackground(0, 0, 0);
    
    // CURRENT ORDER: LO-NATIVE, HI-NATIVE, LO-IMMIGRANT, HI-IMMIGRANT
    amplifier = 2; // this is to amplify the unemployment rates--we can apply it to all of them so they stay consistent but have more exaggerated differences. totally optional though. can be a float.
    
    unemploymentRates[0] = 32 * amplifier;
    unemploymentRates[1] = 6 * amplifier;
    unemploymentRates[2] = 19 * amplifier;
    unemploymentRates[3] = 6 * amplifier;
    
    currentIndex = 0;
    
    rootPoint.x = ofGetWidth()/2 - 100;
    rootPoint.y = 700;
    
    centerPoint.x = rootPoint.x;
    centerPoint.y = ofGetHeight()/2;
    
    // Shoulder points are temporary until attached to Kinect.
    // Actual shoulder should probably be used to set hand positions,
    // but then should be fixed before the player starts to stretch
    // for them.
    leftShoulderPoint.x = ofGetWidth()/2 - 150;
    leftShoulderPoint.y = ofGetHeight()/2 - 80;
    rightShoulderPoint.x = ofGetWidth()/2 - 50;
    rightShoulderPoint.y = ofGetHeight()/2 - 80;
    
    // neutral hand positions; would correspond to
    // zero unemployment
    leftHandPoint.x = ofGetWidth()/2 - 150;
    leftHandPoint.y = ofGetHeight()/2;
    rightHandPoint.x = ofGetWidth()/2 - 50;
    rightHandPoint.y = ofGetHeight()/2;
    
}

void testApp::setupRecording(string _filename) {
    
#if defined (TARGET_OSX) //|| defined(TARGET_LINUX) // only working on Mac/Linux at the moment (but on Linux you need to run as sudo...)
	hardware.setup();				// libusb direct control of motor, LED and accelerometers
	hardware.setLedOption(LED_OFF); // turn off the led just for yacks (or for live installation/performances ;-)
#endif
    
	recordContext.setup();	// all nodes created by code -> NOT using the xml config file at all
	//recordContext.setupUsingXMLFile();
	recordDepth.setup(&recordContext);
	recordImage.setup(&recordContext);
    
	recordUser.setup(&recordContext);
	recordUser.setSmoothing(filterFactor);				// built in openni skeleton smoothing...
	recordUser.setUseMaskPixels(isMasking);
	recordUser.setUseCloudPoints(isCloud);
	recordUser.setMaxNumberOfUsers(2);					// use this to set dynamic max number of users (NB: that a hard upper limit is defined by MAX_NUMBER_USERS in ofxUserGenerator)
    
	recordHandTracker.setup(&recordContext, 4);
	recordHandTracker.setSmoothing(filterFactor);		// built in openni hand track smoothing...
	recordHandTracker.setFilterFactors(filterFactor);	// custom smoothing/filtering (can also set per hand with setFilterFactor)...set them all to 0.1f to begin with
    
	recordContext.toggleRegisterViewport();
	recordContext.toggleMirror();
    
	oniRecorder.setup(&recordContext, ONI_STREAMING);
	//oniRecorder.setup(&recordContext, ONI_CYCLIC, 60);
	//read the warning in ofxOpenNIRecorder about memory usage with ONI_CYCLIC recording!!!
    
}

void testApp::setupPlayback(string _filename) {
    
	playContext.shutdown();
	playContext.setupUsingRecording(ofToDataPath(_filename));
	playDepth.setup(&playContext);
	playImage.setup(&playContext);
    
	playUser.setup(&playContext);
	playUser.setSmoothing(filterFactor);				// built in openni skeleton smoothing...
	playUser.setUseMaskPixels(isMasking);
	playUser.setUseCloudPoints(isCloud);
    
	playHandTracker.setup(&playContext, 4);
	playHandTracker.setSmoothing(filterFactor);			// built in openni hand track smoothing...
	playHandTracker.setFilterFactors(filterFactor);		// custom smoothing/filtering (can also set per hand with setFilterFactor)...set them all to 0.1f to begin with
    
	playContext.toggleRegisterViewport();
	playContext.toggleMirror();
    
}
//--------------------------------------------------------------
void testApp::updateDataPoints() {
    float distanceToCenterPoint = ofDist(centerPoint.x, centerPoint.y, rootPoint.x, rootPoint.y);
    
    // let's calculate where the other circle should be based on the unemployment rate
    otherLegPoint.x = centerPoint.x + cos(ofDegToRad(unemploymentRates[currentIndex])- PI/2) * distanceToCenterPoint;
    otherLegPoint.y = centerPoint.y - sin(ofDegToRad(unemploymentRates[currentIndex])- PI/2) * distanceToCenterPoint;
    
    // let's calculate where the hands should be
    // as unemployment rises, hands are lifted up, and go further away
    // from the shoulders. Results will be random within a 30-degree range.
    handShoulderDistance = ofMap(unemploymentRates[currentIndex]/amplifier, 0, 32, 100, 200);
    baseArmAngle = ofMap(unemploymentRates[currentIndex]/amplifier, 0, 32, 0, 180);
    rightArmAngle = ofMap(baseArmAngle, 0, 180, 360, 180); // swings in opposite direction
    
    leftHandPoint.x = leftShoulderPoint.x + cos(ofDegToRad(baseArmAngle + leftRandom) + PI/2) * handShoulderDistance;
    leftHandPoint.y = leftShoulderPoint.y + sin(ofDegToRad(baseArmAngle + leftRandom) + PI/2) * handShoulderDistance;
    
    rightHandPoint.x = rightShoulderPoint.x + cos(ofDegToRad(rightArmAngle + rightRandom) + PI/2) * handShoulderDistance;
    rightHandPoint.y = rightShoulderPoint.y + sin(ofDegToRad(rightArmAngle + rightRandom) + PI/2) * handShoulderDistance;
    
    cout << "Unemployment: " << unemploymentRates[currentIndex] << "  Angle: " << baseArmAngle << endl;
    cout << "Distance: " << handShoulderDistance << endl;
    
}

//--------------------------------------------------------------
void testApp::update(){
    updateDataPoints();
    
    
#ifdef TARGET_OSX // only working on Mac at the moment
	hardware.update();
#endif
    
	if (isLive) {
        
		// update all nodes
		recordContext.update();
		recordDepth.update();
		recordImage.update();
        
		// demo getting depth pixels directly from depth gen
		depthRangeMask.setFromPixels(recordDepth.getDepthPixels(nearThreshold, farThreshold),
									 recordDepth.getWidth(), recordDepth.getHeight(), OF_IMAGE_GRAYSCALE);
        
		// update tracking/recording nodes
		if (isTracking) recordUser.update();
		if (isRecording) oniRecorder.update();
        
		// demo getting pixels from user gen
		if (isTracking && isMasking) {
			allUserMasks.setFromPixels(recordUser.getUserPixels(), recordUser.getWidth(), recordUser.getHeight(), OF_IMAGE_GRAYSCALE);
			user1Mask.setFromPixels(recordUser.getUserPixels(1), recordUser.getWidth(), recordUser.getHeight(), OF_IMAGE_GRAYSCALE);
			user2Mask.setFromPixels(recordUser.getUserPixels(2), recordUser.getWidth(), recordUser.getHeight(), OF_IMAGE_GRAYSCALE);
		}
        
	} else {
        
		// update all nodes
		playContext.update();
		playDepth.update();
		playImage.update();
        
		// demo getting depth pixels directly from depth gen
		depthRangeMask.setFromPixels(playDepth.getDepthPixels(nearThreshold, farThreshold),
									 playDepth.getWidth(), playDepth.getHeight(), OF_IMAGE_GRAYSCALE);
        
		// update tracking/recording nodes
		if (isTracking) playUser.update();
        
		// demo getting pixels from user gen
		if (isTracking && isMasking) {
			allUserMasks.setFromPixels(playUser.getUserPixels(), playUser.getWidth(), playUser.getHeight(), OF_IMAGE_GRAYSCALE);
			user1Mask.setFromPixels(playUser.getUserPixels(1), playUser.getWidth(), playUser.getHeight(), OF_IMAGE_GRAYSCALE);
			user2Mask.setFromPixels(playUser.getUserPixels(2), playUser.getWidth(), playUser.getHeight(), OF_IMAGE_GRAYSCALE);
		}
	}
    
    
    if (recordUser.getNumberOfTrackedUsers() > 0) {
        // we have a tracked user! yay!
        ofxTrackedUser * tracked = recordUser.getTrackedUser(1);
        // all i'm doing here is setting the points in this array to parts of the skeleton that the kinect finds. if you start to type in tracked-> then it should autofill with the other body parts you can use. i've included z in here in case we do want to try to do depth
        parts[0].pos.set(tracked->left_lower_arm.position[1].X, tracked->left_lower_arm.position[1].Y, tracked->left_lower_arm.position[1].Z);
        parts[0].pos *= 1.6;
        
        parts[1].pos.set(tracked->right_lower_arm.position[1].X, tracked->right_lower_arm.position[1].Y, tracked->right_lower_arm.position[1].Z);
        parts[1].pos *= 1.6;
        
        parts[2].pos.set(tracked->left_lower_leg.position[1].X, tracked->left_lower_leg.position[1].Y, tracked->left_lower_leg.position[1].Z);
        parts[2].pos *= 1.6;
        
        parts[3].pos.set(tracked->right_lower_leg.position[1].X, tracked->right_lower_leg.position[1].Y, tracked->right_lower_leg.position[1].Z);
        parts[3].pos *= 1.6;
    }
    
//    // draw lines
//    ofSetColor(0);
//    ofFill();
//    
//    ofLine(centerPoint, rootLeg.pos);
//    ofLine(centerPoint, otherLeg.pos);
//    
//    ofLine(centerPoint, leftShoulder);
//    ofLine(centerPoint, rightShoulder);
//    
//    // draw root circle
//    ofCircle(rootLeg.pos, 30);
//    
//    // draw other leg circle
//    ofCircle(otherLeg.pos, 30);
//    
//    // draw smaller shoulder circles
//    ofCircle(leftShoulder, 10);
//    ofCircle(rightShoulder, 10);
//    
//    // draw hands
//    ofCircle(leftHand.pos, 30);
//    ofCircle(rightHand.pos, 30);
    
    
    partIsOverRootLeg = false;
    for (int i = 0; i < 4; i++) {
        if (ofDist(rootPoint.x, rootPoint.y, parts[i].pos.x, parts[i].pos.y) < 70) {
            partIsOverRootLeg = true;
        } 
    }
    
    partIsOverOtherLeg = false;
    for (int i = 0; i < 4; i++) {
        if (ofDist(otherLegPoint.x, otherLegPoint.y, parts[i].pos.x, parts[i].pos.y) < 70) {
            partIsOverOtherLeg = true;
        } 
    }
    
    partIsOverLeftHand = false;
    for (int i = 0; i < 4; i++) {
        if (ofDist(leftHandPoint.x, leftHandPoint.y, parts[i].pos.x, parts[i].pos.y) < 70) {
            partIsOverLeftHand = true;
        }
    }
    
    partIsOverRightHand = false;
    for (int i = 0; i < 4; i++) {
        if (ofDist(rightHandPoint.x, rightHandPoint.y, parts[i].pos.x, parts[i].pos.y) < 70) {
            partIsOverRightHand = true;
        } 
    }
    
}

//--------------------------------------------------------------
void testApp::draw(){
    
	ofSetColor(255, 255, 255);
    
	glPushMatrix();
	if (isLive) {
		recordImage.draw(0, 0, 640 * 1.6, 480 * 1.6);        
		if (isTracking) {
            ofScale(1.6, 1.6);
			recordUser.draw();
		}
	} else {        
		if (isTracking) {
			playUser.draw();
            
			if (isMasking) drawMasks();
			if (isCloud) drawPointCloud(&playUser, 0);	// 0 gives you all point clouds; use userID to see point clouds for specific users
            
		}
		if (isTrackingHands)
			playHandTracker.drawHands();
	}
    
	glPopMatrix();
    
    ofSetColor(0);
    ofFill();
    
    if (recordUser.getNumberOfTrackedUsers() > 0) {
        ofPushMatrix();
        //ofScale(1.6, 1.6);
        for (int i = 0; i < 4; i++) {
            parts[i].draw();
        }
        ofPopMatrix();
    }
    
//    ofLine(centerPoint, rootPoint);
//    ofLine(centerPoint, otherLegPoint);
//    
//    ofLine(centerPoint, leftShoulderPoint);
//    ofLine(centerPoint, rightShoulderPoint);
    
    // draw root circle
    if (partIsOverRootLeg)  ofSetColor(0, 255, 0);
    if (!partIsOverRootLeg) ofSetColor(0, 0, 255);
    ofCircle(rootPoint, 30);
    
    // draw other leg circle
    if (partIsOverOtherLeg)  ofSetColor(0, 255, 0);
    if (!partIsOverOtherLeg) ofSetColor(0, 0, 255);
    ofCircle(otherLegPoint, 30);
    
//    // draw smaller shoulder circles
//    ofCircle(leftShoulderPoint, 10);
//    ofCircle(rightShoulderPoint, 10);
    
    // draw hands
    if (partIsOverLeftHand)  ofSetColor(0, 255, 0);
    if (!partIsOverLeftHand) ofSetColor(0, 0, 255);
    ofCircle(leftHandPoint, 30);
    
    if (partIsOverRightHand)  ofSetColor(0, 255, 0);
    if (!partIsOverRightHand) ofSetColor(0, 0, 255);
    ofCircle(rightHandPoint, 30);
    
    // draw group name so we know what we're looking at
    string currentGroup;
    if (currentIndex == 0)      currentGroup = "LOW-ED-NATIVE";
    if (currentIndex == 1)      currentGroup = "HIGH-ED-NATIVE";
    if (currentIndex == 2)      currentGroup = "LOW-ED-IMMIGRANT";
    if (currentIndex == 3)      currentGroup = "HIGH-ED-IMMIGRANT";
    ofDrawBitmapString(currentGroup, 20, 20);
    ofDrawBitmapString(ofToString(unemploymentRates[currentIndex]/amplifier) + "%", 20, 40);
    
    
    
//	ofSetColor(255, 255, 0);
//    
//	string statusPlay		= (string)(isLive ? "LIVE STREAM" : "PLAY STREAM");
//	string statusRec		= (string)(!isRecording ? "READY" : "RECORDING");
//	string statusSkeleton	= (string)(isTracking ? "TRACKING USERS: " + (string)(isLive ? ofToString(recordUser.getNumberOfTrackedUsers()) : ofToString(playUser.getNumberOfTrackedUsers())) + "" : "NOT TRACKING USERS");
//	string statusSmoothSkel = (string)(isLive ? ofToString(recordUser.getSmoothing()) : ofToString(playUser.getSmoothing()));
//	string statusHands		= (string)(isTrackingHands ? "TRACKING HANDS: " + (string)(isLive ? ofToString(recordHandTracker.getNumTrackedHands()) : ofToString(playHandTracker.getNumTrackedHands())) + ""  : "NOT TRACKING");
//	string statusFilter		= (string)(isFiltering ? "FILTERING" : "NOT FILTERING");
//	string statusFilterLvl	= ofToString(filterFactor);
//	string statusSmoothHand = (string)(isLive ? ofToString(recordHandTracker.getSmoothing()) : ofToString(playHandTracker.getSmoothing()));
//	string statusMask		= (string)(!isMasking ? "HIDE" : (isTracking ? "SHOW" : "YOU NEED TO TURN ON TRACKING!!"));
//	string statusCloud		= (string)(isCloud ? "ON" : "OFF");
//	string statusCloudData	= (string)(isCPBkgnd ? "SHOW BACKGROUND" : (isTracking ? "SHOW USER" : "YOU NEED TO TURN ON TRACKING!!"));
//    
//	string statusHardware;
//    
//#ifdef TARGET_OSX // only working on Mac at the moment
//	ofPoint statusAccelerometers = hardware.getAccelerometers();
//	stringstream	statusHardwareStream;
//    
//	statusHardwareStream
//	<< "ACCELEROMETERS:"
//	<< " TILT: " << hardware.getTiltAngle() << "/" << hardware.tilt_angle
//	<< " x - " << statusAccelerometers.x
//	<< " y - " << statusAccelerometers.y
//	<< " z - " << statusAccelerometers.z;
//    
//	statusHardware = statusHardwareStream.str();
//#endif
//    
//	stringstream msg;
//    
//	msg
//	<< "    s : start/stop recording  : " << statusRec << endl
//	<< "    p : playback/live streams : " << statusPlay << endl
//	<< "    t : skeleton tracking     : " << statusSkeleton << endl
//	<< "( / ) : smooth skely (openni) : " << statusSmoothSkel << endl
//	<< "    h : hand tracking         : " << statusHands << endl
//	<< "    f : filter hands (custom) : " << statusFilter << endl
//	<< "[ / ] : filter hands factor   : " << statusFilterLvl << endl
//	<< "; / ' : smooth hands (openni) : " << statusSmoothHand << endl
//	<< "    m : drawing masks         : " << statusMask << endl
//	<< "    c : draw cloud points     : " << statusCloud << endl
//	<< "    b : cloud user data       : " << statusCloudData << endl
//	<< "- / + : nearThreshold         : " << ofToString(nearThreshold) << endl
//	<< "< / > : farThreshold          : " << ofToString(farThreshold) << endl
//	<< endl
//	<< "File  : " << oniRecorder.getCurrentFileName() << endl
//	<< "FPS   : " << ofToString(ofGetFrameRate()) << "  " << statusHardware << endl;
//    
//	ofDrawBitmapString(msg.str(), 20, 560);
    
}

void testApp:: drawMasks() {
	glPushMatrix();
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ZERO);
	allUserMasks.draw(640, 0, 640, 480);
	glDisable(GL_BLEND);
    glPopMatrix();
	user1Mask.draw(320, 480, 320, 240);
	user2Mask.draw(640, 480, 320, 240);
	
}

void testApp::drawPointCloud(ofxUserGenerator * user_generator, int userID) {
    
	glPushMatrix();
    
	int w = user_generator->getWidth();
	int h = user_generator->getHeight();
    
	glTranslatef(w, h/2, -500);
	ofRotateY(pointCloudRotationY);
    
	glBegin(GL_POINTS);
    
	int step = 1;
    
	for(int y = 0; y < h; y += step) {
		for(int x = 0; x < w; x += step) {
			ofPoint pos = user_generator->getWorldCoordinateAt(x, y, userID);
			if (pos.z == 0 && isCPBkgnd) continue;	// gets rid of background -> still a bit weird if userID > 0...
			ofColor color = user_generator->getWorldColorAt(x,y, userID);
			glColor4ub((unsigned char)color.r, (unsigned char)color.g, (unsigned char)color.b, (unsigned char)color.a);
			glVertex3f(pos.x, pos.y, pos.z);
		}
	}
    
	glEnd();
    
	glColor3f(1.0f, 1.0f, 1.0f);
    
	glPopMatrix();
}


//--------------------------------------------------------------
void testApp::keyPressed(int key){
    
	float smooth;
    
	switch (key) {
#ifdef TARGET_OSX // only working on Mac at the moment
		case 357: // up key
			hardware.setTiltAngle(hardware.tilt_angle++);
			break;
		case 359: // down key
			hardware.setTiltAngle(hardware.tilt_angle--);
			break;
#endif
		case 's':
		case 'S':
			if (isRecording) {
				oniRecorder.stopRecord();
				isRecording = false;
				break;
			} else {
				oniRecorder.startRecord(generateFileName());
				isRecording = true;
				break;
			}
			break;
		case 'p':
		case 'P':
			if (oniRecorder.getCurrentFileName() != "" && !isRecording && isLive) {
				setupPlayback(oniRecorder.getCurrentFileName());
				isLive = false;
			} else {
				isLive = true;
			}
			break;
		case 't':
		case 'T':
			isTracking = !isTracking;
			break;
		case 'h':
		case 'H':
			isTrackingHands = !isTrackingHands;
			if(isLive) recordHandTracker.toggleTrackHands();
			if(!isLive) playHandTracker.toggleTrackHands();
			break;
		case 'f':
		case 'F':
			isFiltering = !isFiltering;
			recordHandTracker.isFiltering = isFiltering;
			playHandTracker.isFiltering = isFiltering;
			break;
		case 'm':
		case 'M':
			isMasking = !isMasking;
			recordUser.setUseMaskPixels(isMasking);
			playUser.setUseMaskPixels(isMasking);
			break;
		case 'c':
		case 'C':
			isCloud = !isCloud;
			recordUser.setUseCloudPoints(isCloud);
			playUser.setUseCloudPoints(isCloud);
			break;
		case 'b':
		case 'B':
			isCPBkgnd = !isCPBkgnd;
			break;
		case '9':
		case '(':
			smooth = recordUser.getSmoothing();
			if (smooth - 0.1f > 0.0f) {
				recordUser.setSmoothing(smooth - 0.1f);
				playUser.setSmoothing(smooth - 0.1f);
			}
			break;
		case '0':
		case ')':
			smooth = recordUser.getSmoothing();
			if (smooth + 0.1f <= 1.0f) {
				recordUser.setSmoothing(smooth + 0.1f);
				playUser.setSmoothing(smooth + 0.1f);
			}
			break;
		case '[':
            //case '{':
			if (filterFactor - 0.1f > 0.0f) {
				filterFactor = filterFactor - 0.1f;
				recordHandTracker.setFilterFactors(filterFactor);
				if (oniRecorder.getCurrentFileName() != "") playHandTracker.setFilterFactors(filterFactor);
			}
			break;
		case ']':
            //case '}':
			if (filterFactor + 0.1f <= 1.0f) {
				filterFactor = filterFactor + 0.1f;
				recordHandTracker.setFilterFactors(filterFactor);
				if (oniRecorder.getCurrentFileName() != "") playHandTracker.setFilterFactors(filterFactor);
			}
			break;
		case ';':
		case ':':
			smooth = recordHandTracker.getSmoothing();
			if (smooth - 0.1f > 0.0f) {
				recordHandTracker.setSmoothing(smooth -  0.1f);
				playHandTracker.setSmoothing(smooth -  0.1f);
			}
			break;
		case '\'':
		case '\"':
			smooth = recordHandTracker.getSmoothing();
			if (smooth + 0.1f <= 1.0f) {
				recordHandTracker.setSmoothing(smooth +  0.1f);
				playHandTracker.setSmoothing(smooth +  0.1f);
			}
			break;
		case '>':
		case '.':
			farThreshold += 50;
			if (farThreshold > recordDepth.getMaxDepth()) farThreshold = recordDepth.getMaxDepth();
			break;
		case '<':
		case ',':
			farThreshold -= 50;
			if (farThreshold < 0) farThreshold = 0;
			break;
            
		case '+':
		case '=':
			nearThreshold += 50;
			if (nearThreshold > recordDepth.getMaxDepth()) nearThreshold = recordDepth.getMaxDepth();
			break;
            
		case '-':
		case '_':
			nearThreshold -= 50;
			if (nearThreshold < 0) nearThreshold = 0;
			break;
		case 'r':
			recordContext.toggleRegisterViewport();
			break;
		default:
			break;
	}
}

string testApp::generateFileName() {
    
	string _root = "kinectRecord";
    
	string _timestamp = ofToString(ofGetDay()) +
	ofToString(ofGetMonth()) +
	ofToString(ofGetYear()) +
	ofToString(ofGetHours()) +
	ofToString(ofGetMinutes()) +
	ofToString(ofGetSeconds());
    
	string _filename = (_root + _timestamp + ".oni");
    
	return _filename;
    
}

//--------------------------------------------------------------
void testApp::keyReleased(int key){
    
}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){
    
	if (isCloud) pointCloudRotationY = x;
    
}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){
    
}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){
    
}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){
    rightRandom = ofRandom(-15, 15);
    leftRandom = ofRandom(-15, 15);
    
    
    if (currentIndex <= 2) {
        currentIndex++;
    } else {
        currentIndex = 0;
    }
}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){
    
}

