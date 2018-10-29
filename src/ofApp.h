#pragma once

#include "ofMain.h"
#include "ofxXmlSettings.h"

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void windowResized(int w, int h);
		void gotMessage(ofMessage msg);

private:
    ofxXmlSettings *_settings;
    const string _settings_filename = "settings.xml";
    void loadSettingsAndWriteDefaultIfNeeded();
    void writeDefaultSettings();

    // ui settings
    float _ui_head_radius;

    // sound source specific settings
    float _source_height, _source_radius;

    // shimmer eog
    string _eog_host;

    // motion capture
    int _mocap_receive_port, _mocap_send_port;
    string _mocap_ip;
    bool _use_vicon;

    // network
    int _android_port;
};
