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

/**
 * @author Ivan Gagis <igagis@gmail.com>
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
#define M_AUMIKS_TRACE(x) TRACE(<< "[aumiks] ") TRACE(x)
#else
#define M_AUMIKS_TRACE(x)
#endif



namespace aumiks{


//forward declarations
class Lib;
class Channel;



//base channel class
class Channel : public ting::RefCounted{
	friend class aumiks::Lib;

	ting::Inited<volatile bool, false> isPlaying;

protected:
	ting::Inited<volatile bool, false> stopFlag;

	ting::Inited<volatile ting::u8, ting::u8(-1)> volume;
	
	Channel(){}
	
public:

	inline bool IsPlaying()const{
		return this->isPlaying;
	}

	inline void Play();

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
	
	/**
	 * @brief Called when channel has been removed from pool of playing channels.
	 */
	virtual void OnStop(){}
private:
	//this function is called by SoundThread when it needs more data to play.
	//return true to remove channel from playing channels list
	virtual bool FillSmpBuf11025Mono16(ting::Buffer<ting::s32>& mixBuf){return true;}
	virtual bool FillSmpBuf11025Stereo16(ting::Buffer<ting::s32>& mixBuf){return true;}
	virtual bool FillSmpBuf22050Mono16(ting::Buffer<ting::s32>& mixBuf){return true;}
	virtual bool FillSmpBuf22050Stereo16(ting::Buffer<ting::s32>& mixBuf){return true;}
	virtual bool FillSmpBuf44100Mono16(ting::Buffer<ting::s32>& mixBuf){return true;}
	virtual bool FillSmpBuf44100Stereo16(ting::Buffer<ting::s32>& mixBuf){return true;}
};



enum E_Format{
	MONO_16_11025,
	MONO_16_22050,
	MONO_16_44100,
	STEREO_16_11025,
	STEREO_16_22050,
	STEREO_16_44100
};



class Lib : public ting::Singleton<Lib>{
	friend class aumiks::Channel;
	
public:
	/**
	 * @brief Create sound library singleton instance.
	 * Creates singleton instance of sound library object and
	 * opens sound device.
	 * @param bufferSizeMillis - size of desired playing buffer in milliseconds.
	 * @param format - format of sound output.
	 */
	Lib(ting::u16 bufferSizeMillis = 100, aumiks::E_Format format = STEREO_16_22050);
	
	~Lib();

	void PlayChannel(ting::Ref<Channel> ch);

	inline void SetMuted(bool muted){
		this->thread.mixerBuffer->isMuted = muted;
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
		return this->thread.mixerBuffer->isMuted;
	}

public:
	//Base class for mixer buffers of different formats
	class MixerBuffer{
		friend class aumiks::Lib;
	protected:
		MixerBuffer(unsigned mixBufSize) :
				mixBuf(mixBufSize),
				smpBuf(mixBufSize),
				playBuf(mixBufSize * 2) //2 bytes per sample
		{}

		inline bool FillSmpBuf11025Mono16(const ting::Ref<aumiks::Channel>& ch){
			return ch->FillSmpBuf11025Mono16(this->smpBuf);
		}
		inline bool FillSmpBuf11025Stereo16(const ting::Ref<aumiks::Channel>& ch){
			return ch->FillSmpBuf11025Stereo16(this->smpBuf);
		}
		inline bool FillSmpBuf22050Mono16(const ting::Ref<aumiks::Channel>& ch){
			return ch->FillSmpBuf22050Mono16(this->smpBuf);
		}
		inline bool FillSmpBuf22050Stereo16(const ting::Ref<aumiks::Channel>& ch){
			return ch->FillSmpBuf22050Stereo16(this->smpBuf);
		}
		inline bool FillSmpBuf44100Mono16(const ting::Ref<aumiks::Channel>& ch){
			return ch->FillSmpBuf44100Mono16(this->smpBuf);
		}
		inline bool FillSmpBuf44100Stereo16(const ting::Ref<aumiks::Channel>& ch){
			return ch->FillSmpBuf44100Stereo16(this->smpBuf);
		}
		
		ting::Array<ting::s32> mixBuf;
		ting::Array<ting::s32> smpBuf;
		ting::Array<ting::u8> playBuf;

		ting::Inited<volatile bool, false> isMuted;
	public:
		virtual ~MixerBuffer(){}
		
	private:
		inline void CleanMixBuf(){
			memset(this->mixBuf.Begin(), 0, this->mixBuf.SizeInBytes());
		}
		
		//return true if channel has finished playing and should be removed from playing channels pool
		bool MixToMixBuf(const ting::Ref<aumiks::Channel>& ch);

		void CopyFromMixBufToPlayBuf();
		
		virtual bool FillSmpBuf(const ting::Ref<aumiks::Channel>& ch) = 0;
		
		void MixSmpBufToMixBuf();
	};

	//base class for audio backends
	class AudioBackend{
		friend class aumiks::Lib;
		
		virtual void Write(const ting::Buffer<ting::u8>& buf) = 0;
	protected:
		inline AudioBackend(){}
	public:
		virtual ~AudioBackend(){}
	};
private:
	class SoundThread : public ting::MsgThread{
	public:
		const ting::Ptr<AudioBackend> audioBackend;
		const ting::Ptr<MixerBuffer> mixerBuffer;
		
		ting::Mutex chPoolMutex;
		
		typedef std::list<ting::Ref<aumiks::Channel> > TChPool;
		typedef TChPool::iterator TChIter;
		TChPool chPool;

		TChPool chPoolToAdd;

		SoundThread(ting::u16 bufferSizeMillis, E_Format format);

		//override
		void Run();
		
	private:
		
		//TODO: create mixer buffer based on actual buffer size and format from the backend
		static ting::Ptr<MixerBuffer> CreateMixerBuffer(unsigned bufferSizeMillis, E_Format format);
	};

	SoundThread thread;
};



//base class for all sounds
class Sound : public ting::RefCounted{
protected:
	Sound(){}
public:
	virtual ting::Ref<aumiks::Channel> CreateChannel()const = 0;
};



inline void Channel::Play(){
	aumiks::Lib::Inst().PlayChannel(ting::Ref<aumiks::Channel>(this));
}



} //~namespace