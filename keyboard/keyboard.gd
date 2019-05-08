tool
extends Control
class_name Keyboard

export(int) var keys = 8 setget set_keys
export(int) var offset = 0 setget set_offset

var notes

const WHITE_INTERVALS = [ 2, 2, 1, 2, 2, 2, 1 ]
const IS_WHITE = [ true, false, true, false, true, true, false, true, false, true, false, true]

signal note(note, volume)

# Called when the node enters the scene tree for the first time.
func _ready():
	OS.open_midi_inputs()
	print(OS.get_connected_midi_inputs())

func _unhandled_input(event):
	if event is InputEventMIDI:
		if event.message == 9:
			emit_signal("note", event.pitch, event.velocity/127.0)
			if notes[event.pitch] != null:
				notes[event.pitch].set_pressed_midi(true)
		elif event.message == 8:
			emit_signal("note", event.pitch, 0)
			if notes[event.pitch] != null:
				notes[event.pitch].set_pressed_midi(false)

func set_keys(k):
	keys = k
	update_keyboard()
	
func set_offset(o):
	offset = int(o)
	update_keyboard()

func _get_minimum_size():
	var last_note : Control = get_child(0)
	return last_note.rect_position+last_note.rect_size

func update_keyboard():
	for c in get_children():
		c.queue_free()
	notes = []
	for i in range(128):
		notes.append(null)
	var next_white = 0
	var key : Key
	var current_note = offset
	if !IS_WHITE[offset % 12]:
		current_note -= 1
	var whites = 0
	while whites < keys:
		if IS_WHITE[current_note % 12]:
			key = preload("res://keyboard/key_white.tscn").instance()
			key.rect_position = Vector2(next_white, 0)
			whites += 1
			next_white += key.rect_size.x
			add_child(key)
			move_child(key, 0)
		else:
			key = preload("res://keyboard/key_black.tscn").instance()
			key.rect_position = Vector2(next_white-key.rect_size.x/2, 0)
			add_child(key)
		key.connect("note", self, "on_note", [ current_note ])
		notes[current_note] = key
		current_note += 1
	rect_min_size = _get_minimum_size()
	rect_size = rect_min_size

func update_keyboard_old():
	for c in get_children():
		c.queue_free()
	var x = 0
	var interval = 0
	var key : Key
	var starting_note = 0
	var current_note
	for i in range(offset):
		starting_note += WHITE_INTERVALS[i % WHITE_INTERVALS.size()]
	current_note = starting_note
	for i in range(keys):
		key = preload("res://keyboard/key_white.tscn").instance()
		key.rect_position = Vector2(x, 0)
		add_child(key)
		key.connect("note", self, "on_note", [ current_note ])
		x += key.rect_size.x
		current_note += WHITE_INTERVALS[(offset + i) % WHITE_INTERVALS.size()]
	rect_min_size = key.rect_position+key.rect_size
	rect_size = rect_min_size
	x = 30
	interval = 0
	current_note = starting_note+1
	for i in range(keys-1):
		var next_interval = WHITE_INTERVALS[(offset + i) % WHITE_INTERVALS.size()]
		if next_interval == 2:
			key = preload("res://keyboard/key_black.tscn").instance()
			key.rect_position = Vector2(x, 0)
			add_child(key)
			key.connect("note", self, "on_note", [ current_note ])
		x += get_child(0).rect_size.x
		current_note += WHITE_INTERVALS[(offset + i) % WHITE_INTERVALS.size()]

func on_note(volume, note):
	emit_signal("note", note, volume)


func set_scrolling(value):
	pass # Replace with function body.
