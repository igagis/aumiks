/* The MIT License:

Copyright (c) 2009-2011 Ivan Gagis

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE. */

/*
 * aumiks 0.2
 * File:   aumiks.hpp
 * Author: ivan
 *
 * Created on February 16, 2009, 10:28 PM
 */

#pragma once

#include <ting/types.hpp>
#include <ting/Singleton.hpp>
#include <ting/Exc.hpp>
#include <ting/types.hpp>
#include <ting/debug.hpp>
#include <ting/Buffer.hpp>
#include <ting/Array.hpp>
#include <ting/Ref.hpp>
#include <ting/Thread.hpp>
#include <ting/Ptr.hpp>

#include <list>

#include "Exc.hpp"


//#define M_ENABLE_AUMIKS_TRACE
#ifdef M_ENABLE_AUMIKS_TRACE
#define M_AUMIKS_TRACE(x) TRACE(<<"[aumiks] ") TRACE(x)
#else
#define M_AUMIKS_TRACE(x)
#endif


namespace aumiks{

using namespace ting;

//forward declarations
class Lib;
class Channel;



class Lib : public ting::Singleton<Lib>{
	friend class Channel;

public:
	/**
	 * @brief Create sound library singleton instance.
	 * Creates singleton instance of sound library object and
	 * opens sound device.
	 * @param requestedBufferSizeInFrames - size of desired playing buffer
	 *                                      in frames (1 frame is 4 bytes for 16bit stereo).
	 */
	Lib(unsigned requestedBufferSizeInFrames = 4096);
	~Lib();

	void PlayChannel(ting::Ref<Channel> ch);

	inline void SetMuted(bool muted){
		this->thread.isMuted = muted;
	}

	inline void SetUnmuted(bool unmuted){
		this->SetMuted(!unmuted);
	}

	inline void Mute(){
		this->SetMuted(true);
	}

	inline void Unmute(){
		this->SetMuted(false);
	}

	inline bool IsMuted()const{
		return this->thread.isMuted;
	}

private:
	class SoundThread : public ting::MsgThread{
		unsigned desiredBufferSizeInFrames;
	public:
		ting::Mutex chPoolMutex;
		
		typedef std::list<Ref<aumiks::Channel> > TChPool;
		typedef TChPool::iterator TChIter;
		TChPool chPool;

		TChPool chPoolToAdd;

		volatile bool isMuted;

		SoundThread(unsigned requestedBufferSizeInFrames);

		//override
		void Run();
	};

	SoundThread thread;
};



//base channel class
class Channel : public ting::RefCounted{
	friend class Lib;
	friend class Lib::SoundThread;

	volatile bool isPlaying;
	
protected:
	bool stopFlag;

	ting::u8 volume;
	
	Channel() :
			isPlaying(false),
			stopFlag(false),
			volume(0xff)
	{}
	
public:

	inline bool IsPlaying()const{
		return this->isPlaying;
	}

	inline void Play(){
		aumiks::Lib::Inst().PlayChannel(Ref<aumiks::Channel>(this));
	}

	inline void Stop(){
		this->stopFlag = true;
	}

	inline void SetVolume(ting::u8 vol){
		this->volume = vol;
	}

protected:
	/**
	 * @brief Called when channel has been added to pool of playing channels.
	 */
	virtual void OnStart(){}
private:
	//this function is called by SoundThread when it needs more data to play.
	//retun true to remove channel from playing channels list
	virtual bool MixToMixBuf(Array<s32>& mixBuf) = 0;
};



//base class for all sounds
class Sound : public ting::RefCounted{
protected:
	Sound(){}
public:


};

} //~namespace
