#include "sound_font.h"

#define TSF_IMPLEMENTATION
#include "tsf.h"
#define TML_IMPLEMENTATION
#include "tml.h"

using namespace godot;

void SoundFont::_register_methods() {
	register_property<SoundFont, String>("sound_font", &SoundFont::set_sound_font, &SoundFont::get_sound_font, "");

	register_method("get_preset_names", &SoundFont::get_preset_names);
	register_method("play_note", &SoundFont::play_note);
	register_method("play_midi", &SoundFont::play_midi);
	register_method("get_buffer", &SoundFont::get_buffer);

	//register_signal<SoundFont>((char *)"position_changed", "node", GODOT_VARIANT_TYPE_OBJECT, "new_pos", GODOT_VARIANT_TYPE_VECTOR2);
}

SoundFont::SoundFont()
	: mTsf(NULL)
	, mTsfFile(NULL) {
}

SoundFont::~SoundFont() {
	if (mTsf != NULL) {
		tsf_close(mTsf);
	}
	delete mTsfFile;
}

static int readTsfFile(void* inData, void* inPtr, unsigned int inSize) {
	File                *theFile = (File*)inData;
	const PoolByteArray &theData = theFile->get_buffer(inSize);
	godot_int            theReadSize = theData.size();
	memcpy(inPtr, theData.read().ptr(), theReadSize);
	
	return theReadSize;
}
static int skipTsfFile(void* inData, unsigned int inSize) {
	File *theFile = (File*)inData;
	int64_t theNewPosition = theFile->get_position()+inSize;
	theFile->seek(theNewPosition);
	return (theFile->get_position() == theNewPosition) ? 1 : 0;
}

void SoundFont::_init() {
	mTsf = NULL;
	mTsfFile = File::_new();
	mSoundFontName = "";
	mTsfStream.data = mTsfFile;
	mTsfStream.read = readTsfFile;
	mTsfStream.skip = skipTsfFile;
	mTml = NULL;
	mTmlCurrent = NULL;
	mTmlFile = File::_new();
	mTmlStream.data = mTmlFile;
	mTmlStream.read = readTsfFile;
}

void SoundFont::set_sound_font(String inSoundFontName) {
	mSoundFontName = inSoundFontName;
	if (mTsf != NULL) {
		tsf_close(mTsf);
		mTsf = NULL;
		mTsfFile->close();
	}
	if (mTsfFile->open(mSoundFontName, File::READ) == Error::OK) {
		mTsf = tsf_load(&mTsfStream);
	}
}

String SoundFont::get_sound_font() const {
	return mSoundFontName;
}

PoolStringArray SoundFont::get_preset_names() const {
	PoolStringArray theReturnValue;

	if (mTsf != NULL) {
		unsigned theIndex, theEnd;

		for (theIndex = 0, theEnd = tsf_get_presetcount(mTsf); theIndex < theEnd; ++theIndex) {
			theReturnValue.append(tsf_get_presetname(mTsf, theIndex));
		}
	}
	return theReturnValue;
}

void SoundFont::play_note(godot_int inPresetIndex, godot_int inKey, float inVel) {
	if (mTsf == NULL) {
		return;
	}
	if (inVel > 0.0) {
		tsf_note_on(mTsf, inPresetIndex, inKey, inVel);
	} else {
		tsf_note_off(mTsf, inPresetIndex, inKey);
	}
}

void SoundFont::play_midi(String inMidiFileName) {
	if (mTml != NULL) {
		tml_free(mTml);
		mTml = NULL;
		mTmlCurrent = NULL;
		mTmlFile->close();
	}
	if (mTmlFile->open(inMidiFileName, File::READ) == Error::OK) {
		mTml = tml_load(&mTmlStream);
		mTmlCurrent = mTml;
	}
}


PoolVector2Array SoundFont::get_buffer(godot_int inSize) {
	PoolVector2Array theBuffer;
	
	if (inSize > 0 && mTsf != NULL) {
		theBuffer.resize(inSize);
		Vector2 *theData = theBuffer.write().ptr();
		if (mTmlCurrent != NULL) {
			int SampleBlock;
			int SampleCount = inSize;
			for (SampleBlock = TSF_RENDER_EFFECTSAMPLEBLOCK; SampleCount; SampleCount -= SampleBlock, theData += SampleBlock) {
				//We progress the MIDI playback and then process TSF_RENDER_EFFECTSAMPLEBLOCK samples at once
				if (SampleBlock > SampleCount) SampleBlock = SampleCount;

				//Loop through all MIDI messages which need to be played up until the current playback time
				for (mTmlTime += SampleBlock * (1000.0 / 44100.0); mTmlCurrent && mTmlTime >= mTmlCurrent->time; mTmlCurrent = mTmlCurrent->next) {
					switch (mTmlCurrent->type) {
						case TML_PROGRAM_CHANGE: //channel program (preset) change (special handling for 10th MIDI channel with drums)
							tsf_channel_set_presetnumber(mTsf, mTmlCurrent->channel, mTmlCurrent->program, (mTmlCurrent->channel == 9));
							break;
						case TML_NOTE_ON: //play a note
							tsf_channel_note_on(mTsf, mTmlCurrent->channel, mTmlCurrent->key, mTmlCurrent->velocity / 127.0f);
							break;
						case TML_NOTE_OFF: //stop a note
							tsf_channel_note_off(mTsf, mTmlCurrent->channel, mTmlCurrent->key);
							break;
						case TML_PITCH_BEND: //pitch wheel modification
							tsf_channel_set_pitchwheel(mTsf, mTmlCurrent->channel, mTmlCurrent->pitch_bend);
							break;
						case TML_CONTROL_CHANGE: //MIDI controller messages
							tsf_channel_midi_control(mTsf, mTmlCurrent->channel, mTmlCurrent->control, mTmlCurrent->control_value);
							break;
					}
				}
				// Render the block of audio samples in float format
				tsf_render_float(mTsf, (float*)theData, SampleBlock, 0);
			}
		} else {
			tsf_render_float(mTsf, (float*)theData, inSize, 0);
		}
	}
	return theBuffer;
}
