/*
 *  embeddedSC.cpp
 *  schostExample
 *
 *  Created by Chris on 10/09/2011.
 *  Copyright 2011 Goldsmiths Creative Computing. All rights reserved.
 *
 */

#include "embeddedSC.h"
#include <sys/time.h>

void embeddedSCServer::setup(int _nChannels, int _bufferSize, int _sampleRate) {
	nChannels = _nChannels;
	bufferSize = _bufferSize;
	sampleRate = _sampleRate;

	//see http://stackoverflow.com/questions/2449716/does-this-make-any-sense-apple-documentation
	scBufferList = (AudioBufferList*) malloc(sizeof (AudioBufferList) + ((nChannels - 1) * sizeof(AudioBuffer)));
	scBufferList->mNumberBuffers = nChannels;
	
	for(int i=0; i< nChannels; i++) {
		AudioBuffer scBuffer;
		scBuffer.mNumberChannels = 1;
		scBuffer.mDataByteSize = sizeof(float) * bufferSize * nChannels;
		scBuffer.mData = (void*) malloc(sizeof(float) * bufferSize);
		memset(scBuffer.mData, 0, sizeof(float) * bufferSize);
		scBufferList->mBuffers[i] = scBuffer;
	}
	
	
	superCollider = new SCProcess();
	initialize();
	
}

embeddedSCServer::~embeddedSCServer() {
    if(superCollider){
        delete superCollider;
    }
	
	for(int i=0; i< scBufferList->mNumberBuffers; i++) 
		delete[] scBufferList->mBuffers[i].mData;
	
	delete scBufferList;
}

void embeddedSCServer::syncOSCOffsetWithTimeOfDay()
{
	// generate a value gOSCoffset such that
	// (gOSCOffset + systemTimeInOSCunits)
	// is equal to gettimeofday time in OSCunits.
	// Then if this machine is synced via NTP, we are synced with the world.
	// more accurate way to do this??
	
	struct timeval tv;
	
	int64 systemTimeBefore, systemTimeAfter, diff;
	int64 minDiff = 0x7fffFFFFffffFFFFLL;
	
	// take best of several tries
	const int numberOfTries = 5;
	int64 newOffset = gOSCoffset;
	for (int i=0; i<numberOfTries; ++i) {
		systemTimeBefore = AudioGetCurrentHostTime();
		gettimeofday(&tv, 0);
		systemTimeAfter = AudioGetCurrentHostTime();
		
		diff = systemTimeAfter - systemTimeBefore;
		if (diff < minDiff) {
			minDiff = diff;
			// assume that gettimeofday happens halfway between AudioGetCurrentHostTime calls
			int64 systemTimeBetween = systemTimeBefore + diff/2;
			int64 systemTimeInOSCunits = (int64)((double)AudioConvertHostTimeToNanos(systemTimeBetween) * kNanosToOSCunits);
			int64 timeOfDayInOSCunits  = ((int64)(tv.tv_sec + kSECONDS_FROM_1900_to_1970) << 32)
			+ (int64)(tv.tv_usec * kMicrosToOSCunits);
			newOffset = timeOfDayInOSCunits - systemTimeInOSCunits;
		}
	}
	this->gOSCoffset = newOffset;
}

void embeddedSCServer::initialize()
{
	
    WorldOptions options = kDefaultWorldOptions;
    options.mPreferredSampleRate=sampleRate;
    options.mBufLength=64;
    options.mPreferredHardwareBufferFrameSize = bufferSize;
	options.mMaxWireBufs=64;
    options.mRealTimeMemorySize=8192;
	const char* SC_PLUGIN_PATH = "/Applications/SuperCollider/plugins";
	const char* SC_SYNTHDEF_PATH = "/Users/chris/Library/Application Support/SuperCollider/synthdefs";  //insert your path here
	
    CFStringRef pluginsPath = CFStringCreateWithCString(NULL, SC_PLUGIN_PATH, kCFStringEncodingASCII);
    CFStringRef synthdefsPath = CFStringCreateWithCString(NULL, SC_SYNTHDEF_PATH, kCFStringEncodingASCII);
	
	int udpPortNum = 9989;
	
	syncOSCOffsetWithTimeOfDay();
	
    superCollider->startUp(options, pluginsPath, synthdefsPath, udpPortNum);
	
    scprintf("*******************************************************\n");
    scprintf("SuperColliderAU Initialized \n");
	scprintf("SuperColliderAU provided under the GNU GPL license. http://www.gnu.org/licenses/gpl-2.0.txt \n");
	scprintf("SuperColliderAU source code: http://supercollider.sf.net \n");
    scprintf("SuperColliderAU mPreferredHardwareBufferFrameSize: %d \n",options.mPreferredHardwareBufferFrameSize );
    scprintf("SuperColliderAU mBufLength: %d \n",options.mBufLength );
    scprintf("SuperColliderAU  port: %d \n", superCollider->portNum );
	scprintf("SuperColliderAU  mMaxWireBufs: %d \n", options.mMaxWireBufs );
    scprintf("SuperColliderAU  mRealTimeMemorySize: %d \n", options.mRealTimeMemorySize );
	scprintf("*******************************************************\n");
    fflush(stdout);
}


//int64 SuperColliderAU::getOscTime( const AudioTimeStamp & inTimeStamp){
//	int64 targetTimeOSC;
//	
//	if (inTimeStamp.mFlags & kAudioTimeStampHostTimeValid){
//		targetTimeOSC = (int64)((double)AudioConvertHostTimeToNanos(inTimeStamp.mHostTime) * kNanosToOSCunits);
//		return targetTimeOSC;
//	}
//	else{
//		/*
//		 UInt32 size = sizeof(Float64);
//		 Float64 outPresentationLatency;
//		 ComponentResult result = AudioUnitGetProperty (mComponentInstance, kAudioUnitProperty_PresentationLatency, kAudioUnitScope_Output, 0, &outPresentationLatency, &size);
//		 */
//		int64 hostTime = AudioGetCurrentHostTime();
//		targetTimeOSC = (int64)((double)AudioConvertHostTimeToNanos(hostTime) * kNanosToOSCunits);
//		return this->gOSCoffset + targetTimeOSC;
//	}
//	
//}
//
//

void embeddedSCServer::process (float *buffer, int bufferSize, int nChannels) {
	//deinterleave
	if (nChannels > 1) {
		for(int i=0; i < nChannels; i++) {
			for(int j=0; j < bufferSize; j++) {
				((float*)scBufferList->mBuffers[i].mData)[j] = buffer[(2*j) + 1];
			}
		}
	}
	
	AudioTimeStamp ts;
	//		int64 oscTime = getOscTime( inTimeStamp);
	//use oscTime=0 for now - which means no event scheduling on server
	superCollider->run(scBufferList, scBufferList, bufferSize, ts, sampleRate, 0);
	
	//interleave
	for(int i=0; i < bufferSize; i++) {
		for(int j=0; j < nChannels; j++) {
			buffer[i*nChannels + j] = ((float*)scBufferList->mBuffers[j].mData)[i];
		}
	}
}
