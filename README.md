# phd_experiment_follow
A small openframeworks program to control a sound guidance experiment.
Position and orientation of participants is tracked and forwarded from a [Vicon]
motion tracking system. The position of a sound source rendered by [Sound Scape Renderer]
is manipulated and moved via OSC commands.

## Requirements
* openframeworks
* ofxXmlSettings
* ofxGui
* ofxNetwork
* [osc sound synthesis control]

[Vicon]:http://www.vicon.com
[Sound Scape Renderer]:http://spatialaudio.net/ssr/
[osc sound synthesis control]:https://github.com/felixdollack/osc_sound_synthesis_control
