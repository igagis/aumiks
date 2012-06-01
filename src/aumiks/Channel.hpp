/* The MIT License:

Copyright (c) 2012 Ivan Gagis

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

// Home page: http://aumiks.googlecode.com

/**
 * @author Ivan Gagis <igagis@gmail.com>
 */



#pragma once

#include <ting/Ref.hpp>

#include "Effect.hpp"



namespace aumiks{



class Lib;
class MixChannel;



/**
 * @brief Base class of a channel for playing the sound.
 * Usually, the effects are created by Sound class implementations using CreateChannel() method.
 */
class Channel : public SampleBufferFiller, public virtual ting::RefCounted{
	friend class aumiks::Lib;
	friend class aumiks::MixChannel;
	
	ting::Inited<volatile bool, false> stopNowFlag;
	
	aumiks::SampleBufferFiller* lastFillerInChain;
	
private:
	Effect::T_EffectsList effects;

	inline bool FillSmpBufAndApplyEffects(ting::Buffer<ting::s32>& buf, unsigned freq, unsigned chans){
		ASSERT(buf.Size() % chans == 0)

		if(this->stopNowFlag){
			return true;
		}
		
		ASSERT(this->lastFillerInChain)

		return this->lastFillerInChain->FillSmpBuf(buf, freq, chans);
	}
	
protected:
	//TODO: doxygen
	ting::Inited<volatile bool, false> stopFlag;
	
	Channel() :
			lastFillerInChain(this)
	{}

public:

	virtual ~Channel()throw(){}

	/**
	 * @brief Start playing of this channel.
	 */
	void Play_ts();

	/**
	 * @brief Requests channel to stop playing.
	 * This is a request to stop playing the channel, the channel may stop not
	 * immediately, depending on implementation of the particular Channel.
	 * Once stopped, the channel cannot be started again.
	 * Instead, one needs to create a new channel.
	 */
	inline void Stop_ts()throw(){
		this->stopFlag = true;
	}
	
	/**
	 * @brief Stop playing the channel.
	 * Stops playing the channel immediately.
	 * Once stopped, the channel cannot be started again.
	 * Instead, one needs to create a new channel.
     */
	inline void StopNow_ts()throw(){
		this->stopNowFlag = true;
	}
	

	/**
	 * @brief Add effect to the channel.
	 * It is allowed to add effects during channel playing.
	 * The single effect instance can only be added to one channel. Adding single
	 * Effect instance to more than one channel will result in undefined behavior.
	 * @param effect - the effect to add.
	 */
	void AddEffect_ts(const ting::Ref<aumiks::Effect>& effect);

	/**
	 * @brief Remove effect from the channel.
	 * It is allowed to remove effects during channel playing.
	 * @param effect - effect to remove.
	 */
	void RemoveEffect_ts(const ting::Ref<aumiks::Effect>& effect);

	/**
	 * @brief Remove all effects from channel.
	 */
	void RemoveAllEffects_ts();
};

}//~namespace
