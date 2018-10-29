#pragma once

#include "ofMain.h"
#include "ofxXmlSettings.h"
#include "ofxGui.h"

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
    // ui
    void setupUI();
    ofxPanel _uiPanel;
    ofxLabel _phone_label, _tracking_label, _presentation_label;
    ofxToggle _toggle_button_eog, _toggle_button_sound;
    ofxButton _push_button_eight, _push_button_limacon;
    ofxButton _push_button_connect, _push_button_disconnect;
    ofxButton _reset_head_origin;
    // ui callbacks
    void connectPhone();
    void disconnectPhone();
    void resetHeadOrigin();
    void setPathToEight();
    void setPathToLimacon();
    void toggleRecording(const void *sender, bool &value);
    void toggleSound(const void *sender, bool &value);

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
