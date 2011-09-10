/*
 *  embeddedSC.h
 *  schostExample
 *
 *  Created by Chris on 10/09/2011.
 *  Copyright 2011 Goldsmiths Creative Computing. All rights reserved.
 *
 */

#include "SCProcess.h"

class embeddedSCServer {
public:
	~embeddedSCServer();
	void setup(int nChannels, int bufferSize, int sampleRate);
	void process (float *buffer, int bufferSize, int nChannels);
protected:
	void initialize();
	void syncOSCOffsetWithTimeOfDay();

	int nChannels, bufferSize, sampleRate;
	World* world;
	SCProcess* superCollider;
	int64 gOSCoffset;
	AudioBufferList *scBufferList;
	
};