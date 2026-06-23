extends Label

@export var reference_node: Node3D

func _process(delta: float) -> void:
	if reference_node:
		text = "fps: " + str(round(1/ delta)) + "\nposition: " + str(reference_node.global_position)
	else:
		text = "fps: " + str(round(1/ delta))
