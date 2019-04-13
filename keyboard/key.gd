extends Panel
class_name Key

export(Resource) var key_theme
export(Resource) var key_pressed_theme
var pressed : bool = false
var pressed_midi : bool = false

signal note(volume)

func _ready():
	theme = key_theme

func set_pressed_midi(p):
	if p != pressed:
		pressed = p

func set_pressed(p):
	if p != pressed:
		pressed = p
		theme = key_pressed_theme if pressed else key_theme
		emit_signal("note", 1.0 if pressed else 0.0)

func _on_Key_gui_input(event : InputEvent):
	if event is InputEventMouseButton and event.button_index == BUTTON_LEFT:
		set_pressed(event.pressed)
	elif event is InputEventMouseMotion and (pressed or event.button_mask & BUTTON_MASK_LEFT != 0):
		if pressed and !get_rect().has_point(event.position):
			set_pressed(false)
