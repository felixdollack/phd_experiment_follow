#include "ofApp.h"

void ofApp::exit(){
    this->_eog_trigger->stopRecording();
    disconnectPhone();
    if (this->_android_tcp_server->isConnected()) {
        this->_android_tcp_server->close();
    }
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
    if (this->_android_tcp_server == NULL) {
        this->_android_tcp_server = new ofxTCPServer();
        this->_android_tcp_server->setMessageDelimiter("");
    }
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
    this->_uiPanel.add(this->_push_button_connect.setup("connect to phone"));
    this->_uiPanel.add(this->_push_button_disconnect.setup("disconnect from phone"));

    this->_uiPanel.add(this->_tracking_label.setup("TRACKING",""));
    this->_uiPanel.add(this->_reset_head_origin.setup("reset head origin"));
    this->_uiPanel.add(this->_toggle_button_eog.setup("record eog", false));

    this->_uiPanel.add(this->_presentation_label.setup("PRESENTATION",""));
    this->_uiPanel.add(this->_toggle_button_sound.setup("sound on", false));
    this->_uiPanel.add(this->_push_button_eight.setup("eight"));
    this->_uiPanel.add(this->_push_button_limacon.setup("limacon"));

    // set gui text and box colors
    this->_push_button_connect.setTextColor(ofColor::red);
    this->_push_button_disconnect.setFillColor(ofColor::black);
    this->_push_button_disconnect.setTextColor(ofColor::black);
    this->_toggle_button_eog.setTextColor(ofColor::red);
    this->_toggle_button_sound.setTextColor(ofColor::red);

    // add gui listeners
    this->_push_button_connect.addListener(this, &ofApp::connectPhone);
    this->_reset_head_origin.addListener(this, &ofApp::resetHeadOrigin);
    this->_toggle_button_eog.addListener(this, &ofApp::toggleRecording);
    this->_toggle_button_sound.addListener(this, &ofApp::toggleSound);
    this->_push_button_eight.addListener(this, &ofApp::setPathToEight);
    this->_push_button_limacon.addListener(this, &ofApp::setPathToLimacon);
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
    if (this->_android_tcp_server->getNumClients() > 0) {
        this->_push_button_connect.setTextColor(ofColor::green);
    } else {
        this->_push_button_connect.setTextColor(ofColor::red);
    }
    this->_vicon_receiver.updateData();
    this->_head_data = this->_vicon_receiver.getLatestData();

    // update source and user position
    if (dt > 2*this->_step_duration) {
        updateParticipantPosition();
        if (this->_is_recording == true) {
            if (this->_selected_shape == 0) {
                this->_source_positions = shape_eight(1.0f, -(this->_current_phi + this->_phi_offset), 0.0f) - this->_shape_offset;
            } else {
                this->_source_positions = shape_limacon(0.5f, 1.0f, -(this->_current_phi + this->_phi_offset), 0.0f) - this->_shape_offset;
            }
            this->_current_phi += _path_step*(dt/_step_duration);
            // sound source position
            sendMessageToPhone(0, "SRCPOS/" + ofToString(-this->_source_positions.x) + "/" + ofToString(this->_source_positions.y) + "/" + ofToString(this->_source_height));
            if (this->_current_phi >= (2*PI * this->_path_revolutions)) {
                bool t = false;
                toggleSound(NULL, t);
            }
        }
        this->_time = now;
        // log current states
        ofLogNotice("UPDATE", "," + ofToString(now-this->_logStartTime) + "," + ofToString(-this->_current.x) + "," + ofToString(-this->_current.y) + "," + ofToString(this->_current.z) + "," + ofToString(this->_current.phi) + "," + ofToString(-this->_source_positions.x) + "," + ofToString(this->_source_positions.y) + "," + ofToString(this->_source_height) + "," + ofToString(this->_sound_on));
    }
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
        sendMessageToPhone(0, "POSITION/" + ofToString(-this->_current.x) + "/" + ofToString(-this->_current.y) + "/" + ofToString(this->_current.z) + "/" + ofToString(this->_current.phi));
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
    if (this->_android_tcp_server->getNumClients() <= 0) {
        bool success = this->_android_tcp_server->setup(this->_android_port);
        if (success == true) {
            this->_push_button_connect.removeListener(this, &ofApp::connectPhone);
            this->_push_button_disconnect.addListener(this, &ofApp::disconnectPhone);
            this->_push_button_connect.setFillColor(ofColor::black);
            this->_push_button_disconnect.setFillColor(ofColor::gray);
            this->_push_button_disconnect.setTextColor(ofColor::white);
        }
    }
}

void ofApp::disconnectPhone() {
    if (this->_android_tcp_server->isConnected()) {
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
    }
}

void ofApp::sendMessageToPhone(int client, string message) {
    if (this->_android_tcp_server->isClientConnected(client) == true) {
        char messageLength[4];
        for (int i = 0; i < 4; i++) {
            messageLength[3 - i] = (message.length() >> (i * 8));
        }
        this->_android_tcp_server->sendRawBytes(client, messageLength, 4);
        this->_android_tcp_server->sendRawBytes(client, message.c_str(), message.length());
    }
}

void ofApp::resetHeadOrigin() {
    // round to milimeter accuracy
    this->_origin.x = round(this->_head_data.x_position)/1000;
    this->_origin.y = round(this->_head_data.y_position)/1000;
    // round to 0.1 degree
    this->_origin.phi = 360.0f - round(this->_head_data.z_rot_avg*10)/10;
}

ofVec2f ofApp::shape_eight(float a, float time, float time_offset) {
    ofVec2f xy;
    float x = a * sin(time + time_offset);
    float y = x * cos(time + time_offset);
    xy = ofVec2f(x, y);
    return xy;
}

ofVec2f ofApp::shape_limacon(float b, float a, float time, float time_offset) {
    ofVec2f xy;
    float temp = b+a*cos(time + time_offset);
    float x = temp * cos(time + time_offset);
    float y = temp * sin(time + time_offset);
    xy = ofVec2f(x, y);
    return xy;
}

void ofApp::setPathToEight() {
    this->_selected_shape = 0;
    this->_phi_offset = 45;
    this->_shape_offset = shape_eight(1.0f, -this->_phi_offset, 0.0f);
    this->_full_path.clear();
    for (int kk=0; kk<360; kk++) {
        this->_full_path.push_back(shape_eight(1.0f, -((float)kk + this->_phi_offset)/180*PI, 0.0f) - this->_shape_offset);
    }
}

void ofApp::setPathToLimacon(){
    this->_selected_shape = 1;
    this->_phi_offset = 45;
    this->_shape_offset = shape_limacon(0.5f, 1.0f, -this->_phi_offset, 0.0f);
    this->_full_path.clear();
    for (int kk=0; kk<360; kk++) {
        this->_full_path.push_back(shape_limacon(0.5f, 1.0f, -((float)kk + this->_phi_offset)/180*PI, 0.0f) - this->_shape_offset);
    }
}

void ofApp::toggleRecording(const void *sender, bool &value) {
    if (value == true) {
        if (this->_isLogFileCreated == false) {
            this->_isLogFileCreated = true;
            ofLogToFile(nowToString() + ".txt"); // set output filename
        }
        ofSetLogLevel(OF_LOG_NOTICE); // activate logging
        ofLogNotice("RECORD", ",TIME,HEAD_X,HEAD_Y,HEAD_HEIGHT,HEAD_PHI,SOUND_X,SOUND_Y,SOUND_HEIGHT,SOUND_ON"); // write header
        this->_logStartTime = ofGetElapsedTimef();
        this->_toggle_button_eog.setTextColor(ofColor::green);
        this->_eog_trigger->startRecording();
    } else {
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
        this->_push_button_eight.setFillColor(ofColor::black);
        this->_push_button_limacon.setFillColor(ofColor::black);
        this->_push_button_eight.setTextColor(ofColor::black);
        this->_push_button_limacon.setTextColor(ofColor::black);
        this->_toggle_button_sound.setTextColor(ofColor::green);
        // send sound message
        sendMessageToPhone(0, "PLAY/");
        this->_eog_trigger->sendTrigger("sound_on");
        this->_start_recoring = true;
        this->_is_recording = false;
    } else {
        this->_eog_trigger->sendTrigger("sound_off");
        sendMessageToPhone(0, "STOP/");
        this->_start_recoring = false;
        this->_is_recording = false;
        this->_toggle_button_sound.setTextColor(ofColor::red);
        this->_push_button_eight.setTextColor(ofColor::white);
        this->_push_button_limacon.setTextColor(ofColor::white);
        this->_push_button_eight.setFillColor(ofColor::gray);
        this->_push_button_limacon.setFillColor(ofColor(128));
        this->_push_button_eight.addListener(this, &ofApp::setPathToEight);
        this->_push_button_limacon.addListener(this, &ofApp::setPathToLimacon);
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
            this->_source_height = this->_settings->getValue("ear_height", 1.60f);
            this->_ui_head_radius = this->_settings->getValue("ui_radius", 2.0f);
        }
        this->_settings->popTag();
        this->_settings->pushTag("path");
        {
            this->_path_duration = this->_settings->getValue("duration", 10);
            this->_path_revolutions = this->_settings->getValue("revolutions", 1);
        }
        this->_settings->popTag();
        this->_settings->pushTag("network");
        {
            this->_settings->pushTag("android");
            {
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
            this->_settings->addValue("ear_height", 1.60f);
            this->_settings->addValue("ui_radius", 15.0f);
        }
        this->_settings->popTag();
        this->_settings->addTag("path");
        this->_settings->pushTag("path");
        {
            this->_settings->addValue("duration", 60);
            this->_settings->addValue("revolutions", 2);
        }
        this->_settings->popTag();
        this->_settings->addTag("network");
        this->_settings->pushTag("network");
        {
            this->_settings->addTag("android");
            this->_settings->pushTag("android");
            {
                this->_settings->addValue("port", 12345);
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
