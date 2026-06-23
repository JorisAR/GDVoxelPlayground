extends Node3D

@export var world : VoxelWorld
@export var selected_material : int = 1
@export var radius : int = 8

var cooldown := 0.0

func set_selected_material(value: int) -> void:
	selected_material = value

func _process(delta: float) -> void:
	cooldown -= delta
	if cooldown > 0.0:
		return
	
	if Input.is_action_pressed("left_click"):
		cooldown = 0.25
		world.edit_world(global_position, -global_transform.basis.z, radius, 1000, selected_material);
	if Input.is_action_pressed("right_click"):
		cooldown = 0.25
		world.edit_world(global_position, -global_transform.basis.z, radius, 1000, 0);
