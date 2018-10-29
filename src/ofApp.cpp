#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    loadSettingsAndWriteDefaultIfNeeded();
    setupUI();
    setupVisualFeedback();
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
}

//--------------------------------------------------------------
void ofApp::update(){

}

//--------------------------------------------------------------
void ofApp::draw(){
    ofClear(0);
    drawVisualFeedback();
    this->_uiPanel.draw();
}

void ofApp::drawVisualFeedback() {
    ofSetColor(ofColor::white);
    ofDrawRectangle(this->_ui_world_start.x, this->_ui_world_start.y, this->_ui_world_diameter, this->_ui_world_diameter);
    ofPushMatrix();
    {
        ofTranslate(this->_ui_center);
    }
    ofPopMatrix();
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
}

void ofApp::disconnectPhone() {
}

void ofApp::resetHeadOrigin() {
}

void ofApp::setPathToEight() {
}

void ofApp::setPathToLimacon(){
}

void ofApp::toggleRecording(const void *sender, bool &value) {
    if (value == true) {
        this->_toggle_button_eog.setTextColor(ofColor::green);
    } else {
        this->_toggle_button_eog.setTextColor(ofColor::red);
    }
}

void ofApp::toggleSound(const void *sender, bool &value) {
    if (value == true) {
        // update ui
        this->_push_button_eight.removeListener(this, &ofApp::setPathToEight);
        this->_push_button_limacon.removeListener(this, &ofApp::setPathToLimacon);
        this->_push_button_eight.setFillColor(ofColor::black);
        this->_push_button_limacon.setFillColor(ofColor::black);
        this->_push_button_eight.setTextColor(ofColor::black);
        this->_push_button_limacon.setTextColor(ofColor::black);
        this->_toggle_button_sound.setTextColor(ofColor::green);
    } else {
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
        this->_settings->pushTag("subject");
        {
            this->_source_height = this->_settings->getValue("ear_height", 1.60f);
            this->_ui_head_radius = this->_settings->getValue("ui_radius", 2.0f);
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
        this->_settings->addTag("subject");
        this->_settings->pushTag("subject");
        {
            this->_settings->addValue("ear_height", 1.60f);
            this->_settings->addValue("ui_radius", 15.0f);
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
