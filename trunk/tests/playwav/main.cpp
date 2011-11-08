#include "../../src/aumiks/WavSound.hpp"

#include <ting/FSFile.hpp>


void Play(const std::string& fileName){
	ting::Ref<aumiks::WavSound> snd;
	{
		ting::FSFile fi(fileName);
		snd = aumiks::WavSound::LoadWAV(fi);
	}

	ASSERT(snd)
	
	ting::Ref<aumiks::Channel> ch = snd->Play();
	
	while(ch->IsPlaying()){
//		TRACE(<< "Loop" << std::endl)
		ting::Thread::Sleep(50);
	}
}


int main(int argc, char *argv[]){
	aumiks::Lib aumiksLibrary(100, aumiks::STEREO_16_44100);
	
	Play("sample44100mono16.wav");
	Play("sample44100stereo16.wav");
	
	return 0;
}
