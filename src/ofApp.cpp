#include "ofApp.h"

void ofApp::exit(){
    this->_eog_trigger->stopRecording();
    disconnectPhone();
    /*if (this->_android_tcp_server->isConnected()) {
        this->_android_tcp_server->close();
    }*/
    this->_vicon_receiver.stop();
}

//--------------------------------------------------------------
void ofApp::setup(){
    ofSetLogLevel(OF_LOG_SILENT); // for now keep output silent
    this->_isLogFileCreated = false;
    ofSetFrameRate(60);
    loadSettingsAndWriteDefaultIfNeeded();
    setupUI();
    setupVisualFeedback();
    setupTCPserver();
    setupEogTrigger();
    setupMotionCapture();
    this->_time = 0.0f;
    this->_step_duration = 1.0f/60;
    this->_path_step = (2*PI)/(this->_path_duration*60);
    this->_source_instance = new Blinky(this->_source_radius);
    this->_source_positions = ofVec2f(0, 0);
    this->_source_instance->setPosition(this->_source_positions);
    this->_sound_on = false;
    this->bReproduction = false;
    setPathToEight();
}

void ofApp::setupParticipant() {
    this->_origin.x = 0.0f;
    this->_origin.y = 0.0f;
    this->_origin.z = 0.0f;
    this->_origin.phi = 0.0f;
    this->_current.x = 0.0f;
    this->_current.y = 0.0f;
    this->_current.z = 0.0f;
    this->_current.phi = 0.0f;
    this->_old.x = 0.0f;
    this->_old.y = 0.0f;
    this->_old.z = 0.0f;
    this->_old.phi = 0.0f;
    this->_logStartTime = 0.0f;
}

void ofApp::setupTCPserver() {
    this->_ssr_osc = new ofxOscSender();
    this->_ssr_osc->setup(this->_android_ip, this->_android_port);
    /*if (this->_android_tcp_server == NULL) {
        this->_android_tcp_server = new ofxTCPServer();
        this->_android_tcp_server->setMessageDelimiter("");
    }*/
    this->_tobii_osc = new ofxOscSender();
    this->_tobii_osc->setup(this->_tobii_ip, this->_tobii_port);
}

void ofApp::connectToSSR(bool value) {
    if (this->_ssr_osc != NULL) {
        ofxOscMessage msg = ofxOscMessage();
        msg.setAddress("/connect");
        msg.addStringArg("?");
        if (value == true) {
            msg.addIntArg(1);
        } else {
            msg.addIntArg(0);
        }
        this->_ssr_osc->sendMessage(msg);
    }
}
void ofApp::loadSsrScene() {
    if (this->_ssr_osc != NULL) {
        ofxOscMessage msg = ofxOscMessage();
        msg.setAddress("/load");
        msg.addStringArg("?");
        msg.addStringArg("?");
        this->_ssr_osc->sendMessage(msg);
    }
}
void ofApp::streamSSR(bool value) {
    if (this->_ssr_osc != NULL) {
        ofxOscMessage msg = ofxOscMessage();
        msg.setAddress("/stream");
        msg.addStringArg("?");
        if (value == true) {
            msg.addIntArg(1);
        } else {
            msg.addIntArg(0);
        }
        this->_ssr_osc->sendMessage(msg);
        
        if (value == true) {
            this->_ssr_running = true;
        } else {
            this->_ssr_running = false;
        }
    }
}
void ofApp::updateSoundPos(float x, float y) {
    if (this->_ssr_osc != NULL) {
        ofxOscMessage msg = ofxOscMessage();
        msg.setAddress("/soundpos");
        msg.addFloatArg(x);
        msg.addFloatArg(y);
        this->_ssr_osc->sendMessage(msg);
    }
}
void ofApp::updatePos(float x, float y) {
    if (this->_ssr_osc != NULL) {
        ofxOscMessage msg = ofxOscMessage();
        msg.setAddress("/pos");
        msg.addFloatArg(x);
        msg.addFloatArg(y);
        this->_ssr_osc->sendMessage(msg);
    }
}
void ofApp::updateAngle(float phi) {
    if (this->_ssr_osc != NULL) {
        ofxOscMessage msg = ofxOscMessage();
        msg.setAddress("/azimuth");
        msg.addStringArg("?");
        msg.addFloatArg(phi);
        this->_ssr_osc->sendMessage(msg);
    }
}

void ofApp::setupProjectEyeTracker() {
    ofxOscMessage msg = ofxOscMessage();
    // set project
    msg.setAddress("/set");
    msg.addStringArg("project");
    msg.addStringArg("hospital");
    _tobii_osc->sendMessage(msg);
}

void ofApp::setupSubjectEyeTracker() {
    ofxOscMessage msg = ofxOscMessage();
    // set participant
    msg.setAddress("/set");
    msg.addStringArg("participant");
    msg.addStringArg(this->_username);
    _tobii_osc->sendMessage(msg);
}

void ofApp::connectEyeTracker() {
    ofxOscMessage msg = ofxOscMessage();
    // connect
    msg.setAddress("/connect");
    msg.addStringArg("?");
    msg.addIntArg(1);
    _tobii_osc->sendMessage(msg);
}

void ofApp::streamEyeTracker() {
    ofxOscMessage msg = ofxOscMessage();
    // start streaming / wake up cameras
    msg.setAddress("/stream");
    msg.addStringArg("?");
    msg.addIntArg(1);
    _tobii_osc->sendMessage(msg);
}

void ofApp::stopStreamEyeTracker() {
    ofxOscMessage msg = ofxOscMessage();
    // start streaming / wake up cameras
    msg.setAddress("/stream");
    msg.addStringArg("?");
    msg.addIntArg(0);
    _tobii_osc->sendMessage(msg);
}

void ofApp::stopRecordingEyeTracker() {
    ofxOscMessage msg = ofxOscMessage();
    // stop recording
    msg.setAddress("/record");
    msg.addStringArg("?");
    msg.addIntArg(0);
    _tobii_osc->sendMessage(msg);
}

void ofApp::cleanupEyeTracker() {
    ofxOscMessage msg = ofxOscMessage();
    // disconnect
    msg.setAddress("/connect");
    msg.addStringArg("?");
    msg.addIntArg(0);
    _tobii_osc->sendMessage(msg);
}

void ofApp::calibrateEyeTracker() {
    ofxOscMessage msg = ofxOscMessage();
    msg.setAddress("/set");
    msg.addStringArg("calibration");
    msg.addStringArg("?");
    _tobii_osc->sendMessage(msg);
}

void ofApp::recordEyeTracker() {
    ofxOscMessage msg = ofxOscMessage();
    msg.setAddress("/record");
    msg.addStringArg("?");
    msg.addIntArg(1);
    _tobii_osc->sendMessage(msg);
}

void ofApp::sendEyeTrackerEvent(string message) {
    ofxOscMessage msg = ofxOscMessage();
    msg.setAddress("/set");
    msg.addStringArg("trigger");
    msg.addStringArg(message);
    _tobii_osc->sendMessage(msg);
}

void ofApp::setupEogTrigger() {
    this->_eog_trigger = new UdpTrigger(this->_eog_host);
    this->_eog_trigger->connectToHost();
    this->_start_recoring = false;
    this->_is_recording = false;
}

void ofApp::setupMotionCapture() {
    ofxUDPSettings settings;
    settings.receiveOn(this->_mocap_receive_port);
    settings.blocking = false;
    this->_vicon_receiver.setup(settings);
}

void ofApp::setupUI() {
    this->_uiPanel.setup();
    this->_uiPanel.add(this->_phone_label.setup("PHONE",""));
    this->_uiPanel.add(this->_push_button_connect.setup("connect to tobii"));
    this->_uiPanel.add(this->_push_button_disconnect.setup("disconnect from phone"));

    this->_uiPanel.add(this->_tracking_label.setup("TRACKING",""));
    this->_uiPanel.add(this->_reset_head_origin.setup("reset head origin"));
    this->_uiPanel.add(this->_toggle_button_eog.setup("record eog", false));

    this->_uiPanel.add(this->_presentation_label.setup("PRESENTATION",""));
    this->_uiPanel.add(this->_toggle_button_sound.setup("sound on", false));
    this->_uiPanel.add(this->_push_button_eight.setup("eight"));
    this->_uiPanel.add(this->_push_button_limacon.setup("circle right")); // limacon
    this->_uiPanel.add(this->_push_button_circle.setup("circle left"));

    // set gui text and box colors
    this->_push_button_connect.setTextColor(ofColor::red);
    this->_push_button_disconnect.setFillColor(ofColor::black);
    this->_push_button_disconnect.setTextColor(ofColor::black);
    this->_toggle_button_eog.setTextColor(ofColor::red);
    this->_toggle_button_sound.setTextColor(ofColor::red);

    // add gui listeners
    this->_push_button_connect.addListener(this, &ofApp::connectPhone); // connect to tobii
    this->_reset_head_origin.addListener(this, &ofApp::resetHeadOrigin);
    this->_toggle_button_eog.addListener(this, &ofApp::toggleRecording);
    this->_toggle_button_sound.addListener(this, &ofApp::toggleSound);
    this->_push_button_eight.addListener(this, &ofApp::setPathToEight);
    this->_push_button_limacon.addListener(this, &ofApp::setPathToLimacon);
    this->_push_button_circle.addListener(this, &ofApp::setPathToCircle);
}

void ofApp::setupVisualFeedback() {
    ofSetCircleResolution(100);
    this->_ui_world_diameter = ofGetWindowHeight();
    this->_ui_center = ofVec2f(ofGetWindowWidth(), ofGetWindowHeight()) - this->_ui_world_diameter/2;
    this->_ui_world_start = this->_ui_center  - this->_ui_world_diameter/2;
    this->_ui_max_distance = (this->_ui_world_diameter/2)*0.90;

    this->_max_distance = 3.6f;
}

//--------------------------------------------------------------
void ofApp::update(){
    float now = ofGetElapsedTimef();
    float dt = now - this->_time;

    if (this->_my_ip == "") {
        this->_my_ip = getIPhost();
    }
    /*if (this->_android_tcp_server->getNumClients() > 0) {
        this->_push_button_connect.setTextColor(ofColor::green);
    } else {
        this->_push_button_connect.setTextColor(ofColor::red);
    }*/
    this->_vicon_receiver.updateData();
    this->_head_data = this->_vicon_receiver.getLatestData();

    // update source and user position
    //if (dt > 2*this->_step_duration) {
        updateParticipantPosition();
        if (this->_is_recording == true) {
            if (this->_selected_shape == 0) {
                this->_source_positions = shape_eight(this->_shape_eight_half_size, -(this->_current_phi + (this->_phi_offset + 10.0f)/180*PI), 0.0f) - this->_shape_offset;
            }
            if (this->_selected_shape == 1) {
                //this->_source_positions = shape_limacon(this->_shape_limacon_offset, this->_shape_limacon_center, (this->_current_phi - (this->_phi_offset - 5.0f)/180*PI), 0.0f) - this->_shape_offset;
                this->_source_positions = shape_circle(this->_shape_circle_diameter, (this->_current_phi - (this->_phi_offset - 15.0f)/180*PI), this->_direction) - this->_shape_offset;
            }
            if (this->_selected_shape == 2) {
                this->_source_positions = shape_circle(this->_shape_circle_diameter, (this->_current_phi - (this->_phi_offset - 15.0f)/180*PI), this->_direction) - this->_shape_offset;
            }
            this->_current_phi += _path_step*(dt/_step_duration);
            // sound source position
            if (!this->bReproduction) {
                updateSoundPos(-this->_source_positions.x, this->_source_positions.y);
            }
            //sendMessageToPhone(0, "SRCPOS/" + ofToString(-this->_source_positions.x) + "/" + ofToString(this->_source_positions.y) + "/" + ofToString(this->_source_height));
            if (this->_current_phi >= (2*PI * this->_path_revolutions)) {
                bool t = false;
                toggleSound(NULL, t);
            }
        }
        this->_time = now;
        // log current states
        ofLogNotice("UPDATE", "," + ofToString(now-this->_logStartTime) + "," + ofToString(this->_origin.phi) + "," + ofToString(-this->_current.x) + "," + ofToString(-this->_current.y) + "," + ofToString(this->_current.z) + "," + ofToString(this->_current.phi) + "," + ofToString(-this->_source_positions.x) + "," + ofToString(this->_source_positions.y) + "," + ofToString(this->_source_height) + "," + ofToString(this->_sound_on));
    //}
    if (this->_start_recoring == true){
        this->_current_phi = 0;
        this->_start_recoring = false;
        this->_is_recording = true;
    }
    this->_source_instance->setPosition(this->_source_positions);
    this->_source_instance->update();
}

void ofApp::updateParticipantPosition() {
    this->_old = this->_current;
    // round to milimeter accuracy
    this->_current.x = round(this->_head_data.x_position)/1000 - this->_origin.x;
    this->_current.y = round(this->_head_data.y_position)/1000 - this->_origin.y;
    this->_current.z = round(this->_head_data.z_position)/1000;
    // round to 0.1 degree and invert rotation direction for screen and android
    this->_current.phi = fmod((360.0f - round(this->_head_data.z_rot_avg*10)/10) - this->_origin.phi, 360.0f);

    if ((this->_old.x != this->_current.x) || (this->_old.y != this->_current.y) || (this->_old.z != this->_current.z) || (this->_old.phi != this->_current.phi)) {
        if (this->_ssr_running == true) {
            updatePos(-this->_current.x, -this->_current.y);
            updateAngle(fmod(90 - this->_current.phi, 360.0f));
        }
        //sendMessageToPhone(0, "POSITION/" + ofToString(-this->_current.x) + "/" + ofToString(-this->_current.y) + "/" + ofToString(this->_current.z) + "/" + ofToString(this->_current.phi));
    }
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofClear(0);
    drawVisualFeedback();
    this->_uiPanel.draw();

    ofSetColor(ofColor::white);
    ofDrawBitmapString("head (x/y): " + ofToString(this->_current.x) + "/" + ofToString(this->_current.y), 10, ofGetWindowHeight()-145);
    ofDrawBitmapString("head (phi): " + ofToString(this->_current.phi), 10, ofGetWindowHeight()-130);

    ofDrawBitmapString("origin (x/y): " + ofToString(this->_origin.x) + "/" + ofToString(this->_origin.y), 10, ofGetWindowHeight()-100);
    ofDrawBitmapString("origin (phi): " + ofToString(this->_origin.phi), 10, ofGetWindowHeight()-85);

    ofDrawBitmapString("sound (x/y): " + ofToString(this->_source_positions.x) + "/" + ofToString(this->_source_positions.y), 10, ofGetWindowHeight()-55);

    ofDrawBitmapString("IP: " + this->_my_ip, 10, ofGetWindowHeight()-25);

    ofSetColor(ofColor::black);
    ofDrawBitmapString("\\- window -/", this->_ui_center.x - 45, ofGetWindowHeight()-15);
    ofDrawBitmapString("door", this->_ui_world_start.x, this->_ui_center.y);
}

void ofApp::drawVisualFeedback() {
    ofSetColor(ofColor::white);
    ofDrawRectangle(this->_ui_world_start.x, this->_ui_world_start.y, this->_ui_world_diameter, this->_ui_world_diameter);
    ofPushMatrix();
    {
        ofTranslate(this->_ui_center);
        ofSetColor(ofColor::red);
        ofDrawCircle(0, 0, this->_ui_max_distance + this->_line_width);
        ofSetColor(ofColor::white);
        ofDrawCircle(0, 0, this->_ui_max_distance);

        // draw sound path
        ofSetColor(ofColor::gray);
        for (int kk=0; kk<this->_full_path.size(); kk++) {
            ofDrawCircle(mapPositionToPixel(this->_full_path[kk]), 1.0f);
        }

        ofPushMatrix();
        {
            ofVec2f pos = ofVec2f(this->_current.x, -this->_current.y);
            //ofVec2f pos = ofVec2f(this->_current.y, this->_current.x);
            pos = mapPositionToPixel(pos);
            ofTranslate(pos);
            ofSetColor(ofColor::orange);
            ofDrawCircle(0, 0, this->_ui_head_radius);
            //ofRotateDeg(this->_head_phi, 0, 0, 1); // of_v0.10
            ofRotateDeg(this->_current.phi); // of_v0.9
            ofSetColor(ofColor::red);
            ofDrawCircle(0, this->_ui_head_radius, 3);
        }
        ofPopMatrix();

        // draw sound source
        ofSetColor(ofColor::white);
        ofVec2f pos = mapPositionToPixel(this->_source_instance->getPosition());
        this->_source_instance->draw(pos.x, pos.y);
    }
    ofPopMatrix();
}

ofVec2f ofApp::mapPositionToPixel(ofVec2f pos) {
    float realWorld2PixelFactor = (this->_ui_max_distance / this->_max_distance);
    pos.x *= realWorld2PixelFactor;
    pos.y *= realWorld2PixelFactor;
    return pos;
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    if (key == '1') {
        connectToSSR(true);
    }
    if (key == '2') {
        loadSsrScene();
    }
    if (key == '3') {
        streamSSR(true);
    }
    if (key == '4') {
        updateSoundPos(this->_current.x, this->_current.y);
    }
    if (key == '5') {
        updatePos(this->_current.x, this->_current.y);
    }
    if (key == '6') {
        streamSSR(false);
    }
    if (key == '7') {
        connectToSSR(false);
    }

    if (key == 'r') {
        if (this->bReproduction) {
            this->bReproduction = false;
        } else {
            this->bReproduction = true;
        }
        cout << "reproduction mode " << this->bReproduction << endl;
    }
    if (key == 's') {
        setupProjectEyeTracker();
        ofSleepMillis(1);
        setupSubjectEyeTracker();
        ofSleepMillis(1);
        streamEyeTracker();
    }
    if (key == 'c') {
        calibrateEyeTracker();
    }
    if (key == 'd') {
        stopStreamEyeTracker();
    }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

void ofApp::connectPhone() {
    connectEyeTracker();
    /*if (this->_android_tcp_server->getNumClients() <= 0) {
        bool success = this->_android_tcp_server->setup(this->_android_port);
        if (success == true) {*/
            //this->_push_button_connect.removeListener(this, &ofApp::connectPhone);
            //this->_push_button_connect.setFillColor(ofColor::black);
            /*this->_push_button_disconnect.setFillColor(ofColor::gray);
            this->_push_button_disconnect.addListener(this, &ofApp::disconnectPhone);
            this->_push_button_disconnect.setTextColor(ofColor::white);
        }
    }*/
}

void ofApp::disconnectPhone() {
    connectToSSR(false);
    cleanupEyeTracker();
    /*if (this->_android_tcp_server->isConnected()) {
        for (int clientID = 0; clientID < this->_android_tcp_server->getNumClients(); clientID++) {
            sendMessageToPhone(clientID, "END/");
            this->_android_tcp_server->disconnectClient(clientID);
        }
        this->_push_button_disconnect.removeListener(this, &ofApp::disconnectPhone);
        this->_push_button_connect.addListener(this, &ofApp::connectPhone);
        this->_push_button_connect.setTextColor(ofColor::red);
        this->_push_button_connect.setFillColor(ofColor::gray);
        this->_push_button_disconnect.setFillColor(ofColor::black);
        this->_push_button_disconnect.setTextColor(ofColor::black);
    }*/
}

void ofApp::sendMessageToPhone(int client, string message) {
    /*if (this->_android_tcp_server->isClientConnected(client) == true) {
        char messageLength[4];
        for (int i = 0; i < 4; i++) {
            messageLength[3 - i] = (message.length() >> (i * 8));
        }
        this->_android_tcp_server->sendRawBytes(client, messageLength, 4);
        this->_android_tcp_server->sendRawBytes(client, message.c_str(), message.length());
    }*/
}

void ofApp::resetHeadOrigin() {
    // round to milimeter accuracy
    this->_origin.x = round(this->_head_data.x_position)/1000;
    this->_origin.y = round(this->_head_data.y_position)/1000;
    // round to 0.1 degree
    this->_origin.phi = 360.0f - round(this->_head_data.z_rot_avg*10)/10;
}

ofVec2f ofApp::shape_circle(float radius, float time, bool left) {
    ofVec2f xy;
    float angle = 0.5*PI;
    if (left == true) {
        angle *= -1;
    }
    float y = radius * cos(time);
    float x = -radius * sin(time);
    if (left == true) {
        x *= -1;
    }
    xy = ofVec2f(x * cos(angle) + y * sin(angle), x * -sin(angle) + y * cos(angle));
    return xy;
}

ofVec2f ofApp::shape_eight(float a, float time, float time_offset) {
    ofVec2f xy;
    float angle = -45.0f/180*PI;
    float y = a * cos(time + time_offset);
    float x = y * sin(time + time_offset);
    xy = ofVec2f(x * cos(angle) + y * sin(angle), x * -sin(angle) + y * cos(angle));
    return xy;
}

ofVec2f ofApp::shape_limacon(float b, float a, float time, float time_offset) {
    ofVec2f xy;
    float temp = b+a*cos(time + time_offset);
    float x = temp * cos(time + time_offset);
    float y = temp * sin(time + time_offset);
    //xy = ofVec2f(x, y);
    xy = ofVec2f(y, x);
    return xy;
}

void ofApp::setPathToEight() {
    this->_selected_shape = 0;
    this->_phi_offset = this->_shape_eight_phi_off;
    this->_shape_offset = shape_eight(this->_shape_eight_half_size, -this->_phi_offset/180*PI, 0.0f);
    this->_source_positions = shape_eight(this->_shape_eight_half_size, -(this->_phi_offset+10.0f)/180*PI, 0.0f) - this->_shape_offset;
    this->_full_path.clear();
    for (int kk=0; kk<360; kk++) {
        this->_full_path.push_back(shape_eight(this->_shape_eight_half_size, -((float)kk + this->_phi_offset)/180*PI, 0.0f) - this->_shape_offset);
    }
}

void ofApp::setPathToLimacon(){ // circle right
    /*this->_selected_shape = 1;
    this->_phi_offset = this->_shape_limacon_phi_off;
    this->_shape_offset = shape_limacon(this->_shape_limacon_offset, this->_shape_limacon_center, -this->_phi_offset/180*PI, 0.0f);
    this->_source_positions = shape_limacon(this->_shape_limacon_offset, this->_shape_limacon_center, -(this->_phi_offset-5.0f)/180*PI, 0.0f) - this->_shape_offset;
    this->_full_path.clear();
    for (int kk=0; kk<360; kk++) {
        this->_full_path.push_back(shape_limacon(this->_shape_limacon_offset, this->_shape_limacon_center, -((float)kk + this->_phi_offset)/180*PI, 0.0f) - this->_shape_offset);
    }*/

    this->_selected_shape = 2;
    this->_direction = false;
    this->_phi_offset = 0;
    this->_shape_offset = shape_circle(this->_shape_circle_diameter, -this->_phi_offset/180*PI, this->_direction);
    this->_source_positions = shape_circle(this->_shape_circle_diameter, -(this->_phi_offset - 15.0f)/180*PI, this->_direction) - this->_shape_offset;
    this->_full_path.clear();
    for (int kk=0; kk<360; kk++) {
        this->_full_path.push_back(shape_circle(this->_shape_circle_diameter, -((float)kk + this->_phi_offset)/180*PI, this->_direction) - this->_shape_offset);
    }
}

void ofApp::setPathToCircle() {
    this->_selected_shape = 2;
    this->_direction = true;
    this->_phi_offset = 0;
    this->_shape_offset = shape_circle(this->_shape_circle_diameter, -this->_phi_offset/180*PI, this->_direction);
    this->_source_positions = shape_circle(this->_shape_circle_diameter, -(this->_phi_offset - 15.0f)/180*PI, this->_direction) - this->_shape_offset;
    this->_full_path.clear();
    for (int kk=0; kk<360; kk++) {
        this->_full_path.push_back(shape_circle(this->_shape_circle_diameter, -((float)kk + this->_phi_offset)/180*PI, this->_direction) - this->_shape_offset);
    }
}

void ofApp::toggleRecording(const void *sender, bool &value) {
    if (value == true) {
        recordEyeTracker(); // this needs some time, so I do it first
        if (this->_isLogFileCreated == false) {
            this->_isLogFileCreated = true;
            ofLogToFile(this->_username + "_" + nowToString() + ".txt"); // set output filename
        }
        ofSetLogLevel(OF_LOG_NOTICE); // activate logging
        ofLogNotice("RECORD", ",TIME,HEAD_X,HEAD_Y,HEAD_HEIGHT,HEAD_PHI,SOUND_X,SOUND_Y,SOUND_HEIGHT,SOUND_ON"); // write header
        this->_logStartTime = ofGetElapsedTimef();
        this->_toggle_button_eog.setTextColor(ofColor::green);
        this->_eog_trigger->startRecording();
    } else {
        stopRecordingEyeTracker();
        this->_eog_trigger->stopRecording();
        this->_toggle_button_eog.setTextColor(ofColor::red);
        ofSetLogLevel(OF_LOG_SILENT); // deactivate logging
        this->_isLogFileCreated = false;
    }
}

void ofApp::toggleSound(const void *sender, bool &value) {
    this->_sound_on = value;
    if (value == true) {
        // update ui
        this->_push_button_eight.removeListener(this, &ofApp::setPathToEight);
        this->_push_button_limacon.removeListener(this, &ofApp::setPathToLimacon);
        this->_push_button_circle.removeListener(this, &ofApp::setPathToCircle);
        this->_push_button_eight.setFillColor(ofColor::black);
        this->_push_button_limacon.setFillColor(ofColor::black);
        this->_push_button_circle.setFillColor(ofColor::black);
        this->_push_button_eight.setTextColor(ofColor::black);
        this->_push_button_limacon.setTextColor(ofColor::black);
        this->_push_button_circle.setTextColor(ofColor::black);
        this->_toggle_button_sound.setTextColor(ofColor::green);
        // send sound message
        //sendMessageToPhone(0, "PLAY/");
        streamSSR(true);
        this->_eog_trigger->sendTrigger("sound_on");
        this->_start_recoring = true;
        this->_is_recording = false;
        sendEyeTrackerEvent("sound_on");
    } else {
        sendEyeTrackerEvent("sound_off");
        this->_eog_trigger->sendTrigger("sound_off");
        //sendMessageToPhone(0, "STOP/");
        streamSSR(false);
        this->_start_recoring = false;
        this->_is_recording = false;
        this->_toggle_button_sound.setTextColor(ofColor::red);
        this->_push_button_eight.setTextColor(ofColor::white);
        this->_push_button_limacon.setTextColor(ofColor::white);
        this->_push_button_eight.setFillColor(ofColor::gray);
        this->_push_button_limacon.setFillColor(ofColor(128));
        this->_push_button_circle.setTextColor(ofColor::white);
        this->_push_button_circle.setFillColor(ofColor::gray);
        this->_push_button_eight.addListener(this, &ofApp::setPathToEight);
        this->_push_button_limacon.addListener(this, &ofApp::setPathToLimacon);
        this->_push_button_circle.addListener(this, &ofApp::setPathToCircle);
        loadSsrScene(); // reset scene to be able to continue
    }
}

void ofApp::loadSettingsAndWriteDefaultIfNeeded() {
    this->_settings = new ofxXmlSettings();
    if (this->_settings->loadFile(this->_settings_filename) == false) {
        writeDefaultSettings();
        this->_settings->loadFile(this->_settings_filename);
    }
    this->_settings->pushTag("settings");
    {
        this->_source_radius = this->_settings->getValue("source_size", 12.0f);
        this->_settings->pushTag("subject");
        {
            this->_username = this->_settings->getValue("name", "USER");
            this->_source_height = this->_settings->getValue("ear_height", 1.60f);
            this->_ui_head_radius = this->_settings->getValue("ui_radius", 2.0f);
        }
        this->_settings->popTag();
        this->_settings->pushTag("path");
        {
            this->_path_duration = this->_settings->getValue("duration", 10);
            this->_path_revolutions = this->_settings->getValue("revolutions", 1);
            this->_settings->pushTag("eight");
            {
                this->_shape_eight_half_size = this->_settings->getValue("half_len", 1.50f);
                this->_shape_eight_phi_off = this->_settings->getValue("phi_offset", 180.0f);
            }
            this->_settings->popTag();
            this->_settings->pushTag("limacon");
            {
                this->_shape_limacon_center = this->_settings->getValue("center", 2.50f);
                this->_shape_limacon_offset = this->_settings->getValue("off_center", 0.50f);
                this->_shape_limacon_phi_off = this->_settings->getValue("phi_offset", 45.0f);
            }
            this->_settings->popTag();
            this->_settings->pushTag("circle");
            {
                this->_shape_circle_diameter = this->_settings->getValue("diameter", 1.50f);
            }
            this->_settings->popTag();
        }
        this->_settings->popTag();
        this->_settings->pushTag("network");
        {
            this->_settings->pushTag("tobii");
            {
                this->_tobii_port = this->_settings->getValue("port", 8000);
                this->_tobii_ip = this->_settings->getValue("ip", "192.168.1.1");
            }
            this->_settings->popTag();
            this->_settings->pushTag("android");
            {
                this->_android_ip = this->_settings->getValue("ip", "192.168.1.1");
                this->_android_port = this->_settings->getValue("port", -1);
            }
            this->_settings->popTag();
            this->_settings->pushTag("eog");
            {
                this->_eog_host = this->_settings->getValue("host", "");
            }
            this->_settings->popTag();
            this->_settings->pushTag("mocap");
            {
                this->_use_vicon = this->_settings->getValue("use_vicon", false);
                this->_mocap_ip = this->_settings->getValue("host", "");
                this->_settings->pushTag("port");
                {
                    this->_mocap_receive_port = this->_settings->getValue("in",  -1);
                    this->_mocap_send_port = this->_settings->getValue("out", -1);
                }
                this->_settings->popTag();
            }
            this->_settings->popTag();
        }
        this->_settings->popTag();
    }
    this->_settings->popTag();
}

void ofApp::writeDefaultSettings() {
    this->_settings->addTag("settings");
    this->_settings->pushTag("settings");
    {
        this->_settings->addValue("source_size", 12.0f);
        this->_settings->addTag("subject");
        this->_settings->pushTag("subject");
        {
            this->_settings->addValue("name", "USER");
            this->_settings->addValue("ear_height", 1.60f);
            this->_settings->addValue("ui_radius", 15.0f);
        }
        this->_settings->popTag();
        this->_settings->addTag("path");
        this->_settings->pushTag("path");
        {
            this->_settings->addValue("duration", 60);
            this->_settings->addValue("revolutions", 2);
            this->_settings->addTag("eight");
            this->_settings->pushTag("eight");
            {
                this->_settings->addValue("half_len", 1.50f);
                this->_settings->addValue("phi_offset", 250.0f);
            }
            this->_settings->popTag();
            this->_settings->addTag("limacon");
            this->_settings->pushTag("limacon");
            {
                this->_settings->addValue("center", 2.50f);
                this->_settings->addValue("off_center", 0.50f);
                this->_settings->addValue("phi_offset", 55.0f);
            }
            this->_settings->popTag();
            this->_settings->addTag("circle");
            this->_settings->pushTag("circle");
            {
                this->_settings->addValue("diameter", 1.50f);
            }
            this->_settings->popTag();
        }
        this->_settings->popTag();
        this->_settings->addTag("network");
        this->_settings->pushTag("network");
        {
            this->_settings->addTag("tobii");
            this->_settings->pushTag("tobii");
            {
                this->_settings->addValue("port", 8000);
                this->_settings->addValue("ip", "192.168.1.1");
            }
            this->_settings->popTag();
            this->_settings->addTag("android");
            this->_settings->pushTag("android");
            {
                this->_settings->addValue("ip", "192.168.1.1");
                this->_settings->addValue("port", 9000);
            }
            this->_settings->popTag();
            this->_settings->addTag("eog");
            this->_settings->pushTag("eog");
            {
                this->_settings->addValue("host", "192.168.1.1");
            }
            this->_settings->popTag();
            this->_settings->addTag("mocap"); // maybe the eog trigger can be broadcasted, then mocap has to listen on port 65500 as well
            this->_settings->pushTag("mocap");
            {
                this->_settings->setValue("use_vicon", true);
                this->_settings->addValue("host", "192.168.1.1");
                this->_settings->addTag("port");
                this->_settings->pushTag("port");
                {
                    this->_settings->addValue("in",  18403);
                    this->_settings->addValue("out", 18404);
                }
                this->_settings->popTag();
            }
            this->_settings->popTag();
        }
        this->_settings->popTag();
    }
    this->_settings->popTag();
    this->_settings->saveFile();
}

string ofApp::nowToString() {
    time_t rawtime;
    struct tm * timeinfo;
    char buffer[80];
    time (&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(buffer,sizeof(buffer),"%d-%m-%Y_%H-%M-%S",timeinfo);
    std::string str(buffer);
    return str;
}

string ofApp::getIPhost() {
    string ad = string();
    vector<string> list = getLocalIPs();
    if (!list.empty()) {
        ad = list[0];
    }
    return ad;
}
vector<string> ofApp::getLocalIPs() {
    vector<string> result;
#ifdef TARGET_WIN32
    string commandResult = ofSystem("ipconfig");
    for (int pos = 0; pos >= 0; ) {
        pos = commandResult.find("IPv4", pos);
        if (pos >= 0) {
            pos = commandResult.find(":", pos) + 2;
            int pos2 = commandResult.find("\n", pos);
            string ip = commandResult.substr(pos, pos2 - pos);
            pos = pos2;
            if (ip.substr(0, 3) != "127") { // let's skip loopback addresses
                result.push_back(ip);
            }
        }
    }
#else
    string commandResult = ofSystem("ifconfig");
    for (int pos = 0; pos >= 0; ) {
        pos = commandResult.find("inet ", pos);
        if (pos >= 0) {
            int pos2 = commandResult.find("netmask", pos);
            string ip = commandResult.substr(pos + 5, pos2 - pos - 6);
            pos = pos2;
            if (ip.substr(0, 3) != "127") { // let's skip loopback addresses
                result.push_back(ip);
            }
        }
    }
#endif
    return result;
}
