#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    loadSettingsAndWriteDefaultIfNeeded();
}

//--------------------------------------------------------------
void ofApp::update(){

}

//--------------------------------------------------------------
void ofApp::draw(){

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
