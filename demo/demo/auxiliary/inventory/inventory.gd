extends Control

@export var selected_slot : int = 0
@export var slots : Array[Control]
@export var world_editor : Node

func select_slot(new_slot: int):
	selected_slot = new_slot + 1;
	for i in range(len(slots)):
		if i == new_slot:
			slots[i].modulate = Color(1.0,1.0,1.0,1.0);
		else:
			slots[i].modulate = Color(0.5, 0.5, 0.5,0.5);

	world_editor.set_selected_material(selected_slot);

func _ready() -> void:
	select_slot(0)

func _input(event: InputEvent) -> void:
	if event is InputEventKey and event.pressed:
		match event.keycode:
			KEY_1:
				select_slot(0)
			KEY_2:
				select_slot(1)
			KEY_3:
				select_slot(2)
			KEY_4:
				select_slot(3)
