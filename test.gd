extends Control

export var sound_font : String = "res://TimGM6mb.sf2"
var key_shift = 0

onready var stream_player : AudioStreamPlayer = $AudioStreamPlayer
onready var stream_playback : AudioStreamPlayback = stream_player.get_stream_playback()

func _ready():
	$Container/Select/Preset.clear()
	for n in $TSF.get_preset_names():
		$Container/Select/Preset.add_item(n)
	on_preset_selected(0)

func _process(delta):
	stream_playback.push_buffer($TSF.get_buffer(stream_playback.get_frames_available()))

func _on_Keyboard_note(note, volume):
	$TSF.note_on($Container/Select/Preset.selected, note, volume)

func on_preset_selected(ID):
	pass

func _on_Button_toggled(button_pressed):
	if button_pressed:
		$TSF.play_midi("pathetique.mid")
	else:
		$TSF.play_midi("")

func _on_HSlider_value_changed(value):
	$TSF.midi_speed = value
