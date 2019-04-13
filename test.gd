extends Control

export var sound_font : String = "res://TimGM6mb.sf2"
var key_shift = 0

onready var sound_font_node = $SoundFont

func _ready():
	sound_font_node.load(sound_font)
	$Select/Preset.clear()
	for n in sound_font_node.get_preset_names():
		$Select/Preset.add_item(n)
	on_preset_selected(0)
	on_instrument_selected(0)

func _on_Keyboard_note(note, volume):
	$Instrument.play(note, volume)

func _on_InstrumentSelect_item_selected(ID):
	sound_font_node.get_instrument(ID)

func on_preset_selected(ID):
	$Select/Instrument.clear()
	for n in sound_font_node.get_instrument_names(ID):
		$Select/Instrument.add_item(n)
	on_instrument_selected(0)

func on_instrument_selected(ID):
	$Instrument.set_instrument(sound_font_node.get_instrument($Select/Preset.selected, ID))
